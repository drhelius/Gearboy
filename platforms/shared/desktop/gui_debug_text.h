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

#ifndef GUI_DEBUG_TEXT_H
#define GUI_DEBUG_TEXT_H

#include <stdio.h>
#include <stdarg.h>
#include "imgui.h"

const char k_color_marker_start = '{';
const char k_color_marker_end = '}';

static inline void RemoveColorFromString(char* str)
{
    char* result = str;
    char* index = str;

    while(*index != '\0')
    {
        if(*index == k_color_marker_start)
        {
            while(*index != k_color_marker_end && *index != '\0')
            {
                index++;
            }

            if(*index == k_color_marker_end)
            {
                index++;
            }
        }
        else
        {
            *result = *index;
            result++;
            index++;
        }
    }

    *result = '\0';
}
 
static bool ProcessInlineHexColor(const char* start, const char* end, ImVec4& color)
{
    const int digits = (int)(end - start);

    if(digits == 6 || digits == 8)
    {
        char hex[9];
        memcpy(hex, start, digits);
        hex[digits] = 0;

        unsigned int hex_color = 0;
        if(sscanf(hex, "%x", &hex_color) > 0)
        {
            color.x = static_cast< float >((hex_color & 0x00FF0000) >> 16) / 255.0f;
            color.y = static_cast< float >((hex_color & 0x0000FF00) >> 8) / 255.0f;
            color.z = static_cast< float >((hex_color & 0x000000FF)) / 255.0f;
            color.w = 1.0f;

            if(digits == 8)
            {
                color.w = static_cast< float >((hex_color & 0xFF000000) >> 24) / 255.0f;
            }

            return true;
        }
    }

    return false;
}

static int TextColoredEx(const char* const msg, ...)
{
    char temp_string[4096];

    va_list args;
    va_start(args, msg);
    vsnprintf(temp_string, 4096, msg, args);
    va_end(args);
    temp_string[4095] = '\0';

    bool color_style_pushed = false;
    const char* text_start = temp_string;
    const char* text_position = temp_string;

    int character_counter = 0;

    while(text_position < (temp_string + sizeof(temp_string)) && *text_position != '\0')
    {
        if(*text_position == k_color_marker_start)
        {
            if(text_position != text_start)
            {
                ImGui::TextUnformatted(text_start, text_position);
                ImGui::SameLine(0.0f, 0.0f);
            }

            const char* color_start = text_position + 1;
            do
            {
                text_position++;
            }
            while(*text_position != '\0' && *text_position != k_color_marker_end);

            if(color_style_pushed)
            {
                ImGui::PopStyleColor();
                color_style_pushed = false;
            }

            ImVec4 text_color;
            if(ProcessInlineHexColor(color_start, text_position, text_color))
            {
                ImGui::PushStyleColor(ImGuiCol_Text, text_color);
                color_style_pushed = true;
            }

            text_start = text_position + 1;
        }
        else if(*text_position == '\n')
        {
            ImGui::TextUnformatted(text_start, text_position);
            text_start = text_position + 1;
        }
        else
        {
            character_counter++;
        }

        text_position++;
    }

    if(text_position != text_start)
    {
        ImGui::TextUnformatted(text_start, text_position);
    }
    else
    {
        ImGui::NewLine();
    }

    if(color_style_pushed)
    {
        ImGui::PopStyleColor();
    }

    return character_counter;
}

#endif /* GUI_DEBUG_TEXT_H */