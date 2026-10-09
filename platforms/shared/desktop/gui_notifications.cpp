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

#define GUI_NOTIFICATIONS_IMPORT
#include "gui_notifications.h"

#include <string.h>
#include <math.h>
#include <float.h>
#include "gui.h"
#include "gui_colors.h"
#include "config.h"

#define GUI_NOTIFICATION_COUNT 4

struct Notification
{
    gui_NotificationType type;
    char icon[8];
    char message[256];
    char detail[4096];
    char tag[32];
    Uint64 start;
    Uint64 end;
    float fit_width;
    float fit_size;
    char fit_message[512];
    char fit_detail[512];
};

static const Uint64 k_notification_durations[4] = { 3000, 3000, 4000, 6000 };
static const char* const k_notification_icons[4] = { ICON_MD_INFO, ICON_MD_CHECK_CIRCLE, ICON_MD_WARNING, ICON_MD_ERROR };
static const float k_notification_fade_in = 150.0f;
static const float k_notification_fade_out = 300.0f;
static const Uint64 k_notification_collapse = 150;
static const float k_notification_margin = 12.0f;
static const float k_notification_spacing = 8.0f;
static const float k_notification_rounding = 8.0f;
static const float k_notification_icon_spacing = 10.0f;
static const float k_notification_detail_spacing = 3.0f;
static const float k_notification_detail_scale = 0.88f;
static const float k_notification_max_width = 560.0f;
static const ImVec2 k_notification_padding = ImVec2(14.0f, 9.0f);

static Notification notifications[GUI_NOTIFICATION_COUNT];
static int notification_count = 0;
static bool output_rendered = false;

static int find_notification(const char* message, const char* detail, const char* tag);
static void remove_notification(int index);
static void copy_text(char* dest, const char* src, size_t dest_size);
static void fit_text(char* dest, size_t dest_size, const char* text, ImFont* font, float font_size, float max_width, bool middle);
static float ease(float t);
static ImVec4 type_color(gui_NotificationType type);
static void draw(ImDrawList* draw_list, const ImVec2& min, const ImVec2& max);

void gui_notify(gui_NotificationType type, const char* icon, const char* message, const char* detail, const char* tag, Uint64 milliseconds)
{
    if (message == NULL)
        return;

    if (!config_emulator.show_notifications && (type == gui_NotificationInfo || type == gui_NotificationSuccess))
        return;

    Uint64 now = SDL_GetTicks();
    int index = find_notification(message, detail, tag);

    if (index >= 0 && now >= notifications[index].end)
    {
        remove_notification(index);
        index = -1;
    }

    if (index < 0)
    {
        if (notification_count == GUI_NOTIFICATION_COUNT)
            remove_notification(0);

        index = notification_count;
        notification_count++;
        notifications[index].start = now;
    }

    Notification* notification = &notifications[index];
    notification->type = type;
    notification->end = now + ((milliseconds > 0) ? milliseconds : k_notification_durations[type]);
    notification->fit_width = 0.0f;
    copy_text(notification->icon, (icon != NULL) ? icon : k_notification_icons[type], sizeof(notification->icon));
    copy_text(notification->message, message, sizeof(notification->message));
    copy_text(notification->detail, (detail != NULL) ? detail : "", sizeof(notification->detail));
    copy_text(notification->tag, (tag != NULL) ? tag : "", sizeof(notification->tag));
}

void gui_notifications_render_output(const ImVec2& min, const ImVec2& max)
{
    output_rendered = true;

    if (notification_count > 0)
        draw(ImGui::GetWindowDrawList(), min, max);
}

void gui_notifications_render(void)
{
    if (notification_count > 0 && !output_rendered)
    {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        draw(ImGui::GetForegroundDrawList(viewport), viewport->WorkPos, viewport->WorkPos + viewport->WorkSize);
    }

    output_rendered = false;
    Uint64 now = SDL_GetTicks();

    for (int i = notification_count - 1; i >= 0; i--)
    {
        if (now >= notifications[i].end + k_notification_collapse)
            remove_notification(i);
    }
}

