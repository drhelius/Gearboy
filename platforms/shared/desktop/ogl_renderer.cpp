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

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "emu.h"
#include "config.h"
#include "utils.h"
#include "gearboy.h"

#define OGL_RENDERER_IMPORT
#include "ogl_renderer.h"

static uint32_t system_texture;
static uint32_t frame_buffer_object;
static int current_screen_width;
int current_screen_height;
static bool first_frame;
static bool mix_round_error = false;

static uint32_t quad_shader_program = 0;
static uint32_t quad_vao = 0;
static uint32_t quad_vbo = 0;
static int quad_uniform_texture = -1;
static int quad_uniform_color = -1;
static int quad_uniform_tex_scale = -1;
static int quad_uniform_viewport_size = -1;
static int quad_uniform_use_fragcoord = -1;

static void init_ogl_gui(void);
static void init_ogl_emu(void);
static void init_ogl_debug(void);
static void init_ogl_savestates(void);
static void init_shaders(void);
static void render_gui(void);
static void render_emu_normal(void);
static void render_emu_mix(void);
static void update_emu_texture(void);
static void render_quad(float tex_h, float tex_v);
static void update_system_texture(void);
static void update_debug_textures(void);
static void update_savestates_texture(void);
static void render_matrix(void);

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

    init_shaders();
    init_ogl_gui();
    init_ogl_emu();
    init_ogl_debug();
    init_ogl_savestates();

    first_frame = true;
    return true;
}

void ogl_renderer_destroy(void)
{
    glDeleteFramebuffers(1, &frame_buffer_object); 
    glDeleteTextures(1, &ogl_renderer_emu_texture);
    glDeleteTextures(1, &system_texture);

    glDeleteTextures(1, &ogl_renderer_emu_debug_vram_background);
    glDeleteTextures(40, ogl_renderer_emu_debug_vram_sprites);
    glDeleteTextures(1, &ogl_renderer_emu_debug_vram_tiles);
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
    current_screen_width = GAMEBOY_WIDTH;
    current_screen_height = GAMEBOY_HEIGHT;

    if (config_debug.debug)
    {
        update_debug_textures();
    }

    update_savestates_texture();

    if (config_video.mix_frames)
        render_emu_mix();
    else
        render_emu_normal();

    if (config_video.matrix)
        render_matrix();

    update_emu_texture();

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

static void init_ogl_gui(void)
{
#if defined(__APPLE__)
    ImGui_ImplOpenGL3_Init("#version 150");
#else
    ImGui_ImplOpenGL3_Init("#version 130");
#endif
}

static void init_ogl_emu(void)
{
    glGenFramebuffers(1, &frame_buffer_object);
    glGenTextures(1, &ogl_renderer_emu_texture);
    glGenTextures(1, &system_texture);

    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_object);
    glBindTexture(GL_TEXTURE_2D, ogl_renderer_emu_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ogl_renderer_emu_texture, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glBindTexture(GL_TEXTURE_2D, system_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, GAMEBOY_WIDTH, GAMEBOY_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*) emu_frame_buffer);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

static void init_ogl_debug(void)
{
    glGenTextures(1, &ogl_renderer_emu_debug_vram_background);
    glBindTexture(GL_TEXTURE_2D, ogl_renderer_emu_debug_vram_background);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)emu_debug_background_buffer);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    for (int s = 0; s < 40; s++)
    {
        glGenTextures(1, &ogl_renderer_emu_debug_vram_sprites[s]);
        glBindTexture(GL_TEXTURE_2D, ogl_renderer_emu_debug_vram_sprites[s]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 8, 16, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)emu_debug_oam_buffers[s]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }

    glGenTextures(1, &ogl_renderer_emu_debug_vram_tiles);
    glBindTexture(GL_TEXTURE_2D, ogl_renderer_emu_debug_vram_tiles);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 16 * 8, 24 * 8, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)emu_debug_tile_buffers[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

static void init_ogl_savestates(void)
{
    glGenTextures(1, &ogl_renderer_emu_savestates);
    glBindTexture(GL_TEXTURE_2D, ogl_renderer_emu_savestates);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, SYSTEM_TEXTURE_WIDTH, SYSTEM_TEXTURE_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

static void render_gui(void)
{
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

static void render_emu_normal(void)
{
    float tex_h = (float)current_screen_width / (float)GAMEBOY_WIDTH;
    float tex_v = (float)current_screen_height / (float)GAMEBOY_HEIGHT;

    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_object);

    glDisable(GL_BLEND);

    update_system_texture();

    render_quad(tex_h, tex_v);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void render_emu_mix(void)
{
    float tex_h = (float)current_screen_width / (float)GAMEBOY_WIDTH;
    float tex_v = (float)current_screen_height / (float)GAMEBOY_HEIGHT;

    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_object);

    float alpha = 0.15f + (0.50f * (1.0f - config_video.mix_frames_intensity));

    if (first_frame)
    {
        first_frame = false;
        alpha = 1.0f;
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    float round_color = 1.0f - (mix_round_error ? 0.03f : 0.0f);
    mix_round_error = !mix_round_error;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    update_system_texture();

    int viewportWidth = current_screen_width * FRAME_BUFFER_SCALE;
    int viewportHeight = current_screen_height * FRAME_BUFFER_SCALE;

    glUseProgram(quad_shader_program);
    glUniform2f(quad_uniform_tex_scale, tex_h, tex_v);
    glUniform2f(quad_uniform_viewport_size, (float)viewportWidth, (float)viewportHeight);
    glUniform1i(quad_uniform_use_fragcoord, 0);
    glUniform4f(quad_uniform_color, round_color, round_color, round_color, alpha);

    glViewport(0, 0, viewportWidth, viewportHeight);

    glBindVertexArray(quad_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
    glUseProgram(0);

    glDisable(GL_BLEND);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void update_system_texture(void)
{
    glBindTexture(GL_TEXTURE_2D, system_texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, current_screen_width, current_screen_height,
            GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*) emu_frame_buffer);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    if (config_video.bilinear)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }
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
        glBindTexture(GL_TEXTURE_2D, ogl_renderer_emu_debug_vram_tiles);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 16 * 8, 24 * 8,
                GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*) emu_debug_tile_buffers[0]);
    }
}

