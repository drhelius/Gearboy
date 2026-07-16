/*
 * Gearlynx - Lynx Emulator
 * Copyright (C) 2025  Ignacio Sanchez

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

#ifndef MCP_TOOL_REGISTRY_H
#define MCP_TOOL_REGISTRY_H

#include <cstddef>
#include <string>
#include "json.hpp"

using json = nlohmann::json;

class McpToolRegistry
{
public:
    McpToolRegistry();

    void SetTools(const json& tools);

    bool IsEmpty() const;
    bool HasTool(const std::string& tool_name) const;
    bool HasCategory(const std::string& category) const;
    bool ValidateArguments(const std::string& tool_name, const json& arguments, std::string& error) const;

    json GetStats() const;
    json GetCategories() const;
    json GetCategoryNames() const;
    json GetDirectTools() const;
    json GetToolsInCategory(const std::string& category) const;
    json GetToolInfo(const std::string& tool_name) const;
    std::string GetCategoryTitle(const std::string& category) const;
    std::string GetCategoryDescription(const std::string& category) const;
    int GetCategoryToolCount(const std::string& category) const;
    json SearchTools(const std::string& query) const;

    bool IsRouterTool(const std::string& tool_name) const;
    bool IsRouterTool(const std::string& tool_name, const std::string& router_tool_name) const;
    size_t GetSearchToolLimit() const;

private:
    bool IsRouterToolName(const std::string& tool_name) const;
    std::string NormalizeToolName(std::string tool_name) const;
    std::string ToLower(const std::string& text) const;
    bool StringContains(const std::string& text, const std::string& needle) const;
    bool ToolNameInList(const std::string& name, const char* const* tools, size_t count) const;
    bool IsDirectToolName(const std::string& tool_name) const;
    std::string ToolCategoryForName(const std::string& tool_name) const;
    std::string AliasesForTool(const std::string& tool_name) const;

    const json* FindTool(const std::string& tool_name) const;
    bool HasToolInCategory(const std::string& category, bool include_direct) const;
    bool HasRoutedToolInCategory(const std::string& category) const;
    int CountToolsInCategory(const std::string& category, bool include_direct) const;
    json ToolToSummaryJson(const json& tool) const;
    json ToolToSearchJson(const json& tool) const;
    json ToolToInfoJson(const json& tool) const;

private:
    json m_tools;
};

#endif /* MCP_TOOL_REGISTRY_H */
