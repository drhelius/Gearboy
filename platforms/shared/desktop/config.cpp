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

#include <SDL3/SDL.h>
#include <climits>
#include <cmath>
#include <iomanip>
#include <string>

#define CONFIG_IMPORT
#include "config.h"

#define MINI_CASE_SENSITIVE
#include "ini.h"

#include "utils.h"

static mINI::INIFile* config_ini_file = NULL;
static mINI::INIStructure config_ini_data;

static void process_bool(config_Operation operation, const char* section, const char* key,
                         bool* value, bool default_value);
static void process_int(config_Operation operation, const char* section, const char* key,
                        int* value, int default_value, bool has_minimum, int minimum,
                        bool has_maximum, int maximum);
static void process_float(config_Operation operation, const char* section, const char* key,
                          float* value, float default_value, bool has_minimum, float minimum,
                          bool has_maximum, float maximum);
static void process_string(config_Operation operation, const char* section, const char* key,
                           std::string* value, const char* default_value, bool allow_empty);
static void process_scancode(config_Operation operation, const char* section, const char* key,
                             SDL_Scancode* value, SDL_Scancode default_value);
static void process_int_array(config_Operation operation, const char* section,
                              const char* key_format, int* values, int count,
                              int default_value, bool has_minimum, int minimum,
                              bool has_maximum, int maximum);
static void process_float_array(config_Operation operation, const char* section,
                                const char* key_format, float* values, int count,
                                float default_value, bool has_minimum, float minimum,
                                bool has_maximum, float maximum);
static void process_string_array(config_Operation operation, const char* section,
                                 const char* key_format, std::string* values, int count,
                                 const char* default_value, bool allow_empty);
static void process_hotkey(config_Operation operation, const char* key, config_Hotkey* value,
                           SDL_Scancode default_key, SDL_Keymod default_mod);
static void on_config_defaults(void);
static void on_config_read(int file_version);
static void on_config_write(void);
static char* get_portable_path(bool force_portable);
static bool check_portable(const char* base_path);
static bool get_setting(const char* group, const char* key, std::string* value);
static bool parse_int_string(const std::string& value, int* result);
static bool parse_float_string(const std::string& value, float* result);
static bool parse_bool_string(const std::string& value, bool* result);
static int read_int(const char* group, const char* key, int default_value);
static void write_int(const char* group, const char* key, int integer);
static void write_float(const char* group, const char* key, float value);
static bool read_bool(const char* group, const char* key, bool default_value);
static void write_bool(const char* group, const char* key, bool boolean);
static void write_string(const char* group, const char* key, const std::string& value);
static std::string shader_preset_section_name(const char* preset_file);

#include "config_definitions.inc.h"

void config_init(bool force_portable)
{
    UNUSED(&process_float_array);

    const char* root_path = NULL;
    char* portable_path = get_portable_path(force_portable);

    if (portable_path)
        root_path = portable_path;
    else
        root_path = SDL_GetPrefPath("Geardome", GEARBOY_TITLE);

    if (root_path == NULL)
    {
        Log("Unable to determine config path. Falling back to current directory.");
        root_path = SDL_strdup("./");
    }

    config_root_path = root_path;

    strncpy_fit(config_emu_file_path, config_root_path, sizeof(config_emu_file_path));
    strncat_fit(config_emu_file_path, "config.ini", sizeof(config_emu_file_path));

    strncpy_fit(config_imgui_file_path, config_root_path, sizeof(config_imgui_file_path));
    strncat_fit(config_imgui_file_path, "imgui.ini", sizeof(config_imgui_file_path));

    on_config_defaults();

    config_ini_file = new mINI::INIFile(config_emu_file_path);
}

void config_destroy(void)
{
    SafeDelete(config_ini_file);
    SDL_free((void*)config_root_path);
}

void config_load_defaults(void)
{
    Log("Loading default settings");

    on_config_defaults();
    config_write();
}

void config_push_recent_media(const std::string& path)
{
    if (path.empty())
        return;

    int slot = 0;
    for (slot = 0; slot < config_max_recent_roms; slot++)
    {
        if (config_emulator.recent_roms[slot].compare(path) == 0)
            break;
    }

    if (slot >= config_max_recent_roms)
        slot = config_max_recent_roms - 1;

    for (int index = slot; index > 0; index--)
    {
        config_emulator.recent_roms[index] = config_emulator.recent_roms[index - 1];
    }

    config_emulator.recent_roms[0] = path;
}

void config_read(void)
{
    if (!config_ini_file->read(config_ini_data))
    {
        Log("Unable to load settings from %s", config_emu_file_path);
        return;
    }

    int file_version = read_int("General", "Version", 0);

    if (file_version < config_minimum_version)
    {
        Log("Settings version %d is outdated (current: %d). Using defaults.", file_version, config_version);
        config_write();
        return;
    }

    if (file_version < config_version)
        Log("Migrating settings version %d to %d", file_version, config_version);

    Log("Loading settings from %s (version %d)", config_emu_file_path, file_version);

    on_config_read(file_version);

    Debug("Settings loaded");
}

