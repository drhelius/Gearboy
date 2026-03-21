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

#define GUI_DEBUG_YM2413_IMPORT
#include "gui_debug_ym2413.h"

#include "imgui.h"
#include "fonts/IconsMaterialDesign.h"
#include "gearboy.h"
#include "gui.h"
#include "gui_debug_constants.h"
#include "config.h"
#include "emu.h"
#include "utils.h"

static const char* k_instrument_names[16] = {
    "User",
    "Violin",
    "Guitar",
    "Piano",
    "Flute",
    "Clarinet",
    "Oboe",
    "Trumpet",
    "Organ",
    "Horn",
    "Synthesizer",
    "Harpsichord",
    "Vibraphone",
    "Synth Bass",
    "Acoustic Bass",
    "Electric Guitar"
};

static const char* k_rhythm_names[5] = {
    "Bass Drum", "Snare Drum", "Tom-Tom", "Top Cymbal", "High Hat"
};

static const char* k_eg_state_names[6] = {
    "OFF", "REL", "SUS", "DEC", "ATT", "DMP"
};

static const ImVec4 k_eg_state_colors[6] = {
    gray,    // OFF
    red,     // REL
    yellow,  // SUS
    orange,  // DEC
    green,   // ATT
    violet   // DMP
};

static const char* k_mul_names[16] = {
    "x1/2", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
    "x8", "x9", "x10", "x10", "x12", "x12", "x15", "x15"
};

void gui_debug_ym2413_init(void)
{
}

void gui_debug_ym2413_destroy(void)
{
}

