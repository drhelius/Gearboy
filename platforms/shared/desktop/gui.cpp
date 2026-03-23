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

#include <math.h>
#include "imgui.h"
#include "implot.h"
#include "fonts/RobotoMedium.h"
#include "fonts/MaterialIcons.h"
#include "fonts/IconsMaterialDesign.h"
#include "config.h"
#include "application.h"
#include "emu.h"
#include "ogl_renderer.h"
#include "utils.h"
#include "gearboy.h"

#define GUI_IMPORT
#include "gui.h"
#include "gui_menus.h"
#include "gui_popups.h"
#include "gui_actions.h"
#include "gui_debug_disassembler.h"
#include "gui_debug_memory.h"
#include "gui_debug.h"

static bool status_message_active = false;
static char status_message[4096] = "";
static Uint64 status_message_start_time = 0;
static Uint64 status_message_duration = 0;
static bool error_window_active = false;
static char error_message[4096] = "";
static bool loading_rom_active = false;
static char loading_rom_path[4096] = "";


static void main_window(void);
static void push_recent_rom(std::string path);
static void show_status_message(void);
static void show_error_window(void);
static void show_loading_popup(void);
static void finish_loading_rom(void);
static void set_style(void);
static ImVec4 lerp(const ImVec4& a, const ImVec4& b, float t);

Cartridge::CartridgeTypes gui_get_mbc(int index)
{
    switch (index)
    {
        case 0: return Cartridge::CartridgeNotSupported;
        case 1: return Cartridge::CartridgeNoMBC;
        case 2: return Cartridge::CartridgeMBC1;
        case 3: return Cartridge::CartridgeMBC2;
        case 4: return Cartridge::CartridgeMBC3;
        case 5: return Cartridge::CartridgeMBC5;
        case 6: return Cartridge::CartridgeMBC1Multi;
        default: return Cartridge::CartridgeNotSupported;
    }
}

bool gui_init(void)
{
    gui_main_window_width = 0;
    gui_main_window_height = 0;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGui::StyleColorsDark();
    ImGuiIO& io = ImGui::GetIO();

    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigDockingWithShift = true;
    io.IniFilename = config_imgui_file_path;

#if defined(__APPLE__) || defined(_WIN32)
    if (config_debug.multi_viewport)
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
#endif

    gui_roboto_font = io.Fonts->AddFontFromMemoryCompressedTTF(RobotoMedium_compressed_data, RobotoMedium_compressed_size, 17.0f, NULL, io.Fonts->GetGlyphRangesCyrillic());

    float iconFontSize = 20.0f;
    static const ImWchar icons_ranges[] = { ICON_MIN_MD, ICON_MAX_16_MD, 0 };
    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    icons_config.GlyphMinAdvanceX = iconFontSize;
    icons_config.GlyphOffset = { 0.0f, 5.0f };
    gui_material_icons_font = io.Fonts->AddFontFromMemoryCompressedTTF(MaterialIcons_compressed_data, MaterialIcons_compressed_size, iconFontSize, &icons_config, icons_ranges);

    ImFontConfig font_cfg;

    for (int i = 0; i < 4; i++)
    {
        font_cfg.SizePixels = (13.0f + (i * 3));
        gui_default_fonts[i] = io.Fonts->AddFontDefault(&font_cfg);
    }

    gui_default_font = gui_default_fonts[config_debug.font_size];

    set_style();

    emu_audio_volume(config_audio.enable ? 1.0f : 0.0f);

    strcpy(gui_dmg_bootrom_path, config_emulator.dmg_bootrom_path.c_str());
    strcpy(gui_gbc_bootrom_path, config_emulator.gbc_bootrom_path.c_str());

    if (strlen(gui_dmg_bootrom_path) > 0)
        emu_load_bootrom_dmg(gui_dmg_bootrom_path);
    if (strlen(gui_gbc_bootrom_path) > 0)
        emu_load_bootrom_gbc(gui_gbc_bootrom_path);

    emu_enable_bootrom_dmg(config_emulator.dmg_bootrom);
    emu_enable_bootrom_gbc(config_emulator.gbc_bootrom);
    emu_color_correction(config_video.color_correction);

    strcpy(gui_savefiles_path, config_emulator.savefiles_path.c_str());
    strcpy(gui_savestates_path, config_emulator.savestates_path.c_str());
    strcpy(gui_screenshots_path, config_emulator.screenshots_path.c_str());

    gui_debug_init();
    gui_init_menus();

    return true;
}

