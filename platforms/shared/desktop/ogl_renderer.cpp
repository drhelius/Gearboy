/*
 * Gearboy - Nintendo Game Boy Emulator
 * Copyright (C) 2012  Ignacio Sanchez

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/
 *
 */

#if defined(__APPLE__)
    #define GL_SILENCE_DEPRECATION
    #include <OpenGL/gl3.h>
#else
    #define GLAD_GL_IMPLEMENTATION
    #include <glad.h>
#endif

#include <string>
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "emu.h"
#include "config.h"
#include "gearboy.h"

#define OGL_RENDERER_IMPORT
#include "ogl_renderer.h"
#include "ogl_shader_chain.h"
#include "ogl_shader_program.h"

static uint32_t system_texture;
static uint32_t frame_buffer_object;
static ShaderPresetSourcePalette applied_shader_source_palette = ShaderPresetSourcePalette_Default;
static GB_RuntimeInfo current_runtime;
static OglRendererScreenGeometry screen_geometry;
static int savestates_texture_slot = -1;
static u32 savestates_texture_generation = 0;

static uint32_t quad_shader_program = 0;
static uint32_t quad_vao = 0;
static uint32_t quad_vbo = 0;
static int quad_uniform_texture = -1;
static int quad_uniform_color = -1;
static int quad_uniform_tex_scale = -1;

static void init_ogl_gui(void);
static bool init_ogl_emu(void);
static void init_ogl_debug(void);
static void init_ogl_savestates(void);
static bool init_shaders(void);
static void render_gui(void);
static bool should_use_internal_shader_chain(void);
static void render_internal_shader_chain(void);
static void render_external_shader_chain(void);
static void render_internal_shader_chain_feedback(void);
static void render_emu_normal(void);
static void bind_texture_unit(int unit, uint32_t texture, uint32_t fallback_texture);
static void render_quad(uint32_t program, uint32_t texture, int viewport_width, int viewport_height, float tex_h, float tex_v, float red, float green, float blue, float alpha);
static void render_quad_preset(int pass_index, uint32_t program, uint32_t texture, int input_width, int input_height, int viewport_width, int viewport_height);
static void apply_shader_source_palette(void);
static void apply_configured_dmg_palette(void);
static void update_system_texture(void);
static void update_debug_textures(void);
static void update_savestates_texture(void);
static void update_current_runtime(void);
static void load_configured_shader_preset(void);
static void apply_shader_parameter_config(void);
static bool get_active_shader_preset_file(char* preset_file, size_t preset_file_size);
static float get_original_aspect(void);
static void configure_texture_2d(bool filter_linear);
static void create_texture_2d(uint32_t* texture, int width, int height, int internal_format, uint32_t format, uint32_t type, const void* pixels, bool filter_linear);
static void resize_texture_2d(uint32_t texture, int width, int height, int internal_format, uint32_t format, uint32_t type, const void* pixels, bool filter_linear);
static bool check_framebuffer_complete(const char* name);

bool ogl_renderer_init(void)
{
#if !defined(__APPLE__)
    int version = gladLoadGL((GLADloadfunc) SDL_GL_GetProcAddress);

    if (version == 0)
    {
        Error("GLAD: Failed to initialize OpenGL context");
        return false;
    }

    Log("GLAD: OpenGL %d.%d", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));
#endif

    ogl_renderer_opengl_version = (const char*)glGetString(GL_VERSION);
    Log("Starting OpenGL %s", ogl_renderer_opengl_version);

    glDisable(GL_FRAMEBUFFER_SRGB);

    if (!init_shaders())
        return false;

    init_ogl_gui();

    if (!init_ogl_emu())
        return false;

    if (!ogl_shader_chain_init("internal shader-chain framebuffer"))
        return false;

    load_configured_shader_preset();

    init_ogl_debug();
    init_ogl_savestates();

    return true;
}

