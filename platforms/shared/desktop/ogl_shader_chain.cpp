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
    #include <glad.h>
#endif

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include "gearboy.h"
#include "ogl_shader_program.h"
#include "shader_preset.h"

#define OGL_SHADER_CHAIN_IMPORT
#include "ogl_shader_chain.h"

#define OGL_SHADER_CHAIN_MAX_TEXTURE_UNITS 16

static uint32_t source_texture = 0;
static uint32_t pass_texture = 0;
static uint32_t pass_fbo = 0;
static uint32_t feedback_texture = 0;
static uint32_t feedback_fbo = 0;
static uint32_t history_copy_fbo = 0;
static uint32_t black_texture = 0;
static uint32_t intermediate_textures[SHADER_PRESET_MAX_PASSES - 1];
static uint32_t intermediate_fbos[SHADER_PRESET_MAX_PASSES - 1];
static int intermediate_widths[SHADER_PRESET_MAX_PASSES - 1];
static int intermediate_heights[SHADER_PRESET_MAX_PASSES - 1];
static bool intermediate_filter_linears[SHADER_PRESET_MAX_PASSES - 1];
static bool intermediate_float_framebuffers[SHADER_PRESET_MAX_PASSES - 1];
static uint32_t pass_history_textures[SHADER_PRESET_MAX_PASSES][SHADER_PRESET_MAX_HISTORY_TEXTURES];
static int pass_history_widths[SHADER_PRESET_MAX_PASSES];
static int pass_history_heights[SHADER_PRESET_MAX_PASSES];
static bool pass_history_filter_linears[SHADER_PRESET_MAX_PASSES];
static int pass_history_counts[SHADER_PRESET_MAX_PASSES];
static int pass_history_write_indices[SHADER_PRESET_MAX_PASSES];
static bool pass_history_float_framebuffers[SHADER_PRESET_MAX_PASSES];
static uint32_t pass_feedback_textures[SHADER_PRESET_MAX_PASSES - 1];
static int pass_feedback_widths[SHADER_PRESET_MAX_PASSES - 1];
static int pass_feedback_heights[SHADER_PRESET_MAX_PASSES - 1];
static bool pass_feedback_filter_linears[SHADER_PRESET_MAX_PASSES - 1];
static bool pass_feedback_float_framebuffers[SHADER_PRESET_MAX_PASSES - 1];
static bool pass_feedback_used[SHADER_PRESET_MAX_PASSES - 1];
static int source_width = 1;
static int source_height = 1;
static bool source_filter_linear = false;
static int pass_width = 1;
static int pass_height = 1;
static bool pass_filter_linear = false;
static bool feedback_filter_linear = false;
static bool pass_float_framebuffer = false;
static bool initialized = false;
static const char* framebuffer_name = "shader-chain framebuffer";
static ShaderPreset active_preset;
enum PresetTextureKind
{
    PresetTexture_Source = 0,
    PresetTexture_Original,
    PresetTexture_Feedback,
    PresetTexture_SourceHistory,
    PresetTexture_PassOutput,
    PresetTexture_PassFeedback
};
struct PresetTextureBinding
{
    PresetTextureKind kind;
    int index;
};
struct PresetProgramState
{
    uint32_t program;
    int texture_count;
    PresetTextureBinding textures[OGL_SHADER_CHAIN_MAX_TEXTURE_UNITS];
    int uniform_source_size;
    int uniform_original_size;
    int uniform_output_size;
    int uniform_final_viewport_size;
    int uniform_feedback_size;
    int uniform_source_history_size;
    int uniform_pass_output_size[SHADER_PRESET_MAX_PASSES];
    int uniform_source_history_count;
    int uniform_frame_count;
    int uniform_frame_direction;
    int uniform_original_aspect;
    int uniform_background_color;
    int uniform_parameters[SHADER_PRESET_MAX_PARAMETERS];
};
static PresetProgramState preset_programs[SHADER_PRESET_MAX_PASSES];
static bool preset_loaded = false;
static char last_error[2048];
static int preset_frame_count = 0;

static void configure_texture_2d(bool filter_linear);
static void resize_texture_2d(uint32_t texture, int width, int height, int internal_format, uint32_t format, uint32_t type, const void* pixels, bool filter_linear);
static bool check_framebuffer_complete(const char* name);
static bool resize_framebuffer_texture(uint32_t texture, uint32_t fbo, int width, int height, bool filter_linear, bool float_framebuffer, const char* name);
static bool ensure_intermediate_texture(int index);
static bool ensure_history_copy_framebuffer(void);
static bool load_preset_programs(const ShaderPreset* preset, PresetProgramState* programs, char* error, size_t error_size);
static void bind_preset_uniform_locations(int index);
static bool add_texture_binding(PresetProgramState* state, const char* name, PresetTextureKind kind, int index);
static int get_pass_output_size_location(uint32_t program, int index);
static void clear_preset_state(void);
static void delete_preset_programs(void);
static void clear_feedback_texture(void);
static void release_intermediate_textures(int keep_count);
static void clear_pass_history_textures(int index);
static void clear_all_pass_history_textures(void);
static bool resize_pass_history_textures(int index, int width, int height, bool filter_linear, bool float_framebuffer);
static void release_pass_feedback_textures(void);
static bool resize_pass_feedback_texture(int index, int width, int height, bool filter_linear, bool float_framebuffer);
static bool copy_texture(uint32_t source, uint32_t target, int width, int height);
static void clear_texture(uint32_t texture, int width, int height);
static void set_last_error(const char* message);
static int scale_dimension(ShaderPresetScaleType scale_type, float scale, int absolute_size, int source_size, int previous_size, int viewport_size);
static float safe_inverse(int value);