void gui_destroy(void)
{
    gui_debug_auto_save_settings();
    gui_debug_destroy();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
}

void gui_render(void)
{
    ImGui::NewFrame();

    if (config_debug.debug)
        ImGui::DockSpaceOverViewport();

    gui_in_use = gui_dialog_in_use;

    gui_main_menu();

    gui_main_window_hovered = false;

    if((!config_debug.debug && !emu_is_empty()) || (config_debug.debug && config_debug.show_screen))
        main_window();

    gui_debug_windows();

    if (config_emulator.show_info)
        gui_show_info();

    show_loading_popup();
    show_status_message();
    show_error_window();

    ImGui::Render();
}

void gui_shortcut(gui_ShortCutEvent event)
{
    switch (event)
    {  
    case gui_ShortcutOpenROM:
        gui_shortcut_open_rom = true;
        break;
    case gui_ShortcutReloadROM:
        if (config_debug.debug)
            gui_action_reload_rom();
        break;
    case gui_ShortcutReset:
        gui_action_reset();
        break;
    case gui_ShortcutPause:
        gui_action_pause();
        break;
    case gui_ShortcutFFWD:
        config_emulator.ffwd = !config_emulator.ffwd;
        gui_action_ffwd();
        break;
    case gui_ShortcutSaveState:
    {
        std::string message("Saving state to slot ");
        message += std::to_string(config_emulator.save_slot + 1);
        gui_set_status_message(message.c_str(), 3000);
        emu_save_state_slot(config_emulator.save_slot + 1);
        break;
    }
    case gui_ShortcutLoadState:
    {
        std::string message("Loading state from slot ");
        message += std::to_string(config_emulator.save_slot + 1);
        gui_set_status_message(message.c_str(), 3000);
        emu_load_state_slot(config_emulator.save_slot + 1);
        break;
    }
    case gui_ShortcutScreenshot:
        gui_action_save_screenshot(NULL);
        break;
    case gui_ShortcutDebugStepOver:
        if (config_debug.debug)
            emu_debug_step_over();
        break;
    case gui_ShortcutDebugStepInto:
        if (config_debug.debug)
            emu_debug_step_into();
        break;
    case gui_ShortcutDebugStepOut:
        if (config_debug.debug)
            emu_debug_step_out();
        break;
    case gui_ShortcutDebugStepFrame:
        if (config_debug.debug)
        {
            emu_debug_step_frame();
        }
        break;
    case gui_ShortcutDebugBreak:
        if (config_debug.debug)
            emu_debug_break();
        break;
    case gui_ShortcutDebugContinue:
        if (config_debug.debug)
            emu_debug_continue();
        break;
    case gui_ShortcutDebugRuntocursor:
        if (config_debug.debug)
            gui_debug_runtocursor();
        break;
    case gui_ShortcutDebugGoBack:
        if (config_debug.debug)
            gui_debug_go_back();
        break;
    case gui_ShortcutDebugBreakpoint:
        if (config_debug.debug)
            gui_debug_toggle_breakpoint();
        break;
    case gui_ShortcutDebugCopy:
        gui_debug_memory_copy();
        break;
    case gui_ShortcutDebugPaste:
        gui_debug_memory_paste();
        break;
    case gui_ShortcutDebugSelectAll:
        gui_debug_memory_select_all();
        break;
    case gui_ShortcutShowMainMenu:
        config_emulator.always_show_menu = !config_emulator.always_show_menu;
        break;
    default:
        break;
    }
}

