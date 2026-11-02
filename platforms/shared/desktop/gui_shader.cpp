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

#define GUI_SHADER_IMPORT
#include "gui_shader.h"

#include <math.h>
#include <stdio.h>
#include <string.h>
#include "imgui.h"
#include "gearboy.h"
#include "config.h"
#include "gui_colors.h"
#include "gui_notifications.h"
#include "ogl_renderer.h"
#include "ogl_shader_chain.h"
#include "shader_preset.h"

static bool show_parameters = false;
static bool focus_parameters = false;
static int center_parameters = 0;
static char parameters_preset_path[SHADER_PRESET_MAX_PATH] = "";

static void menu_preset(const ShaderPresetInfo* info, bool selected);
static void load_preset(const ShaderPresetInfo* info, int variant);
static bool is_active_preset(const ShaderPresetInfo* info);
static void draw_variant_combo(float width);
static void draw_parameters(float label_width, float value_x, float value_width);
static void draw_parameter(int index, const ShaderPresetParameter* parameter, float label_width, float value_x, float value_width);
static void draw_parameter_tooltip(const ShaderPresetParameter* parameter, bool slider);
static float get_label_width(bool include_preset_name);
static const char* get_parameter_label(const ShaderPresetParameter* parameter);
static const char* get_parameter_format(const ShaderPresetParameter* parameter);
static int get_decimals(float value);
static bool parameter_is_toggle(const ShaderPresetParameter* parameter);
static bool parameter_is_integer(const ShaderPresetParameter* parameter);
static int parameter_round_to_int(float value);

void gui_shader_menu(void)
{
    if (!ImGui::BeginMenu("Shader"))
        return;

    bool has_preset = ogl_shader_chain_has_preset() && config_video.shader_mode == config_ShaderMode_External;

    if (ImGui::MenuItem("Pixel Perfect", NULL, !has_preset) && has_preset)
    {
        ogl_renderer_unload_shader_preset();
        gui_notify(gui_NotificationInfo, ICON_MD_TUNE, "Shader preset loaded", "Pixel Perfect", "shader");
    }

    ImGui::Separator();

    int count = shader_preset_get_bundled_count();

    for (int i = 0; i < count; i++)
    {
        const ShaderPresetInfo* info = shader_preset_get_bundled(i);

        if (info->group[0] == '\0')
        {
            menu_preset(info, has_preset && is_active_preset(info));
            continue;
        }

        int last = i;
        while (last + 1 < count && strcmp(shader_preset_get_bundled(last + 1)->group, info->group) == 0)
            last++;

        if (ImGui::BeginMenu(info->group))
        {
            for (int j = i; j <= last; j++)
            {
                const ShaderPresetInfo* item = shader_preset_get_bundled(j);
                menu_preset(item, has_preset && is_active_preset(item));
            }

            ImGui::EndMenu();
        }

        i = last;
    }

    ImGui::Separator();

    bool has_parameters = has_preset && ogl_shader_chain_get_parameter_count() > 0;

    if (ImGui::MenuItem("Parameters...", NULL, show_parameters && has_parameters, has_parameters))
    {
        show_parameters = !show_parameters;
        focus_parameters = show_parameters;
    }

    if (!has_preset && ogl_shader_chain_get_last_error()[0] != '\0')
    {
        ImGui::Separator();
        ImGui::TextColored(red, "%s", ogl_shader_chain_get_last_error());
    }

    ImGui::EndMenu();
}

