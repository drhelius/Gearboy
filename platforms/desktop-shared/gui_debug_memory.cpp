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

#define GUI_DEBUG_MEMORY_IMPORT
#include "gui_debug_memory.h"

#include "../../src/gearboy.h"
#include "imgui/imgui.h"
#include "gui_debug_memeditor.h"
#include "gui_debug_constants.h"
#include "config.h"
#include "gui.h"
#include "emu.h"
#include "nfd/nfd.h"
#include "nfd/nfd_sdl2.h"
#include "application.h"

static MemEditor mem_edit[MEMORY_EDITOR_MAX];
static int mem_edit_select = -1;
static int current_mem_edit = 0;
static char set_value_buffer[5] = {0};

static void memory_editor_menu(void);
static void draw_tabs(void);
static void file_dialog_save_memory_dump(bool binary);

void gui_debug_memory_init(void)
{
    gui_debug_memory_reset();
}

void gui_debug_memory_reset(void)
{
    GearboyCore* core = emu_get_core();
    Memory* memory = core->GetMemory();

    mem_edit[0].Reset("ROM0", memory->GetROM0(), 0x4000);
    mem_edit[1].Reset("ROM1", memory->GetROM1(), 0x4000, 0x4000);
    mem_edit[2].Reset("VRAM", memory->GetVRAM(), 0x2000, 0x8000);
    mem_edit[3].Reset("RAM", memory->GetRAM(), 0x2000, 0xA000);
    mem_edit[4].Reset("WRAM0", memory->GetWRAM0(), 0x1000, 0xC000);
    mem_edit[5].Reset("WRAM1", memory->GetWRAM1(), 0x1000, 0xD000);
    mem_edit[6].Reset("WRAM", memory->GetWRAM0(), 0x2000, 0xC000);
    mem_edit[7].Reset("OAM", memory->GetMemoryMap() + 0xFE00, 0x00A0, 0xFE00);
    mem_edit[8].Reset("IO", memory->GetMemoryMap() + 0xFF00, 0x0080, 0xFF00);
    mem_edit[9].Reset("HIRAM", memory->GetMemoryMap() + 0xFF80, 0x007F, 0xFF80);
}

