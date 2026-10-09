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
#include "gui_cheats.h"
#include "gui_menus.h"
#include "gui_popups.h"
#include "gui_actions.h"
#include "gui_notifications.h"
#include "gui_colors.h"
#include "gui_debug_disassembler.h"
#include "gui_debug_memory.h"
#include "gui_debug.h"

static bool error_window_active = false;
static char error_message[4096] = "";
static bool loading_rom_active = false;
static char loading_rom_path[4096] = "";
static char loading_symbol_path[4096] = "";


static void main_window(void);
static void show_error_window(void);
static void show_loading_popup(void);
static void select_save_slot(int slot);
static bool finish_loading_rom(void);
static void update_window_visibility_padding(void);
static ImVec2 snap_to_physical_pixel(const ImVec2& pos);
static void set_style(void);
static void set_style_light(ImGuiStyle& style);
static void set_style_dark(ImGuiStyle& style);
static ImVec4 make_color(unsigned int rgb, float alpha = 1.0f);
static ImVec4 lerp(const ImVec4& a, const ImVec4& b, float t);


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

    gui_apply_settings();

    if (strlen(gui_dmg_bootrom_path) > 0)
        emu_load_bootrom_dmg(gui_dmg_bootrom_path);
    if (strlen(gui_gbc_bootrom_path) > 0)
        emu_load_bootrom_gbc(gui_gbc_bootrom_path);

    gui_debug_init();
    gui_cheats_init();
    gui_init_menus();

    return true;
}

void gui_apply_settings(void)
{
    gui_default_font = gui_default_fonts[config_debug.font_size];
    set_style();

    emu_audio_sync = config_audio.sync;
    emu_audio_mute(!config_audio.enable);
    emu_audio_set_master_volume(config_audio.master_volume);

    emu_enable_bootrom_dmg(config_emulator.dmg_bootrom);
    emu_enable_bootrom_gbc(config_emulator.gbc_bootrom);
    emu_color_correction(config_video.color_correction);
    emu_video_no_sprite_limit(config_video.sprite_limit);
    emu_set_disassembler_syntax(config_debug.dis_syntax);
    emu_set_barcode_boy_mode(config_emulator.barcode_boy_mode);
    emu_link_cable_set_normal_barrier_stall_us((u32)config_emulator.link_cable_stall_us);

    strncpy_fit(gui_savefiles_path, config_emulator.savefiles_path.c_str(), sizeof(gui_savefiles_path));
    strncpy_fit(gui_savestates_path, config_emulator.savestates_path.c_str(), sizeof(gui_savestates_path));
    strncpy_fit(gui_screenshots_path, config_emulator.screenshots_path.c_str(), sizeof(gui_screenshots_path));
    strncpy_fit(gui_video_recordings_path, config_emulator.video_recordings_path.c_str(), sizeof(gui_video_recordings_path));
    strncpy_fit(gui_dmg_bootrom_path, config_emulator.dmg_bootrom_path.c_str(), sizeof(gui_dmg_bootrom_path));
    strncpy_fit(gui_gbc_bootrom_path, config_emulator.gbc_bootrom_path.c_str(), sizeof(gui_gbc_bootrom_path));
    strncpy_fit(gui_mcp_http_address, config_emulator.mcp_http_address.c_str(), sizeof(gui_mcp_http_address));
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

    update_window_visibility_padding();

    if (config_debug.debug)
        ImGui::DockSpaceOverViewport();

    gui_in_use = gui_dialog_in_use;

    gui_main_menu();

    gui_main_window_hovered = false;

    if((!config_debug.debug && !emu_is_empty()) || (config_debug.debug && config_debug.show_screen))
        main_window();

    gui_debug_windows();
    gui_cheats_window();

    if (config_emulator.show_info)
        gui_show_info();

    show_loading_popup();
    gui_notifications_render();
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
    case gui_ShortcutMute:
        config_audio.enable = !config_audio.enable;
        emu_audio_mute(!config_audio.enable);
        gui_notify(gui_NotificationInfo, config_audio.enable ? ICON_MD_VOLUME_UP : ICON_MD_VOLUME_OFF,
            config_audio.enable ? "Audio unmuted" : "Audio muted", NULL, "mute", 1500);
        break;
    case gui_ShortcutSaveState:
        gui_action_save_state(NULL);
        break;
    case gui_ShortcutLoadState:
        gui_action_load_state(NULL);
        break;
    case gui_ShortcutSelectSlot1:
        select_save_slot(0);
        break;
    case gui_ShortcutSelectSlot2:
        select_save_slot(1);
        break;
    case gui_ShortcutSelectSlot3:
        select_save_slot(2);
        break;
    case gui_ShortcutSelectSlot4:
        select_save_slot(3);
        break;
    case gui_ShortcutSelectSlot5:
        select_save_slot(4);
        break;
    case gui_ShortcutScreenshot:
        gui_action_save_screenshot(NULL);
        break;
    case gui_ShortcutVideoRecording:
        gui_action_toggle_video_recording();
        break;
    case gui_ShortcutFullscreen:
        config_emulator.fullscreen = !config_emulator.fullscreen;
        application_trigger_fullscreen(config_emulator.fullscreen);
        break;
    case gui_ShortcutCaptureMouse:
        config_emulator.capture_mouse = !config_emulator.capture_mouse;
        gui_notify(gui_NotificationInfo, ICON_MD_MOUSE,
            config_emulator.capture_mouse ? "Mouse captured" : "Mouse released", NULL, "mouse", 1500);
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
    case gui_ShortcutScanBarcode:
        gui_shortcut_scan_barcode = true;
        break;
    default:
        break;
    }
}