void ogl_renderer_destroy(void)
{
    glDeleteFramebuffers(1, &frame_buffer_object); 
    glDeleteTextures(1, &ogl_renderer_emu_texture);
    glDeleteTextures(1, &system_texture);
    ogl_shader_chain_destroy();

    glDeleteTextures(1, &ogl_renderer_emu_debug_vram_background);
    glDeleteTextures(40, ogl_renderer_emu_debug_vram_sprites);
    glDeleteTextures(2, ogl_renderer_emu_debug_vram_tiles);
    glDeleteTextures(1, &ogl_renderer_emu_savestates);

    if (quad_shader_program)
        glDeleteProgram(quad_shader_program);
    if (quad_vao)
        glDeleteVertexArrays(1, &quad_vao);
    if (quad_vbo)
        glDeleteBuffers(1, &quad_vbo);

    quad_shader_program = 0;
    quad_vao = 0;
    quad_vbo = 0;

    ImGui_ImplOpenGL3_Shutdown();
}

void ogl_renderer_begin_render(void)
{
    ImGui_ImplOpenGL3_NewFrame();
}

void ogl_renderer_render(void)
{
    update_current_runtime();
    apply_shader_source_palette();

    if (config_debug.debug)
    {
        update_debug_textures();
    }

    update_savestates_texture();

    bool use_internal_shader_chain = should_use_internal_shader_chain();

    if (use_internal_shader_chain)
        render_internal_shader_chain();
    else
        render_emu_normal();

    ImVec4 clear_color = ImVec4(config_video.background_color[0], config_video.background_color[1], config_video.background_color[2], 1.00f);

    ImGuiIO& io = ImGui::GetIO();
    int fb_width = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
    int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);

    glDisable(GL_FRAMEBUFFER_SRGB);

    glViewport(0, 0, fb_width, fb_height);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);

    render_gui();
}

void ogl_renderer_end_render(void)
{
#if defined(__APPLE__) || defined(_WIN32)
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
        SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
    }
#endif
}

void ogl_renderer_set_screen_geometry(const OglRendererScreenGeometry* geometry)
{
    if (!geometry)
        return;

    screen_geometry = *geometry;
}

uint32_t ogl_renderer_get_screen_texture(void)
{
    if (should_use_internal_shader_chain() && ogl_shader_chain_get_pass_texture() != 0)
        return ogl_shader_chain_get_pass_texture();

    return ogl_renderer_emu_texture;
}

void ogl_renderer_get_screen_uv(float* u, float* v)
{
    if (should_use_internal_shader_chain())
    {
        if (u)
            *u = 1.0f;
        if (v)
            *v = 1.0f;
        return;
    }

    GB_RuntimeInfo runtime;
    GearboyCore* core = emu_get_core();

    if (IsValidPointer(core))
        core->GetRuntimeInfo(runtime);
    else
    {
        runtime.screen_width = GAMEBOY_WIDTH;
        runtime.screen_height = GAMEBOY_HEIGHT;
    }

    if (u)
        *u = (float)runtime.screen_width / (float)SYSTEM_TEXTURE_WIDTH;
    if (v)
        *v = (float)runtime.screen_height / (float)SYSTEM_TEXTURE_HEIGHT;
}

bool ogl_renderer_load_shader_preset(const char* path)
{
    char resolved_path[SHADER_PRESET_MAX_PATH];

    if (!shader_preset_resolve_config_path(path, resolved_path, sizeof(resolved_path)))
    {
        Error("Shader preset is not in the bundled shader directory: %s", path ? path : "");
        return false;
    }

    if (!ogl_shader_chain_load_preset(resolved_path))
        return false;

    config_video.shader_mode = config_ShaderMode_External;
    apply_shader_parameter_config();

    char config_path[SHADER_PRESET_MAX_PATH];
    if (shader_preset_get_config_path(resolved_path, config_path, sizeof(config_path)))
        config_video.shader_preset_path.assign(config_path);

    ogl_renderer_save_shader_parameter_config();
    return true;
}