static void update_savestates_texture(void)
{
    int i = config_emulator.save_slot;

    if (IsValidPointer(emu_savestates_screenshots[i].data))
    {
        int width = emu_savestates_screenshots[i].width;
        int height = emu_savestates_screenshots[i].height;
        glBindTexture(GL_TEXTURE_2D, ogl_renderer_emu_savestates);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*) emu_savestates_screenshots[i].data);
    }
}

static void update_emu_texture(void)
{
    glBindTexture(GL_TEXTURE_2D, ogl_renderer_emu_texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    if (config_video.matrix)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }
}

static void render_quad(float tex_h, float tex_v)
{
    int viewportWidth = current_screen_width * FRAME_BUFFER_SCALE;
    int viewportHeight = current_screen_height * FRAME_BUFFER_SCALE;

    glUseProgram(quad_shader_program);
    glUniform2f(quad_uniform_tex_scale, tex_h, tex_v);
    glUniform2f(quad_uniform_viewport_size, (float)viewportWidth, (float)viewportHeight);
    glUniform1i(quad_uniform_use_fragcoord, 0);
    glUniform4f(quad_uniform_color, 1.0f, 1.0f, 1.0f, 1.0f);

    glViewport(0, 0, viewportWidth, viewportHeight);

    glBindVertexArray(quad_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
    glUseProgram(0);
}

static void render_matrix(void)
{
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_object);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    int viewportWidth = current_screen_width * FRAME_BUFFER_SCALE;
    int viewportHeight = current_screen_height * FRAME_BUFFER_SCALE;

    float tex_h = (float)current_screen_width;
    float tex_v = (float)current_screen_height;

    glUseProgram(quad_shader_program);
    glUniform2f(quad_uniform_tex_scale, tex_h, tex_v);
    glUniform2f(quad_uniform_viewport_size, (float)viewportWidth, (float)viewportHeight);
    glUniform1i(quad_uniform_use_fragcoord, 3);
    glUniform4f(quad_uniform_color, 1.0f, 1.0f, 1.0f, config_video.matrix_intensity);

    glViewport(0, 0, viewportWidth, viewportHeight);

    glBindVertexArray(quad_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);

    glUseProgram(0);

    glDisable(GL_BLEND);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void init_shaders(void)
{
#if defined(__APPLE__)
    const char* version = "#version 150\n";
#else
    const char* version = "#version 130\n";
#endif

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
        "uniform vec2 uViewportSize;\n"
        "uniform int uUseFragCoord;\n"
        "void main() {\n"
        "    if (uUseFragCoord == 2) {\n"
        "        float row = mod(floor(gl_FragCoord.y), 4.0);\n"
        "        FragColor = (row < 2.0) ? vec4(0.0, 0.0, 0.0, uColor.a) : vec4(1.0, 1.0, 1.0, 0.0);\n"
        "    } else if (uUseFragCoord == 3) {\n"
        "        float col = mod(floor(gl_FragCoord.x), 8.0);\n"
        "        float row = mod(floor(gl_FragCoord.y), 8.0);\n"
        "        bool dark = (col >= 7.0) || (row >= 7.0);\n"
        "        FragColor = dark ? vec4(0.0, 0.0, 0.0, uColor.a) : vec4(1.0, 1.0, 1.0, 0.0);\n"
        "    } else {\n"
        "        vec2 texCoord = uUseFragCoord == 1 ? gl_FragCoord.xy / uViewportSize * uTexScale : vTexCoord;\n"
        "        FragColor = texture(uTexture, texCoord) * uColor;\n"
        "    }\n"
        "}\n";

    const char* vs_sources[2] = { version, vs_body };
    const char* fs_sources[2] = { version, fs_body };

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 2, vs_sources, NULL);
    glCompileShader(vs);

    GLint compiled = 0;
    glGetShaderiv(vs, GL_COMPILE_STATUS, &compiled);
    if (!compiled)
    {
        GLchar info[512];
        glGetShaderInfoLog(vs, 512, NULL, info);
        Error("Vertex shader compile error: %s", info);
    }

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 2, fs_sources, NULL);
    glCompileShader(fs);

    glGetShaderiv(fs, GL_COMPILE_STATUS, &compiled);
    if (!compiled)
    {
        GLchar info[512];
        glGetShaderInfoLog(fs, 512, NULL, info);
        Error("Fragment shader compile error: %s", info);
    }

    quad_shader_program = glCreateProgram();
    glAttachShader(quad_shader_program, vs);
    glAttachShader(quad_shader_program, fs);
    glLinkProgram(quad_shader_program);

    GLint linked = 0;
    glGetProgramiv(quad_shader_program, GL_LINK_STATUS, &linked);
    if (!linked)
    {
        GLchar info[512];
        glGetProgramInfoLog(quad_shader_program, 512, NULL, info);
        Error("Shader program link error: %s", info);
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    quad_uniform_tex_scale = glGetUniformLocation(quad_shader_program, "uTexScale");
    quad_uniform_texture = glGetUniformLocation(quad_shader_program, "uTexture");
    quad_uniform_color = glGetUniformLocation(quad_shader_program, "uColor");
    quad_uniform_viewport_size = glGetUniformLocation(quad_shader_program, "uViewportSize");
    quad_uniform_use_fragcoord = glGetUniformLocation(quad_shader_program, "uUseFragCoord");

    glUseProgram(quad_shader_program);
    glUniform1i(quad_uniform_texture, 0);
    glUniform1i(quad_uniform_use_fragcoord, 0);
    glUseProgram(0);

    float quad_vertices[] = {
        -1.0f, -1.0f,  0.0f,  0.0f,
         1.0f, -1.0f,  1.0f,  0.0f,
        -1.0f,  1.0f,  0.0f,  1.0f,
         1.0f,  1.0f,  1.0f,  1.0f,
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
}