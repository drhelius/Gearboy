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

#define GUI_CHEATS_IMPORT
#include "gui_cheats.h"

#include <list>
#include <string>
#include <vector>
#include <fstream>
#include "gui.h"
#include "gui_colors.h"
#include "gui_filedialogs.h"
#include "emu.h"
#include "utils.h"

#define CHEAT_MAX_PARTS 16

struct gui_CheatImport
{
    bool used;
    bool enabled;
    std::string code;
    std::string description;
};

static bool show_cheats = false;
static bool focus_cheats = false;
static int focus_cheat_row = -1;
static std::list<gui_CheatEntry> cheat_list;

static void cheats_apply(void);
static bool cheat_code_is_hex(char value);
static bool cheat_code_is_hex_format(const char* code, int length, int separator_a, int separator_b);
static bool cheat_code_normalize(const char* input, char* output, size_t output_size);
static int cheat_code_parse(const char* input, char output[][16]);
static bool cheats_add_entry(const char* code, const char* description, bool enabled);
static std::list<gui_CheatEntry>::iterator cheats_find(int index);
static std::string cheats_trim(const std::string& str);

void gui_cheats_init(void)
{
    show_cheats = false;
    focus_cheats = false;
    focus_cheat_row = -1;
    cheat_list.clear();
}

void gui_cheats_show(void)
{
    show_cheats = true;
    focus_cheats = true;
}

void gui_cheats_clear(void)
{
    cheat_list.clear();
    focus_cheat_row = -1;
    emu_clear_cheats();
}

int gui_cheats_add(const char* code, const char* description)
{
    if (!cheats_add_entry(code, description, true))
        return -1;

    cheats_apply();
    return (int)cheat_list.size() - 1;
}

bool gui_cheats_remove(int index)
{
    std::list<gui_CheatEntry>::iterator it = cheats_find(index);

    if (it == cheat_list.end())
        return false;

    cheat_list.erase(it);
    cheats_apply();
    return true;
}

int gui_cheats_get_count(void)
{
    return (int)cheat_list.size();
}

const gui_CheatEntry* gui_cheats_get(int index)
{
    std::list<gui_CheatEntry>::iterator it = cheats_find(index);

    if (it == cheat_list.end())
        return NULL;

    return &(*it);
}

bool gui_cheats_is_valid(const char* code)
{
    char normalized_code[CHEAT_MAX_PARTS][16];
    return cheat_code_parse(code, normalized_code) > 0;
}

bool gui_cheats_import(const char* path, int* imported, int* skipped)
{
    std::ifstream file;
    open_ifstream_utf8(file, path, std::ios::in);

    if (!file.is_open())
        return false;

    std::vector<gui_CheatImport> entries;
    gui_CheatImport empty_entry;
    empty_entry.used = false;
    empty_entry.enabled = false;
    std::string line;

    while (std::getline(file, line))
    {
        size_t equal = line.find('=');

        if (equal == std::string::npos)
            continue;

        std::string key = cheats_trim(line.substr(0, equal));
        std::string value = cheats_trim(line.substr(equal + 1));

        if (!value.empty() && (value[0] == '"'))
            value.erase(0, 1);
        if (!value.empty() && (value[value.length() - 1] == '"'))
            value.erase(value.length() - 1);

        if (key.compare(0, 5, "cheat") != 0)
            continue;

        size_t underscore = key.find('_', 5);

        if ((underscore == std::string::npos) || (underscore == 5) || (underscore > 9))
            continue;

        int index = 0;
        bool is_number = true;

        for (size_t i = 5; i < underscore; i++)
        {
            if ((key[i] < '0') || (key[i] > '9'))
            {
                is_number = false;
                break;
            }

            index = (index * 10) + (key[i] - '0');
        }

        if (!is_number)
            continue;

        std::string field = key.substr(underscore + 1);

        if ((field != "desc") && (field != "code") && (field != "enable"))
            continue;

        if ((int)entries.size() <= index)
            entries.resize(index + 1, empty_entry);

        gui_CheatImport& entry = entries[index];
        entry.used = true;

        if (field == "desc")
            entry.description = value;
        else if (field == "code")
            entry.code = value;
        else
            entry.enabled = (value == "true") || (value == "1");
    }

    int imported_count = 0;
    int skipped_count = 0;

    for (size_t i = 0; i < entries.size(); i++)
    {
        if (!entries[i].used)
            continue;

        if (entries[i].code.empty() || (entries[i].code.length() >= sizeof(gui_CheatEntry::code)) ||
            !cheats_add_entry(entries[i].code.c_str(), entries[i].description.c_str(), entries[i].enabled))
        {
            skipped_count++;
        }
        else
            imported_count++;
    }

    if (imported_count > 0)
        cheats_apply();

    if (IsValidPointer(imported))
        *imported = imported_count;
    if (IsValidPointer(skipped))
        *skipped = skipped_count;

    return true;
}

