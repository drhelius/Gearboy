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

#define GUI_DEBUG_PSG_IMPORT
#include "gui_debug_psg.h"

#include "imgui.h"
#include "implot.h"
#include "fonts/IconsMaterialDesign.h"
#include "gearboy.h"
#include "gui.h"
#include "gui_debug_constants.h"
#include "config.h"
#include "emu.h"
#include "utils.h"

static bool exclusive_channel[4] = { false, false, false, false };
static float* wave_buffer = NULL;

static const char* k_tab_names[4] = { "Square 1", "Square 2", "Wave", "Noise" };
static const char* k_channel_labels[4] = { "SQUARE 1", "SQUARE 2", "WAVE", "NOISE" };
static const char* k_duty_names[4] = { "12.5%", "25%", "50%", "75%" };
static const char* k_wave_vol_names[4] = { "Mute", "100%", "50%", "25%" };

static inline int get_val_helper(const gb_apu_state_t::val_t& v)
{
#if GB_APU_CUSTOM_STATE
    return v;
#else
    return (int)(v[3] * 0x1000000u + v[2] * 0x10000u + v[1] * 0x100u + v[0]);
#endif
}

void gui_debug_psg_init(void)
{
    wave_buffer = new float[AUDIO_BUFFER_SIZE];
}

void gui_debug_psg_destroy(void)
{
    SafeDeleteArray(wave_buffer);
}