bool gui_load_rom(const char* path, const char* symbol_path)
{
    if (loading_rom_active)
        return false;

    gui_debug_auto_save_settings();
    config_push_recent_media(path);
    emu_resume();

    strncpy(loading_rom_path, path, sizeof(loading_rom_path) - 1);
    loading_rom_path[sizeof(loading_rom_path) - 1] = '\0';
    if (IsValidPointer(symbol_path) && (strlen(symbol_path) > 0))
    {
        strncpy(loading_symbol_path, symbol_path, sizeof(loading_symbol_path) - 1);
        loading_symbol_path[sizeof(loading_symbol_path) - 1] = '\0';
    }
    else
        loading_symbol_path[0] = '\0';
    loading_rom_active = true;

    emu_load_rom_async(path, config_emulator.force_dmg, gui_get_mbc(config_emulator.mbc), config_emulator.force_gba);

    return true;
}

bool gui_is_rom_loading(void)
{
    return loading_rom_active;
}

bool gui_finish_loading_rom(void)
{
    if (!loading_rom_active || emu_is_rom_loading())
        return false;

    loading_rom_active = false;
    gui_dialog_in_use = false;
    bool success = emu_finish_rom_loading();

    if (success)
        success = finish_loading_rom();
    else
    {
        std::string message("Error loading ROM:\n");
        message += loading_rom_path;
        gui_set_error_message(message.c_str());
    }

    return success;
}

void gui_set_error_message(const char* message)
{
    strncpy_fit(error_message, message, sizeof(error_message));
    error_window_active = true;
}

void gui_set_style(void)
{
    set_style();
}

static void update_window_visibility_padding(void)
{
    static bool initialized = false;
    static ImVec2 previous_work_size(0.0f, 0.0f);

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    if (!viewport || viewport->WorkSize.x <= 0.0f || viewport->WorkSize.y <= 0.0f)
        return;

    ImGuiStyle& style = ImGui::GetStyle();
    bool viewport_shrank = initialized && ((viewport->WorkSize.x < previous_work_size.x) || (viewport->WorkSize.y < previous_work_size.y));

    style.DisplayWindowPadding = viewport_shrank ? ImVec2(100.0f, 70.0f) : ImVec2(19.0f, 19.0f);

    previous_work_size = viewport->WorkSize;
    initialized = true;
}

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
        case 7: return Cartridge::CartridgeHuC1;
        case 8: return Cartridge::CartridgeHuC3;
        case 9: return Cartridge::CartridgeMMM01;
        case 10: return Cartridge::CartridgeCamera;
        case 11: return Cartridge::CartridgeMBC7;
        case 12: return Cartridge::CartridgeTAMA5;
        case 13: return Cartridge::CartridgeWisdomTree;
        case 14: return Cartridge::CartridgeM161;
        case 15: return Cartridge::CartridgeSachenMMC1;
        case 16: return Cartridge::CartridgeSachenMMC2;
        case 17: return Cartridge::CartridgePKJD;
        case 18: return Cartridge::CartridgeBungEMS;
        case 19: return Cartridge::CartridgePoke2in1;
        case 20: return Cartridge::CartridgeMBC6;
        case 21: return Cartridge::CartridgeRocket;
        case 22: return Cartridge::CartridgeBHGOS;
        case 23: return Cartridge::CartridgeLiCheng;
        case 24: return Cartridge::CartridgeNTNew;
        case 25: return Cartridge::CartridgeGGB81;
        case 26: return Cartridge::CartridgeHitek;
        case 27: return Cartridge::CartridgeVF001;
        case 28: return Cartridge::CartridgeVF001A;
        case 29: return Cartridge::CartridgeSintax;
        case 30: return Cartridge::CartridgeNTOld1;
        case 31: return Cartridge::CartridgeNTOld2;
        default: return Cartridge::CartridgeNotSupported;
    }
}