void ogl_renderer_unload_shader_preset(void)
{
    ogl_shader_chain_unload_preset();
    config_video.shader_mode = config_ShaderMode_PixelPerfect;
    config_video.shader_preset_path.clear();
}

void ogl_renderer_save_shader_parameter_config(void)
{
    char preset_file[SHADER_PRESET_MAX_PATH];

    if (!get_active_shader_preset_file(preset_file, sizeof(preset_file)))
        return;

    int count = ogl_shader_chain_get_parameter_count();

    for (int i = 0; i < count; i++)
    {
        const ShaderPresetParameter* parameter = ogl_shader_chain_get_parameter(i);
        if (!parameter)
            continue;

        config_write_shader_parameter(preset_file, parameter->name, parameter->value);
    }
}

static void init_ogl_gui(void)
{
#if defined(__APPLE__)
    ImGui_ImplOpenGL3_Init("#version 150");
#else
    ImGui_ImplOpenGL3_Init("#version 130");
#endif
}

static bool init_ogl_emu(void)
{
    glGenFramebuffers(1, &frame_buffer_object);
    create_texture_2d(&ogl_renderer_emu_texture, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE, NULL, false);
    create_texture_2d(&system_texture, SYSTEM_TEXTURE_WIDTH, SYSTEM_TEXTURE_HEIGHT, GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*) emu_frame_buffer, false);

    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_object);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ogl_renderer_emu_texture, 0);

    bool complete = check_framebuffer_complete("emulator framebuffer");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return complete;
}

static void init_ogl_debug(void)
{
    create_texture_2d(&ogl_renderer_emu_debug_vram_background, 256, 256, GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)emu_debug_background_buffer, false);

    for (int s = 0; s < 40; s++)
    {
        create_texture_2d(&ogl_renderer_emu_debug_vram_sprites[s], 8, 16, GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)emu_debug_oam_buffers[s], false);
    }

    for (int b = 0; b < 2; b++)
        create_texture_2d(&ogl_renderer_emu_debug_vram_tiles[b], 128, 256, GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE, NULL, false);
}

static void init_ogl_savestates(void)
{
    create_texture_2d(&ogl_renderer_emu_savestates, SYSTEM_TEXTURE_WIDTH, SYSTEM_TEXTURE_HEIGHT, GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE, NULL, false);
    savestates_texture_slot = -1;
    savestates_texture_generation = 0;
}

