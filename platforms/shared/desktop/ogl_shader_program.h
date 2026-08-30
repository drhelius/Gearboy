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