static int find_notification(const char* message, const char* detail, const char* tag)
{
    for (int i = 0; i < notification_count; i++)
    {
        Notification* notification = &notifications[i];

        if (tag != NULL && tag[0] != 0)
        {
            if (strcmp(notification->tag, tag) == 0)
                return i;
        }
        else if (notification->tag[0] == 0 && strcmp(notification->message, message) == 0 &&
            strcmp(notification->detail, (detail != NULL) ? detail : "") == 0)
        {
            return i;
        }
    }

    return -1;
}

static void remove_notification(int index)
{
    for (int i = index; i < notification_count - 1; i++)
        notifications[i] = notifications[i + 1];

    notification_count--;
}

static void copy_text(char* dest, const char* src, size_t dest_size)
{
    size_t length = strlen(src);

    if (length >= dest_size)
    {
        length = dest_size - 1;

        while (length > 0 && (src[length] & 0xC0) == 0x80)
            length--;
    }

    for (size_t i = 0; i < length; i++)
        dest[i] = (src[i] == '\n' || src[i] == '\r' || src[i] == '\t') ? ' ' : src[i];

    dest[length] = 0;
}

static void fit_text(char* dest, size_t dest_size, const char* text, ImFont* font, float font_size, float max_width, bool middle)
{
    int length = (int)strlen(text);
    int limit = (int)dest_size - 4;
    int head = length;
    int tail = 0;

    if (middle)
        head = (length > limit) ? (limit / 3) : (length / 2);
    else if (length > limit)
        head = limit;

    while (head > 0 && (text[head] & 0xC0) == 0x80)
        head--;

    if (middle)
        tail = (length > limit) ? (limit - head) : (length - head);

    while (tail > 0 && (text[length - tail] & 0xC0) == 0x80)
        tail--;

    bool cut = (head + tail) < length;
    float ellipsis_width = font->CalcTextSizeA(font_size, FLT_MAX, 0.0f, "...").x;
    float head_width = font->CalcTextSizeA(font_size, FLT_MAX, 0.0f, text, text + head).x;
    float tail_width = font->CalcTextSizeA(font_size, FLT_MAX, 0.0f, text + length - tail, text + length).x;

    while (head + tail > 0)
    {
        if (head_width + tail_width + (cut ? ellipsis_width : 0.0f) <= max_width)
            break;

        cut = true;

        if (!middle || (head > tail / 2))
        {
            int end = head;
            head--;

            while (head > 0 && (text[head] & 0xC0) == 0x80)
                head--;

            head_width -= font->CalcTextSizeA(font_size, FLT_MAX, 0.0f, text + head, text + end).x;
        }
        else
        {
            int start = length - tail;
            tail--;

            while (tail > 0 && (text[length - tail] & 0xC0) == 0x80)
                tail--;

            tail_width -= font->CalcTextSizeA(font_size, FLT_MAX, 0.0f, text + start, text + length - tail).x;
        }
    }

    int pos = head;
    memcpy(dest, text, head);

    if (cut)
    {
        memcpy(dest + pos, "...", 3);
        pos += 3;
    }

    memcpy(dest + pos, text + length - tail, tail);
    dest[pos + tail] = 0;
}

static float ease(float t)
{
    if (t <= 0.0f)
        return 0.0f;

    if (t >= 1.0f)
        return 1.0f;

    float inverse = 1.0f - t;
    return 1.0f - (inverse * inverse * inverse);
}

static ImVec4 type_color(gui_NotificationType type)
{
    switch (type)
    {
    case gui_NotificationSuccess:
        return green;
    case gui_NotificationWarning:
        return amber;
    case gui_NotificationError:
        return red;
    default:
        return accent;
    }
}

