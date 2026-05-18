#ifndef OGL_SHADER_CHAIN_H
#define OGL_SHADER_CHAIN_H

#include <stdint.h>
#include "shader_preset.h"

#ifdef OGL_SHADER_CHAIN_IMPORT
    #define EXTERN
#else
    #define EXTERN extern
#endif

struct OglShaderChainPassSize
{
    int source_width;
    int source_height;
    int previous_width;
    int previous_height;
    int viewport_width;
    int viewport_height;
};

struct OglShaderChainUniforms
{
    int source_width;
    int source_height;
    int input_width;
    int input_height;
    int output_width;
    int output_height;
    int viewport_width;
    int viewport_height;
    float original_aspect;
    float background_color[3];
};

struct OglShaderChainSourceTexture
{
    int width;
    int height;
    const void* pixels;
    bool filter_linear;
};

struct OglShaderChainFramebufferTexture
{
    int width;
    int height;
    bool filter_linear;
    bool float_framebuffer;
};

EXTERN bool ogl_shader_chain_init(const char* framebuffer_name);
EXTERN void ogl_shader_chain_destroy(void);
EXTERN bool ogl_shader_chain_is_initialized(void);

EXTERN bool ogl_shader_chain_update_source_texture(const OglShaderChainSourceTexture* texture);
EXTERN bool ogl_shader_chain_resize_pass_texture(const OglShaderChainFramebufferTexture* texture);

EXTERN bool ogl_shader_chain_load_preset(const char* path);
EXTERN void ogl_shader_chain_unload_preset(void);
EXTERN bool ogl_shader_chain_has_preset(void);
EXTERN uint32_t ogl_shader_chain_get_preset_program(void);
EXTERN uint32_t ogl_shader_chain_get_preset_pass_program(int index);
EXTERN const char* ogl_shader_chain_get_preset_name(void);
EXTERN const char* ogl_shader_chain_get_preset_path(void);
EXTERN const char* ogl_shader_chain_get_last_error(void);
EXTERN bool ogl_shader_chain_get_preset_filter_linear(void);
EXTERN int ogl_shader_chain_get_preset_pass_count(void);
EXTERN bool ogl_shader_chain_get_preset_pass_filter_linear(int index);
EXTERN bool ogl_shader_chain_get_preset_pass_float_framebuffer(int index);
EXTERN bool ogl_shader_chain_get_preset_pass_uses_history(int index);
EXTERN bool ogl_shader_chain_preset_uses_feedback(void);
EXTERN void ogl_shader_chain_get_preset_pass_output_size(int index, const OglShaderChainPassSize* pass_size, int* width, int* height);
EXTERN int ogl_shader_chain_get_parameter_count(void);
EXTERN const ShaderPresetParameter* ogl_shader_chain_get_parameter(int index);
EXTERN bool ogl_shader_chain_set_parameter(int index, float value);
EXTERN bool ogl_shader_chain_restore_default_parameters(void);
EXTERN void ogl_shader_chain_apply_preset_uniforms(int index, const OglShaderChainUniforms* uniforms);

EXTERN uint32_t ogl_shader_chain_get_source_texture(void);
EXTERN uint32_t ogl_shader_chain_get_pass_texture(void);
EXTERN uint32_t ogl_shader_chain_get_pass_framebuffer(void);
EXTERN uint32_t ogl_shader_chain_get_feedback_texture(void);
EXTERN uint32_t ogl_shader_chain_get_feedback_framebuffer(void);
EXTERN bool ogl_shader_chain_resize_intermediate_texture(int index, const OglShaderChainFramebufferTexture* texture);
EXTERN bool ogl_shader_chain_store_pass_history(int index, uint32_t texture, int width, int height, bool filter_linear, bool float_framebuffer);
EXTERN uint32_t ogl_shader_chain_get_intermediate_texture(int index);
EXTERN uint32_t ogl_shader_chain_get_intermediate_framebuffer(int index);
EXTERN uint32_t ogl_shader_chain_get_pass_history_texture(int pass_index, int history_index);
EXTERN int ogl_shader_chain_get_intermediate_width(int index);
EXTERN int ogl_shader_chain_get_intermediate_height(int index);
EXTERN int ogl_shader_chain_get_pass_width(void);
EXTERN int ogl_shader_chain_get_pass_height(void);

#undef OGL_SHADER_CHAIN_IMPORT
#undef EXTERN

#endif /* OGL_SHADER_CHAIN_H */