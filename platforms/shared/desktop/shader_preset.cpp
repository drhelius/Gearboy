#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <string>
#if defined(_WIN32)
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <windows.h>
#else
    #include <dirent.h>
#endif

#include "gearboy.h"
#define MINI_CASE_SENSITIVE
#include "ini.h"
#include "utils.h"

#define SHADER_PRESET_IMPORT
#include "shader_preset.h"

static void clear_preset(ShaderPreset* preset);
static bool load_ini(const char* path, mINI::INIStructure& ini, char* error, size_t error_size);
static bool load_passes(const mINI::INIStructure& ini, ShaderPreset* preset, char* error, size_t error_size);
static void load_parameters(const mINI::INIStructure& ini, ShaderPreset* preset);
static bool resolve_shader_path(const char* path, const char* preset_dir, char* out_path, size_t out_path_size, char* error, size_t error_size);
static bool get_candidate_resource_root(int index, char* path, size_t path_size);
static bool scan_directory_for_presets(const char* directory, ShaderPresetInfo* presets, int max_presets, int* count);
static bool add_discovered_preset(const char* path, ShaderPresetInfo* presets, int max_presets, int* count);
static void scan_bundled_presets_once(void);
static void sort_discovered_presets(ShaderPresetInfo* presets, int count);
static bool parse_bool(const std::string& value, bool default_value);
static int parse_int(const std::string& value, int default_value);
static float parse_float(const std::string& value, float default_value);
static ShaderPresetScaleType parse_scale_type(const std::string& value, ShaderPresetScaleType default_value);
static void set_error(char* error, size_t error_size, const char* message);
static void set_error_path(char* error, size_t error_size, const char* prefix, const char* path);

static ShaderPresetInfo bundled_presets[SHADER_PRESET_MAX_DISCOVERED];
static int bundled_preset_count = 0;
static bool bundled_presets_scanned = false;

bool shader_preset_load(const char* path, ShaderPreset* preset, char* error, size_t error_size)
{
    if (error && error_size > 0)
        error[0] = '\0';

    if (!path || path[0] == '\0')
    {
        set_error(error, error_size, "No shader preset path provided");
        return false;
    }

    if (!shader_preset_is_preset_path(path))
    {
        set_error(error, error_size, "Shader presets must use the .gshader extension");
        return false;
    }

    if (!preset)
    {
        set_error(error, error_size, "Invalid shader preset destination");
        return false;
    }

    mINI::INIStructure ini;
    if (!load_ini(path, ini, error, error_size))
        return false;

    clear_preset(preset);
    strncpy_fit(preset->preset_path, path, sizeof(preset->preset_path));
    get_directory(path, preset->preset_dir, sizeof(preset->preset_dir));

    std::string name = ini.get("Preset").get("Name");
    if (name.empty())
        get_filename_without_extension(path, preset->name, sizeof(preset->name));
    else
        strncpy_fit(preset->name, name.c_str(), sizeof(preset->name));

    if (!load_passes(ini, preset, error, error_size))
        return false;

    load_parameters(ini, preset);

    return true;
}

bool shader_preset_read_text_file(const char* path, std::string& text, char* error, size_t error_size)
{
    text.clear();

    std::ifstream file;
    open_ifstream_utf8(file, path, std::ios::in | std::ios::binary);
    if (!file.is_open())
    {
        set_error_path(error, error_size, "Unable to open shader file", path);
        return false;
    }

    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    if (size < 0)
    {
        set_error_path(error, error_size, "Unable to read shader file size", path);
        return false;
    }

    file.seekg(0, std::ios::beg);
    text.resize((size_t)size);
    if (size > 0 && !file.read(&text[0], size))
    {
        set_error_path(error, error_size, "Unable to read shader file", path);
        return false;
    }

    return true;
}

bool shader_preset_get_resource_root(char* path, size_t path_size)
{
    if (!path || path_size == 0)
        return false;

    for (int i = 0; i < 8; i++)
    {
        char candidate[SHADER_PRESET_MAX_PATH];
        if (!get_candidate_resource_root(i, candidate, sizeof(candidate)))
            continue;

        if (shader_preset_path_exists(candidate))
        {
            strncpy_fit(path, candidate, path_size);
            return true;
        }
    }

    path[0] = '\0';
    return false;
}

int shader_preset_scan_bundled(ShaderPresetInfo* presets, int max_presets)
{
    if (!presets || max_presets <= 0)
        return 0;

    scan_bundled_presets_once();

    int count = max_presets < bundled_preset_count ? max_presets : bundled_preset_count;
    for (int i = 0; i < count; i++)
        presets[i] = bundled_presets[i];

    return count;
}