void gui_load_rom(const char* path)
{
    if (loading_rom_active)
        return;

    gui_debug_auto_save_settings();
    push_recent_rom(path);
    emu_resume();

    strncpy(loading_rom_path, path, sizeof(loading_rom_path) - 1);
    loading_rom_path[sizeof(loading_rom_path) - 1] = '\0';
    loading_rom_active = true;

    emu_load_rom_async(path, config_emulator.force_dmg, gui_get_mbc(config_emulator.mbc), config_emulator.force_gba);
}

void gui_set_status_message(const char* message, Uint64 milliseconds)
{
    if (config_emulator.status_messages)
    {
        strcpy(status_message, message);
        status_message_active = true;
        status_message_start_time = SDL_GetTicks();
        status_message_duration = milliseconds;
    }
}

void gui_set_error_message(const char* message)
{
    strcpy(error_message, message);
    error_window_active = true;
}

static void main_window(void)
{
    int screen_width = GAMEBOY_WIDTH;
    int screen_height = GAMEBOY_HEIGHT;

    int w = (int)ImGui::GetIO().DisplaySize.x;
    int h = (int)ImGui::GetIO().DisplaySize.y - (application_show_menu ? gui_main_menu_height : 0);

    int selected_ratio = config_debug.debug ? 0 : config_video.ratio;
    float ratio = 0;

    switch (selected_ratio)
    {
        case 1:
            ratio = 4.0f / 3.0f;
            break;
        case 2:
            ratio = 16.0f / 9.0f;
            break;
        case 3:
            ratio = 16.0f / 10.0f;
            break;
        case 4:
            ratio = 6.0f / 5.0f;
            break;
        default:
            ratio = (float)screen_width / (float)screen_height;
    }

    if (!config_debug.debug && config_video.scale == 3)
    {
        ratio = (float)w / (float)h;
    }

    int base_width = (int)screen_width;
    int base_height = (int)(screen_height);

    int w_corrected, h_corrected;
    int scale_multiplier = 0;

    if (config_debug.debug)
    {
        scale_multiplier = config_debug.scale;
        w_corrected = base_width;
        h_corrected = base_height;
    }
    else
    {
        if (selected_ratio == 0)
        {
            w_corrected = base_width;
            h_corrected = base_height;
        }
        else
        {
            w_corrected = (int)round(base_height * ratio);
            h_corrected = base_height;
        }

        switch (config_video.scale)
        {
        case 0:
        {
            int factor_w = w / w_corrected;
            int factor_h = h / h_corrected;
            scale_multiplier = (factor_w < factor_h) ? factor_w : factor_h;
            break;
        }
        case 1:
            scale_multiplier = config_video.scale_manual;
            break;
        case 2:
            scale_multiplier = 1;
            h_corrected = h;
            w_corrected = (int)round(h * ratio);
            break;
        case 3:
            scale_multiplier = 1;
            w_corrected = w;
            h_corrected = h;
            break;
        default:
            scale_multiplier = 1;
            break;
        }
    }

    gui_main_window_width = w_corrected * scale_multiplier;
    gui_main_window_height = h_corrected * scale_multiplier;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar;
    
    if (config_debug.debug)
    {
        flags |= ImGuiWindowFlags_AlwaysAutoResize;

        ImGui::SetNextWindowPos(ImVec2(631, 26), ImGuiCond_FirstUseEver);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

        ImGui::Begin("Output###debug_output", &config_debug.show_screen, flags);
        gui_main_window_hovered = ImGui::IsWindowHovered();
    }
    else
    {
        int window_x = (w - (w_corrected * scale_multiplier)) / 2;
        int window_y = ((h - (h_corrected * scale_multiplier)) / 2) + (application_show_menu ? gui_main_menu_height : 0);

        ImGui::SetNextWindowSize(ImVec2((float)gui_main_window_width, (float)gui_main_window_height));
        ImGui::SetNextWindowPos(ImGui::GetMainViewport()->Pos + ImVec2((float)window_x, (float)window_y));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

        flags |= ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBringToFrontOnFocus;

        ImGui::Begin(GEARBOY_TITLE, 0, flags);
        gui_main_window_hovered = ImGui::IsWindowHovered();
    }

    float tex_h = (float)screen_width / (float)SYSTEM_TEXTURE_WIDTH;
    float tex_v = (float)screen_height / (float)SYSTEM_TEXTURE_HEIGHT;

    ImGui::Image((ImTextureID)(intptr_t)ogl_renderer_emu_texture, ImVec2((float)gui_main_window_width, (float)gui_main_window_height), ImVec2(0, 0), ImVec2(tex_h, tex_v));

    if (config_video.fps)
        gui_show_fps();

    ImGui::End();

    ImGui::PopStyleVar();
    ImGui::PopStyleVar();
    ImGui::PopStyleVar();
}

