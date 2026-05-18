#if defined(__APPLE__)
    #define GL_SILENCE_DEPRECATION
    #include <OpenGL/gl3.h>
#else
    #include <glad.h>
#endif

#include <string.h>
#include "gearboy.h"

#define OGL_SHADER_PROGRAM_IMPORT
#include "ogl_shader_program.h"

static bool source_has_version(const char* source);
static void set_error(char* error, size_t error_size, const char* message);

uint32_t ogl_shader_program_create_fragment(const char* program_name, const char* fragment_source, char* error, size_t error_size)
{
    if (error && error_size > 0)
        error[0] = '\0';

    if (!fragment_source || fragment_source[0] == '\0')
    {
        set_error(error, error_size, "Shader source is empty");
        return 0;
    }

    const char* version = ogl_shader_program_get_glsl_version();

    const char* vs_body =
        "in vec2 aPos;\n"
        "in vec2 aTexCoord;\n"
        "out vec2 vTexCoord;\n"
        "void main() {\n"
        "    gl_Position = vec4(aPos, 0.0, 1.0);\n"
        "    vTexCoord = aTexCoord;\n"
        "}\n";

    const char* vs_sources[2] = { version, vs_body };

    const char* fs_sources_with_version[2] = { version, fragment_source };
    const char* fs_sources_without_version[1] = { fragment_source };
    const char** fs_sources = source_has_version(fragment_source) ? fs_sources_without_version : fs_sources_with_version;
    int fs_source_count = source_has_version(fragment_source) ? 1 : 2;

    uint32_t vertex_shader = ogl_shader_program_compile_shader(GL_VERTEX_SHADER, "Shader preset vertex", vs_sources, 2, error, error_size);
    if (!vertex_shader)
        return 0;

    uint32_t fragment_shader = ogl_shader_program_compile_shader(GL_FRAGMENT_SHADER, program_name ? program_name : "Shader preset fragment", fs_sources, fs_source_count, error, error_size);
    if (!fragment_shader)
    {
        glDeleteShader(vertex_shader);
        return 0;
    }

    uint32_t program = ogl_shader_program_link(vertex_shader, fragment_shader, program_name ? program_name : "Shader preset", error, error_size);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return program;
}

const char* ogl_shader_program_get_glsl_version(void)
{
#if defined(__APPLE__)
    return "#version 150\n";
#else
    return "#version 130\n";
#endif
}

static bool source_has_version(const char* source)
{
    while (*source == ' ' || *source == '\t' || *source == '\r' || *source == '\n')
        source++;

    return strncmp(source, "#version", 8) == 0;
}

uint32_t ogl_shader_program_compile_shader(uint32_t shader_type, const char* shader_name, const char** sources, int source_count, char* error, size_t error_size)
{
    uint32_t shader = glCreateShader(shader_type);
    glShaderSource(shader, source_count, sources, NULL);
    glCompileShader(shader);

    GLint compiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled)
    {
        GLchar info[4096];
        glGetShaderInfoLog(shader, sizeof(info), NULL, info);
        Error("%s shader compile error: %s", shader_name, info);
        set_error(error, error_size, info);
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

uint32_t ogl_shader_program_link(uint32_t vertex_shader, uint32_t fragment_shader, const char* program_name, char* error, size_t error_size)
{
    uint32_t program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glBindAttribLocation(program, 0, "aPos");
    glBindAttribLocation(program, 1, "aTexCoord");
    glLinkProgram(program);

    GLint linked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked)
    {
        GLchar info[4096];
        glGetProgramInfoLog(program, sizeof(info), NULL, info);
        Error("%s program link error: %s", program_name, info);
        set_error(error, error_size, info);
        glDeleteProgram(program);
        return 0;
    }

    return program;
}

static void set_error(char* error, size_t error_size, const char* message)
{
    if (!error || error_size == 0)
        return;

    if (!message)
        message = "Unknown shader error";

    strncpy_fit(error, message, error_size);
}