bool shader_preset_path_exists(const char* path)
{
    return path_exists(path);
}

bool shader_preset_is_preset_path(const char* path)
{
    return path && ends_with_no_case(path, SHADER_PRESET_EXTENSION);
}

bool shader_preset_get_config_path(const char* preset_path, char* config_path, size_t config_path_size)
{
    if (!config_path || config_path_size == 0)
        return false;

    config_path[0] = '\0';

    if (!preset_path || preset_path[0] == '\0')
        return false;

    const char* filename = get_filename(preset_path);
    if (!filename || filename[0] == '\0')
        return false;

    strncpy_fit(config_path, filename, config_path_size);
    return true;
}

bool shader_preset_resolve_config_path(const char* config_path, char* preset_path, size_t preset_path_size)
{
    if (!preset_path || preset_path_size == 0)
        return false;

    preset_path[0] = '\0';

    if (!config_path || config_path[0] == '\0')
        return false;

    ShaderPresetInfo presets[SHADER_PRESET_MAX_DISCOVERED];
    int count = shader_preset_scan_bundled(presets, SHADER_PRESET_MAX_DISCOVERED);
    for (int i = 0; i < count; i++)
    {
        if (shader_preset_config_path_matches(config_path, presets[i].path))
        {
            strncpy_fit(preset_path, presets[i].path, preset_path_size);
            return true;
        }
    }

    return false;
}

bool shader_preset_config_path_matches(const char* config_path, const char* preset_path)
{
    if (!config_path || config_path[0] == '\0' || !preset_path || preset_path[0] == '\0')
        return false;

    if (strcmp(config_path, preset_path) == 0)
        return true;

    return strcmp(get_filename(config_path), get_filename(preset_path)) == 0;
}

static void clear_preset(ShaderPreset* preset)
{
    memset(preset, 0, sizeof(*preset));
    preset->filter_linear = false;
}

static bool load_ini(const char* path, mINI::INIStructure& ini, char* error, size_t error_size)
{
    if (!shader_preset_path_exists(path))
    {
        set_error_path(error, error_size, "Shader preset not found", path);
        return false;
    }

    mINI::INIFile file(path);
    if (!file.read(ini))
    {
        set_error_path(error, error_size, "Unable to read shader preset", path);
        return false;
    }

    return true;
}

static bool load_passes(const mINI::INIStructure& ini, ShaderPreset* preset, char* error, size_t error_size)
{
    int pass_count = parse_int(ini.get("Preset").get("Passes"), 1);
    pass_count = CLAMP(pass_count, 1, SHADER_PRESET_MAX_PASSES);

    for (int i = 0; i < pass_count; i++)
    {
        ShaderPresetPass* pass = &preset->passes[i];
        memset(pass, 0, sizeof(*pass));

        pass->filter_linear = false;
        pass->feedback = false;
        pass->history = false;
        pass->float_framebuffer = false;
        pass->scale_type_x = ShaderPresetScale_Viewport;
        pass->scale_type_y = ShaderPresetScale_Viewport;
        pass->scale_x = 1.0f;
        pass->scale_y = 1.0f;
        pass->absolute_width = 0;
        pass->absolute_height = 0;

        std::string section = "Pass" + std::to_string(i);
        std::string shader_path = ini.get(section).get("Path");

        if (i == 0 && shader_path.empty())
            shader_path = ini.get("Shader").get("Path");
        if (i == 0 && shader_path.empty())
            shader_path = ini.get("Preset").get("Shader");

        if (shader_path.empty())
        {
            std::string message = section + " does not define Path";
            set_error(error, error_size, message.c_str());
            return false;
        }

        if (!resolve_shader_path(shader_path.c_str(), preset->preset_dir, pass->shader_path, sizeof(pass->shader_path), error, error_size))
            return false;

        std::string filter = ini.get(section).get("Filter");
        if (i == 0 && filter.empty())
            filter = ini.get("Shader").get("Filter");
        pass->filter_linear = parse_bool(filter, false) || filter == "Linear" || filter == "linear" || filter == "Bilinear" || filter == "bilinear";

        pass->feedback = parse_bool(ini.get(section).get("Feedback"), false);
        pass->history = parse_bool(ini.get(section).get("History"), false);
        pass->float_framebuffer = parse_bool(ini.get(section).get("FloatFramebuffer"), false);

        std::string scale_type = ini.get(section).get("ScaleType");
        pass->scale_type_x = parse_scale_type(ini.get(section).get("ScaleTypeX"), parse_scale_type(scale_type, ShaderPresetScale_Viewport));
        pass->scale_type_y = parse_scale_type(ini.get(section).get("ScaleTypeY"), parse_scale_type(scale_type, ShaderPresetScale_Viewport));

        float scale = parse_float(ini.get(section).get("Scale"), 1.0f);
        pass->scale_x = parse_float(ini.get(section).get("ScaleX"), scale);
        pass->scale_y = parse_float(ini.get(section).get("ScaleY"), scale);
        pass->absolute_width = parse_int(ini.get(section).get("AbsoluteWidth"), 0);
        pass->absolute_height = parse_int(ini.get(section).get("AbsoluteHeight"), 0);

        if (pass->scale_x <= 0.0f)
            pass->scale_x = 1.0f;
        if (pass->scale_y <= 0.0f)
            pass->scale_y = 1.0f;
    }

    preset->pass_count = pass_count;
    preset->filter_linear = preset->passes[0].filter_linear;
    strncpy_fit(preset->shader_path, preset->passes[0].shader_path, sizeof(preset->shader_path));
    return true;
}

