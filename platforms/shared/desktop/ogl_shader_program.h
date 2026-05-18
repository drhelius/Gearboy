#ifndef OGL_SHADER_PROGRAM_H
#define OGL_SHADER_PROGRAM_H

#include <stddef.h>
#include <stdint.h>

#ifdef OGL_SHADER_PROGRAM_IMPORT
    #define EXTERN
#else
    #define EXTERN extern
#endif

EXTERN uint32_t ogl_shader_program_create_fragment(const char* program_name, const char* fragment_source, char* error, size_t error_size);
EXTERN const char* ogl_shader_program_get_glsl_version(void);
EXTERN uint32_t ogl_shader_program_compile_shader(uint32_t shader_type, const char* shader_name, const char** sources, int source_count, char* error, size_t error_size);
EXTERN uint32_t ogl_shader_program_link(uint32_t vertex_shader, uint32_t fragment_shader, const char* program_name, char* error, size_t error_size);

#undef OGL_SHADER_PROGRAM_IMPORT
#undef EXTERN
#endif /* OGL_SHADER_PROGRAM_H */
