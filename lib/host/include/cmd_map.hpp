#pragma once

#include <string>
#include <algorithm>
#include <unordered_map>

#include "cmd_type.hpp"

namespace rvi::host
{
    const std::unordered_map<std::string, cmd_type> cmd_map = 
    {
        { "select_frame",   cmd_type::SELECT_FRAME },
        { "release_frame",  cmd_type::RELEASE_FRAME },
        { "delete_frame",   cmd_type::DELETE_FRAME },
        { "draw_line",      cmd_type::DRAW_LINE },
        { "set_color",      cmd_type::SET_COLOR },
        { "set_transform",  cmd_type::SET_TRANSFORM },
        { "set_position",   cmd_type::SET_POSITION },
        { "set_scale",      cmd_type::SET_SCALE },
        { "set_rotation",   cmd_type::SET_ROTATION },
        { "define",         cmd_type::DEFINE },
        { "undefine",       cmd_type::UNDEFINE }
    };

    std::string get_cmd_name(cmd_type ct)
    {
        auto it = std::find_if(cmd_map.begin(), cmd_map.end(), [](auto&& it_pair){ it_pair->second == ct});
        if(it != cmd_map.end())
        {
            return it->first;
        }
        else
        {
            return "ERROR: Command typename not found";
        }
    }
}