static bool cheat_code_is_hex(char value)
{
    return ((value >= '0') && (value <= '9')) ||
        ((value >= 'A') && (value <= 'F')) ||
        ((value >= 'a') && (value <= 'f'));
}

static bool cheat_code_is_hex_format(const char* code, int length, int separator_a, int separator_b)
{
    for (int i = 0; i < length; i++)
    {
        if ((i != separator_a) && (i != separator_b) && !cheat_code_is_hex(code[i]))
            return false;
    }

    return true;
}

static bool cheat_code_normalize(const char* input, char* output, size_t output_size)
{
    if (!input || !output || (output_size < 12))
        return false;

    size_t input_length = strlen(input);
    size_t begin = 0;
    size_t end = input_length;

    while ((begin < end) && ((input[begin] == ' ') || (input[begin] == '\t') ||
        (input[begin] == '\r') || (input[begin] == '\n')))
    {
        begin++;
    }

    while ((end > begin) && ((input[end - 1] == ' ') || (input[end - 1] == '\t') ||
        (input[end - 1] == '\r') || (input[end - 1] == '\n')))
    {
        end--;
    }

    size_t length = end - begin;
    char code[16];

    if ((length == 0) || (length >= sizeof(code)))
        return false;

    for (size_t i = 0; i < length; i++)
    {
        char value = input[begin + i];
        if ((value >= 'a') && (value <= 'f'))
            value = (char)(value - ('a' - 'A'));
        code[i] = value;
    }
    code[length] = '\0';

    if ((length == 7) && ((code[3] == '-') || (code[3] == ' ') || (code[3] == '+')) &&
        cheat_code_is_hex_format(code, 7, 3, -1))
    {
        snprintf(output, output_size, "%c%c%c-%c%c%c", code[0], code[1], code[2],
            code[4], code[5], code[6]);
        return true;
    }

    if ((length == 11) && ((code[3] == '-') || (code[3] == ' ') || (code[3] == '+')) &&
        ((code[7] == '-') || (code[7] == ' ') || (code[7] == '+')) &&
        cheat_code_is_hex_format(code, 11, 3, 7))
    {
        snprintf(output, output_size, "%c%c%c-%c%c%c-%c%c%c", code[0], code[1], code[2],
            code[4], code[5], code[6], code[8], code[9], code[10]);
        return true;
    }

    if ((length == 8) && cheat_code_is_hex_format(code, 8, -1, -1))
    {
        strncpy_fit(output, code, output_size);
        return true;
    }

    return false;
}

static int cheat_code_parse(const char* input, char output[][16])
{
    if (cheat_code_normalize(input, output[0], 16))
        return 1;

    if (strchr(input, '+') == NULL)
        return 0;

    int count = 0;
    const char* part = input;

    while (true)
    {
        const char* plus = strchr(part, '+');
        const char* begin = part;
        const char* end = (plus != NULL) ? plus : (part + strlen(part));

        while ((begin < end) && ((*begin == ' ') || (*begin == '\t')))
            begin++;

        while ((end > begin) && ((end[-1] == ' ') || (end[-1] == '\t')))
            end--;

        size_t length = end - begin;
        char buffer[16];

        if ((count >= CHEAT_MAX_PARTS) || (length >= sizeof(buffer)))
            return 0;

        memcpy(buffer, begin, length);
        buffer[length] = '\0';

        if (!cheat_code_normalize(buffer, output[count], 16))
            return 0;

        count++;

        if (plus == NULL)
            break;

        part = plus + 1;
    }

    return count;
}

static bool cheats_add_entry(const char* code, const char* description, bool enabled)
{
    if (cheat_list.size() >= GUI_CHEATS_MAX)
        return false;

    gui_CheatEntry cheat = { };
    cheat.enabled = enabled;
    strncpy_fit(cheat.code, code, sizeof(cheat.code));
    strncpy_fit(cheat.description, description, sizeof(cheat.description));
    cheat_list.push_back(cheat);

    return true;
}

static std::list<gui_CheatEntry>::iterator cheats_find(int index)
{
    if ((index < 0) || (index >= (int)cheat_list.size()))
        return cheat_list.end();

    std::list<gui_CheatEntry>::iterator it = cheat_list.begin();

    for (int i = 0; i < index; i++)
        it++;

    return it;
}

static std::string cheats_trim(const std::string& str)
{
    size_t begin = str.find_first_not_of(" \t\r\n");

    if (begin == std::string::npos)
        return "";

    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(begin, end - begin + 1);
}

static void cheats_apply(void)
{
    char normalized_code[CHEAT_MAX_PARTS][16];
    emu_clear_cheats();

    for (std::list<gui_CheatEntry>::iterator it = cheat_list.begin(); it != cheat_list.end(); it++)
    {
        if (!it->enabled)
            continue;

        int count = cheat_code_parse(it->code, normalized_code);

        for (int i = 0; i < count; i++)
            emu_add_cheat(normalized_code[i]);
    }
}