void gui_debug_window_psg(void)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
    ImGui::SetNextWindowPos(ImVec2(180, 45), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(310, 370), ImGuiCond_FirstUseEver);
    ImGui::Begin("PSG", &config_debug.show_psg);

    GearboyCore* core = emu_get_core();
    Audio* audio = core->GetAudio();
    Gb_Apu* apu = audio->GetApu();

    gb_apu_state_t apu_state;
    apu->save_state(&apu_state);

    bool is_cgb = core->IsCGB();

    if (ImGui::BeginTabBar("##psg_tabs", ImGuiTabBarFlags_None))
    {
        for (int c = 0; c < 4; c++)
        {
            if (ImGui::BeginTabItem(k_tab_names[c]))
            {
                ImGui::PushFont(gui_default_font);

                bool* mute_ptr = apu->get_mute_ptr(c);

                if (ImGui::BeginTable("##audio", 2, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoPadOuterX))
                {
                    ImGui::TableNextColumn();

                    ImGui::PushStyleColor(ImGuiCol_Text, *mute_ptr ? mid_gray : white);
                    ImGui::PushFont(gui_material_icons_font);

                    char label[32];
                    snprintf(label, 32, "%s##mute%d", *mute_ptr ? ICON_MD_MUSIC_OFF : ICON_MD_MUSIC_NOTE, c);
                    if (ImGui::Button(label))
                    {
                        for (int i = 0; i < 4; i++)
                            exclusive_channel[i] = false;
                        *mute_ptr = !*mute_ptr;
                    }
                    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                        ImGui::SetTooltip("Mute Channel");

                    snprintf(label, 32, "%s##exc%d", ICON_MD_STAR, c);

                    ImGui::PushStyleColor(ImGuiCol_Text, exclusive_channel[c] ? yellow : white);
                    if (ImGui::Button(label))
                    {
                        exclusive_channel[c] = !exclusive_channel[c];
                        *mute_ptr = false;
                        for (int i = 0; i < 4; i++)
                        {
                            if (i != c)
                            {
                                exclusive_channel[i] = false;
                                bool* other = apu->get_mute_ptr(i);
                                *other = exclusive_channel[c] ? true : false;
                            }
                        }
                    }
                    ImGui::PopStyleColor();
                    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                    {
                        ImGui::SetTooltip("Solo Channel");
                    }
                    ImGui::PopFont();
                    ImGui::PopStyleColor();

                    ImGui::TableNextColumn();

                    ImPlot::PushStyleVar(ImPlotStyleVar_PlotPadding, ImVec2(1, 1));

                    blip_sample_t* ch_buf = audio->GetDebugChannelBuffer(c);
                    int data_size = audio->GetDebugChannelSamples(c);

                    if (ch_buf && data_size > 0)
                    {
                        for (int i = 0; i < data_size; i++)
                            wave_buffer[i] = (float)ch_buf[i] / 32768.0f * 6.0f;
                    }
                    else
                    {
                        data_size = 1;
                        wave_buffer[0] = 0.0f;
                    }

                    int trigger = 0;
                    for (int i = 100; i < data_size; ++i)
                    {
                        if (wave_buffer[i - 1] < 0.0f && wave_buffer[i] >= 0.0f)
                        {
                            trigger = i;
                            break;
                        }
                    }

                    int half_window_size = 100;
                    int x_min = MAX(0, trigger - half_window_size);
                    int x_max = MIN(data_size, trigger + half_window_size);

                    ImPlotAxisFlags flags = ImPlotAxisFlags_NoGridLines | ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_NoLabel | ImPlotAxisFlags_NoHighlight | ImPlotAxisFlags_Lock | ImPlotAxisFlags_NoTickMarks;

                    if (ImPlot::BeginPlot("Waveform", ImVec2(180, 50), ImPlotFlags_CanvasOnly))
                    {
                        ImPlot::SetupAxes("x", "y", flags, flags);
                        ImPlot::SetupAxesLimits(x_min, x_max, -1.0f, 1.0f, ImPlotCond_Always);
                        ImPlot::SetNextLineStyle(green, 1.0f);
                        ImPlot::PlotLine("Wave", wave_buffer, data_size);
                        ImPlot::EndPlot();
                    }

                    ImPlot::PopStyleVar();

                    ImGui::EndTable();
                }

                ImGui::Separator();

                u8* regs = apu_state.regs;

                bool ch_enabled = get_val_helper(apu_state.enabled[c]) != 0;
                int env_volume = (c != 2) ? get_val_helper(apu_state.env_volume[c < 2 ? c : 2]) : 0;
                int length_ctr = get_val_helper(apu_state.length_ctr[c]);
                int ch_phase = get_val_helper(apu_state.phase[c]);

                ImGui::TextColored(violet, "ENABLED     "); ImGui::SameLine();
                ImGui::TextColored(ch_enabled ? green : gray, "%s", ch_enabled ? "YES" : "NO");

                if (c < 2)
                {
                    // Square channels
                    int base = c * 5;
                    int freq = (regs[base + 3] | ((regs[base + 4] & 0x07) << 8));
                    int duty = (regs[base + 1] >> 6) & 0x03;
                    int env_init = (regs[base + 2] >> 4) & 0x0F;
                    int env_dir = (regs[base + 2] >> 3) & 0x01;
                    int env_pace = regs[base + 2] & 0x07;
                    bool dac_on = (regs[base + 2] & 0xF8) != 0;
                    bool length_en = (regs[base + 4] & 0x40) != 0;

                    ImGui::TextColored(violet, "DAC         "); ImGui::SameLine();
                    ImGui::TextColored(dac_on ? green : gray, "%s", dac_on ? "ON" : "OFF");

                    ImGui::TextColored(violet, "DUTY        "); ImGui::SameLine();
                    ImGui::TextColored(white, "%s", k_duty_names[duty]); ImGui::SameLine();
                    ImGui::TextColored(gray, " (%d)", duty);

                    ImGui::TextColored(violet, "VOLUME      "); ImGui::SameLine();
                    ImGui::TextColored(env_volume == 0 ? gray : white, "$%X", env_volume); ImGui::SameLine();
                    if (env_volume == 0)
                        ImGui::TextColored(gray, " (OFF)");
                    else if (env_volume == 15)
                        ImGui::TextColored(green, " (MAX)");
                    else
                        ImGui::TextColored(gray, " (%d/15)", env_volume);

                    ImGui::TextColored(violet, "ENVELOPE    "); ImGui::SameLine();
                    ImGui::TextColored(white, "Init:$%X %s Pace:%d", env_init, env_dir ? "UP" : "DN", env_pace);

                    ImGui::TextColored(violet, "FREQUENCY   "); ImGui::SameLine();
                    ImGui::TextColored(white, "$%03X", freq);

                    ImGui::TextColored(violet, "PHASE       "); ImGui::SameLine();
                    ImGui::TextColored(white, "%d", ch_phase);

                    ImGui::TextColored(violet, "LENGTH      "); ImGui::SameLine();
                    ImGui::TextColored(length_en ? white : gray, "%d", length_ctr); ImGui::SameLine();
                    ImGui::TextColored(gray, " (%s)", length_en ? "ON" : "OFF");

                    ImGui::TextColored(violet, "OUTPUT HZ   "); ImGui::SameLine();
                    if (freq > 0)
                    {
                        float freq_hz = 131072.0f / (2048.0f - freq);
                        if (freq_hz >= 1000.0f)
                            ImGui::TextColored(cyan, "%.2f KHz", freq_hz / 1000.0f);
                        else
                            ImGui::TextColored(cyan, "%.2f Hz", freq_hz);
                    }
                    else
                    {
                        ImGui::TextColored(gray, "DC");
                    }

                    if (c == 0)
                    {
                        int sweep_pace = (regs[0] >> 4) & 0x07;
                        int sweep_dir = (regs[0] >> 3) & 0x01;
                        int sweep_step = regs[0] & 0x07;

                        ImGui::TextColored(violet, "SWEEP       "); ImGui::SameLine();
                        ImGui::TextColored(white, "Pace:%d %s Step:%d", sweep_pace, sweep_dir ? "DN" : "UP", sweep_step);
                    }
                }
                else if (c == 2)
                {
                    // Wave channel
                    int base = 2 * 5;
                    int freq = (regs[base + 3] | ((regs[base + 4] & 0x07) << 8));
                    int vol_code = (regs[base + 2] >> 5) & 0x03;
                    bool dac_on = (regs[base + 0] & 0x80) != 0;
                    bool length_en = (regs[base + 4] & 0x40) != 0;

                    ImGui::TextColored(violet, "DAC         "); ImGui::SameLine();
                    ImGui::TextColored(dac_on ? green : gray, "%s", dac_on ? "ON" : "OFF");

                    ImGui::TextColored(violet, "VOLUME      "); ImGui::SameLine();
                    ImGui::TextColored(vol_code == 0 ? gray : white, "%s", k_wave_vol_names[vol_code]); ImGui::SameLine();
                    ImGui::TextColored(gray, " (%d)", vol_code);

                    ImGui::TextColored(violet, "FREQUENCY   "); ImGui::SameLine();
                    ImGui::TextColored(white, "$%03X", freq);

                    ImGui::TextColored(violet, "PHASE       "); ImGui::SameLine();
                    ImGui::TextColored(white, "%d", ch_phase);

                    ImGui::TextColored(violet, "LENGTH      "); ImGui::SameLine();
                    ImGui::TextColored(length_en ? white : gray, "%d", length_ctr); ImGui::SameLine();
                    ImGui::TextColored(gray, " (%s)", length_en ? "ON" : "OFF");

                    ImGui::TextColored(violet, "OUTPUT HZ   "); ImGui::SameLine();
                    if (freq > 0)
                    {
                        float freq_hz = 65536.0f / (2048.0f - freq);
                        if (freq_hz >= 1000.0f)
                            ImGui::TextColored(cyan, "%.2f KHz", freq_hz / 1000.0f);
                        else
                            ImGui::TextColored(cyan, "%.2f Hz", freq_hz);
                    }
                    else
                    {
                        ImGui::TextColored(gray, "DC");
                    }

                    ImGui::Separator();
                    ImGui::TextColored(violet, "WAVE RAM");

                    char wave_text[64];
                    int pos = 0;
                    for (int i = 0; i < 16; i++)
                    {
                        pos += snprintf(wave_text + pos, 64 - pos, "%02X ", regs[0x20 + i]);
                        if (i == 7)
                        {
                            ImGui::TextColored(orange, "  %s", wave_text);
                            pos = 0;
                        }
                    }
                    ImGui::TextColored(orange, "  %s", wave_text);
                }
                else
                {
                    // Noise channel
                    int base = 3 * 5;
                    int env_init = (regs[base + 2] >> 4) & 0x0F;
                    int env_dir = (regs[base + 2] >> 3) & 0x01;
                    int env_pace = regs[base + 2] & 0x07;
                    bool dac_on = (regs[base + 2] & 0xF8) != 0;
                    int clock_shift = (regs[base + 3] >> 4) & 0x0F;
                    int lfsr_width = (regs[base + 3] >> 3) & 0x01;
                    int divisor_code = regs[base + 3] & 0x07;
                    bool length_en = (regs[base + 4] & 0x40) != 0;

                    static const int divisors[8] = { 8, 16, 32, 48, 64, 80, 96, 112 };

                    ImGui::TextColored(violet, "DAC         "); ImGui::SameLine();
                    ImGui::TextColored(dac_on ? green : gray, "%s", dac_on ? "ON" : "OFF");

                    ImGui::TextColored(violet, "VOLUME      "); ImGui::SameLine();
                    ImGui::TextColored(env_volume == 0 ? gray : white, "$%X", env_volume); ImGui::SameLine();
                    if (env_volume == 0)
                        ImGui::TextColored(gray, " (OFF)");
                    else if (env_volume == 15)
                        ImGui::TextColored(green, " (MAX)");
                    else
                        ImGui::TextColored(gray, " (%d/15)", env_volume);

                    ImGui::TextColored(violet, "ENVELOPE    "); ImGui::SameLine();
                    ImGui::TextColored(white, "Init:$%X %s Pace:%d", env_init, env_dir ? "UP" : "DN", env_pace);

                    ImGui::TextColored(violet, "CLK SHIFT   "); ImGui::SameLine();
                    ImGui::TextColored(white, "%d", clock_shift);

                    ImGui::TextColored(violet, "LFSR WIDTH  "); ImGui::SameLine();
                    ImGui::TextColored(white, "%s", lfsr_width ? "7-bit" : "15-bit");

                    ImGui::TextColored(violet, "DIVISOR     "); ImGui::SameLine();
                    ImGui::TextColored(white, "%d", divisor_code); ImGui::SameLine();
                    ImGui::TextColored(gray, " (r=%d)", divisors[divisor_code]);

                    ImGui::TextColored(violet, "LFSR        "); ImGui::SameLine();
                    ImGui::TextColored(white, "$%04X", ch_phase);

                    ImGui::TextColored(violet, "LENGTH      "); ImGui::SameLine();
                    ImGui::TextColored(length_en ? white : gray, "%d", length_ctr); ImGui::SameLine();
                    ImGui::TextColored(gray, " (%s)", length_en ? "ON" : "OFF");

                    ImGui::TextColored(violet, "OUTPUT HZ   "); ImGui::SameLine();
                    if (clock_shift <= 13)
                    {
                        float freq_hz = 262144.0f / (float)divisors[divisor_code] / (float)(1 << clock_shift);
                        if (freq_hz >= 1000.0f)
                            ImGui::TextColored(cyan, "%.2f KHz", freq_hz / 1000.0f);
                        else
                            ImGui::TextColored(cyan, "%.2f Hz", freq_hz);
                    }
                    else
                    {
                        ImGui::TextColored(gray, "N/A");
                    }
                }

                ImGui::Separator();

                int nr50 = regs[0x14];
                int nr51 = regs[0x15];
                int nr52 = regs[0x16];

                ImGui::TextColored(violet, "CHIP        "); ImGui::SameLine();
                ImGui::TextColored(white, "%s", is_cgb ? "CGB" : "DMG");

                ImGui::TextColored(violet, "MASTER      "); ImGui::SameLine();
                ImGui::TextColored((nr52 & 0x80) ? green : gray, "%s", (nr52 & 0x80) ? "ON" : "OFF");

                {
                    bool left = (nr51 >> (c + 4)) & 0x01;
                    bool right = (nr51 >> c) & 0x01;
                    int left_vol = (nr50 >> 4) & 0x07;
                    int right_vol = nr50 & 0x07;

                    ImGui::TextColored(violet, "PANNING     "); ImGui::SameLine();
                    ImGui::TextColored(left ? green : gray, "L:%s", left ? "ON " : "OFF"); ImGui::SameLine();
                    ImGui::TextColored(right ? green : gray, " R:%s", right ? "ON" : "OFF");

                    ImGui::TextColored(violet, "MASTER VOL  "); ImGui::SameLine();
                    ImGui::TextColored(white, "L:%d R:%d", left_vol, right_vol);
                }

                ImGui::PopFont();
                ImGui::EndTabItem();
            }
        }

        if (ImGui::BeginTabItem("Panning"))
        {
            ImGui::PushFont(gui_default_font);

            int nr50 = apu_state.regs[0x14];
            int nr51 = apu_state.regs[0x15];
            int nr52 = apu_state.regs[0x16];

            ImGui::TextColored(violet, "NR50  "); ImGui::SameLine();
            ImGui::TextColored(white, "$%02X", nr50);

            ImGui::TextColored(violet, "NR51  "); ImGui::SameLine();
            ImGui::TextColored(white, "$%02X", nr51);

            ImGui::TextColored(violet, "NR52  "); ImGui::SameLine();
            ImGui::TextColored(white, "$%02X", nr52); ImGui::SameLine();
            ImGui::TextColored((nr52 & 0x80) ? green : gray, " (%s)", (nr52 & 0x80) ? "ON" : "OFF");

            ImGui::Separator();

            for (int ch = 0; ch < 4; ch++)
            {
                bool left = (nr51 >> (ch + 4)) & 0x01;
                bool right = (nr51 >> ch) & 0x01;

                ImGui::TextColored(violet, "%-12s", k_channel_labels[ch]); ImGui::SameLine();
                ImGui::TextColored(left ? green : gray, "L:%s", left ? "ON " : "OFF"); ImGui::SameLine();
                ImGui::TextColored(right ? green : gray, " R:%s", right ? "ON " : "OFF");
            }

            ImGui::Separator();

            int left_vol = (nr50 >> 4) & 0x07;
            int right_vol = nr50 & 0x07;
            ImGui::TextColored(violet, "MASTER VOL  "); ImGui::SameLine();
            ImGui::TextColored(white, "L:%d R:%d", left_vol, right_vol);

            bool vin_left = (nr50 >> 7) & 0x01;
            bool vin_right = (nr50 >> 3) & 0x01;
            ImGui::TextColored(violet, "VIN         "); ImGui::SameLine();
            ImGui::TextColored(vin_left ? green : gray, "L:%s", vin_left ? "ON " : "OFF"); ImGui::SameLine();
            ImGui::TextColored(vin_right ? green : gray, " R:%s", vin_right ? "ON" : "OFF");

            ImGui::PopFont();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
    ImGui::PopStyleVar();
}