void config_write(void)
{
    Log("Saving settings to %s", config_emu_file_path);

    on_config_write();

    if (config_ini_file->write(config_ini_data, true))
    {
        Debug("Settings saved");
    }
    else
    {
        Error("Unable to save settings to %s", config_emu_file_path);
    }
}

static void process_bool(config_Operation operation, const char* section, const char* key,
                         bool* value, bool default_value)
{
    bool processed = default_value;

    if (operation == config_Operation_Read)
    {
        std::string stored;
        if (!get_setting(section, key, &stored) || !parse_bool_string(stored, &processed))
            processed = default_value;
    }
    else if (operation == config_Operation_Write)
    {
        processed = *value;
    }

    *value = processed;

    if (operation == config_Operation_Write)
        write_bool(section, key, processed);
    else if (operation == config_Operation_Read)
    {
        Debug("Load bool setting: [%s][%s]=%s", section, key, processed ? "true" : "false");
    }
}

static void process_int(config_Operation operation, const char* section, const char* key,
                        int* value, int default_value, bool has_minimum, int minimum,
                        bool has_maximum, int maximum)
{
    if (has_minimum && has_maximum && minimum > maximum)
        Error("Invalid integer setting range: [%s][%s]=%d..%d", section, key, minimum, maximum);
    if ((has_minimum && default_value < minimum) || (has_maximum && default_value > maximum))
        Error("Invalid integer setting default: [%s][%s]=%d", section, key, default_value);

    int processed = default_value;

    if (operation == config_Operation_Read)
    {
        std::string stored;
        if (!get_setting(section, key, &stored) || !parse_int_string(stored, &processed))
            processed = default_value;
    }
    else if (operation == config_Operation_Write)
    {
        processed = *value;
    }

    if (has_minimum && processed < minimum)
        processed = minimum;
    if (has_maximum && processed > maximum)
        processed = maximum;

    *value = processed;

    if (operation == config_Operation_Write)
        write_int(section, key, processed);
    else if (operation == config_Operation_Read)
    {
        Debug("Load integer setting: [%s][%s]=%d", section, key, processed);
    }
}

static void process_float(config_Operation operation, const char* section, const char* key,
                          float* value, float default_value, bool has_minimum, float minimum,
                          bool has_maximum, float maximum)
{
    if ((has_minimum && !std::isfinite(minimum)) || (has_maximum && !std::isfinite(maximum)) ||
        (has_minimum && has_maximum && minimum > maximum))
    {
        Error("Invalid float setting range: [%s][%s]", section, key);
    }
    if (!std::isfinite(default_value) || (has_minimum && default_value < minimum) ||
        (has_maximum && default_value > maximum))
    {
        Error("Invalid float setting default: [%s][%s]=%f", section, key, default_value);
    }

    float processed = default_value;

    if (operation == config_Operation_Read)
    {
        std::string stored;
        if (!get_setting(section, key, &stored) || !parse_float_string(stored, &processed))
            processed = default_value;
    }
    else if (operation == config_Operation_Write)
    {
        processed = *value;
        if (!std::isfinite(processed))
            processed = default_value;
    }

    if (has_minimum && processed < minimum)
        processed = minimum;
    if (has_maximum && processed > maximum)
        processed = maximum;

    *value = processed;

    if (operation == config_Operation_Write)
        write_float(section, key, processed);
    else if (operation == config_Operation_Read)
    {
        Debug("Load float setting: [%s][%s]=%.2f", section, key, processed);
    }
}

static void process_string(config_Operation operation, const char* section, const char* key,
                           std::string* value, const char* default_value, bool allow_empty)
{
    const char* default_string = default_value ? default_value : "";
    if (!allow_empty && default_string[0] == '\0')
        Error("Invalid empty string setting default: [%s][%s]", section, key);

    std::string processed = default_string;

    if (operation == config_Operation_Read)
    {
        if (!get_setting(section, key, &processed))
            processed = default_string;
    }
    else if (operation == config_Operation_Write)
    {
        processed = *value;
    }

    if (!allow_empty && processed.empty())
        processed = default_string;

    *value = processed;

    if (operation == config_Operation_Write)
        write_string(section, key, processed);
    else if (operation == config_Operation_Read)
    {
        Debug("Load string setting: [%s][%s]=%s", section, key, processed.c_str());
    }
}

static void process_scancode(config_Operation operation, const char* section, const char* key,
                             SDL_Scancode* value, SDL_Scancode default_value)
{
    int processed = operation == config_Operation_Defaults ? (int)default_value : (int)*value;
    process_int(operation, section, key, &processed, (int)default_value,
                       false, 0, false, 0);
    *value = (SDL_Scancode)processed;
}