bool ogl_shader_chain_init(const char* name)
{
    ogl_shader_chain_destroy();

    framebuffer_name = name ? name : "shader-chain framebuffer";
    source_width = 1;
    source_height = 1;
    source_filter_linear = false;
    pass_width = 1;
    pass_height = 1;
    pass_filter_linear = false;
    feedback_filter_linear = false;
    pass_float_framebuffer = false;

    glGenTextures(1, &source_texture);
    resize_texture_2d(source_texture, 1, 1, GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE, NULL, false);

    glGenTextures(1, &pass_texture);
    resize_texture_2d(pass_texture, 1, 1, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, NULL, false);

    glGenFramebuffers(1, &pass_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, pass_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pass_texture, 0);
    bool pass_complete = check_framebuffer_complete(framebuffer_name);

    glGenTextures(1, &feedback_texture);
    resize_texture_2d(feedback_texture, 1, 1, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, NULL, false);

    const uint8_t black_pixel[4] = { 0, 0, 0, 0 };
    glGenTextures(1, &black_texture);
    resize_texture_2d(black_texture, 1, 1, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, black_pixel, false);

    glGenFramebuffers(1, &feedback_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, feedback_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, feedback_texture, 0);
    bool feedback_complete = check_framebuffer_complete(framebuffer_name);

    for (int i = 0; i < SHADER_PRESET_MAX_PASSES - 1; i++)
    {
        intermediate_textures[i] = 0;
        intermediate_fbos[i] = 0;
        intermediate_widths[i] = 1;
        intermediate_heights[i] = 1;
        intermediate_filter_linears[i] = false;
        intermediate_float_framebuffers[i] = false;
    }

    for (int i = 0; i < SHADER_PRESET_MAX_PASSES; i++)
    {
        pass_history_widths[i] = 1;
        pass_history_heights[i] = 1;
        pass_history_filter_linears[i] = false;
        pass_history_counts[i] = 0;
        pass_history_write_indices[i] = 0;
        pass_history_float_framebuffers[i] = false;

        for (int j = 0; j < SHADER_PRESET_MAX_HISTORY_TEXTURES; j++)
            pass_history_textures[i][j] = 0;
    }

    for (int i = 0; i < SHADER_PRESET_MAX_PASSES - 1; i++)
    {
        pass_feedback_textures[i] = 0;
        pass_feedback_widths[i] = 1;
        pass_feedback_heights[i] = 1;
        pass_feedback_filter_linears[i] = false;
        pass_feedback_float_framebuffers[i] = false;
        pass_feedback_used[i] = false;
    }

    initialized = pass_complete && feedback_complete;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return initialized;
}

void ogl_shader_chain_destroy(void)
{
    ogl_shader_chain_unload_preset();

    if (pass_fbo)
        glDeleteFramebuffers(1, &pass_fbo);

    if (feedback_fbo)
        glDeleteFramebuffers(1, &feedback_fbo);

    if (history_copy_fbo)
        glDeleteFramebuffers(1, &history_copy_fbo);

    release_intermediate_textures(0);
    release_pass_feedback_textures();

    for (int i = 0; i < SHADER_PRESET_MAX_PASSES; i++)
    {
        for (int j = 0; j < SHADER_PRESET_MAX_HISTORY_TEXTURES; j++)
        {
            if (pass_history_textures[i][j])
                glDeleteTextures(1, &pass_history_textures[i][j]);
            pass_history_textures[i][j] = 0;
        }

        pass_history_widths[i] = 1;
        pass_history_heights[i] = 1;
        pass_history_filter_linears[i] = false;
        pass_history_counts[i] = 0;
        pass_history_write_indices[i] = 0;
        pass_history_float_framebuffers[i] = false;
    }

    if (feedback_texture)
        glDeleteTextures(1, &feedback_texture);

    if (black_texture)
        glDeleteTextures(1, &black_texture);

    if (pass_texture)
        glDeleteTextures(1, &pass_texture);

    if (source_texture)
        glDeleteTextures(1, &source_texture);

    pass_fbo = 0;
    feedback_fbo = 0;
    history_copy_fbo = 0;
    feedback_texture = 0;
    black_texture = 0;
    pass_texture = 0;
    source_texture = 0;
    source_width = 1;
    source_height = 1;
    source_filter_linear = false;
    pass_width = 1;
    pass_height = 1;
    pass_filter_linear = false;
    feedback_filter_linear = false;
    pass_float_framebuffer = false;
    initialized = false;
}

bool ogl_shader_chain_is_initialized(void)
{
    return initialized;
}