static void main_window(void)
{
    int screen_width = GAMEBOY_WIDTH;
    int screen_height = GAMEBOY_HEIGHT;

    GearboyCore* core = emu_get_core();
    if (IsValidPointer(core))
    {
        GB_RuntimeInfo rt_info;
        core->GetRuntimeInfo(rt_info);
        screen_width = rt_info.screen_width;
        screen_height = rt_info.screen_height;
    }

    ImGuiIO& io = ImGui::GetIO();

    float framebuffer_scale_x = io.DisplayFramebufferScale.x;
    float framebuffer_scale_y = io.DisplayFramebufferScale.y;

    if (framebuffer_scale_x <= 0.0f)
        framebuffer_scale_x = 1.0f;
    if (framebuffer_scale_y <= 0.0f)
        framebuffer_scale_y = 1.0f;

    float logical_w = io.DisplaySize.x;
    float logical_h = io.DisplaySize.y - (application_show_menu ? (float)gui_main_menu_height : 0.0f);
    int w = (int)logical_w;
    int h = (int)logical_h;
    int physical_w = (int)floorf(logical_w * framebuffer_scale_x);
    int physical_h = (int)floorf(logical_h * framebuffer_scale_y);

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
        default:
            ratio = (float)screen_width / (float)screen_height;
    }

    if (!config_debug.debug && config_video.scale == 3)
    {
        ratio = logical_w / logical_h;
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
            int factor_w = physical_w / w_corrected;
            int factor_h = physical_h / h_corrected;
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

        if (config_video.scale <= 1)
        {
            if (scale_multiplier < 1)
                scale_multiplier = 1;
        }
    }

    float image_w = (float)(w_corrected * scale_multiplier);
    float image_h = (float)(h_corrected * scale_multiplier);

    if (config_debug.debug || config_video.scale <= 1)
    {
        image_w /= framebuffer_scale_x;
        image_h /= framebuffer_scale_y;
    }

    int image_logical_width = (int)ceilf(image_w);
    int image_logical_height = (int)ceilf(image_h);
    int image_physical_width = (int)roundf(image_w * framebuffer_scale_x);
    int image_physical_height = (int)roundf(image_h * framebuffer_scale_y);

    if (image_physical_width < 1)
        image_physical_width = 1;
    if (image_physical_height < 1)
        image_physical_height = 1;

    gui_main_window_width = image_logical_width;
    gui_main_window_height = image_logical_height;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar;
    bool window_visible = false;

    if (config_debug.debug)
    {
        flags |= ImGuiWindowFlags_AlwaysAutoResize;

        ImGui::SetNextWindowPos(ImVec2(631, 26), ImGuiCond_FirstUseEver);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

        window_visible = ImGui::Begin("Output###debug_output", &config_debug.show_screen, flags);
        gui_main_window_hovered = ImGui::IsWindowHovered();
    }
    else
    {
        float window_x = (logical_w - image_w) * 0.5f;
        float window_y = ((logical_h - image_h) * 0.5f) + (application_show_menu ? (float)gui_main_menu_height : 0.0f);

        window_x = roundf(window_x * framebuffer_scale_x) / framebuffer_scale_x;
        window_y = roundf(window_y * framebuffer_scale_y) / framebuffer_scale_y;

        ImGui::SetNextWindowSize(ImVec2(image_w, image_h));
        ImGui::SetNextWindowPos(ImGui::GetMainViewport()->Pos + ImVec2(window_x, window_y));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

        flags |= ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBringToFrontOnFocus;

        window_visible = ImGui::Begin(GEARBOY_TITLE, 0, flags);
        gui_main_window_hovered = ImGui::IsWindowHovered();
    }

    OglRendererScreenGeometry screen_geometry;
    screen_geometry.physical_width = image_physical_width;
    screen_geometry.physical_height = image_physical_height;
    screen_geometry.framebuffer_scale_x = framebuffer_scale_x;
    screen_geometry.framebuffer_scale_y = framebuffer_scale_y;
    ogl_renderer_set_screen_geometry(&screen_geometry);

    float tex_h = 1.0f;
    float tex_v = 1.0f;
    ogl_renderer_get_screen_uv(&tex_h, &tex_v);

    ImVec2 image_size(image_w, image_h);
    ImVec2 layout_max = ImGui::GetCursorScreenPos() + image_size;
    ImVec2 image_min = snap_to_physical_pixel(ImGui::GetCursorScreenPos());
    ImVec2 image_max = image_min + image_size;

    ImGui::Dummy(image_size);

    if (window_visible)
    {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 clip_min = draw_list->GetClipRectMin();
        ImVec2 clip_max = draw_list->GetClipRectMax();

        if (!ImGui::IsWindowDocked())
        {
            if (clip_max.x > layout_max.x - 1.0f)
                clip_max.x = fmaxf(clip_max.x, image_max.x);
            if (clip_max.y > layout_max.y - 1.0f)
                clip_max.y = fmaxf(clip_max.y, image_max.y);
        }

        draw_list->PushClipRect(clip_min, clip_max, false);
        draw_list->AddImage((ImTextureID)(intptr_t)ogl_renderer_get_screen_texture(), image_min, image_max, ImVec2(0, 0), ImVec2(tex_h, tex_v));
        draw_list->PopClipRect();
        gui_notifications_render_output(image_min, image_max);
    }

    if (config_video.fps)
        gui_show_fps();

    ImGui::End();

    ImGui::PopStyleVar();
    ImGui::PopStyleVar();
    ImGui::PopStyleVar();
}