static void render_gui(void)
{
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

static bool should_use_internal_shader_chain(void)
{
    return ogl_shader_chain_is_initialized() &&
            config_video.shader_mode == config_ShaderMode_External &&
            ogl_shader_chain_has_preset() &&
            !config_debug.debug &&
            screen_geometry.physical_width > 0 &&
            screen_geometry.physical_height > 0;
}

static void render_internal_shader_chain(void)
{
    bool filter_linear = ogl_shader_chain_get_preset_filter_linear();

    OglShaderChainSourceTexture source_texture;
    source_texture.width = current_runtime.screen_width;
    source_texture.height = current_runtime.screen_height;
    source_texture.pixels = emu_frame_buffer;
    source_texture.filter_linear = filter_linear;

    if (!ogl_shader_chain_update_source_texture(&source_texture))
    {
        render_emu_normal();
        return;
    }

    render_external_shader_chain();
}

static void render_external_shader_chain(void)
{
    glDisable(GL_BLEND);

    int pass_count = ogl_shader_chain_get_preset_pass_count();
    if (pass_count <= 0)
        return;

    int original_width = current_runtime.screen_width;
    int original_height = current_runtime.screen_height;
    int input_width = original_width;
    int input_height = original_height;
    uint32_t input_texture = ogl_shader_chain_get_source_texture();
    bool input_float_framebuffer = false;

    for (int i = 0; i < pass_count; i++)
    {
        int output_width = screen_geometry.physical_width;
        int output_height = screen_geometry.physical_height;
        bool final_pass = i == pass_count - 1;
        OglShaderChainPassSize pass_size;
        pass_size.source_width = original_width;
        pass_size.source_height = original_height;
        pass_size.previous_width = input_width;
        pass_size.previous_height = input_height;
        pass_size.viewport_width = screen_geometry.physical_width;
        pass_size.viewport_height = screen_geometry.physical_height;

        ogl_shader_chain_get_preset_pass_output_size(i, &pass_size, &output_width, &output_height);

        uint32_t output_fbo = 0;
        uint32_t output_texture = 0;
        OglShaderChainFramebufferTexture framebuffer_texture;
        framebuffer_texture.width = output_width;
        framebuffer_texture.height = output_height;
        framebuffer_texture.float_framebuffer = ogl_shader_chain_get_preset_pass_float_framebuffer(i);

        if (final_pass)
        {
            framebuffer_texture.filter_linear = false;
            if (!ogl_shader_chain_resize_pass_texture(&framebuffer_texture))
                return;
            output_fbo = ogl_shader_chain_get_pass_framebuffer();
            output_texture = ogl_shader_chain_get_pass_texture();
        }
        else
        {
            framebuffer_texture.filter_linear = ogl_shader_chain_get_preset_pass_filter_linear(i + 1);
            if (!ogl_shader_chain_resize_intermediate_texture(i, &framebuffer_texture))
                return;
            output_fbo = ogl_shader_chain_get_intermediate_framebuffer(i);
            output_texture = ogl_shader_chain_get_intermediate_texture(i);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, output_fbo);
        render_quad_preset(i, ogl_shader_chain_get_preset_pass_program(i), input_texture, input_width, input_height, output_width, output_height);

        if (ogl_shader_chain_get_preset_pass_uses_history(i))
        {
            ogl_shader_chain_store_pass_history(i, input_texture, input_width, input_height,
                ogl_shader_chain_get_preset_pass_filter_linear(i), input_float_framebuffer);
        }

        input_texture = output_texture;
        input_width = output_width;
        input_height = output_height;
        input_float_framebuffer = framebuffer_texture.float_framebuffer;
    }


    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (ogl_shader_chain_preset_uses_feedback())
        render_internal_shader_chain_feedback();
}

static void render_internal_shader_chain_feedback(void)
{
    glBindFramebuffer(GL_FRAMEBUFFER, ogl_shader_chain_get_feedback_framebuffer());
    glDisable(GL_BLEND);

    render_quad(quad_shader_program, ogl_shader_chain_get_pass_texture(),
            ogl_shader_chain_get_pass_width(), ogl_shader_chain_get_pass_height(),
            1.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void render_emu_normal(void)
{
    float tex_h = (float)current_runtime.screen_width / (float)SYSTEM_TEXTURE_WIDTH;
    float tex_v = (float)current_runtime.screen_height / (float)SYSTEM_TEXTURE_HEIGHT;

    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_object);

    glDisable(GL_BLEND);

    update_system_texture();

    int viewport_width = current_runtime.screen_width * FRAME_BUFFER_SCALE;
    int viewport_height = current_runtime.screen_height * FRAME_BUFFER_SCALE;

    render_quad(quad_shader_program, system_texture, viewport_width, viewport_height, tex_h, tex_v, 1.0f, 1.0f, 1.0f, 1.0f);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void update_system_texture(void)
{
    glBindTexture(GL_TEXTURE_2D, system_texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, current_runtime.screen_width, current_runtime.screen_height,
            GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*) emu_frame_buffer);
}

static void update_debug_textures(void)
{
    if (config_debug.show_video_nametable)
    {
        glBindTexture(GL_TEXTURE_2D, ogl_renderer_emu_debug_vram_background);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 256,
            GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*) emu_debug_background_buffer);
    }

    if (config_debug.show_video_sprites)
    {
        for (int s = 0; s < 40; s++)
        {
            glBindTexture(GL_TEXTURE_2D, ogl_renderer_emu_debug_vram_sprites[s]);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 8, 16,
                    GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*) emu_debug_oam_buffers[s]);
        }
    }

    if (config_debug.show_video_tiles)
    {
        for (int b = 0; b < 2; b++)
        {
            glBindTexture(GL_TEXTURE_2D, ogl_renderer_emu_debug_vram_tiles[b]);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 16 * 8, 24 * 8,
                    GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*) emu_debug_tile_buffers[b]);
        }
    }
}