static void draw(ImDrawList* draw_list, const ImVec2& min, const ImVec2& max)
{
    ImGui::PushFont(gui_roboto_font);

    ImFont* font = ImGui::GetFont();
    float font_size = ImGui::GetFontSize();
    float detail_size = font_size * k_notification_detail_scale;
    float icon_width = font->CalcTextSizeA(font_size, FLT_MAX, 0.0f, ICON_MD_INFO).x;
    float max_width = fminf(max.x - min.x - (k_notification_margin * 2.0f), k_notification_max_width);
    float text_width = max_width - (k_notification_padding.x * 2.0f) - icon_width - k_notification_icon_spacing;

    if (text_width < font_size * 3.0f)
    {
        ImGui::PopFont();
        return;
    }

    const ImGuiStyle& style = ImGui::GetStyle();
    Uint64 now = SDL_GetTicks();
    float bottom = max.y - k_notification_margin;

    draw_list->PushClipRect(min, max, true);

    for (int i = notification_count - 1; i >= 0; i--)
    {
        Notification* notification = &notifications[i];
        bool has_detail = notification->detail[0] != 0;

        if (notification->fit_width != text_width || notification->fit_size != font_size)
        {
            fit_text(notification->fit_message, sizeof(notification->fit_message), notification->message, font,
                font_size, text_width, false);
            fit_text(notification->fit_detail, sizeof(notification->fit_detail), notification->detail, font,
                detail_size, text_width, true);
            notification->fit_width = text_width;
            notification->fit_size = font_size;
        }

        float content_width = font->CalcTextSizeA(font_size, FLT_MAX, 0.0f, notification->fit_message).x;
        float height = (k_notification_padding.y * 2.0f) + font_size;

        if (has_detail)
        {
            float detail_width = font->CalcTextSizeA(detail_size, FLT_MAX, 0.0f, notification->fit_detail).x;
            content_width = fmaxf(content_width, detail_width);
            height += k_notification_detail_spacing + detail_size;
        }

        float width = (k_notification_padding.x * 2.0f) + icon_width + k_notification_icon_spacing + content_width;
        float appear = ease((float)(now - notification->start) / k_notification_fade_in);
        float remaining = (float)((Sint64)notification->end - (Sint64)now);
        float alpha = appear * fminf(fmaxf(remaining / k_notification_fade_out, 0.0f), 1.0f);
        float collapse = 1.0f;

        if (now > notification->end)
            collapse = 1.0f - ease((float)(now - notification->end) / (float)k_notification_collapse);

        float top = bottom - height + ((1.0f - appear) * (height + k_notification_spacing));
        bottom -= (height + k_notification_spacing) * appear * collapse;

        if (alpha <= 0.0f)
            continue;

        ImVec2 box_min(roundf(min.x + k_notification_margin), roundf(top));
        ImVec2 box_max(box_min.x + roundf(width), box_min.y + roundf(height));
        ImVec4 color = type_color(notification->type);
        ImVec4 background = style.Colors[ImGuiCol_PopupBg];
        ImVec4 border = color;
        ImVec4 message_color = style.Colors[ImGuiCol_Text];
        ImVec4 detail_color = style.Colors[ImGuiCol_TextDisabled];
        background.w = 0.95f * alpha;
        border.w = 0.80f * alpha;
        color.w = alpha;
        message_color.w *= alpha;
        detail_color.w *= alpha;

        float icon_y = box_min.y + ((height - font_size) * 0.5f);
        float text_x = box_min.x + k_notification_padding.x + icon_width + k_notification_icon_spacing;
        float text_y = box_min.y + k_notification_padding.y;

        draw_list->AddRectFilled(box_min, box_max, ImGui::ColorConvertFloat4ToU32(background), k_notification_rounding);
        draw_list->AddRect(box_min, box_max, ImGui::ColorConvertFloat4ToU32(border), k_notification_rounding);
        draw_list->AddText(font, font_size, ImVec2(box_min.x + k_notification_padding.x, icon_y),
            ImGui::ColorConvertFloat4ToU32(color), notification->icon);
        draw_list->AddText(font, font_size, ImVec2(text_x, text_y), ImGui::ColorConvertFloat4ToU32(message_color),
            notification->fit_message);

        if (has_detail)
        {
            draw_list->AddText(font, detail_size, ImVec2(text_x, text_y + font_size + k_notification_detail_spacing),
                ImGui::ColorConvertFloat4ToU32(detail_color), notification->fit_detail);
        }
    }

    draw_list->PopClipRect();
    ImGui::PopFont();
}