bool ogl_shader_chain_update_source_texture(const OglShaderChainSourceTexture* texture)
{
    if (!source_texture || !texture)
        return false;

    int width = texture->width;
    int height = texture->height;

    if (width < 1)
        width = 1;
    if (height < 1)
        height = 1;

    if (source_width != width || source_height != height)
    {
        resize_texture_2d(source_texture, width, height, GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE, NULL, texture->filter_linear);
        source_width = width;
        source_height = height;
        source_filter_linear = texture->filter_linear;
    }

    glBindTexture(GL_TEXTURE_2D, source_texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, texture->pixels);
    if (source_filter_linear != texture->filter_linear)
    {
        configure_texture_2d(texture->filter_linear);
        source_filter_linear = texture->filter_linear;
    }

    return true;
}

bool ogl_shader_chain_resize_pass_texture(const OglShaderChainFramebufferTexture* texture)
{
    if (!pass_texture || !pass_fbo || !feedback_texture || !feedback_fbo || !texture)
        return false;

    int width = texture->width;
    int height = texture->height;

    if (width < 1)
        width = 1;
    if (height < 1)
        height = 1;

    if (pass_width == width && pass_height == height && pass_float_framebuffer == texture->float_framebuffer)
    {
        if (pass_filter_linear != texture->filter_linear)
        {
            glBindTexture(GL_TEXTURE_2D, pass_texture);
            configure_texture_2d(texture->filter_linear);
            pass_filter_linear = texture->filter_linear;
        }

        if (feedback_filter_linear != texture->filter_linear)
        {
            glBindTexture(GL_TEXTURE_2D, feedback_texture);
            configure_texture_2d(texture->filter_linear);
            feedback_filter_linear = texture->filter_linear;
        }

        return true;
    }

    preset_frame_count = 0;

    bool pass_complete = resize_framebuffer_texture(pass_texture, pass_fbo, width, height, texture->filter_linear, texture->float_framebuffer, framebuffer_name);
    bool feedback_complete = resize_framebuffer_texture(feedback_texture, feedback_fbo, width, height, texture->filter_linear, texture->float_framebuffer, framebuffer_name);

    bool complete = pass_complete && feedback_complete;

    if (complete)
    {
        pass_width = width;
        pass_height = height;
        pass_filter_linear = texture->filter_linear;
        feedback_filter_linear = texture->filter_linear;
        pass_float_framebuffer = texture->float_framebuffer;
        clear_feedback_texture();
    }

    return complete;
}

bool ogl_shader_chain_resize_intermediate_texture(int index, const OglShaderChainFramebufferTexture* texture)
{
    if (index < 0 || index >= SHADER_PRESET_MAX_PASSES - 1 || !texture)
        return false;

    if (!ensure_intermediate_texture(index))
        return false;

    int width = texture->width;
    int height = texture->height;

    if (width < 1)
        width = 1;
    if (height < 1)
        height = 1;

    if (intermediate_widths[index] == width && intermediate_heights[index] == height && intermediate_float_framebuffers[index] == texture->float_framebuffer)
    {
        if (intermediate_filter_linears[index] != texture->filter_linear)
        {
            glBindTexture(GL_TEXTURE_2D, intermediate_textures[index]);
            configure_texture_2d(texture->filter_linear);
            intermediate_filter_linears[index] = texture->filter_linear;
        }

        return true;
    }

    if (!resize_framebuffer_texture(intermediate_textures[index], intermediate_fbos[index], width, height, texture->filter_linear, texture->float_framebuffer, framebuffer_name))
        return false;

    intermediate_widths[index] = width;
    intermediate_heights[index] = height;
    intermediate_filter_linears[index] = texture->filter_linear;
    intermediate_float_framebuffers[index] = texture->float_framebuffer;
    return true;
}

bool ogl_shader_chain_load_preset(const char* path)
{
    static ShaderPreset new_preset;
    static PresetProgramState new_programs[SHADER_PRESET_MAX_PASSES];
    char error[sizeof(last_error)];

    if (!shader_preset_load(path, &new_preset, error, sizeof(error)))
    {
        set_last_error(error);
        Error("Shader preset load failed: %s", last_error);
        return false;
    }

    memset(new_programs, 0, sizeof(new_programs));
    if (!load_preset_programs(&new_preset, new_programs, error, sizeof(error)))
    {
        set_last_error(error);
        Error("Shader preset compile failed: %s", last_error);
        return false;
    }

    delete_preset_programs();

    active_preset = new_preset;
    memcpy(preset_programs, new_programs, sizeof(preset_programs));
    preset_loaded = true;
    preset_frame_count = 0;
    last_error[0] = '\0';

    release_pass_feedback_textures();

    for (int i = 0; i < active_preset.pass_count; i++)
        bind_preset_uniform_locations(i);

    release_intermediate_textures(active_preset.pass_count > 1 ? active_preset.pass_count - 1 : 0);

    clear_feedback_texture();
    clear_all_pass_history_textures();

    Log("Loaded shader preset: %s", active_preset.preset_path);
    for (int i = 0; i < active_preset.pass_count; i++)
        Debug("Shader preset pass %d: %s", i, active_preset.passes[i].shader_path);

    return true;
}

void ogl_shader_chain_unload_preset(void)
{
    delete_preset_programs();
    clear_preset_state();
}

bool ogl_shader_chain_has_preset(void)
{
    return preset_loaded && active_preset.pass_count > 0 && preset_programs[0].program != 0;
}

uint32_t ogl_shader_chain_get_preset_pass_program(int index)
{
    if (!preset_loaded || index < 0 || index >= active_preset.pass_count)
        return 0;

    return preset_programs[index].program;
}