static void update_savestates_texture(void)
{
    int i = config_emulator.save_slot;
    if (i < 0 || i >= 5)
        return;

    if ((savestates_texture_slot == i) && (savestates_texture_generation == emu_savestates_generation))
        return;

    savestates_texture_slot = i;
    savestates_texture_generation = emu_savestates_generation;

    if (IsValidPointer(emu_savestates_screenshots[i].data))
    {
        int width = emu_savestates_screenshots[i].width;
        int height = emu_savestates_screenshots[i].height;
        glBindTexture(GL_TEXTURE_2D, ogl_renderer_emu_savestates);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*) emu_savestates_screenshots[i].data);
    }
}

static void render_quad(uint32_t program, uint32_t texture, int viewport_width, int viewport_height, float tex_h, float tex_v, float red, float green, float blue, float alpha)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glUseProgram(program);
    glUniform2f(quad_uniform_tex_scale, tex_h, tex_v);
    glUniform4f(quad_uniform_color, red, green, blue, alpha);

    glViewport(0, 0, viewport_width, viewport_height);

    glBindVertexArray(quad_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
    glUseProgram(0);

    glActiveTexture(GL_TEXTURE0);
}

static void render_quad_preset(int pass_index, uint32_t program, uint32_t texture, int input_width, int input_height, int viewport_width, int viewport_height)
{
    uint32_t source_texture = ogl_shader_chain_get_source_texture();
    uint32_t fallback_texture = source_texture ? source_texture : texture;

    bind_texture_unit(0, texture, fallback_texture);
    bind_texture_unit(1, source_texture, fallback_texture);
    bind_texture_unit(2, ogl_shader_chain_get_feedback_texture(), fallback_texture);

    for (int i = 0; i < SHADER_PRESET_MAX_HISTORY_TEXTURES; i++)
    {
        if (!ogl_shader_chain_get_preset_pass_uses_history_sampler(pass_index, i))
            continue;

        bind_texture_unit(3 + i, ogl_shader_chain_get_pass_history_texture(pass_index, i), fallback_texture);
    }

    for (int i = 0; i < SHADER_PRESET_MAX_PASS_OUTPUT_TEXTURES; i++)
    {
        if (!ogl_shader_chain_get_preset_pass_uses_pass_output_sampler(pass_index, i))
            continue;

        bind_texture_unit(3 + SHADER_PRESET_MAX_HISTORY_TEXTURES + i, (i < pass_index) ? ogl_shader_chain_get_intermediate_texture(i) : 0, fallback_texture);
    }

    glUseProgram(program);
    OglShaderChainUniforms uniforms;
    uniforms.source_width = current_runtime.screen_width;
    uniforms.source_height = current_runtime.screen_height;
    uniforms.input_width = input_width;
    uniforms.input_height = input_height;
    uniforms.output_width = viewport_width;
    uniforms.output_height = viewport_height;
    uniforms.viewport_width = screen_geometry.physical_width;
    uniforms.viewport_height = screen_geometry.physical_height;
    uniforms.original_aspect = get_original_aspect();
    uniforms.background_color[0] = config_video.background_color[0];
    uniforms.background_color[1] = config_video.background_color[1];
    uniforms.background_color[2] = config_video.background_color[2];

    ogl_shader_chain_apply_preset_uniforms(pass_index, &uniforms);

    glViewport(0, 0, viewport_width, viewport_height);

    glBindVertexArray(quad_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
    glUseProgram(0);

    glActiveTexture(GL_TEXTURE0);
}

static void bind_texture_unit(int unit, uint32_t texture, uint32_t fallback_texture)
{
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, texture ? texture : fallback_texture);
}