static void process_int_array(config_Operation operation, const char* section,
                              const char* key_format, int* values, int count,
                              int default_value, bool has_minimum, int minimum,
                              bool has_maximum, int maximum)
{
    for (int i = 0; i < count; i++)
    {
        char key[64];
        snprintf(key, sizeof(key), key_format, i);
        process_int(operation, section, key, &values[i], default_value,
                           has_minimum, minimum, has_maximum, maximum);
    }
}

static void process_float_array(config_Operation operation, const char* section,
                                const char* key_format, float* values, int count,
                                float default_value, bool has_minimum, float minimum,
                                bool has_maximum, float maximum)
{
    for (int i = 0; i < count; i++)
    {
        char key[64];
        snprintf(key, sizeof(key), key_format, i);
        process_float(operation, section, key, &values[i], default_value,
                             has_minimum, minimum, has_maximum, maximum);
    }
}

static void process_string_array(config_Operation operation, const char* section,
                                 const char* key_format, std::string* values, int count,
                                 const char* default_value, bool allow_empty)
{
    for (int i = 0; i < count; i++)
    {
        char key[64];
        snprintf(key, sizeof(key), key_format, i);
        process_string(operation, section, key, &values[i], default_value, allow_empty);
    }
}

static void process_hotkey(config_Operation operation, const char* key, config_Hotkey* value,
                           SDL_Scancode default_key, SDL_Keymod default_mod)
{
    char scancode_key[64];
    char mod_key[64];
    snprintf(scancode_key, sizeof(scancode_key), "%sScancode", key);
    snprintf(mod_key, sizeof(mod_key), "%sMod", key);

    int scancode = operation == config_Operation_Defaults ? (int)default_key : (int)value->key;
    int mod = operation == config_Operation_Defaults ? (int)default_mod : (int)value->mod;
    process_int(operation, "Hotkeys", scancode_key, &scancode, (int)default_key,
                       false, 0, false, 0);
    process_int(operation, "Hotkeys", mod_key, &mod, (int)default_mod,
                       false, 0, false, 0);
    value->key = (SDL_Scancode)scancode;
    value->mod = (SDL_Keymod)mod;
    config_update_hotkey_string(value);
}

static void on_config_defaults(void)
{
    before_defaults();
    process(config_Operation_Defaults);
    normalize();
    after_defaults();
}

static void on_config_read(int file_version)
{
    before_read(file_version);
    process(config_Operation_Read);
    normalize();
    after_read(file_version);
}

static void on_config_write(void)
{
    before_write();
    normalize();
    write_int("General", "Version", config_version);
    process(config_Operation_Write);
    after_write();
}

static char* get_portable_path(bool force_portable)
{
    const char* base_path = SDL_GetBasePath();
    if (base_path == NULL)
        return NULL;

#if defined(__APPLE__)
    std::string app_path = base_path;
    const std::string app_contents = ".app/Contents/";
    size_t app_contents_pos = app_path.rfind(app_contents);

    if (app_contents_pos != std::string::npos)
    {
        size_t app_dir_pos = app_path.rfind('/', app_contents_pos);

        if (app_dir_pos != std::string::npos)
        {
            std::string portable_path = app_path.substr(0, app_dir_pos + 1);

            if (force_portable || check_portable(portable_path.c_str()))
                return SDL_strdup(portable_path.c_str());
        }
    }
#endif

    if (force_portable || check_portable(base_path))
        return SDL_strdup(base_path);

    return NULL;
}

static bool check_portable(const char* base_path)
{
    char portable_file_path[512];

    if (base_path == NULL)
        return false;

    if (snprintf(portable_file_path, sizeof(portable_file_path), "%sportable.ini", base_path) >= (int)sizeof(portable_file_path))
        return false;

    FILE* file = fopen_utf8(portable_file_path, "r");

    if (IsValidPointer(file))
    {
        fclose(file);
        return true;
    }

    return false;
}

static bool get_setting(const char* group, const char* key, std::string* value)
{
    if (!value || !config_ini_data.has(group))
        return false;

    mINI::INIMap<std::string> section = config_ini_data.get(group);
    if (!section.has(key))
        return false;

    *value = section.get(key);
    return true;
}

static bool parse_int_string(const std::string& value, int* result)
{
    if (!result)
        return false;

    std::istringstream converter(value);
    converter.imbue(std::locale::classic());
    long long parsed = 0;

    converter >> std::ws;
    if (!(converter >> parsed))
        return false;
    converter >> std::ws;

    if (!converter.eof() || parsed < INT_MIN || parsed > INT_MAX)
        return false;

    *result = (int)parsed;
    return true;
}