const char* ogl_shader_chain_get_preset_name(void)
{
    return preset_loaded ? active_preset.name : "";
}

const char* ogl_shader_chain_get_preset_path(void)
{
    return preset_loaded ? active_preset.preset_path : "";
}

const char* ogl_shader_chain_get_last_error(void)
{
    return last_error;
}

bool ogl_shader_chain_get_preset_filter_linear(void)
{
    return ogl_shader_chain_get_preset_pass_filter_linear(0);
}

ShaderPresetSourcePalette ogl_shader_chain_get_preset_source_palette(void)
{
    return preset_loaded ? active_preset.source_palette : ShaderPresetSourcePalette_Default;
}

int ogl_shader_chain_get_preset_pass_count(void)
{
    return preset_loaded ? active_preset.pass_count : 0;
}

bool ogl_shader_chain_get_preset_pass_filter_linear(int index)
{
    if (!preset_loaded || index < 0 || index >= active_preset.pass_count)
        return false;

    return active_preset.passes[index].filter_linear;
}

bool ogl_shader_chain_get_preset_pass_float_framebuffer(int index)
{
    if (!preset_loaded || index < 0 || index >= active_preset.pass_count)
        return false;

    return active_preset.passes[index].float_framebuffer;
}

bool ogl_shader_chain_get_preset_pass_uses_history(int index)
{
    if (!preset_loaded || index < 0 || index >= active_preset.pass_count)
        return false;

    return active_preset.passes[index].history;
}

bool ogl_shader_chain_preset_uses_feedback(void)
{
    if (!preset_loaded)
        return false;

    for (int i = 0; i < active_preset.pass_count; i++)
    {
        if (active_preset.passes[i].feedback)
            return true;
    }

    return false;
}

void ogl_shader_chain_get_preset_pass_output_size(int index, const OglShaderChainPassSize* pass_size, int* width, int* height)
{
    if (width)
        *width = pass_size ? pass_size->viewport_width : 1;
    if (height)
        *height = pass_size ? pass_size->viewport_height : 1;

    if (!pass_size || !preset_loaded || index < 0 || index >= active_preset.pass_count)
        return;

    const ShaderPresetPass* pass = &active_preset.passes[index];

    int output_width = scale_dimension(pass->scale_type_x, pass->scale_x, pass->absolute_width, pass_size->source_width, pass_size->previous_width, pass_size->viewport_width);

    int output_height = scale_dimension(pass->scale_type_y, pass->scale_y, pass->absolute_height, pass_size->source_height, pass_size->previous_height, pass_size->viewport_height);

    if (width)
        *width = MAX(1, output_width);
    if (height)
        *height = MAX(1, output_height);
}

int ogl_shader_chain_get_parameter_count(void)
{
    return preset_loaded ? active_preset.parameter_count : 0;
}

const ShaderPresetParameter* ogl_shader_chain_get_parameter(int index)
{
    if (!preset_loaded || index < 0 || index >= active_preset.parameter_count)
        return NULL;

    return &active_preset.parameters[index];
}

bool ogl_shader_chain_set_parameter(int index, float value)
{
    if (!preset_loaded || index < 0 || index >= active_preset.parameter_count)
        return false;

    ShaderPresetParameter* parameter = &active_preset.parameters[index];
    parameter->value = CLAMP(value, parameter->minimum, parameter->maximum);
    return true;
}

bool ogl_shader_chain_restore_default_parameters(void)
{
    if (!preset_loaded || active_preset.parameter_count <= 0)
        return false;

    shader_preset_apply_variant(&active_preset, active_preset.variant);
    return true;
}

int ogl_shader_chain_get_variant_count(void)
{
    return preset_loaded ? active_preset.variant_count : 0;
}

const char* ogl_shader_chain_get_variant_name(int index)
{
    if (!preset_loaded || index < 0 || index >= active_preset.variant_count)
        return "";

    return active_preset.variants[index].name;
}

int ogl_shader_chain_get_variant(void)
{
    return preset_loaded ? active_preset.variant : 0;
}

bool ogl_shader_chain_set_variant(int index)
{
    if (!preset_loaded || index < 0 || index >= active_preset.variant_count)
        return false;

    shader_preset_apply_variant(&active_preset, index);
    return true;
}

int ogl_shader_chain_find_variant(const char* name)
{
    return preset_loaded ? shader_preset_find_variant(&active_preset, name) : -1;
}