static ImVec2 snap_to_physical_pixel(const ImVec2& pos)
{
    ImGuiViewport* viewport = ImGui::GetWindowViewport();
    ImVec2 scale = viewport->FramebufferScale;

    if (scale.x <= 0.0f || scale.y <= 0.0f)
        scale = ImGui::GetIO().DisplayFramebufferScale;

    float x = ceilf(((pos.x - viewport->Pos.x) * scale.x) - 0.5f - 0.01f);
    float y = ceilf(((pos.y - viewport->Pos.y) * scale.y) - 0.5f - 0.01f);

    return ImVec2(viewport->Pos.x + (x / scale.x), viewport->Pos.y + (y / scale.y));
}

static void show_loading_popup(void)
{
    if (!loading_rom_active)
        return;

    if (!emu_is_rom_loading())
    {
        gui_finish_loading_rom();
        return;
    }

    gui_dialog_in_use = true;

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    const ImGuiStyle& style = ImGui::GetStyle();
    ImVec4 loading_highlight = style.Colors[ImGuiCol_HeaderHovered];
    ImVec4 loading_background = style.Colors[ImGuiCol_PopupBg];
    ImVec4 loading_border = loading_highlight;
    loading_background.w = 0.95f;
    loading_border.w = 0.80f;

    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(30.0f, 20.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f, 12.0f));
    ImGui::PushStyleColor(ImGuiCol_PopupBg, loading_background);
    ImGui::PushStyleColor(ImGuiCol_Border, loading_border);
    ImGui::OpenPopup("##loading");

    if (ImGui::BeginPopupModal("##loading", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove))
    {
        ImGui::PushFont(gui_roboto_font);

        ImGui::PushStyleColor(ImGuiCol_Text, loading_highlight);
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

static void select_save_slot(int slot)
{
    config_emulator.save_slot = slot;

    char message[32];
    snprintf(message, sizeof(message), "Save slot %d selected", slot + 1);
    gui_notify(gui_NotificationInfo, ICON_MD_SAVE, message, NULL, "slot", 1500);
}

static bool finish_loading_rom(void)
{
    gui_cheats_clear();

    gui_debug_reset();

    if (loading_symbol_path[0] != '\0')
        gui_debug_load_symbols_file(loading_symbol_path);
    else
    {
        std::string str(loading_rom_path);
        str = str.substr(0, str.find_last_of("."));
        gui_debug_load_symbols_file((str + ".sym").c_str());
        gui_debug_load_symbols_file((str + ".noi").c_str());
    }

    gui_debug_auto_load_settings();

    if (config_emulator.start_paused)
    {
        emu_pause();

        for (int i = 0; i < (SGB_SCREEN_WIDTH * SGB_SCREEN_HEIGHT); i++)
        {
            emu_frame_buffer[i].red = 0;
            emu_frame_buffer[i].green = 0;
            emu_frame_buffer[i].blue = 0;
        }
    }

    if (!emu_is_empty())
    {
        application_update_title_with_rom(emu_get_core()->GetCartridge()->GetFileName());
        gui_notify(gui_NotificationSuccess, ICON_MD_VIDEOGAME_ASSET, "ROM loaded",
            emu_get_core()->GetCartridge()->GetFileName());
    }

    update_savestates_data();

    return true;
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

    if (config_emulator.theme == config_Theme_Light)
        set_style_light(style);
    else
        set_style_dark(style);
}

static void set_style_light(ImGuiStyle& style)
{
    ImGui::StyleColorsLight();

    const ImVec4 text = make_color(0x21201C);
    const ImVec4 text_secondary = make_color(0x63635E);
    const ImVec4 window = make_color(0xF9F9F8);
    const ImVec4 child = make_color(0xFDFDFC);
    const ImVec4 popup = make_color(0xFDFDFC);
    const ImVec4 control = make_color(0xF1F0EF);
    const ImVec4 neutral_active = make_color(0xE2E1DE);
    const ImVec4 border_subtle = make_color(0xDAD9D6);
    const ImVec4 border = make_color(0xCFCECA);
    const ImVec4 grab = make_color(0xBCBBB5);
    const ImVec4 grab_hover = make_color(0x82827C);
    const ImVec4 slider_grab = make_color(0x8D8D86);
    const ImVec4 accent_soft = make_color(0xEEE7FF);
    const ImVec4 accent_hover = make_color(0xEBE4FF);
    const ImVec4 accent_selected = make_color(0xE1D9FF);
    const ImVec4 accent_edge = make_color(0xAA99EC);
    const ImVec4 accent_ink = make_color(0x6550B9);
    const ImVec4 accent_strong = make_color(0x2F265F);
    const ImVec4 accent_bright = make_color(0x856ED0);

    style.Colors[ImGuiCol_Text] = text;
    style.Colors[ImGuiCol_TextDisabled] = text_secondary;
    style.Colors[ImGuiCol_WindowBg] = window;
    style.Colors[ImGuiCol_ChildBg] = child;
    style.Colors[ImGuiCol_PopupBg] = popup;
    style.Colors[ImGuiCol_Border] = border;
    style.Colors[ImGuiCol_BorderShadow] = make_color(0x000000, 0.0f);
    style.Colors[ImGuiCol_FrameBg] = control;
    style.Colors[ImGuiCol_FrameBgHovered] = accent_soft;
    style.Colors[ImGuiCol_FrameBgActive] = accent_selected;
    style.Colors[ImGuiCol_TitleBg] = control;
    style.Colors[ImGuiCol_TitleBgActive] = accent_bright;
    style.Colors[ImGuiCol_TitleBgCollapsed] = control;
    style.Colors[ImGuiCol_MenuBarBg] = window;
    style.Colors[ImGuiCol_ScrollbarBg] = window;
    style.Colors[ImGuiCol_ScrollbarGrab] = grab;
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = grab_hover;
    style.Colors[ImGuiCol_ScrollbarGrabActive] = accent_ink;
    style.Colors[ImGuiCol_CheckMark] = accent_ink;
    style.Colors[ImGuiCol_SliderGrab] = slider_grab;
    style.Colors[ImGuiCol_SliderGrabActive] = accent_ink;
    style.Colors[ImGuiCol_Button] = control;
    style.Colors[ImGuiCol_ButtonHovered] = accent_soft;
    style.Colors[ImGuiCol_ButtonActive] = accent_selected;
    style.Colors[ImGuiCol_Header] = control;
    style.Colors[ImGuiCol_HeaderHovered] = accent_soft;
    style.Colors[ImGuiCol_HeaderActive] = accent_selected;
    style.Colors[ImGuiCol_Separator] = border_subtle;
    style.Colors[ImGuiCol_SeparatorHovered] = accent_ink;
    style.Colors[ImGuiCol_SeparatorActive] = accent_ink;
    style.Colors[ImGuiCol_ResizeGrip] = make_color(0xBCBBB5, 0.55f);
    style.Colors[ImGuiCol_ResizeGripHovered] = make_color(0xAA99EC, 0.80f);
    style.Colors[ImGuiCol_ResizeGripActive] = make_color(0x6550B9, 0.95f);
    style.Colors[ImGuiCol_Tab] = control;
    style.Colors[ImGuiCol_TabHovered] = accent_soft;
    style.Colors[ImGuiCol_TabActive] = accent_selected;
    style.Colors[ImGuiCol_TabUnfocused] = window;
    style.Colors[ImGuiCol_TabUnfocusedActive] = neutral_active;
    style.Colors[ImGuiCol_PlotLines] = accent_ink;
    style.Colors[ImGuiCol_PlotLinesHovered] = accent_strong;
    style.Colors[ImGuiCol_PlotHistogram] = accent_ink;
    style.Colors[ImGuiCol_PlotHistogramHovered] = accent_strong;
    style.Colors[ImGuiCol_TableHeaderBg] = control;
    style.Colors[ImGuiCol_TableBorderStrong] = border;
    style.Colors[ImGuiCol_TableBorderLight] = border_subtle;
    style.Colors[ImGuiCol_TableRowBg] = make_color(0x000000, 0.0f);
    style.Colors[ImGuiCol_TableRowBgAlt] = make_color(0x21201C, 0.03f);
    style.Colors[ImGuiCol_TextSelectedBg] = accent_hover;
    style.Colors[ImGuiCol_DragDropTarget] = accent_bright;
    style.Colors[ImGuiCol_NavHighlight] = make_color(0x856ED0, 0.90f);
    style.Colors[ImGuiCol_NavWindowingHighlight] = make_color(0x21201C, 0.70f);
    style.Colors[ImGuiCol_NavWindowingDimBg] = make_color(0x21201C, 0.20f);
    style.Colors[ImGuiCol_ModalWindowDimBg] = make_color(0x21201C, 0.30f);

    style.Colors[ImGuiCol_DockingPreview] = make_color(0x856ED0, 0.45f);
    style.Colors[ImGuiCol_DockingEmptyBg] = ImVec4(
        config_video.background_color_debugger[config_Theme_Light][0],
        config_video.background_color_debugger[config_Theme_Light][1],
        config_video.background_color_debugger[config_Theme_Light][2], 1.0f);
    style.Colors[ImGuiCol_TabSelected] = accent_selected;
    style.Colors[ImGuiCol_TabSelectedOverline] = accent_bright;
    style.Colors[ImGuiCol_TabDimmed] = window;
    style.Colors[ImGuiCol_TabDimmedSelected] = control;
    style.Colors[ImGuiCol_TabDimmedSelectedOverline] = accent_edge;
}

static void set_style_dark(ImGuiStyle& style)
{
    ImGui::StyleColorsDark();

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
    style.Colors[ImGuiCol_DockingEmptyBg] = ImVec4(config_video.background_color_debugger[config_emulator.theme][0], config_video.background_color_debugger[config_emulator.theme][1], config_video.background_color_debugger[config_emulator.theme][2], 1.00f);
    style.Colors[ImGuiCol_TabHovered] = style.Colors[ImGuiCol_HeaderHovered];
    //style.Colors[ImGuiCol_Tab] = lerp(style.Colors[ImGuiCol_Header], style.Colors[ImGuiCol_TitleBgActive], 0.80f);
    style.Colors[ImGuiCol_TabSelected] = lerp(style.Colors[ImGuiCol_HeaderActive], style.Colors[ImGuiCol_TitleBgActive], 0.60f);
    style.Colors[ImGuiCol_TabSelectedOverline] = style.Colors[ImGuiCol_HeaderActive];
    style.Colors[ImGuiCol_TabDimmed] = lerp(style.Colors[ImGuiCol_Tab], style.Colors[ImGuiCol_TitleBg], 0.80f);
    style.Colors[ImGuiCol_TabDimmedSelected] = lerp(style.Colors[ImGuiCol_TabSelected], style.Colors[ImGuiCol_TitleBg], 0.40f);
    style.Colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
}

static ImVec4 make_color(unsigned int rgb, float alpha)
{
    return ImVec4(((rgb >> 16) & 0xFF) / 255.0f, ((rgb >> 8) & 0xFF) / 255.0f, (rgb & 0xFF) / 255.0f, alpha);
}

static ImVec4 lerp(const ImVec4& a, const ImVec4& b, float t)
{
    return ImVec4(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t, a.w + (b.w - a.w) * t);
}