void gui_cheats_window(void)
{
    if (!show_cheats)
        return;

    if (focus_cheats)
    {
        ImGui::SetNextWindowFocus();
        focus_cheats = false;
    }

    ImGui::SetNextWindowSize(ImVec2(640.0f, 300.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSizeConstraints(ImVec2(620.0f, 180.0f), ImVec2(1000.0f, 800.0f));

    if (!ImGui::Begin("Cheats", &show_cheats))
    {
        ImGui::End();
        return;
    }

    if (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows) ||
        ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
    {
        gui_in_use = true;
    }

    float right_edge = ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x;
    ImGui::TextDisabled("GG: XXX-XXX[-XXX]      GS: XXXXXXXX");

    ImGuiStyle& style = ImGui::GetStyle();
    float import_button_width = ImGui::CalcTextSize("Import .cht...").x + (style.FramePadding.x * 2.0f);
    float add_button_width = ImGui::CalcTextSize("Add Cheat").x + (style.FramePadding.x * 2.0f);
    ImGui::SameLine();
    ImGui::SetCursorPosX(right_edge - add_button_width - style.ItemSpacing.x - import_button_width);

    bool maximum_reached = cheat_list.size() >= GUI_CHEATS_MAX;
    ImGui::BeginDisabled(maximum_reached);
    bool import_cheats = ImGui::Button("Import .cht...##import_cheats");
    ImGui::EndDisabled();

    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
        ImGui::SetTooltip(maximum_reached ? "Maximum of 50 cheats" : "Import cheats from a RetroArch .cht file");

    ImGui::SameLine();
    ImGui::BeginDisabled(maximum_reached);
    bool add_cheat = ImGui::Button("Add Cheat##add_cheat");
    ImGui::EndDisabled();

    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
        ImGui::SetTooltip(maximum_reached ? "Maximum of 50 cheats" : "Add cheat");

    if (import_cheats)
        gui_file_dialog_import_cheats();

    if (add_cheat)
    {
        gui_CheatEntry cheat = { };
        cheat.enabled = true;
        focus_cheat_row = (int)cheat_list.size();
        cheat_list.push_back(cheat);
    }

    float footer_height = cheat_list.empty() ? 0.0f : ImGui::GetFrameHeightWithSpacing();
    float table_height = ImGui::GetContentRegionAvail().y - footer_height;

    if (table_height < 80.0f)
        table_height = 80.0f;

    ImGuiTableFlags table_flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_ScrollY;

    bool apply_changes = false;

    std::list<gui_CheatEntry>::iterator remove_cheat = cheat_list.end();

    if (ImGui::BeginTable("cheats_table", 4, table_flags, ImVec2(0.0f, table_height)))
    {
        ImGui::TableSetupColumn("Enabled", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, 60.0f);
        ImGui::TableSetupColumn("Code", ImGuiTableColumnFlags_WidthFixed, 200.0f);
        ImGui::TableSetupColumn("Description", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, 30.0f);
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableHeadersRow();

        int row = 0;
        char normalized_code[CHEAT_MAX_PARTS][16];

        for (std::list<gui_CheatEntry>::iterator it = cheat_list.begin(); it != cheat_list.end(); it++, row++)
        {
            bool valid = cheat_code_parse(it->code, normalized_code) > 0;
            bool invalid = (it->code[0] != '\0') && !valid;

            ImGui::PushID(&(*it));
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::BeginDisabled(!valid);

            if (ImGui::Checkbox("##enabled", &it->enabled))
                apply_changes = true;
            ImGui::EndDisabled();

            ImGui::TableSetColumnIndex(1);
            if (invalid)
            {
                ImGui::PushStyleColor(ImGuiCol_Border, red);
                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
            }

            if (focus_cheat_row == row)
            {
                ImGui::SetKeyboardFocusHere();
                focus_cheat_row = -1;
            }

            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            bool code_edited = ImGui::InputTextWithHint("##code", "Code", it->code, IM_ARRAYSIZE(it->code), ImGuiInputTextFlags_CharsUppercase);

            if (invalid)
            {
                ImGui::PopStyleVar();
                ImGui::PopStyleColor();
            }

            if (invalid && ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("Use XXX-XXX, XXX-XXX-XXX, or XXXXXXXX, join codes with +.");
            }

            if (code_edited)
                apply_changes = true;

            ImGui::TableSetColumnIndex(2);
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            ImGui::InputTextWithHint("##description", "Description", it->description, IM_ARRAYSIZE(it->description));

            ImGui::TableSetColumnIndex(3);
            bool remove = ImGui::SmallButton("X##remove_cheat");

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Remove Cheat");

            if (remove)
                remove_cheat = it;

            ImGui::PopID();
        }

        if (cheat_list.empty())
        {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(1);
            ImGui::TextDisabled("No cheats");
        }

        ImGui::EndTable();
    }

    if (remove_cheat != cheat_list.end())
    {
        cheat_list.erase(remove_cheat);
        apply_changes = true;
    }

    if (apply_changes)
        cheats_apply();

    if (!cheat_list.empty() && ImGui::Button("Remove All"))
        gui_cheats_clear();

    ImGui::End();
}