static void load_parameters(const mINI::INIStructure& ini, ShaderPreset* preset)
{
    mINI::INIMap<std::string> parameters = ini.get("Parameters");

    for (mINI::INIMap<std::string>::const_iterator it = parameters.begin(); it != parameters.end(); ++it)
    {
        if (preset->parameter_count >= SHADER_PRESET_MAX_PARAMETERS)
            break;

        ShaderPresetParameter* parameter = &preset->parameters[preset->parameter_count];
        memset(parameter, 0, sizeof(*parameter));

        strncpy_fit(parameter->name, it->first.c_str(), sizeof(parameter->name));
        strncpy_fit(parameter->label, it->first.c_str(), sizeof(parameter->label));
        parameter->value = parse_float(it->second, 0.0f);
        parameter->minimum = 0.0f;
        parameter->maximum = 1.0f;
        parameter->step = 0.01f;

        std::string section = "Parameter." + it->first;
        std::string label = ini.get(section).get("Label");
        if (!label.empty())
            strncpy_fit(parameter->label, label.c_str(), sizeof(parameter->label));

        std::string min_value = ini.get(section).get("Min");
        std::string max_value = ini.get(section).get("Max");
        std::string step_value = ini.get(section).get("Step");

        parameter->minimum = parse_float(min_value, parameter->minimum);
        parameter->maximum = parse_float(max_value, parameter->maximum);
        parameter->step = parse_float(step_value, parameter->step);

        if (parameter->maximum < parameter->minimum)
        {
            float tmp = parameter->maximum;
            parameter->maximum = parameter->minimum;
            parameter->minimum = tmp;
        }

        parameter->value = CLAMP(parameter->value, parameter->minimum, parameter->maximum);
        parameter->default_value = parameter->value;
        preset->parameter_count++;
    }
}

static bool resolve_shader_path(const char* path, const char* preset_dir, char* out_path, size_t out_path_size, char* error, size_t error_size)
{
    if (is_absolute_path(path))
    {
        if (shader_preset_path_exists(path))
        {
            strncpy_fit(out_path, path, out_path_size);
            return true;
        }

        set_error_path(error, error_size, "Shader file not found", path);
        return false;
    }

    char candidate[SHADER_PRESET_MAX_PATH];
    if (join_path(preset_dir, path, candidate, sizeof(candidate)) && shader_preset_path_exists(candidate))
    {
        strncpy_fit(out_path, candidate, out_path_size);
        return true;
    }

    char resource_root[SHADER_PRESET_MAX_PATH];
    if (shader_preset_get_resource_root(resource_root, sizeof(resource_root)))
    {
        if (join_path(resource_root, path, candidate, sizeof(candidate)) && shader_preset_path_exists(candidate))
        {
            strncpy_fit(out_path, candidate, out_path_size);
            return true;
        }
    }

    set_error_path(error, error_size, "Shader file not found", path);
    return false;
}

static bool get_candidate_resource_root(int index, char* path, size_t path_size)
{
    char base[SHADER_PRESET_MAX_PATH];
    get_executable_path(base, sizeof(base));

    switch (index)
    {
        case 0:
            return join_path(base, "shaders", path, path_size);
        case 1:
            return join_path(base, "../shared/desktop/shaders", path, path_size);
        case 2:
            return join_path(base, "../../platforms/shared/desktop/shaders", path, path_size);
        case 3:
            return join_path(base, "../../shared/desktop/shaders", path, path_size);
        case 4:
            return join_path(".", "shaders", path, path_size);
        case 5:
            return join_path(".", "../shared/desktop/shaders", path, path_size);
        case 6:
            return join_path(".", "platforms/shared/desktop/shaders", path, path_size);
        default:
            return false;
    }
}