static void apply_shader_source_palette(void)
{
    ShaderPresetSourcePalette source_palette = ShaderPresetSourcePalette_Default;

    if (should_use_internal_shader_chain() && ogl_shader_chain_has_preset())
        source_palette = ogl_shader_chain_get_preset_source_palette();

    if (source_palette == applied_shader_source_palette)
        return;

    if (source_palette == ShaderPresetSourcePalette_BlackWhite)
        emu_dmg_predefined_palette_override(2);
    else
    {
        emu_dmg_predefined_palette_override(-1);
        apply_configured_dmg_palette();
    }

    applied_shader_source_palette = source_palette;
    emu_render_current_frame();
}

static void apply_configured_dmg_palette(void)
{
    if (config_video.palette < 6)
    {
        emu_dmg_predefined_palette(config_video.palette);
    }
    else
    {
        int custom = config_video.palette - 6;
        if (custom < 0 || custom >= config_max_custom_palettes)
            return;

        GB_Color c0 = config_video.color[custom][0];
        GB_Color c1 = config_video.color[custom][1];
        GB_Color c2 = config_video.color[custom][2];
        GB_Color c3 = config_video.color[custom][3];
        emu_dmg_palette(c0, c1, c2, c3);
    }
}

static void update_current_runtime(void)
{
    current_runtime.screen_width = GAMEBOY_WIDTH;
    current_runtime.screen_height = GAMEBOY_HEIGHT;

    GearboyCore* core = emu_get_core();
    if (IsValidPointer(core))
        core->GetRuntimeInfo(current_runtime);
}

static void load_configured_shader_preset(void)
{
    if (config_video.shader_mode != config_ShaderMode_External || config_video.shader_preset_path.empty())
        return;

    char resolved_path[SHADER_PRESET_MAX_PATH];

    if (!shader_preset_resolve_config_path(config_video.shader_preset_path.c_str(), resolved_path, sizeof(resolved_path)))
    {
        Error("Configured shader preset is not available in the bundled shader directory: %s", config_video.shader_preset_path.c_str());
        ogl_renderer_unload_shader_preset();
        return;
    }

    if (!ogl_shader_chain_load_preset(resolved_path))
    {
        Error("Unable to load configured shader preset: %s", ogl_shader_chain_get_last_error());
        ogl_renderer_unload_shader_preset();
        return;
    }

    char config_path[SHADER_PRESET_MAX_PATH];

    if (shader_preset_get_config_path(resolved_path, config_path, sizeof(config_path)))
        config_video.shader_preset_path.assign(config_path);
    else
        config_video.shader_preset_path.clear();

    apply_shader_parameter_config();
}

static void apply_shader_parameter_config(void)
{
    char preset_file[SHADER_PRESET_MAX_PATH];

    if (!get_active_shader_preset_file(preset_file, sizeof(preset_file)))
        return;

    int parameter_count = ogl_shader_chain_get_parameter_count();

    for (int i = 0; i < parameter_count; i++)
    {
        const ShaderPresetParameter* parameter = ogl_shader_chain_get_parameter(i);
        if (!parameter)
            continue;

        float value = 0.0f;
        if (config_read_shader_parameter(preset_file, parameter->name, &value))
            ogl_shader_chain_set_parameter(i, value);
    }
}

static bool get_active_shader_preset_file(char* preset_file, size_t preset_file_size)
{
    if (!preset_file || preset_file_size == 0 || !ogl_shader_chain_has_preset())
        return false;

    return shader_preset_get_config_path(ogl_shader_chain_get_preset_path(), preset_file, preset_file_size);
}

static float get_original_aspect(void)
{
    if (current_runtime.screen_height <= 0)
        return 1.0f;

    return (float)current_runtime.screen_width / (float)current_runtime.screen_height;
}