void gui_debug_window_ym2413(void)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
    ImGui::SetNextWindowPos(ImVec2(500, 45), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(266, 616), ImGuiCond_FirstUseEver);
    ImGui::Begin("YM2413 FM", &config_debug.show_ym2413);

    GearsystemCore* core = emu_get_core();
    YM2413_OPLL* opll = (YM2413_OPLL*)YM2413GetContextPtr();

    bool rhythm_mode = (opll->rhythm & 0x20) != 0;

    GS_RuntimeInfo runtime;
    core->GetRuntimeInfo(runtime);
    int master_clock = (runtime.region == Region_PAL) ? GS_MASTER_CLOCK_PAL : GS_MASTER_CLOCK_NTSC;

    if (ImGui::BeginTabBar("##fm_tabs", ImGuiTabBarFlags_None))
    {
        for (int c = 0; c < 9; c++)
        {
            YM2413_OPLL_CH* ch = &opll->P_CH[c];
            u8 instvol = opll->instvol_r[c];
            int inst = (instvol >> 4) & 0x0F;
            int vol = instvol & 0x0F;

            int fnum = ch->block_fnum & 0x1FF;
            int block = (ch->block_fnum >> 9) & 0x07;
            bool key_on = (ch->SLOT[1].key != 0);
            bool sustain = (ch->sus != 0);

            bool is_rhythm_ch = rhythm_mode && (c >= 6);

            char tab_name[16];
            snprintf(tab_name, 16, "%d", c + 1);

            if (ImGui::BeginTabItem(tab_name))
            {
                ImGui::PushFont(gui_default_font);

                ImGui::TextColored(violet, "MODE        "); ImGui::SameLine();
                if (is_rhythm_ch)
                {
                    if (c == 6)
                        ImGui::TextColored(orange, "Bass Drum");
                    else if (c == 7)
                        ImGui::TextColored(orange, "HH / Snare");
                    else
                        ImGui::TextColored(orange, "Tom / Cymbal");
                }
                else
                {
                    ImGui::TextColored(gray, "Melody");
                }

                ImGui::TextColored(violet, "INSTRUMENT  "); ImGui::SameLine();
                if (!is_rhythm_ch)
                {
                    ImGui::TextColored(white, "%X", inst); ImGui::SameLine();
                    ImGui::TextColored(gray, " (%s)", k_instrument_names[inst]);
                }
                else
                {
                    ImGui::TextColored(gray, "N/A");
                }

                ImGui::TextColored(violet, "VOLUME      "); ImGui::SameLine();
                ImGui::TextColored(vol == 15 ? gray : white, "%X", vol); ImGui::SameLine();
                if (vol == 15)
                    ImGui::TextColored(gray, " (OFF)");
                else if (vol == 0)
                    ImGui::TextColored(green, " (MAX)");
                else
                    ImGui::TextColored(gray, " (-%.1f dB)", vol * 3.0f);

                ImGui::TextColored(violet, "KEY-ON      "); ImGui::SameLine();
                ImGui::TextColored(key_on ? green : gray, "%s", key_on ? "YES" : "NO");

                ImGui::TextColored(violet, "SUSTAIN     "); ImGui::SameLine();
                ImGui::TextColored(sustain ? green : gray, "%s", sustain ? "YES" : "NO");

                ImGui::Separator();

                ImGui::TextColored(violet, "F-NUMBER    "); ImGui::SameLine();
                ImGui::TextColored(white, "%03X", fnum);

                ImGui::TextColored(violet, "BLOCK       "); ImGui::SameLine();
                ImGui::TextColored(white, "%d", block);

                float freq_hz = 0.0f;
                if (fnum > 0)
                {
                    freq_hz = ((float)master_clock / 72.0f) * (float)fnum / (float)(1 << (19 - block));
                }

                ImGui::TextColored(violet, "OUTPUT HZ   "); ImGui::SameLine();
                if (freq_hz > 0.0f)
                {
                    if (freq_hz >= 1000.0f)
                        ImGui::TextColored(cyan, "%.2f KHz", freq_hz / 1000.0f);
                    else
                        ImGui::TextColored(cyan, "%.2f Hz", freq_hz);
                }
                else
                    ImGui::TextColored(gray, "N/A");

                ImGui::Separator();

                for (int s = 0; s < 2; s++)
                {
                    YM2413_OPLL_SLOT* slot = &ch->SLOT[s];
                    const char* slot_name = (s == 0) ? "MODULATOR" : "CARRIER";

                    ImGui::TextColored(green, "%s", slot_name);

                    int eg_state = slot->state;
                    if (eg_state < 0 || eg_state > 5) eg_state = 0;
                    ImGui::TextColored(violet, " EG STATE   "); ImGui::SameLine();
                    ImGui::TextColored(k_eg_state_colors[eg_state], "%s", k_eg_state_names[eg_state]);

                    int eg_vol = slot->volume >> 2;
                    int eg_vol_pct = (eg_vol > 127) ? 0 : (127 - eg_vol);
                    float bar_pct = (float)eg_vol_pct / 127.0f;
                    ImGui::TextColored(violet, " EG LEVEL   "); ImGui::SameLine();
                    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, green);
                    char eg_overlay[16];
                    snprintf(eg_overlay, 16, "%d", eg_vol_pct);
                    ImGui::ProgressBar(bar_pct, ImVec2(100, 12), eg_overlay);
                    ImGui::PopStyleColor();

                    ImGui::TextColored(violet, " TL         "); ImGui::SameLine();
                    ImGui::TextColored(white, "%02X", slot->TL >> 2);

                    ImGui::TextColored(violet, " AR/DR      "); ImGui::SameLine();
                    ImGui::TextColored(white, "%X/%X", slot->ar >> 2, slot->dr >> 2);

                    ImGui::TextColored(violet, " SL/RR      "); ImGui::SameLine();
                    int sl_display = 0;
                    for (int i = 0; i < 16; i++)
                    {
                        if (slot->sl == (unsigned int)(i * 0x20))
                        {
                            sl_display = i;
                            break;
                        }
                    }
                    ImGui::TextColored(white, "%X/%X", sl_display, slot->rr >> 2);

                    ImGui::TextColored(violet, " MULTI      "); ImGui::SameLine();
                    int mul_idx = 0;
                    static const uint8_t mul_tab[] = {1, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 20, 24, 24, 30, 30};
                    for (int i = 0; i < 16; i++)
                    {
                        if (slot->mul == mul_tab[i])
                        {
                            mul_idx = i;
                            break;
                        }
                    }
                    ImGui::TextColored(white, "%X", mul_idx); ImGui::SameLine();
                    ImGui::TextColored(gray, " (%s)", k_mul_names[mul_idx]);

                    ImGui::TextColored(violet, " KSR        "); ImGui::SameLine();
                    ImGui::TextColored(slot->KSR ? green : gray, "%s", slot->KSR ? "YES" : "NO");

                    ImGui::TextColored(violet, " AM/VIB     "); ImGui::SameLine();
                    bool am = (slot->AMmask != 0);
                    ImGui::TextColored(am ? green : gray, "AM:%s", am ? "ON " : "OFF"); ImGui::SameLine();
                    ImGui::TextColored(slot->vib ? green : gray, " VIB:%s", slot->vib ? "ON" : "OFF");

                    ImGui::TextColored(violet, " WAVE       "); ImGui::SameLine();
                    ImGui::TextColored(white, "%s", slot->wavetable ? "Half-Sine" : "Sine");

                    ImGui::TextColored(violet, " EG TYPE    "); ImGui::SameLine();
                    ImGui::TextColored(white, "%s", slot->eg_type ? "Sustained" : "Percussive");

                    ImGui::TextColored(violet, " FB SHIFT   "); ImGui::SameLine();
                    if (s == 0 && slot->fb_shift)
                        ImGui::TextColored(white, "%d", slot->fb_shift);
                    else
                        ImGui::TextColored(gray, "%s", s == 0 ? "OFF" : "N/A");
                }

                ImGui::PopFont();
                ImGui::EndTabItem();
            }
        }

        if (ImGui::BeginTabItem("Global"))
        {
            ImGui::PushFont(gui_default_font);

            ImGui::TextColored(violet, "RHYTHM      "); ImGui::SameLine();
            ImGui::TextColored(rhythm_mode ? green : gray, "%s", rhythm_mode ? "ON" : "OFF");

            ImGui::TextColored(violet, "REG 0E      "); ImGui::SameLine();
            ImGui::TextColored(rhythm_mode ? white : gray, "%02X", opll->rhythm);

            ImGui::Separator();

            for (int d = 0; d < 5; d++)
            {
                bool drum_on = rhythm_mode && (opll->rhythm & (0x10 >> d)) != 0;
                ImGui::TextColored(rhythm_mode ? violet : gray, "%-12s", k_rhythm_names[d]); ImGui::SameLine();
                ImGui::TextColored(drum_on ? green : gray, "%s", drum_on ? "ON" : "OFF");
            }

            ImGui::Separator();

            ImGui::TextColored(rhythm_mode ? green : gray, "DRUM VOLUMES");

            u8 vol67 = opll->instvol_r[6];
            u8 vol78 = opll->instvol_r[7];
            u8 vol89 = opll->instvol_r[8];

            ImGui::TextColored(rhythm_mode ? violet : gray, " Bass Drum  "); ImGui::SameLine();
            ImGui::TextColored(rhythm_mode ? white : gray, "%X", vol67 & 0x0F);

            ImGui::TextColored(rhythm_mode ? violet : gray, " High Hat   "); ImGui::SameLine();
            ImGui::TextColored(rhythm_mode ? white : gray, "%X", (vol78 >> 4) & 0x0F);

            ImGui::TextColored(rhythm_mode ? violet : gray, " Snare Drum "); ImGui::SameLine();
            ImGui::TextColored(rhythm_mode ? white : gray, "%X", vol78 & 0x0F);

            ImGui::TextColored(rhythm_mode ? violet : gray, " Tom-Tom    "); ImGui::SameLine();
            ImGui::TextColored(rhythm_mode ? white : gray, "%X", (vol89 >> 4) & 0x0F);

            ImGui::TextColored(rhythm_mode ? violet : gray, " Top Cymbal "); ImGui::SameLine();
            ImGui::TextColored(rhythm_mode ? white : gray, "%X", vol89 & 0x0F);

            ImGui::Separator();

            ImGui::TextColored(green, "USER INSTRUMENT");

            u8* user = opll->inst_tab[0];
            ImGui::TextColored(violet, " MULT       "); ImGui::SameLine();
            ImGui::TextColored(white, "M:%X C:%X", user[0] & 0x0F, user[1] & 0x0F);

            ImGui::TextColored(violet, " KSL/TL     "); ImGui::SameLine();
            ImGui::TextColored(white, "%02X", user[2]);

            ImGui::TextColored(violet, " FB/WAVE    "); ImGui::SameLine();
            ImGui::TextColored(white, "%02X", user[3]);

            ImGui::TextColored(violet, " AR/DR MOD  "); ImGui::SameLine();
            ImGui::TextColored(white, "%X/%X", (user[4] >> 4) & 0x0F, user[4] & 0x0F);

            ImGui::TextColored(violet, " AR/DR CAR  "); ImGui::SameLine();
            ImGui::TextColored(white, "%X/%X", (user[5] >> 4) & 0x0F, user[5] & 0x0F);

            ImGui::TextColored(violet, " SL/RR MOD  "); ImGui::SameLine();
            ImGui::TextColored(white, "%X/%X", (user[6] >> 4) & 0x0F, user[6] & 0x0F);

            ImGui::TextColored(violet, " SL/RR CAR  "); ImGui::SameLine();
            ImGui::TextColored(white, "%X/%X", (user[7] >> 4) & 0x0F, user[7] & 0x0F);

            ImGui::Separator();

            ImGui::TextColored(violet, "ADDRESS REG "); ImGui::SameLine();
            ImGui::TextColored(white, "%02X", opll->address);

            ImGui::TextColored(violet, "STATUS      "); ImGui::SameLine();
            ImGui::TextColored(white, "%02X", opll->status);

            ImGui::TextColored(violet, "NOISE LFSR  "); ImGui::SameLine();
            ImGui::TextColored(white, "%06X", opll->noise_rng & 0x7FFFFF);

            ImGui::PopFont();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
    ImGui::PopStyleVar();
}