static bool scan_directory_for_presets(const char* directory, ShaderPresetInfo* presets, int max_presets, int* count)
{
#if defined(_WIN32)
    char pattern[SHADER_PRESET_MAX_PATH];
    if (!join_path(directory, "*.gshader", pattern, sizeof(pattern)))
        return false;

    WIN32_FIND_DATAA find_data;
    HANDLE handle = FindFirstFileA(pattern, &find_data);
    if (handle == INVALID_HANDLE_VALUE)
        return false;

    do
    {
        if ((find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
        {
            char path[SHADER_PRESET_MAX_PATH];
            if (join_path(directory, find_data.cFileName, path, sizeof(path)))
                add_discovered_preset(path, presets, max_presets, count);
        }
    } while (*count < max_presets && FindNextFileA(handle, &find_data));

    FindClose(handle);
    return true;
#else
    DIR* dir = opendir(directory);
    if (!dir)
        return false;

    struct dirent* entry = NULL;
    while (*count < max_presets && (entry = readdir(dir)) != NULL)
    {
        if (!shader_preset_is_preset_path(entry->d_name))
            continue;

        char path[SHADER_PRESET_MAX_PATH];
        if (join_path(directory, entry->d_name, path, sizeof(path)))
            add_discovered_preset(path, presets, max_presets, count);
    }

    closedir(dir);
    return true;
#endif
}

static bool add_discovered_preset(const char* path, ShaderPresetInfo* presets, int max_presets, int* count)
{
    if (*count >= max_presets)
        return false;

    ShaderPreset preset;
    char error[512];
    if (!shader_preset_load(path, &preset, error, sizeof(error)))
    {
        Debug("Skipping shader preset %s: %s", path, error);
        return false;
    }

    strncpy_fit(presets[*count].name, preset.name, sizeof(presets[*count].name));
    strncpy_fit(presets[*count].path, preset.preset_path, sizeof(presets[*count].path));
    (*count)++;
    return true;
}

static void scan_bundled_presets_once(void)
{
    if (bundled_presets_scanned)
        return;

    bundled_presets_scanned = true;
    bundled_preset_count = 0;

    char root[SHADER_PRESET_MAX_PATH];
    if (!shader_preset_get_resource_root(root, sizeof(root)))
        return;

    scan_directory_for_presets(root, bundled_presets, SHADER_PRESET_MAX_DISCOVERED, &bundled_preset_count);
    sort_discovered_presets(bundled_presets, bundled_preset_count);
}

static void sort_discovered_presets(ShaderPresetInfo* presets, int count)
{
    for (int i = 0; i < count - 1; i++)
    {
        for (int j = i + 1; j < count; j++)
        {
            if (strcmp(presets[i].name, presets[j].name) <= 0)
                continue;

            ShaderPresetInfo tmp = presets[i];
            presets[i] = presets[j];
            presets[j] = tmp;
        }
    }
}

static bool parse_bool(const std::string& value, bool default_value)
{
    if (value.empty())
        return default_value;

    if (value == "1" || value == "true" || value == "True" || value == "yes" || value == "Yes")
        return true;
    if (value == "0" || value == "false" || value == "False" || value == "no" || value == "No")
        return false;

    return default_value;
}

static int parse_int(const std::string& value, int default_value)
{
    if (value.empty())
        return default_value;

    char* end = NULL;
    long result = strtol(value.c_str(), &end, 10);
    if (end == value.c_str())
        return default_value;

    return (int)result;
}

static float parse_float(const std::string& value, float default_value)
{
    if (value.empty())
        return default_value;

    char* end = NULL;
    float result = strtof(value.c_str(), &end);
    if (end == value.c_str())
        return default_value;

    return result;
}

static ShaderPresetScaleType parse_scale_type(const std::string& value, ShaderPresetScaleType default_value)
{
    if (value.empty())
        return default_value;

    if (value == "Source" || value == "source")
        return ShaderPresetScale_Source;
    if (value == "Viewport" || value == "viewport" || value == "FinalViewport" || value == "final_viewport")
        return ShaderPresetScale_Viewport;
    if (value == "Previous" || value == "previous")
        return ShaderPresetScale_Previous;
    if (value == "Absolute" || value == "absolute")
        return ShaderPresetScale_Absolute;

    return default_value;
}

static void set_error(char* error, size_t error_size, const char* message)
{
    if (!error || error_size == 0)
        return;

    strncpy_fit(error, message ? message : "Unknown shader preset error", error_size);
}

static void set_error_path(char* error, size_t error_size, const char* prefix, const char* path)
{
    if (!error || error_size == 0)
        return;

    snprintf(error, error_size, "%s: %s", prefix ? prefix : "Shader preset error", path ? path : "");
    error[error_size - 1] = '\0';
}
