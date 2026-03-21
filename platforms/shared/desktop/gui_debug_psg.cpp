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

static const char* k_noise_rate_names[4] = { "N/512", "N/1024", "N/2048", "Tone 3" };

void gui_debug_psg_init(void)
{
    wave_buffer = new float[GS_AUDIO_BUFFER_SIZE];
}

void gui_debug_psg_destroy(void)
{
    SafeDeleteArray(wave_buffer);
}

void gui_debug_window_psg(void)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
    ImGui::SetNextWindowPos(ImVec2(180, 45), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(296, 316), ImGuiCond_FirstUseEver);
    ImGui::Begin("PSG", &config_debug.show_psg);

    GearsystemCore* core = emu_get_core();
    Audio* audio = core->GetAudio();

    Sms_Apu* psg = audio->GetPSG();
    Sms_Apu_State psg_state = psg->GetState();

    bool is_gg = core->GetCartridge()->IsGameGear();
    bool is_sg = core->GetCartridge()->IsSG1000() && !core->GetCartridge()->IsSG1000II();

    GS_RuntimeInfo runtime;
    core->GetRuntimeInfo(runtime);
    int master_clock = (runtime.region == Region_PAL) ? GS_MASTER_CLOCK_PAL : GS_MASTER_CLOCK_NTSC;

    if (ImGui::BeginTabBar("##psg_tabs", ImGuiTabBarFlags_None))
    {
        for (int c = 0; c < 4; c++)
        {
            Sms_Apu_State::Channel* ch = &psg_state.channels[c];

            char tab_name[32];
            snprintf(tab_name, 32, "%s", c < 3 ? "Tone" : "Noise");
            if (c < 3)
                snprintf(tab_name, 32, "Tone %d", c + 1);

            if (ImGui::BeginTabItem(tab_name))
            {
                ImGui::PushFont(gui_default_font);

                if (ImGui::BeginTable("##audio", 2, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoPadOuterX))
                {
                    ImGui::TableNextColumn();

                    ImGui::PushStyleColor(ImGuiCol_Text, *ch->mute ? mid_gray : white);
                    ImGui::PushFont(gui_material_icons_font);

                    char label[32];
                    snprintf(label, 32, "%s##mute%d", *ch->mute ? ICON_MD_MUSIC_OFF : ICON_MD_MUSIC_NOTE, c);
                    if (ImGui::Button(label))
                    {
                        for (int i = 0; i < 4; i++)
                            exclusive_channel[i] = false;
                        *ch->mute = !*ch->mute;
                    }
                    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                        ImGui::SetTooltip("Mute Channel");

                    snprintf(label, 32, "%s##exc%d", ICON_MD_STAR, c);

                    ImGui::PushStyleColor(ImGuiCol_Text, exclusive_channel[c] ? yellow : white);
                    if (ImGui::Button(label))
                    {
                        exclusive_channel[c] = !exclusive_channel[c];
                        *ch->mute = false;
                        for (int i = 0; i < 4; i++)
                        {
                            if (i != c)
                            {
                                exclusive_channel[i] = false;
                                *psg_state.channels[i].mute = exclusive_channel[c] ? true : false;
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
                            wave_buffer[i] = (float)ch_buf[i] / 32768.0f * 8.0f;
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

                ImGui::TextColored(violet, "ATTENUATION "); ImGui::SameLine();
                ImGui::TextColored(ch->volume_reg == 15 ? gray : white, "$%X", ch->volume_reg); ImGui::SameLine();
                if (ch->volume_reg == 15)
                    ImGui::TextColored(gray, " (OFF)");
                else if (ch->volume_reg == 0)
                    ImGui::TextColored(green, " (MAX)");
                else
                    ImGui::TextColored(gray, " (-%.1f dB)", ch->volume_reg * 2.0f);

                if (c < 3)
                {
                    int raw_period = ch->period >> 4;

                    ImGui::TextColored(violet, "PERIOD      "); ImGui::SameLine();
                    ImGui::TextColored(white, "$%03X", raw_period);

                    ImGui::TextColored(violet, "FREQ LOW    "); ImGui::SameLine();
                    ImGui::TextColored(white, "$%02X", raw_period & 0x0F);

                    ImGui::TextColored(violet, "FREQ HIGH   "); ImGui::SameLine();
                    ImGui::TextColored(white, "$%02X", (raw_period >> 4) & 0x3F);

                    ImGui::TextColored(violet, "PHASE       "); ImGui::SameLine();
                    ImGui::TextColored(white, "%d", ch->phase);

                    ImGui::TextColored(violet, "OUTPUT HZ   "); ImGui::SameLine();
                    if (raw_period > 0)
                    {
                        float freq_hz = (float)master_clock / (32.0f * raw_period);
                        if (freq_hz >= 1000.0f)
                            ImGui::TextColored(cyan, "%.2f KHz", freq_hz / 1000.0f);
                        else
                            ImGui::TextColored(cyan, "%.2f Hz", freq_hz);
                    }
                    else
                    {
                        ImGui::TextColored(gray, "DC");
                    }
                }
                else
                {
                    int noise_ctrl = psg_state.noise_rate | (psg_state.noise_white ? 0x04 : 0x00);

                    ImGui::TextColored(violet, "NOISE CTRL  "); ImGui::SameLine();
                    ImGui::TextColored(white, "$%02X", noise_ctrl);

                    ImGui::TextColored(violet, "TYPE        "); ImGui::SameLine();
                    ImGui::TextColored(psg_state.noise_white ? white : white, "%s", psg_state.noise_white ? "White" : "Periodic");

                    ImGui::TextColored(violet, "RATE        "); ImGui::SameLine();
                    ImGui::TextColored(white, "%s", k_noise_rate_names[psg_state.noise_rate]);

                    ImGui::TextColored(violet, "OUTPUT HZ   "); ImGui::SameLine();
                    if (psg_state.noise_rate < 3)
                    {
                        static const int noise_dividers[3] = { 512, 1024, 2048 };
                        float freq_hz = (float)master_clock / (float)noise_dividers[psg_state.noise_rate];
                        if (freq_hz >= 1000.0f)
                            ImGui::TextColored(cyan, "%.2f KHz", freq_hz / 1000.0f);
                        else
                            ImGui::TextColored(cyan, "%.2f Hz", freq_hz);
                    }
                    else
                    {
                        int ch3_raw = psg_state.channels[2].period >> 4;
                        if (ch3_raw > 0)
                        {
                            float freq_hz = (float)master_clock / (32.0f * ch3_raw);
                            ImGui::TextColored(cyan, "%.2f Hz", freq_hz); ImGui::SameLine();
                            ImGui::TextColored(gray, " (Tone 3)");
                        }
                        else
                            ImGui::TextColored(gray, "N/A");
                    }

                    ImGui::TextColored(violet, "LFSR        "); ImGui::SameLine();
                    ImGui::TextColored(white, "$%04X", psg_state.noise_shifter);

                    ImGui::TextColored(violet, "FEEDBACK    "); ImGui::SameLine();
                    ImGui::TextColored(white, "$%04X", psg_state.noise_feedback);
                }

                ImGui::Separator();

                ImGui::TextColored(violet, "CHIP        "); ImGui::SameLine();
                ImGui::TextColored(white, "%s", is_sg ? "SN76489" : "ASIC");

                int latch_ch = (psg_state.latch >> 5) & 3;
                bool latch_vol = (psg_state.latch & 0x10) != 0;
                ImGui::TextColored(violet, "LATCH       "); ImGui::SameLine();
                ImGui::TextColored(white, "$%02X", psg_state.latch); ImGui::SameLine();
                ImGui::TextColored(gray, " (CH%d %s)", latch_ch, latch_vol ? "VOL" : "TONE");

                {
                    int gg_flags = psg_state.ggstereo >> c;
                    bool right = is_gg && (gg_flags & 0x01) != 0;
                    bool left = is_gg && (gg_flags >> 4 & 0x01) != 0;

                    ImGui::TextColored(is_gg ? violet : mid_gray, "GG STEREO   "); ImGui::SameLine();
                    ImGui::TextColored(is_gg ? white : gray, "$%02X", psg_state.ggstereo);

                    ImGui::TextColored(is_gg ? violet : mid_gray, "OUTPUT      "); ImGui::SameLine();
                    ImGui::TextColored(left ? green : gray, "L:%s", left ? "ON " : "OFF"); ImGui::SameLine();
                    ImGui::TextColored(right ? green : gray, " R:%s", right ? "ON" : "OFF");
                }

                ImGui::PopFont();
                ImGui::EndTabItem();
            }
        }

        ImGui::BeginDisabled(!is_gg);
        if (ImGui::BeginTabItem("Stereo"))
        {
            ImGui::PushFont(gui_default_font);

            ImGui::TextColored(violet, "GG STEREO   "); ImGui::SameLine();
            ImGui::TextColored(white, "$%02X", psg_state.ggstereo);

            ImGui::Separator();

            for (int ch = 0; ch < 4; ch++)
            {
                int flags = psg_state.ggstereo >> ch;
                bool left = (flags >> 4 & 0x01) != 0;
                bool right = (flags & 0x01) != 0;

                ImGui::TextColored(violet, "%s:  ", ch < 3 ? "TONE " : "NOISE"); ImGui::SameLine();
                if (ch < 3)
                {
                    ImGui::TextColored(violet, "%d  ", ch + 1); ImGui::SameLine();
                }
                else
                {
                    ImGui::TextColored(violet, "   "); ImGui::SameLine();
                }
                ImGui::TextColored(left ? green : gray, "L:%s", left ? "ON " : "OFF"); ImGui::SameLine();
                ImGui::TextColored(right ? green : gray, " R:%s", right ? "ON " : "OFF");
            }

            ImGui::PopFont();
            ImGui::EndTabItem();
        }
        ImGui::EndDisabled();

        ImGui::EndTabBar();
    }

    ImGui::End();
    ImGui::PopStyleVar();
}