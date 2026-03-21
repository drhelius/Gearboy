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

#ifndef GUI_DEBUG_WIDGETS_H
#define GUI_DEBUG_WIDGETS_H

#include "imgui.h"
#include "gearboy.h"
#include "common.h"
#include "gui_debug_constants.h"
#include "utils.h"
#include <cstring>
#include <cmath>

typedef void (*RegisterWriteCallback1)(u16 address, u8 bit_index, bool value, void* user_data);
typedef void (*RegisterWriteCallback8)(u16 address, u8 value, void* user_data);
typedef void (*RegisterWriteCallback16)(u16 address, u16 value, void* user_data);

enum EditableRegisterFlags
{
    EditableRegisterFlags_None       = 0,
    EditableRegisterFlags_ShowAddr   = 1 << 0,
    EditableRegisterFlags_ShowName   = 1 << 1,
    EditableRegisterFlags_ShowBinary = 1 << 2,
    EditableRegisterFlags_Default    = EditableRegisterFlags_ShowAddr | EditableRegisterFlags_ShowName | EditableRegisterFlags_ShowBinary
};

inline bool EditableRegister1(
    u16 address,
    u8 bit_index,
    bool current_value,
    RegisterWriteCallback1 write_callback,
    void* user_data,
    ImVec4 true_color = green,
    ImVec4 false_color = white)
{
    bool modified = false;

    ImGui::PushID((address << 8) | bit_index);

    const char* bit_str = current_value ? "1" : "0";
    ImVec4 color = current_value ? true_color : false_color;

    if (write_callback != nullptr)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, color);
        if (ImGui::Selectable(bit_str, false, ImGuiSelectableFlags_None, ImGui::CalcTextSize(bit_str)))
        {
            write_callback(address, bit_index, !current_value, user_data);
            modified = true;
        }
        ImGui::PopStyleColor();
    }
    else
    {
        ImGui::TextColored(color, "%s", bit_str);
    }

    ImGui::PopID();
    return modified;
}

inline bool EditableRegister8(
    const char* label,
    const char* addr_str,
    u16 address,
    u8 current_value,
    RegisterWriteCallback8 write_callback,
    void* user_data,
    EditableRegisterFlags flags = EditableRegisterFlags_Default,
    ImVec4 name_color = orange,
    ImVec4 addr_color = cyan)
{
    static ImGuiID editing_id = 0;
    static int frames_editing = 0;
    static char edit_buffer[8] = {0};
    bool modified = false;

    ImGui::PushID(address);
    ImGuiID widget_id = ImGui::GetID("##edit");

    if ((flags & EditableRegisterFlags_ShowAddr) && addr_str)
    {
        ImGui::TextColored(addr_color, "%s ", addr_str);
        ImGui::SameLine();
    }
    if ((flags & EditableRegisterFlags_ShowName) && label)
    {
        ImGui::TextColored(name_color, "%s ", label);
        ImGui::SameLine();
    }

    if (editing_id == widget_id)
    {
        float text_height = ImGui::GetTextLineHeight();
        float frame_height = ImGui::GetFrameHeight();
        float padding_reduction = (frame_height - text_height) * 0.5f;
        ImVec2 original_padding = ImGui::GetStyle().FramePadding;
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(original_padding.x, original_padding.y - padding_reduction));
        ImGui::PushItemWidth(ImGui::CalcTextSize("FF").x + 6);
        ImGuiInputTextFlags input_flags = ImGuiInputTextFlags_CharsHexadecimal |
                                          ImGuiInputTextFlags_CharsUppercase |
                                          ImGuiInputTextFlags_EnterReturnsTrue |
                                          ImGuiInputTextFlags_AutoSelectAll;

        if (frames_editing == 0)
        {
            ImGui::SetKeyboardFocusHere();
        }
        bool enter_pressed = ImGui::InputText("##edit", edit_buffer, sizeof(edit_buffer), input_flags);
        bool lost_focus = (frames_editing > 1) && !ImGui::IsItemActive();
        frames_editing++;

        if (enter_pressed)
        {
            u8 new_value = 0;
            if (parse_hex_string(edit_buffer, strlen(edit_buffer), &new_value))
            {
            if (write_callback)
                    write_callback(address, new_value, user_data);
                modified = true;
            }
            editing_id = 0;
        }

        if (ImGui::IsKeyPressed(ImGuiKey_Escape) || lost_focus)
        {
            editing_id = 0;
        }

        ImGui::PopItemWidth();
        ImGui::PopStyleVar();

        if (flags & EditableRegisterFlags_ShowBinary)
        {
            ImGui::SameLine();
            ImGui::TextColored(gray, "(" BYTE_TO_BINARY_PATTERN_SPACED ")", BYTE_TO_BINARY(current_value));
        }
    }
    else
    {
        char value_str[16];
        snprintf(value_str, sizeof(value_str), "$%02X", current_value);

        if (write_callback != nullptr)
        {
            if (ImGui::Selectable(value_str, false, 0, ImVec2(0, 0)))
            {
                editing_id = widget_id;
                frames_editing = 0;
                snprintf(edit_buffer, sizeof(edit_buffer), "%02X", current_value);
            }
        }
        else
        {
            ImGui::Text("%s", value_str);
        }

        if (flags & EditableRegisterFlags_ShowBinary)
        {
            ImGui::SameLine();
            ImGui::TextColored(gray, "(" BYTE_TO_BINARY_PATTERN_SPACED ")", BYTE_TO_BINARY(current_value));
        }
    }

    ImGui::PopID();
    return modified;
}