static bool parse_float_string(const std::string& value, float* result)
{
    if (!result)
        return false;

    std::istringstream converter(value);
    converter.imbue(std::locale::classic());
    float parsed = 0.0f;

    converter >> std::ws;
    if (!(converter >> parsed))
        return false;
    converter >> std::ws;

    if (!converter.eof() || !std::isfinite(parsed))
        return false;

    *result = parsed;
    return true;
}

static bool parse_bool_string(const std::string& value, bool* result)
{
    if (!result)
        return false;

    std::istringstream converter(value);
    converter.imbue(std::locale::classic());
    bool parsed = false;

    converter >> std::ws;
    if (!(converter >> std::boolalpha >> parsed))
        return false;
    converter >> std::ws;

    if (!converter.eof())
        return false;

    *result = parsed;
    return true;
}

static int read_int(const char* group, const char* key, int default_value)
{
    int ret = default_value;
    std::string value;

    if (!get_setting(group, key, &value) || !parse_int_string(value, &ret))
        ret = default_value;

    Debug("Load integer setting: [%s][%s]=%d", group, key, ret);
    return ret;
}

static void write_int(const char* group, const char* key, int integer)
{
    std::string value = std::to_string(integer);
    config_ini_data[group][key] = value;
    Debug("Save integer setting: [%s][%s]=%s", group, key, value.c_str());
}

static void write_float(const char* group, const char* key, float value)
{
    std::ostringstream oss;
    oss.imbue(std::locale::classic());
    oss << std::fixed << std::setprecision(2) << value;
    std::string value_str = oss.str();
    config_ini_data[group][key] = value_str;
    Debug("Save float setting: [%s][%s]=%s", group, key, value_str.c_str());
}

static bool read_bool(const char* group, const char* key, bool default_value)
{
    bool ret = default_value;
    std::string value;

    if (!get_setting(group, key, &value) || !parse_bool_string(value, &ret))
        ret = default_value;

    Debug("Load bool setting: [%s][%s]=%s", group, key, ret ? "true" : "false");
    return ret;
}

static void write_bool(const char* group, const char* key, bool boolean)
{
    std::stringstream converter;
    converter << std::boolalpha << boolean;
    std::string value;
    value = converter.str();
    config_ini_data[group][key] = value;
    Debug("Save bool setting: [%s][%s]=%s", group, key, value.c_str());
}

static void write_string(const char* group, const char* key, const std::string& value)
{
    config_ini_data[group][key] = value;
    Debug("Save string setting: [%s][%s]=%s", group, key, value.c_str());
}

static std::string shader_preset_section_name(const char* preset_file)
{
    return std::string("ShaderPreset.") + get_filename(preset_file);
}

bool config_read_shader_parameter(const char* preset_file, const char* parameter_name, float* value)
{
    if (!preset_file || preset_file[0] == '\0' || !parameter_name || parameter_name[0] == '\0' || !value)
        return false;

    std::string section = shader_preset_section_name(preset_file);
    if (!config_ini_data.has(section))
        return false;

    mINI::INIMap<std::string> parameters = config_ini_data.get(section);
    if (!parameters.has(parameter_name))
        return false;

    return parse_float_string(parameters.get(parameter_name), value);
}

void config_write_shader_parameter(const char* preset_file, const char* parameter_name, float value)
{
    if (!preset_file || preset_file[0] == '\0' || !parameter_name || parameter_name[0] == '\0')
        return;

    std::string section = shader_preset_section_name(preset_file);
    write_float(section.c_str(), parameter_name, value);
}

void config_update_hotkey_string(config_Hotkey* hotkey)
{
    if (hotkey->key == SDL_SCANCODE_UNKNOWN)
    {
        strcpy(hotkey->str, "");
        return;
    }

    std::string result = "";

    if (hotkey->mod & (SDL_KMOD_CTRL | SDL_KMOD_LCTRL | SDL_KMOD_RCTRL))
        result += "Ctrl+";
    if (hotkey->mod & (SDL_KMOD_SHIFT | SDL_KMOD_LSHIFT | SDL_KMOD_RSHIFT))
        result += "Shift+";
    if (hotkey->mod & (SDL_KMOD_ALT | SDL_KMOD_LALT | SDL_KMOD_RALT))
        result += "Alt+";
    if (hotkey->mod & (SDL_KMOD_GUI | SDL_KMOD_LGUI | SDL_KMOD_RGUI))
        result += "Cmd+";

    const char* key_name = SDL_GetScancodeName(hotkey->key);
    if (key_name && strlen(key_name) > 0)
        result += key_name;
    else
        result += "Unknown";

    strncpy(hotkey->str, result.c_str(), sizeof(hotkey->str) - 1);
    hotkey->str[sizeof(hotkey->str) - 1] = '\0';
}