static void push_recent_rom(std::string path)
{
    int slot = 0;
    for (slot = 0; slot < config_max_recent_roms; slot++)
    {
        if (config_emulator.recent_roms[slot].compare(path) == 0)
        {
            break;
        }
    }

    slot = MIN(slot, config_max_recent_roms - 1);

    for (int i = slot; i > 0; i--)
    {
        config_emulator.recent_roms[i] = config_emulator.recent_roms[i - 1];
    }

    config_emulator.recent_roms[0] = path;
}

static void show_status_message(void)
{
    if (status_message_active)
    {
        Uint64 current_time = SDL_GetTicks();
        if ((current_time - status_message_start_time) > status_message_duration)
            status_message_active = false;
        else
            ImGui::OpenPopup("Status");
    }

    if (status_message_active)
    {
        ImGui::SetNextWindowPos(ImVec2(0.0f, application_show_menu ? gui_main_menu_height : 0.0f));
        ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, 0.0f));
        ImGui::SetNextWindowBgAlpha(0.9f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNav;

        if (ImGui::BeginPopup("Status", flags))
        {
            ImGui::PushFont(gui_default_font);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.1f,0.9f,0.1f,1.0f));
            ImGui::TextWrapped("%s", status_message);
            ImGui::PopStyleColor();
            ImGui::PopFont();
            ImGui::EndPopup();
        }

        ImGui::PopStyleVar();
    }
}

static void show_loading_popup(void)
{
    if (!loading_rom_active)
        return;

    if (!emu_is_rom_loading())
    {
        loading_rom_active = false;
        gui_dialog_in_use = false;
        bool success = emu_finish_rom_loading();

        if (success)
        {
            finish_loading_rom();
        }
        else
        {
            std::string message("Error loading ROM:\n");
            message += loading_rom_path;
            gui_set_error_message(message.c_str());
        }
        return;
    }

    gui_dialog_in_use = true;

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(30.0f, 20.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f, 12.0f));
    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.10f, 0.10f, 0.10f, 0.95f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.41f, 0.70f, 0.02f, 0.80f));
    ImGui::OpenPopup("##loading");

    if (ImGui::BeginPopupModal("##loading", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove))
    {
        ImGui::PushFont(gui_roboto_font);

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.41f, 0.70f, 0.02f, 1.0f));
        ImGui::TextUnformatted(ICON_MD_HOURGLASS_EMPTY);
        ImGui::PopStyleColor();

        ImGui::SameLine();
        ImGui::Text("LOADING...");

        ImGui::PopFont();
        ImGui::EndPopup();
    }

    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(3);
}