void ogl_shader_chain_apply_preset_uniforms(int index, const OglShaderChainUniforms* uniforms)
{
    if (!uniforms || !preset_loaded || index < 0 || index >= active_preset.pass_count)
        return;

    PresetProgramState* state = &preset_programs[index];
    if (!state->program)
        return;

    if (state->uniform_source_size >= 0)
        glUniform4f(state->uniform_source_size, (float)uniforms->input_width, (float)uniforms->input_height, safe_inverse(uniforms->input_width), safe_inverse(uniforms->input_height));

    if (state->uniform_original_size >= 0)
        glUniform4f(state->uniform_original_size, (float)uniforms->source_width, (float)uniforms->source_height, safe_inverse(uniforms->source_width), safe_inverse(uniforms->source_height));

    if (state->uniform_output_size >= 0)
        glUniform4f(state->uniform_output_size, (float)uniforms->output_width, (float)uniforms->output_height, safe_inverse(uniforms->output_width), safe_inverse(uniforms->output_height));

    if (state->uniform_final_viewport_size >= 0)
        glUniform4f(state->uniform_final_viewport_size, (float)uniforms->viewport_width, (float)uniforms->viewport_height, safe_inverse(uniforms->viewport_width), safe_inverse(uniforms->viewport_height));

    if (state->uniform_feedback_size >= 0)
        glUniform4f(state->uniform_feedback_size, (float)pass_width, (float)pass_height, safe_inverse(pass_width), safe_inverse(pass_height));

    if (state->uniform_source_history_size >= 0)
        glUniform4f(state->uniform_source_history_size, (float)pass_history_widths[index], (float)pass_history_heights[index], safe_inverse(pass_history_widths[index]), safe_inverse(pass_history_heights[index]));

    for (int i = 0; i < index; i++)
    {
        if (state->uniform_pass_output_size[i] >= 0)
            glUniform4f(state->uniform_pass_output_size[i], (float)intermediate_widths[i], (float)intermediate_heights[i], safe_inverse(intermediate_widths[i]), safe_inverse(intermediate_heights[i]));
    }

    if (state->uniform_source_history_count >= 0)
        glUniform1i(state->uniform_source_history_count, pass_history_counts[index]);

    if (state->uniform_frame_count >= 0)
        glUniform1i(state->uniform_frame_count, preset_frame_count);

    if (state->uniform_frame_direction >= 0)
        glUniform1i(state->uniform_frame_direction, 1);

    if (state->uniform_original_aspect >= 0)
        glUniform1f(state->uniform_original_aspect, uniforms->original_aspect);

    if (state->uniform_background_color >= 0)
        glUniform4f(state->uniform_background_color, uniforms->background_color[0], uniforms->background_color[1], uniforms->background_color[2], 1.0f);

    for (int i = 0; i < active_preset.parameter_count; i++)
    {
        if (state->uniform_parameters[i] >= 0)
            glUniform1f(state->uniform_parameters[i], active_preset.parameters[i].value);
    }

    if (index == active_preset.pass_count - 1)
        preset_frame_count++;
}

void ogl_shader_chain_bind_preset_textures(int index, uint32_t input_texture)
{
    if (!preset_loaded || index < 0 || index >= active_preset.pass_count)
        return;

    const PresetProgramState* state = &preset_programs[index];
    uint32_t fallback_texture = source_texture ? source_texture : input_texture;

    for (int i = 0; i < state->texture_count; i++)
    {
        const PresetTextureBinding* binding = &state->textures[i];
        uint32_t texture = 0;

        switch (binding->kind)
        {
            case PresetTexture_Source:
                texture = input_texture;
                break;
            case PresetTexture_Original:
                texture = source_texture;
                break;
            case PresetTexture_Feedback:
                texture = feedback_texture;
                break;
            case PresetTexture_SourceHistory:
                texture = ogl_shader_chain_get_pass_history_texture(index, binding->index);
                break;
            case PresetTexture_PassOutput:
                texture = intermediate_textures[binding->index];
                break;
            case PresetTexture_PassFeedback:
                texture = pass_feedback_textures[binding->index] ? pass_feedback_textures[binding->index] : black_texture;
                break;
        }

        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, texture ? texture : fallback_texture);
    }

    glActiveTexture(GL_TEXTURE0);
}

uint32_t ogl_shader_chain_get_source_texture(void)
{
    return source_texture;
}

uint32_t ogl_shader_chain_get_pass_texture(void)
{
    return pass_texture;
}

uint32_t ogl_shader_chain_get_pass_framebuffer(void)
{
    return pass_fbo;
}

uint32_t ogl_shader_chain_get_feedback_texture(void)
{
    return feedback_texture;
}

uint32_t ogl_shader_chain_get_feedback_framebuffer(void)
{
    return feedback_fbo;
}

uint32_t ogl_shader_chain_get_intermediate_texture(int index)
{
    if (index < 0 || index >= SHADER_PRESET_MAX_PASSES - 1)
        return 0;

    return intermediate_textures[index];
}

uint32_t ogl_shader_chain_get_intermediate_framebuffer(int index)
{
    if (index < 0 || index >= SHADER_PRESET_MAX_PASSES - 1)
        return 0;

    return intermediate_fbos[index];
}

bool ogl_shader_chain_store_pass_history(int index, uint32_t texture, int width, int height, bool filter_linear, bool float_framebuffer)
{
    if (!initialized || !texture || index < 0 || index >= SHADER_PRESET_MAX_PASSES)
        return false;

    if (!ensure_history_copy_framebuffer())
        return false;

    if (!resize_pass_history_textures(index, width, height, filter_linear, float_framebuffer))
        return false;

    int target = pass_history_write_indices[index];

    if (!copy_texture(texture, pass_history_textures[index][target], pass_history_widths[index], pass_history_heights[index]))
        return false;

    pass_history_write_indices[index] = (pass_history_write_indices[index] + 1) % SHADER_PRESET_MAX_HISTORY_TEXTURES;
    if (pass_history_counts[index] < SHADER_PRESET_MAX_HISTORY_TEXTURES)
        pass_history_counts[index]++;

    return true;
}