void gui_debug_window_memory(void)
{
    for (int i = 0; i < MEMORY_EDITOR_MAX; i++)
    {
        mem_edit[i].SetGuiFont(gui_roboto_font);
        mem_edit[i].WatchPopup();
        mem_edit[i].BookMarkPopup();
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
    ImGui::SetNextWindowPos(ImVec2(60, 60), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(670, 330), ImGuiCond_FirstUseEver);

    ImGui::Begin("Memory Editor", &config_debug.show_memory, ImGuiWindowFlags_MenuBar);

    memory_editor_menu();

    GearboyCore* core = emu_get_core();
    Memory* memory = core->GetMemory();

    ImGui::PushFont(gui_default_font);
    ImGui::TextColored(magenta, "  BANKS: ");ImGui::SameLine();
    ImGui::TextColored(cyan, "ROM1");ImGui::SameLine();
    ImGui::Text("$%02X", memory->GetCurrentRule()->GetCurrentRomBank1Index()); ImGui::SameLine();
    ImGui::TextColored(cyan, "  RAM");ImGui::SameLine();
    ImGui::Text("$%02X", memory->GetCurrentRule()->GetCurrentRamBankIndex()); ImGui::SameLine();
    ImGui::TextColored(cyan, "  WRAM1");ImGui::SameLine();
    ImGui::Text("$%02X", memory->GetCurrentCGBRAMBank()); ImGui::SameLine();
    ImGui::TextColored(cyan, "  VRAM");ImGui::SameLine();
    ImGui::Text("$%02X", memory->GetCurrentLCDRAMBank());
    ImGui::PopFont();

    if (ImGui::BeginTabBar("##memory_tabs", ImGuiTabBarFlags_None))
    {
        draw_tabs();
        ImGui::EndTabBar();
    }

    ImGui::End();
    ImGui::PopStyleVar();
}

void gui_debug_memory_search_window(void)
{
    for (int i = 0; i < MEMORY_EDITOR_MAX; i++)
    {
        ImGui::PushFont(gui_default_font);
        mem_edit[i].DrawSearchWindow();
        ImGui::PopFont();
    }
}

void gui_debug_memory_watches_window(void)
{
    for (int i = 0; i < MEMORY_EDITOR_MAX; i++)
    {
        ImGui::PushFont(gui_default_font);
        mem_edit[i].DrawWatchWindow();
        ImGui::PopFont();
    }
}

void gui_debug_memory_step_frame(void)
{
    for (int i = 0; i < MEMORY_EDITOR_MAX; i++)
    {
        mem_edit[i].StepFrame();
    }
}

void gui_debug_memory_copy(void)
{
    mem_edit[current_mem_edit].Copy();
}

void gui_debug_memory_paste(void)
{
    mem_edit[current_mem_edit].Paste();
}

void gui_debug_memory_select_all(void)
{
    mem_edit[current_mem_edit].SelectAll();
}

void gui_debug_memory_goto(int editor, int address)
{
    mem_edit_select = editor;
    mem_edit[mem_edit_select].JumpToAddress(address);
}

void gui_debug_memory_save_dump(const char* file_path, bool binary)
{
    if (binary)
        mem_edit[current_mem_edit].SaveToBinaryFile(file_path);
    else
        mem_edit[current_mem_edit].SaveToTextFile(file_path);
}

static void draw_tabs(void)
{
    for (int i = 0; i < MEMORY_EDITOR_MAX; i++)
    {
        if (emu_is_cgb())
        {
            if (i == MEMORY_EDITOR_WRAM)
                continue;
        }
        else
        {
            if (i == MEMORY_EDITOR_WRAM0 || i == MEMORY_EDITOR_WRAM1)
                continue;
        }

        if (ImGui::BeginTabItem(mem_edit[i].GetTitle(), NULL, mem_edit_select == i ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None))
        {
            ImGui::PushFont(gui_default_font);
            if (mem_edit_select == i)
                mem_edit_select = -1;
            current_mem_edit = i;
            mem_edit[i].Draw();
            ImGui::PopFont();
            ImGui::EndTabItem();
        }
    }
}

static void memory_editor_menu(void)
{
    ImGui::BeginMenuBar();

    if (ImGui::BeginMenu("File"))
    {
        if (ImGui::MenuItem("Save Memory As Text..."))
        {
            file_dialog_save_memory_dump(false);
        }

        if (ImGui::MenuItem("Save Memory As Binary..."))
        {
            file_dialog_save_memory_dump(true);
        }

        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Edit"))
    {
        if (ImGui::MenuItem("Copy", "Ctrl+C"))
        {
            gui_debug_memory_copy();
        }

        if (ImGui::MenuItem("Paste", "Ctrl+V"))
        {
            gui_debug_memory_paste();
        }

        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Selection"))
    {
        if (ImGui::MenuItem("Select All", "Ctrl+A"))
        {
            mem_edit[current_mem_edit].SelectAll();
        }

        if (ImGui::MenuItem("Clear Selection"))
        {
            mem_edit[current_mem_edit].ClearSelection();
        }

        if (ImGui::BeginMenu("Set value"))
        {
            ImVec2 character_size = ImGui::CalcTextSize("X");
            int word_bytes = mem_edit[current_mem_edit].GetWordBytes();
            ImGui::SetNextItemWidth(((word_bytes * 2) + 1) * character_size.x);
            if (ImGui::InputTextWithHint("##set_value", word_bytes == 1 ? "XX" : "XXXX", set_value_buffer, IM_ARRAYSIZE(set_value_buffer), ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase))
            {
                try
                {
                    mem_edit[current_mem_edit].SetValueToSelection((int)std::stoul(set_value_buffer, 0, 16));
                    set_value_buffer[0] = 0;
                }
                catch(const std::invalid_argument&)
                {
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Set!", ImVec2(40, 0)))
            {
                try
                {
                    mem_edit[current_mem_edit].SetValueToSelection((int)std::stoul(set_value_buffer, 0, 16));
                    set_value_buffer[0] = 0;
                }
                catch(const std::invalid_argument&)
                {
                }
            }
            ImGui::EndMenu();
        }

        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Bookmarks"))
    {
        if (ImGui::MenuItem("Add Bookmark"))
        {
            mem_edit[current_mem_edit].AddBookmark();
        }

        if (ImGui::MenuItem("Clear All"))
        {
            mem_edit[current_mem_edit].RemoveBookmarks();
        }

        std::vector<MemEditor::Bookmark>* bookmarks = mem_edit[current_mem_edit].GetBookmarks();

        if (bookmarks->size() > 0)
            ImGui::Separator();

        for (long unsigned int i = 0; i < bookmarks->size(); i++)
        {
            MemEditor::Bookmark* bookmark = &(*bookmarks)[i];

            char label[80];
            snprintf(label, 80, "$%04X: %s", bookmark->address, bookmark->name);

            if (ImGui::MenuItem(label))
            {
                mem_edit[current_mem_edit].JumpToAddress(bookmark->address);
            }
        }

        ImGui::EndMenu();
    }

    char label[64];

    if (ImGui::BeginMenu("Watches"))
    {
        snprintf(label, 64, "Open %s Watch Window", mem_edit[current_mem_edit].GetTitle());
        if (ImGui::MenuItem(label))
        {
            mem_edit[current_mem_edit].OpenWatchWindow();
        }

        snprintf(label, 64, "Add %s Watch", mem_edit[current_mem_edit].GetTitle());
        if (ImGui::MenuItem(label))
        {
            mem_edit[current_mem_edit].AddWatch();
        }

        snprintf(label, 64, "Clear All %s Watches", mem_edit[current_mem_edit].GetTitle());
        if (ImGui::MenuItem(label))
        {
            mem_edit[current_mem_edit].RemoveWatches();
        }

        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Search"))
    {
        snprintf(label, 64, "Open %s Search Window", mem_edit[current_mem_edit].GetTitle());
        if (ImGui::MenuItem(label))
        {
            mem_edit[current_mem_edit].OpenSearchWindow();
        }

        ImGui::EndMenu();
    }

    ImGui::EndMenuBar();
}

static void file_dialog_save_memory_dump(bool binary)
{
    nfdchar_t *outPath;
    nfdfilteritem_t filterItem[1] = { { "Memory Dump Files", binary ? "bin" : "txt" } };
    nfdsavedialogu8args_t args = { };
    args.filterList = filterItem;
    args.filterCount = 1;
    args.defaultPath = NULL;
    args.defaultName = NULL;
    if (!NFD_GetNativeWindowFromSDLWindow(application_sdl_window, &args.parentWindow))
    {
        Log("NFD_GetNativeWindowFromSDLWindow failed: %s\n", SDL_GetError());
    }

    nfdresult_t result = NFD_SaveDialogU8_With(&outPath, &args);
    if (result == NFD_OKAY)
    {
        gui_debug_memory_save_dump(outPath, binary);
        NFD_FreePath(outPath);
    }
    else if (result != NFD_CANCEL)
    {
        Log("Save Memory Dump Error: %s", NFD_GetError());
    }
}
