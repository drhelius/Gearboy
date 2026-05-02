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

#define GUI_DEBUG_REWIND_IMPORT
#include "gui_debug_rewind.h"
#include "imgui.h"
#include "fonts/IconsMaterialDesign.h"
#include "gui.h"
#include "gui_debug_constants.h"
#include "config.h"
#include "emu.h"
#include "rewind.h"

static int seek_position = 0;
static bool scrubbing = false;

static void draw_transport_bar(void);
static void draw_timeline(void);

void gui_debug_window_rewind(void)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
    ImGui::SetNextWindowPos(ImVec2(180, 300), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_FirstUseEver);

    ImGui::Begin("Rewind", &config_debug.show_rewind, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Dummy(ImVec2(280, 0));

    draw_transport_bar();
    ImGui::Spacing();
    ImGui::Spacing();
    draw_timeline();
    ImGui::Spacing();

    ImGui::End();
    ImGui::PopStyleVar();
}

static void draw_transport_bar(void)
{
    int snapshot_count = rewind_get_snapshot_count();
    bool has_snapshots = snapshot_count > 0;
    bool is_empty = emu_is_empty();
    bool is_paused = emu_is_paused() || emu_is_debug_idle();
    bool can_scrub = has_snapshots && is_paused && !is_empty;
    bool at_newest = seek_position <= 0;
    bool at_oldest = seek_position >= snapshot_count - 1;

    ImGui::PushFont(gui_material_icons_font);

    if (is_paused)
    {
        if (ImGui::Button(ICON_MD_PLAY_ARROW "##rw"))
        {
            if (config_debug.debug)
                emu_debug_continue();
            else
                emu_resume();
            scrubbing = false;
            seek_position = 0;
        }
    }
    else
    {
        if (ImGui::Button(ICON_MD_PAUSE "##rw"))
        {
            if (config_debug.debug)
                emu_debug_break();
            else
                emu_pause();
            scrubbing = false;
            seek_position = 0;
        }
    }

    ImGui::SameLine();
    ImGui::BeginDisabled(!can_scrub || at_oldest);
    if (ImGui::Button(ICON_MD_SKIP_PREVIOUS "##rw"))
    {
        gui_debug_rewind_seek(snapshot_count - 1);
    }
    ImGui::EndDisabled();

    ImGui::SameLine();
    ImGui::BeginDisabled(!can_scrub || at_oldest);
    if (ImGui::Button(ICON_MD_FAST_REWIND "##rw"))
    {
        gui_debug_rewind_seek(seek_position + 1);
    }
    ImGui::EndDisabled();

    ImGui::SameLine();
    ImGui::BeginDisabled(!can_scrub || at_newest);
    if (ImGui::Button(ICON_MD_FAST_FORWARD "##rw"))
    {
        gui_debug_rewind_seek(seek_position - 1);
    }
    ImGui::EndDisabled();

    ImGui::SameLine();
    ImGui::BeginDisabled(!can_scrub || at_newest);
    if (ImGui::Button(ICON_MD_SKIP_NEXT "##rw"))
    {
        gui_debug_rewind_seek(0);
    }
    ImGui::EndDisabled();

    ImGui::PopFont();

    ImGui::SameLine();
    ImGui::TextColored(is_paused ? red : green, is_paused ? "    PAUSED" : "    RUNNING");
}

static void draw_timeline(void)
{
    int snapshot_count = rewind_get_snapshot_count();
    bool is_empty = emu_is_empty();
    bool is_paused = emu_is_paused() || emu_is_debug_idle();
    bool can_scrub = snapshot_count > 0 && is_paused && !is_empty;

    if (!can_scrub)
    {
        ImGui::BeginDisabled(true);
        int dummy = 0;
        ImGui::SetNextItemWidth(-1);
        ImGui::SliderInt("##rw_timeline", &dummy, 0, 0, snapshot_count > 0 ? "Pause to scrub" : "No snapshots");
        ImGui::EndDisabled();
        return;
    }

    int max_age = snapshot_count - 1;

    if (seek_position > max_age)
        seek_position = max_age;

    int slider_val = max_age - seek_position;

    int fps = rewind_get_frames_per_snapshot();
    if (fps < 1)
        fps = 1;

    float age_seconds = (float)(seek_position * fps) / 60.0f;
    char label[64];
    snprintf(label, sizeof(label), "%.1fs ago  (snapshot %d / %d)", age_seconds, slider_val + 1, snapshot_count);

    ImGui::SetNextItemWidth(-1);
    if (ImGui::SliderInt("##rw_timeline", &slider_val, 0, max_age, label))
    {
        gui_debug_rewind_seek(max_age - slider_val);
    }
}

bool gui_debug_rewind_seek(int age)
{
    int snapshot_count = rewind_get_snapshot_count();
    if (age < 0 || age >= snapshot_count)
        return false;

    if (!rewind_seek(age))
        return false;

    seek_position = age;
    scrubbing = true;
    return true;
}