void ogl_shader_chain_store_pass_feedbacks(void)
{
    if (!initialized || !preset_loaded)
        return;

    for (int i = 0; i < active_preset.pass_count - 1; i++)
    {
        if (!pass_feedback_used[i] || !intermediate_textures[i])
            continue;

        if (!ensure_history_copy_framebuffer())
            return;

        if (!resize_pass_feedback_texture(i, intermediate_widths[i], intermediate_heights[i], intermediate_filter_linears[i], intermediate_float_framebuffers[i]))
            continue;

        copy_texture(intermediate_textures[i], pass_feedback_textures[i], pass_feedback_widths[i], pass_feedback_heights[i]);
    }
}

uint32_t ogl_shader_chain_get_pass_history_texture(int pass_index, int history_index)
{
    if (pass_index < 0 || pass_index >= SHADER_PRESET_MAX_PASSES || history_index < 0 || history_index >= SHADER_PRESET_MAX_HISTORY_TEXTURES)
        return 0;

    if (pass_history_counts[pass_index] <= history_index)
        return pass_history_textures[pass_index][0];

    int index = pass_history_write_indices[pass_index] - 1 - history_index;
    while (index < 0)
        index += SHADER_PRESET_MAX_HISTORY_TEXTURES;

    return pass_history_textures[pass_index][index];
}

int ogl_shader_chain_get_pass_width(void)
{
    return pass_width;
}

int ogl_shader_chain_get_pass_height(void)
{
    return pass_height;
}

static bool load_preset_programs(const ShaderPreset* preset, PresetProgramState* programs, char* error, size_t error_size)
{
    for (int i = 0; i < preset->pass_count; i++)
    {
        std::string source;
        if (!shader_preset_read_text_file(preset->passes[i].shader_path, source, error, error_size))
        {
            for (int j = 0; j < i; j++)
            {
                if (programs[j].program)
                    glDeleteProgram(programs[j].program);
            }
            return false;
        }

        uint32_t new_program = ogl_shader_program_create_fragment(preset->passes[i].shader_path, source.c_str(), error, error_size);
        if (!new_program)
        {
            for (int j = 0; j < i; j++)
            {
                if (programs[j].program)
                    glDeleteProgram(programs[j].program);
            }
            return false;
        }

        programs[i].program = new_program;
    }

    return true;
}

static void bind_preset_uniform_locations(int index)
{
    PresetProgramState* state = &preset_programs[index];
    uint32_t program = state->program;
    char uniform_name[SHADER_PRESET_MAX_PARAMETER_NAME + 16];

    glUseProgram(program);

    state->texture_count = 0;
    add_texture_binding(state, "Source", PresetTexture_Source, 0);
    add_texture_binding(state, "Original", PresetTexture_Original, 0);
    add_texture_binding(state, "PassFeedback0", PresetTexture_Feedback, 0);

    for (int i = 0; i < SHADER_PRESET_MAX_HISTORY_TEXTURES; i++)
    {
        snprintf(uniform_name, sizeof(uniform_name), "SourceHistory%d", i);
        add_texture_binding(state, uniform_name, PresetTexture_SourceHistory, i);
    }

    for (int i = 0; i < SHADER_PRESET_MAX_PASSES; i++)
        state->uniform_pass_output_size[i] = -1;

    for (int i = 0; i < index; i++)
    {
        snprintf(uniform_name, sizeof(uniform_name), "PassOutput%d", i);
        add_texture_binding(state, uniform_name, PresetTexture_PassOutput, i);

        if (active_preset.passes[i].alias[0] != '\0')
            add_texture_binding(state, active_preset.passes[i].alias, PresetTexture_PassOutput, i);

        state->uniform_pass_output_size[i] = get_pass_output_size_location(program, i);
    }

    for (int i = 0; i < active_preset.pass_count - 1; i++)
    {
        if (active_preset.passes[i].alias[0] == '\0')
            continue;

        snprintf(uniform_name, sizeof(uniform_name), "%sFeedback", active_preset.passes[i].alias);
        if (add_texture_binding(state, uniform_name, PresetTexture_PassFeedback, i))
            pass_feedback_used[i] = true;
    }

    state->uniform_source_size = glGetUniformLocation(program, "SourceSize");
    state->uniform_original_size = glGetUniformLocation(program, "OriginalSize");
    state->uniform_output_size = glGetUniformLocation(program, "OutputSize");
    state->uniform_final_viewport_size = glGetUniformLocation(program, "FinalViewportSize");
    state->uniform_feedback_size = glGetUniformLocation(program, "PassFeedback0Size");
    state->uniform_source_history_size = glGetUniformLocation(program, "SourceHistorySize");
    state->uniform_source_history_count = glGetUniformLocation(program, "SourceHistoryCount");
    state->uniform_frame_count = glGetUniformLocation(program, "FrameCount");
    state->uniform_frame_direction = glGetUniformLocation(program, "FrameDirection");
    state->uniform_original_aspect = glGetUniformLocation(program, "OriginalAspect");
    state->uniform_background_color = glGetUniformLocation(program, "BackgroundColor");

    for (int i = 0; i < SHADER_PRESET_MAX_PARAMETERS; i++)
        state->uniform_parameters[i] = -1;

    for (int i = 0; i < active_preset.parameter_count; i++)
        state->uniform_parameters[i] = glGetUniformLocation(program, active_preset.parameters[i].name);

    glUseProgram(0);
}

