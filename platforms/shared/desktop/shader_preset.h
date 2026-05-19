#ifndef SHADER_PRESET_H
#define SHADER_PRESET_H

#include <stddef.h>
#include <stdint.h>
#include <string>

#define SHADER_PRESET_EXTENSION ".gshader"
#define SHADER_PRESET_MAX_PATH 1024
#define SHADER_PRESET_MAX_NAME 128
#define SHADER_PRESET_MAX_PARAMETER_NAME 64
#define SHADER_PRESET_MAX_PARAMETERS 32
#define SHADER_PRESET_MAX_DISCOVERED 32
#define SHADER_PRESET_MAX_PASSES 8
#define SHADER_PRESET_MAX_HISTORY_TEXTURES 8
#define SHADER_PRESET_MAX_PASS_OUTPUT_TEXTURES 4

enum ShaderPresetScaleType
{
    ShaderPresetScale_Source = 0,
    ShaderPresetScale_Viewport,
    ShaderPresetScale_Previous,
    ShaderPresetScale_Absolute
};

struct ShaderPresetParameter
{
    char name[SHADER_PRESET_MAX_PARAMETER_NAME];
    char label[SHADER_PRESET_MAX_NAME];
    float value;
    float default_value;
    float minimum;
    float maximum;
    float step;
};

struct ShaderPresetPass
{
    char shader_path[SHADER_PRESET_MAX_PATH];
    bool filter_linear;
    bool feedback;
    bool history;
    bool float_framebuffer;
    ShaderPresetScaleType scale_type_x;
    ShaderPresetScaleType scale_type_y;
    float scale_x;
    float scale_y;
    int absolute_width;
    int absolute_height;
};

struct ShaderPreset
{
    char name[SHADER_PRESET_MAX_NAME];
    char preset_path[SHADER_PRESET_MAX_PATH];
    char preset_dir[SHADER_PRESET_MAX_PATH];
    char shader_path[SHADER_PRESET_MAX_PATH];
    bool filter_linear;
    int pass_count;
    ShaderPresetPass passes[SHADER_PRESET_MAX_PASSES];
    int parameter_count;
    ShaderPresetParameter parameters[SHADER_PRESET_MAX_PARAMETERS];
};

struct ShaderPresetInfo
{
    char name[SHADER_PRESET_MAX_NAME];
    char path[SHADER_PRESET_MAX_PATH];
};

#ifdef SHADER_PRESET_IMPORT
    #define EXTERN
#else
    #define EXTERN extern
#endif

EXTERN bool shader_preset_load(const char* path, ShaderPreset* preset, char* error, size_t error_size);
EXTERN bool shader_preset_read_text_file(const char* path, std::string& text, char* error, size_t error_size);
EXTERN bool shader_preset_get_resource_root(char* path, size_t path_size);
EXTERN int shader_preset_scan_bundled(ShaderPresetInfo* presets, int max_presets);
EXTERN bool shader_preset_path_exists(const char* path);
EXTERN bool shader_preset_is_preset_path(const char* path);
EXTERN bool shader_preset_get_config_path(const char* preset_path, char* config_path, size_t config_path_size);
EXTERN bool shader_preset_resolve_config_path(const char* config_path, char* preset_path, size_t preset_path_size);
EXTERN bool shader_preset_config_path_matches(const char* config_path, const char* preset_path);

#undef SHADER_PRESET_IMPORT
#undef EXTERN
#endif /* SHADER_PRESET_H */