inline bool EditableRegister16(
    const char* label,
    const char* addr_str,
    u16 address,
    u16 current_value,
    RegisterWriteCallback16 write_callback,
    void* user_data,
    EditableRegisterFlags flags = EditableRegisterFlags_Default,
    ImVec4 name_color = orange,
    ImVec4 addr_color = cyan)
{
    static ImGuiID editing_id = 0;
    static int frames_editing = 0;
    static char edit_buffer[8] = {0};
    bool modified = false;

    ImGui::PushID(address + 0x10000);
    ImGuiID widget_id = ImGui::GetID("##edit");

    if ((flags & EditableRegisterFlags_ShowAddr) && addr_str)
    {
        ImGui::TextColored(addr_color, "%s ", addr_str);
        ImGui::SameLine();
    }
    if ((flags & EditableRegisterFlags_ShowName) && label)
    {
        ImGui::TextColored(name_color, "%s ", label);
        ImGui::SameLine();
    }

    if (editing_id == widget_id)
    {
        float text_height = ImGui::GetTextLineHeight();
        float frame_height = ImGui::GetFrameHeight();
        float padding_reduction = (frame_height - text_height) * 0.5f;
        ImVec2 original_padding = ImGui::GetStyle().FramePadding;
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(original_padding.x, original_padding.y - padding_reduction));
        ImGui::PushItemWidth(ImGui::CalcTextSize("FFFF").x + 6);
        ImGuiInputTextFlags input_flags = ImGuiInputTextFlags_CharsHexadecimal |
                                          ImGuiInputTextFlags_CharsUppercase |
                                          ImGuiInputTextFlags_EnterReturnsTrue |
                                          ImGuiInputTextFlags_AutoSelectAll;

        if (frames_editing == 0)
        {
            ImGui::SetKeyboardFocusHere();
        }
        bool enter_pressed = ImGui::InputText("##edit", edit_buffer, sizeof(edit_buffer), input_flags);
        bool lost_focus = (frames_editing > 1) && !ImGui::IsItemActive();
        frames_editing++;

        if (enter_pressed)
        {
            u16 new_value = 0;
            if (parse_hex_string(edit_buffer, strlen(edit_buffer), &new_value))
            {
                if (write_callback)
                    write_callback(address, new_value, user_data);
                modified = true;
            }
            editing_id = 0;
        }

        if (ImGui::IsKeyPressed(ImGuiKey_Escape) || lost_focus)
        {
            editing_id = 0;
        }

        ImGui::PopItemWidth();
        ImGui::PopStyleVar();

        if (flags & EditableRegisterFlags_ShowBinary)
        {
            ImGui::SameLine();
            ImGui::TextColored(gray, "(" BYTE_TO_BINARY_PATTERN_SPACED " " BYTE_TO_BINARY_PATTERN_SPACED ")",
                     BYTE_TO_BINARY((current_value >> 8) & 0xFF),
                     BYTE_TO_BINARY(current_value & 0xFF));
        }
    }
    else
    {
        char value_str[16];
        snprintf(value_str, sizeof(value_str), "$%04X", current_value);

        if (write_callback != nullptr)
        {
            if (ImGui::Selectable(value_str, false, 0, ImVec2(0, 0)))
            {
                editing_id = widget_id;
                frames_editing = 0;
                snprintf(edit_buffer, sizeof(edit_buffer), "%04X", current_value);
            }
        }
        else
        {
            ImGui::Text("%s", value_str);
        }

        if (flags & EditableRegisterFlags_ShowBinary)
        {
            ImGui::SameLine();
            ImGui::TextColored(gray, "(" BYTE_TO_BINARY_PATTERN_SPACED " " BYTE_TO_BINARY_PATTERN_SPACED ")",
                     BYTE_TO_BINARY((current_value >> 8) & 0xFF),
                     BYTE_TO_BINARY(current_value & 0xFF));
        }
    }

    ImGui::PopID();
    return modified;
}

inline bool SliderFloatWithSteps(const char* label, float* v, float v_min, float v_max, float v_step, const char* display_format)
{
    if (!display_format)
        display_format = "%.3f";

    float v_f = *v;
    bool value_changed = ImGui::SliderFloat(label, &v_f, v_min, v_max, display_format, ImGuiSliderFlags_AlwaysClamp);
    float remain = fmodf((v_f - v_min), v_step);
    *v = (v_f - remain);
    return value_changed;
}

inline bool SliderIntWithSteps(const char* label, int* v, int v_min, int v_max, int v_step, const char* display_format)
{
    if (!display_format)
        display_format = "%d";

    if (v_step <= 0)
        v_step = 1;

    int v_i = *v;
    bool value_changed = ImGui::SliderInt(label, &v_i, v_min, v_max, display_format, ImGuiSliderFlags_AlwaysClamp);

    int diff = v_i - v_min;
    int remain = diff % v_step;

    if (remain < 0)
        remain += v_step;

    *v = v_i - remain;
    return value_changed;
}

#endif // GUI_DEBUG_WIDGETS_H