static bool add_texture_binding(PresetProgramState* state, const char* name, PresetTextureKind kind, int index)
{
    int location = glGetUniformLocation(state->program, name);
    if (location < 0)
        return false;

    if (state->texture_count >= OGL_SHADER_CHAIN_MAX_TEXTURE_UNITS)
    {
        Log("Shader preset sampler %s exceeds %d textures per pass", name, OGL_SHADER_CHAIN_MAX_TEXTURE_UNITS);
        return false;
    }

    glUniform1i(location, state->texture_count);
    state->textures[state->texture_count].kind = kind;
    state->textures[state->texture_count].index = index;
    state->texture_count++;
    return true;
}

static int get_pass_output_size_location(uint32_t program, int index)
{
    char uniform_name[SHADER_PRESET_MAX_PARAMETER_NAME + 16];
    int location = -1;

    if (active_preset.passes[index].alias[0] != '\0')
    {
        snprintf(uniform_name, sizeof(uniform_name), "%sSize", active_preset.passes[index].alias);
        location = glGetUniformLocation(program, uniform_name);
    }

    if (location < 0)
    {
        snprintf(uniform_name, sizeof(uniform_name), "PassOutputSize%d", index);
        location = glGetUniformLocation(program, uniform_name);
    }

    if (location < 0)
    {
        snprintf(uniform_name, sizeof(uniform_name), "PassOutput%dSize", index);
        location = glGetUniformLocation(program, uniform_name);
    }

    return location;
}

static void clear_preset_state(void)
{
    memset(&active_preset, 0, sizeof(active_preset));
    memset(preset_programs, 0, sizeof(preset_programs));
    preset_loaded = false;
    preset_frame_count = 0;
    release_intermediate_textures(0);
    release_pass_feedback_textures();
    clear_all_pass_history_textures();
}

static void delete_preset_programs(void)
{
    for (int i = 0; i < SHADER_PRESET_MAX_PASSES; i++)
    {
        if (preset_programs[i].program)
            glDeleteProgram(preset_programs[i].program);
        preset_programs[i].program = 0;
    }
}

static bool ensure_intermediate_texture(int index)
{
    if (index < 0 || index >= SHADER_PRESET_MAX_PASSES - 1)
        return false;

    if (!intermediate_textures[index])
        glGenTextures(1, &intermediate_textures[index]);

    if (!intermediate_fbos[index])
        glGenFramebuffers(1, &intermediate_fbos[index]);

    return intermediate_textures[index] != 0 && intermediate_fbos[index] != 0;
}

static bool ensure_history_copy_framebuffer(void)
{
    if (!history_copy_fbo)
        glGenFramebuffers(1, &history_copy_fbo);

    return history_copy_fbo != 0;
}

static void release_intermediate_textures(int keep_count)
{
    keep_count = CLAMP(keep_count, 0, SHADER_PRESET_MAX_PASSES - 1);

    for (int i = keep_count; i < SHADER_PRESET_MAX_PASSES - 1; i++)
    {
        if (intermediate_fbos[i])
            glDeleteFramebuffers(1, &intermediate_fbos[i]);

        if (intermediate_textures[i])
            glDeleteTextures(1, &intermediate_textures[i]);

        intermediate_fbos[i] = 0;
        intermediate_textures[i] = 0;
        intermediate_widths[i] = 1;
        intermediate_heights[i] = 1;
        intermediate_filter_linears[i] = false;
        intermediate_float_framebuffers[i] = false;
    }
}

static void clear_feedback_texture(void)
{
    if (!feedback_fbo)
        return;

    glBindFramebuffer(GL_FRAMEBUFFER, feedback_fbo);
    glViewport(0, 0, pass_width, pass_height);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void clear_pass_history_textures(int index)
{
    if (index < 0 || index >= SHADER_PRESET_MAX_PASSES)
        return;

    if (history_copy_fbo)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, history_copy_fbo);
        glViewport(0, 0, pass_history_widths[index], pass_history_heights[index]);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        for (int i = 0; i < SHADER_PRESET_MAX_HISTORY_TEXTURES; i++)
        {
            if (!pass_history_textures[index][i])
                continue;

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pass_history_textures[index][i], 0);
            glClear(GL_COLOR_BUFFER_BIT);
        }

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    pass_history_counts[index] = 0;
    pass_history_write_indices[index] = 0;
}

static void clear_all_pass_history_textures(void)
{
    for (int i = 0; i < SHADER_PRESET_MAX_PASSES; i++)
        clear_pass_history_textures(i);
}