static void configure_texture_2d(bool filter_linear)
{
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter_linear ? GL_LINEAR : GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter_linear ? GL_LINEAR : GL_NEAREST);
}

static void create_texture_2d(uint32_t* texture, int width, int height, int internal_format, uint32_t format, uint32_t type, const void* pixels, bool filter_linear)
{
    glGenTextures(1, texture);
    resize_texture_2d(*texture, width, height, internal_format, format, type, pixels, filter_linear);
}

static void resize_texture_2d(uint32_t texture, int width, int height, int internal_format, uint32_t format, uint32_t type, const void* pixels, bool filter_linear)
{
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, type, pixels);
    configure_texture_2d(filter_linear);
}

static bool check_framebuffer_complete(const char* name)
{
    uint32_t status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (status == GL_FRAMEBUFFER_COMPLETE)
        return true;

    Error("OpenGL framebuffer incomplete (%s): 0x%04X", name, status);
    return false;
}

static bool init_shaders(void)
{
    const char* version = ogl_shader_program_get_glsl_version();

    const char* vs_body =
        "in vec2 aPos;\n"
        "in vec2 aTexCoord;\n"
        "out vec2 vTexCoord;\n"
        "uniform vec2 uTexScale;\n"
        "void main() {\n"
        "    gl_Position = vec4(aPos, 0.0, 1.0);\n"
        "    vTexCoord = aTexCoord * uTexScale;\n"
        "}\n";

    const char* fs_body =
        "in vec2 vTexCoord;\n"
        "out vec4 FragColor;\n"
        "uniform sampler2D uTexture;\n"
        "uniform vec4 uColor;\n"
        "uniform vec2 uTexScale;\n"
        "void main() {\n"
        "    FragColor = texture(uTexture, vTexCoord) * uColor;\n"
        "}\n";

    const char* vs_sources[2] = { version, vs_body };
    const char* fs_sources[2] = { version, fs_body };

    uint32_t vs = ogl_shader_program_compile_shader(GL_VERTEX_SHADER, "Quad vertex", vs_sources, 2, NULL, 0);
    if (!vs)
        return false;

    uint32_t fs = ogl_shader_program_compile_shader(GL_FRAGMENT_SHADER, "Quad fragment", fs_sources, 2, NULL, 0);
    if (!fs)
    {
        glDeleteShader(vs);
        return false;
    }

    quad_shader_program = ogl_shader_program_link(vs, fs, "Quad shader", NULL, 0);

    glDeleteShader(vs);
    glDeleteShader(fs);

    if (!quad_shader_program)
        return false;

    quad_uniform_tex_scale = glGetUniformLocation(quad_shader_program, "uTexScale");
    quad_uniform_texture = glGetUniformLocation(quad_shader_program, "uTexture");
    quad_uniform_color = glGetUniformLocation(quad_shader_program, "uColor");

    glUseProgram(quad_shader_program);
    glUniform1i(quad_uniform_texture, 0);
    glUseProgram(0);

    float quad_vertices[] = {
        -1.0f, -1.0f,  0.0f,  0.0f,
         3.0f, -1.0f,  2.0f,  0.0f,
        -1.0f,  3.0f,  0.0f,  2.0f,
    };

    glGenVertexArrays(1, &quad_vao);
    glGenBuffers(1, &quad_vbo);

    glBindVertexArray(quad_vao);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);

    GLint pos_attrib = glGetAttribLocation(quad_shader_program, "aPos");
    GLint tex_attrib = glGetAttribLocation(quad_shader_program, "aTexCoord");

    glEnableVertexAttribArray(pos_attrib);
    glVertexAttribPointer(pos_attrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(tex_attrib);
    glVertexAttribPointer(tex_attrib, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    Debug("Quad shader initialized (program=%u, vao=%u, vbo=%u)", quad_shader_program, quad_vao, quad_vbo);
    return true;
}