static void finish_loading_rom(void)
{
    gui_cheat_list.clear();
    emu_clear_cheats();

    gui_debug_reset();

    std::string str(loading_rom_path);
    str = str.substr(0, str.find_last_of("."));
    gui_debug_load_symbols_file((str + ".sym").c_str());
    gui_debug_load_symbols_file((str + ".noi").c_str());

    gui_debug_auto_load_settings();

    if (config_emulator.start_paused)
    {
        emu_pause();

        for (int i = 0; i < (GAMEBOY_WIDTH * GAMEBOY_HEIGHT); i++)
        {
            emu_frame_buffer[i].red = 0;
            emu_frame_buffer[i].green = 0;
            emu_frame_buffer[i].blue = 0;
        }
    }

    if (!emu_is_empty())
        application_update_title_with_rom(emu_get_core()->GetCartridge()->GetFileName());

    update_savestates_data();
}

static void show_error_window(void)
{
    if (error_window_active)
    {
        error_window_active = false;
        ImGui::OpenPopup("Error");
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("Error", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("%s\n\n", error_message);
        ImGui::Separator();
        if (ImGui::Button("OK"))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

/*
static Cartridge::CartridgeTypes get_mapper(int index)
{
    switch (index)
    {
        case 0:
            return Cartridge::CartridgeNotSupported;
        case 1:
            return Cartridge::CartridgeNoMBC;
        case 2:
            return Cartridge::CartridgeNotSupported;
        case 3:
            return Cartridge::CartridgeNotSupported;
        case 4:
            return Cartridge::CartridgeNotSupported;
        case 5:
            return Cartridge::CartridgeNotSupported;
        case 6:
            return Cartridge::CartridgeNotSupported;
        case 7:
            return Cartridge::CartridgeNotSupported;
        case 8:
            return Cartridge::CartridgeKorean2000XOR1FMapper;
        case 9:
            return Cartridge::CartridgeKoreanMSX32KB2000Mapper;
        case 10:
            return Cartridge::CartridgeKoreanMSXSMS8000Mapper;
        case 11:
            return Cartridge::CartridgeKoreanSMS32KB2000Mapper;
        case 12:
            return Cartridge::CartridgeKoreanMSX8KB0300Mapper;
        case 13:
            return Cartridge::CartridgeKorean0000XORFFMapper;
        case 14:
            return Cartridge::CartridgeKoreanFFFFHiComMapper;
        case 15:
            return Cartridge::CartridgeKoreanFFFEMapper;
        case 16:
            return Cartridge::CartridgeKoreanBFFCMapper;
        case 17:
            return Cartridge::CartridgeKoreanFFF3FFFCMapper;
        case 18:
            return Cartridge::CartridgeKoreanMDFFF5Mapper;
        case 19:
            return Cartridge::CartridgeKoreanMDFFF0Mapper;
        case 20:
            return Cartridge::CartridgeJumboDahjeeMapper;
        case 21:
            return Cartridge::CartridgeEeprom93C46Mapper;
        case 22:
            return Cartridge::CartridgeMulti4PAKAllActionMapper;
        case 23:
            return Cartridge::CartridgeIratahackMapper;
        default:
            return Cartridge::CartridgeNotSupported;
    }
}
*/

/*
static Cartridge::CartridgeZones get_zone(int index)
{
    switch (index)
    {
        case 0:
            return Cartridge::CartridgeUnknownZone;
        case 1:
            return Cartridge::CartridgeJapanSMS;
        case 2:
            return Cartridge::CartridgeExportSMS;
        case 3:
            return Cartridge::CartridgeJapanGG;
        case 4:
            return Cartridge::CartridgeExportGG;
        case 5:
            return Cartridge::CartridgeInternationalGG;
        default:
            return Cartridge::CartridgeUnknownZone;
    }
}
*/

/*
static Cartridge::CartridgeSystem get_system(int index)
{
    switch (index)
    {
        case 0:
            return Cartridge::CartridgeUnknownSystem;
        case 1:
            return Cartridge::CartridgeSMS;
        case 2:
            return Cartridge::CartridgeGG2ASIC;
        case 3:
            return Cartridge::CartridgeGG2ASICSMSMode;
        case 4:
            return Cartridge::CartridgeGG1ASIC;
        case 5:
            return Cartridge::CartridgeGG1ASICSMSMode;
        case 6:
            return Cartridge::CartridgeSG1000;
        case 7:
            return Cartridge::CartridgeSG1000II;
        default:
            return Cartridge::CartridgeUnknownSystem;
    }
}
*/

/*
static Cartridge::CartridgeRegions get_region(int index)
{
    //"Auto\0NTSC (60 Hz)\0PAL (50 Hz)\0\0");
    switch (index)
    {
        case 0:
            return Cartridge::CartridgeUnknownRegion;
        case 1:
            return Cartridge::CartridgeNTSC;
        case 2:
            return Cartridge::CartridgePAL;
        default:
            return Cartridge::CartridgeUnknownRegion;
    }
}
*/

static void set_style(void)
{
    ImGuiStyle& style = ImGui::GetStyle();

    style.Alpha = 1.0f;
    style.DisabledAlpha = 0.6000000238418579f;
    style.WindowPadding = ImVec2(8.0f, 8.0f);
    style.WindowRounding = 4.0f;
    style.WindowBorderSize = 1.0f;
    style.WindowMinSize = ImVec2(32.0f, 32.0f);
    style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
    style.WindowMenuButtonPosition = ImGuiDir_Left;
    style.ChildRounding = 0.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupRounding = 4.0f;
    style.PopupBorderSize = 1.0f;
    style.FramePadding = ImVec2(4.0f, 3.0f);
    style.FrameRounding = 2.5f;
    style.FrameBorderSize = 0.0f;
    style.ItemSpacing = ImVec2(8.0f, 4.0f);
    style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
    style.CellPadding = ImVec2(4.0f, 2.0f);
    style.IndentSpacing = 21.0f;
    style.ColumnsMinSpacing = 6.0f;
    style.ScrollbarSize = 11.0f;
    style.ScrollbarRounding = 2.5f;
    style.GrabMinSize = 10.0f;
    style.GrabRounding = 2.0f;
    style.TabRounding = 3.5f;
    style.TabBorderSize = 0.0f;
    style.TabCloseButtonMinWidthUnselected = 0.0f;
    style.ColorButtonPosition = ImGuiDir_Right;
    style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
    style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

    style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.5921568870544434f, 0.5921568870544434f, 0.5921568870544434f, 1.0f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.060085229575634f, 0.060085229575634f, 0.06008583307266235f, 1.0f);
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.05882352963089943f, 0.05882352963089943f, 0.05882352963089943f, 1.0f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.1176470592617989f, 0.1176470592617989f, 0.1176470592617989f, 1.0f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.1802574992179871f, 0.1802556961774826f, 0.1802556961774826f, 1.0f);
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.3058823645114899f, 0.3058823645114899f, 0.3058823645114899f, 1.0f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.1843137294054031f, 0.1843137294054031f, 0.1843137294054031f, 1.0f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.270386278629303f, 0.2703835666179657f, 0.2703848779201508f, 1.0f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.5215686274509804f, 0.43137254901960786f, 0.8156862745098039f, 1.0f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.5215686274509804f, 0.43137254901960786f, 0.8156862745098039f, 1.0f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.1176470592617989f, 0.1176470592617989f, 0.1176470592617989f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.1176470592617989f, 0.1176470592617989f, 0.1176470592617989f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.6266094446182251f, 0.6266031861305237f, 0.6266063451766968f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.9999899864196777f, 0.9999899864196777f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.9999899864196777f, 0.9999899864196777f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.5215686274509804f, 0.43137254901960786f, 0.8156862745098039f, 1.0f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.5215686274509804f, 0.43137254901960786f, 0.8156862745098039f, 1.0f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.5215686274509804f, 0.43137254901960786f, 0.8156862745098039f, 1.0f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.184547483921051f, 0.184547483921051f, 0.1845493316650391f, 1.0f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.5215686274509804f, 0.43137254901960786f, 0.8156862745098039f, 1.0f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.5215686274509804f, 0.43137254901960786f, 0.8156862745098039f, 1.0f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.1843137294054031f, 0.1843137294054031f, 0.1843137294054031f, 1.0f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.5215686274509804f, 0.43137254901960786f, 0.8156862745098039f, 1.0f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.5215686274509804f, 0.43137254901960786f, 0.8156862745098039f, 1.0f);
    style.Colors[ImGuiCol_Separator] = ImVec4(0.1803921610116959f, 0.1803921610116959f, 0.1803921610116959f, 1.0f);
    style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.1803921610116959f, 0.1803921610116959f, 0.1803921610116959f, 1.0f);
    style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.1803921610116959f, 0.1803921610116959f, 0.1803921610116959f, 1.0f);
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.2489270567893982f, 0.2489245682954788f, 0.2489245682954788f, 1.0f);
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.0f, 0.9999899864196777f, 0.9999899864196777f, 1.0f);
    style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.0f, 0.9999899864196777f, 0.9999899864196777f, 1.0f);
    style.Colors[ImGuiCol_Tab] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
    style.Colors[ImGuiCol_TabHovered] = ImVec4(0.5215686274509804f, 0.43137254901960786f, 0.8156862745098039f, 1.0f);
    style.Colors[ImGuiCol_TabActive] = ImVec4(0.5215686274509804f, 0.43137254901960786f, 0.8156862745098039f, 1.0f);
    style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
    style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.5215686274509804f, 0.43137254901960786f, 0.8156862745098039f, 1.0f);
    style.Colors[ImGuiCol_PlotLines] = ImVec4(0.5215686274509804f, 0.43137254901960786f, 0.8156862745098039f, 1.0f);
    style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.5215686274509804f, 0.43137254901960786f, 0.8156862745098039f, 1.0f);
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.5215686274509804f, 0.43137254901960786f, 0.8156862745098039f, 1.0f);
    style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.5215686274509804f, 0.43137254901960786f, 0.8156862745098039f, 1.0f);
    style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.1882352977991104f, 0.1882352977991104f, 0.2000000029802322f, 1.0f);
    style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.3098039329051971f, 0.3098039329051971f, 0.3490196168422699f, 1.0f);
    style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.2274509817361832f, 0.2274509817361832f, 0.2470588237047195f, 1.0f);
    style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.05999999865889549f);
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.5215686274509804f, 0.43137254901960786f, 0.8156862745098039f, 1.0f);
    style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
    style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
    style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.699999988079071f);
    style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.2000000029802322f);
    style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 0.7f);

    style.Colors[ImGuiCol_DockingPreview] = style.Colors[ImGuiCol_HeaderActive] * ImVec4(1.0f, 1.0f, 1.0f, 0.7f);
    style.Colors[ImGuiCol_DockingEmptyBg] = ImVec4(config_video.background_color_debugger[0], config_video.background_color_debugger[1], config_video.background_color_debugger[2], 1.00f);
    style.Colors[ImGuiCol_TabHovered] = style.Colors[ImGuiCol_HeaderHovered];
    //style.Colors[ImGuiCol_Tab] = lerp(style.Colors[ImGuiCol_Header], style.Colors[ImGuiCol_TitleBgActive], 0.80f);
    style.Colors[ImGuiCol_TabSelected] = lerp(style.Colors[ImGuiCol_HeaderActive], style.Colors[ImGuiCol_TitleBgActive], 0.60f);
    style.Colors[ImGuiCol_TabSelectedOverline] = style.Colors[ImGuiCol_HeaderActive];
    style.Colors[ImGuiCol_TabDimmed] = lerp(style.Colors[ImGuiCol_Tab], style.Colors[ImGuiCol_TitleBg], 0.80f);
    style.Colors[ImGuiCol_TabDimmedSelected] = lerp(style.Colors[ImGuiCol_TabSelected], style.Colors[ImGuiCol_TitleBg], 0.40f);
    style.Colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
}

static ImVec4 lerp(const ImVec4& a, const ImVec4& b, float t)
{
    return ImVec4(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t, a.w + (b.w - a.w) * t);
}