static bool resize_pass_history_textures(int index, int width, int height, bool filter_linear, bool float_framebuffer)
{
    if (index < 0 || index >= SHADER_PRESET_MAX_PASSES)
        return false;

    if (width < 1)
        width = 1;
    if (height < 1)
        height = 1;

    bool same_size = pass_history_widths[index] == width && pass_history_heights[index] == height && pass_history_float_framebuffers[index] == float_framebuffer;
    bool same_filter = pass_history_filter_linears[index] == filter_linear;
    int internal_format = float_framebuffer ? GL_RGBA16F : GL_RGBA8;
    uint32_t type = float_framebuffer ? GL_FLOAT : GL_UNSIGNED_BYTE;
    bool needs_create = false;

    for (int i = 0; i < SHADER_PRESET_MAX_HISTORY_TEXTURES; i++)
    {
        if (!pass_history_textures[index][i])
            needs_create = true;
    }

    bool resize = !same_size || needs_create;

    for (int i = 0; i < SHADER_PRESET_MAX_HISTORY_TEXTURES; i++)
    {
        if (!pass_history_textures[index][i])
            glGenTextures(1, &pass_history_textures[index][i]);

        if (!pass_history_textures[index][i])
            return false;

        if (resize)
        {
            resize_texture_2d(pass_history_textures[index][i], width, height, internal_format, GL_RGBA, type, NULL, filter_linear);
        }
        else if (!same_filter)
        {
            glBindTexture(GL_TEXTURE_2D, pass_history_textures[index][i]);
            configure_texture_2d(filter_linear);
        }
    }

    if (resize)
    {
        pass_history_widths[index] = width;
        pass_history_heights[index] = height;
        pass_history_filter_linears[index] = filter_linear;
        pass_history_float_framebuffers[index] = float_framebuffer;
        clear_pass_history_textures(index);
    }
    else if (!same_filter)
    {
        pass_history_filter_linears[index] = filter_linear;
    }

    return true;
}

static void release_pass_feedback_textures(void)
{
    for (int i = 0; i < SHADER_PRESET_MAX_PASSES - 1; i++)
    {
        if (pass_feedback_textures[i])
            glDeleteTextures(1, &pass_feedback_textures[i]);

        pass_feedback_textures[i] = 0;
        pass_feedback_widths[i] = 1;
        pass_feedback_heights[i] = 1;
        pass_feedback_filter_linears[i] = false;
        pass_feedback_float_framebuffers[i] = false;
        pass_feedback_used[i] = false;
    }
}

static bool resize_pass_feedback_texture(int index, int width, int height, bool filter_linear, bool float_framebuffer)
{
    if (width < 1)
        width = 1;
    if (height < 1)
        height = 1;

    bool create = !pass_feedback_textures[index];

    if (create)
        glGenTextures(1, &pass_feedback_textures[index]);

    if (!pass_feedback_textures[index])
        return false;

    bool same_size = pass_feedback_widths[index] == width && pass_feedback_heights[index] == height;

    if (create || !same_size || pass_feedback_float_framebuffers[index] != float_framebuffer)
    {
        int internal_format = float_framebuffer ? GL_RGBA16F : GL_RGBA8;
        uint32_t type = float_framebuffer ? GL_FLOAT : GL_UNSIGNED_BYTE;
        resize_texture_2d(pass_feedback_textures[index], width, height, internal_format, GL_RGBA, type, NULL, filter_linear);
        pass_feedback_widths[index] = width;
        pass_feedback_heights[index] = height;
        pass_feedback_filter_linears[index] = filter_linear;
        pass_feedback_float_framebuffers[index] = float_framebuffer;
        clear_texture(pass_feedback_textures[index], width, height);
    }
    else if (pass_feedback_filter_linears[index] != filter_linear)
    {
        glBindTexture(GL_TEXTURE_2D, pass_feedback_textures[index]);
        configure_texture_2d(filter_linear);
        pass_feedback_filter_linears[index] = filter_linear;
    }

    return true;
}

static bool copy_texture(uint32_t source, uint32_t target, int width, int height)
{
    glBindFramebuffer(GL_FRAMEBUFFER, history_copy_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, source, 0);
    glReadBuffer(GL_COLOR_ATTACHMENT0);

    if (!check_framebuffer_complete(framebuffer_name))
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return false;
    }

    glBindTexture(GL_TEXTURE_2D, target);
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, width, height);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return true;
}

static void clear_texture(uint32_t texture, int width, int height)
{
    glBindFramebuffer(GL_FRAMEBUFFER, history_copy_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    glViewport(0, 0, width, height);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void set_last_error(const char* message)
{
    strncpy_fit(last_error, message ? message : "Unknown shader preset error", sizeof(last_error));
}

static float safe_inverse(int value)
{
    return value > 0 ? 1.0f / (float)value : 0.0f;
}

static void configure_texture_2d(bool filter_linear)
{
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter_linear ? GL_LINEAR : GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter_linear ? GL_LINEAR : GL_NEAREST);
}

static bool resize_framebuffer_texture(uint32_t texture, uint32_t fbo, int width, int height, bool filter_linear, bool float_framebuffer, const char* name)
{
    int internal_format = float_framebuffer ? GL_RGBA16F : GL_RGBA8;
    uint32_t type = float_framebuffer ? GL_FLOAT : GL_UNSIGNED_BYTE;
    resize_texture_2d(texture, width, height, internal_format, GL_RGBA, type, NULL, filter_linear);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    bool complete = check_framebuffer_complete(name);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return complete;
}

static int scale_dimension(ShaderPresetScaleType scale_type, float scale, int absolute_size, int source_size, int previous_size, int viewport_size)
{
    switch (scale_type)
    {
        case ShaderPresetScale_Source:
            return (int)((float)source_size * scale + 0.5f);
        case ShaderPresetScale_Previous:
            return (int)((float)previous_size * scale + 0.5f);
        case ShaderPresetScale_Absolute:
            return absolute_size > 0 ? absolute_size : viewport_size;
        case ShaderPresetScale_Viewport:
        default:
            return (int)((float)viewport_size * scale + 0.5f);
    }
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