void gui_shader_parameters_window(void)
{
    if (!show_parameters)
        return;

    if (!ogl_shader_chain_has_preset() || config_video.shader_mode != config_ShaderMode_External ||
        ogl_shader_chain_get_parameter_count() <= 0)
    {
        show_parameters = false;
        return;
    }

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    const char* preset_path = ogl_shader_chain_get_preset_path();

    if (focus_parameters || strcmp(preset_path, parameters_preset_path) != 0)
    {
        snprintf(parameters_preset_path, sizeof(parameters_preset_path), "%s", preset_path);
        center_parameters = 4;
    }

    if (center_parameters > 0)
    {
        ImGui::SetNextWindowPos(viewport->GetWorkCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        center_parameters--;
    }

    if (focus_parameters)
    {
        ImGui::SetNextWindowFocus();
        focus_parameters = false;
    }

    if (!ImGui::Begin("Shader Parameters", &show_parameters, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDocking))
    {
        ImGui::End();
        return;
    }

    ImGuiStyle& style = ImGui::GetStyle();
    bool has_variants = ogl_shader_chain_get_variant_count() > 0;
    float label_width = get_label_width(has_variants);
    float value_x = label_width + (style.ItemSpacing.x * 2.0f);
    float value_width = ImGui::GetFontSize() * 12.0f;
    float start_x = ImGui::GetCursorPosX();
    const char* preset_name = ogl_shader_chain_get_preset_name();

    if (has_variants)
    {
        ImGui::AlignTextToFramePadding();
        ImGui::TextColored(cyan, "%s", preset_name);

        if (ImGui::CalcTextSize(preset_name).x + style.ItemSpacing.x <= value_x)
            ImGui::SameLine(start_x + value_x);
        else
            ImGui::SetCursorPosX(start_x + value_x);

        draw_variant_combo(value_width);
    }
    else
        ImGui::TextColored(cyan, "%s", preset_name);

    ImGui::Separator();

    ImGui::SetNextWindowSizeConstraints(ImVec2(0.0f, 0.0f), ImVec2(FLT_MAX, viewport->WorkSize.y * 0.6f));
    ImGui::BeginChild("##shader_parameters", ImVec2(0.0f, 0.0f), ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);

    draw_parameters(label_width, value_x, value_width);

    float scrollbar_margin = ImGui::GetScrollMaxY() > 0.0f ? style.ItemSpacing.x : 0.0f;
    ImGui::Dummy(ImVec2(value_x + value_width + scrollbar_margin, 0.0f));
    ImGui::EndChild();

    ImGui::Separator();

    if (ImGui::Button("Restore Defaults"))
    {
        if (ogl_shader_chain_restore_default_parameters())
        {
            ogl_renderer_save_shader_parameter_config();
            gui_notify(gui_NotificationSuccess, ICON_MD_TUNE, "Shader parameters restored");
        }
    }

    ImGui::End();
}

static void menu_preset(const ShaderPresetInfo* info, bool selected)
{
    if (info->variant_count <= 0)
    {
        if (ImGui::MenuItem(info->name, NULL, selected) && !selected)
            load_preset(info, -1);

        return;
    }

    if (!ImGui::BeginMenu(info->name))
        return;

    int active_variant = selected ? ogl_shader_chain_get_variant() : -1;

    for (int i = 0; i < info->variant_count; i++)
    {
        if (ImGui::MenuItem(info->variants[i], NULL, i == active_variant) && i != active_variant)
            load_preset(info, i);
    }

    ImGui::EndMenu();
}

static void load_preset(const ShaderPresetInfo* info, int variant)
{
    if (!is_active_preset(info) || !ogl_shader_chain_has_preset() || config_video.shader_mode != config_ShaderMode_External)
    {
        if (!ogl_renderer_load_shader_preset(info->path))
        {
            gui_notify(gui_NotificationError, NULL, "Unable to load shader preset", ogl_shader_chain_get_last_error(), "shader");
            return;
        }
    }

    if (variant < 0)
    {
        gui_notify(gui_NotificationInfo, ICON_MD_TUNE, "Shader preset loaded", ogl_shader_chain_get_preset_name(), "shader");
        return;
    }

    ogl_shader_chain_set_variant(variant);
    ogl_renderer_save_shader_parameter_config();

    char detail[SHADER_PRESET_MAX_NAME + SHADER_PRESET_MAX_PARAMETER_NAME + 4];
    snprintf(detail, sizeof(detail), "%s (%s)", ogl_shader_chain_get_preset_name(), ogl_shader_chain_get_variant_name(variant));
    gui_notify(gui_NotificationInfo, ICON_MD_TUNE, "Shader preset loaded", detail, "shader");
}

static bool is_active_preset(const ShaderPresetInfo* info)
{
    return shader_preset_config_path_matches(config_video.shader_preset_path.c_str(), info->path);
}

static void draw_variant_combo(float width)
{
    int count = ogl_shader_chain_get_variant_count();
    int variant = ogl_shader_chain_get_variant();

    ImGui::SetNextItemWidth(width);

    if (!ImGui::BeginCombo("##shader_variant", ogl_shader_chain_get_variant_name(variant), ImGuiComboFlags_HeightLarge))
        return;

    for (int i = 0; i < count; i++)
    {
        bool selected = i == variant;

        if (ImGui::Selectable(ogl_shader_chain_get_variant_name(i), selected) && !selected)
        {
            ogl_shader_chain_set_variant(i);
            ogl_renderer_save_shader_parameter_config();
        }

        if (selected)
            ImGui::SetItemDefaultFocus();
    }

    ImGui::EndCombo();
}

static void draw_parameters(float label_width, float value_x, float value_width)
{
    int count = ogl_shader_chain_get_parameter_count();
    int i = 0;

    while (i < count)
    {
        const char* group = ogl_shader_chain_get_parameter(i)->group;
        int last = i;

        while (last + 1 < count && strcmp(ogl_shader_chain_get_parameter(last + 1)->group, group) == 0)
            last++;

        bool open = true;

        if (group[0] != '\0')
        {
            ImGui::PushID(i);
            open = ImGui::CollapsingHeader(group);
            ImGui::PopID();
        }

        if (open)
        {
            for (int j = i; j <= last; j++)
                draw_parameter(j, ogl_shader_chain_get_parameter(j), label_width, value_x, value_width);

            if (group[0] != '\0' && last + 1 < count)
                ImGui::Spacing();
        }

        i = last + 1;
    }
}

static void draw_parameter(int index, const ShaderPresetParameter* parameter, float label_width, float value_x, float value_width)
{
    float x = ImGui::GetCursorPosX();
    float value = parameter->value;

    ImGui::PushID(index);
    ImGui::AlignTextToFramePadding();
    ImGui::PushTextWrapPos(x + label_width);
    ImGui::TextUnformatted(get_parameter_label(parameter));
    ImGui::PopTextWrapPos();
    ImGui::SameLine(x + value_x);
    ImGui::SetNextItemWidth(value_width);

    if (parameter_is_toggle(parameter))
    {
        bool enabled = value >= 0.5f;

        if (ImGui::Checkbox("##value", &enabled))
        {
            ogl_shader_chain_set_parameter(index, enabled ? 1.0f : 0.0f);
            ogl_renderer_save_shader_parameter_config();
        }

        draw_parameter_tooltip(parameter, false);
    }
    else if (parameter_is_integer(parameter))
    {
        int int_value = parameter_round_to_int(value);
        int min_value = parameter_round_to_int(parameter->minimum);
        int max_value = parameter_round_to_int(parameter->maximum);

        if (ImGui::SliderInt("##value", &int_value, min_value, max_value))
        {
            ogl_shader_chain_set_parameter(index, (float)int_value);
            ogl_renderer_save_shader_parameter_config();
        }

        draw_parameter_tooltip(parameter, true);
    }
    else
    {
        if (ImGui::SliderFloat("##value", &value, parameter->minimum, parameter->maximum, get_parameter_format(parameter)))
        {
            ogl_shader_chain_set_parameter(index, value);
            ogl_renderer_save_shader_parameter_config();
        }

        draw_parameter_tooltip(parameter, true);
    }

    ImGui::PopID();
}

static void draw_parameter_tooltip(const ShaderPresetParameter* parameter, bool slider)
{
    if (!ImGui::IsItemHovered() || ImGui::IsItemActive())
        return;

    if (!slider)
    {
        ImGui::SetTooltip("%s", parameter->name);
        return;
    }

#if defined(__APPLE__)
    ImGui::SetTooltip("%s\nCmd+Click to enter a value", parameter->name);
#else
    ImGui::SetTooltip("%s\nCtrl+Click to enter a value", parameter->name);
#endif
}

static float get_label_width(bool include_preset_name)
{
    float width = include_preset_name ? ImGui::CalcTextSize(ogl_shader_chain_get_preset_name()).x : 0.0f;
    int count = ogl_shader_chain_get_parameter_count();

    for (int i = 0; i < count; i++)
    {
        float label_width = ImGui::CalcTextSize(get_parameter_label(ogl_shader_chain_get_parameter(i))).x;

        if (label_width > width)
            width = label_width;
    }

    float max_width = ImGui::GetFontSize() * 20.0f;
    return width < max_width ? width : max_width;
}

static const char* get_parameter_label(const ShaderPresetParameter* parameter)
{
    return parameter->label[0] != '\0' ? parameter->label : parameter->name;
}

static const char* get_parameter_format(const ShaderPresetParameter* parameter)
{
    static const char* formats[] = { "%.0f", "%.1f", "%.2f", "%.3f", "%.4f" };

    if (parameter->step <= 0.0f)
        return formats[3];

    int decimals = get_decimals(parameter->step);
    int value_decimals = get_decimals(parameter->value);

    return formats[value_decimals > decimals ? value_decimals : decimals];
}

static int get_decimals(float value)
{
    float scaled = fabsf(value);
    int decimals = 0;

    while (decimals < 4 && fabsf(scaled - floorf(scaled + 0.5f)) > 0.0005f)
    {
        scaled *= 10.0f;
        decimals++;
    }

    return decimals;
}

static bool parameter_is_toggle(const ShaderPresetParameter* parameter)
{
    return parameter->minimum == 0.0f && parameter->maximum == 1.0f && parameter->step >= 1.0f;
}

static bool parameter_is_integer(const ShaderPresetParameter* parameter)
{
    return parameter->step >= 1.0f;
}

static int parameter_round_to_int(float value)
{
    return value >= 0.0f ? (int)(value + 0.5f) : (int)(value - 0.5f);
}
