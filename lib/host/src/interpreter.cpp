#include "interpreter.hpp"

#include <algorithm>
#include <cctype>
#include <cinttypes>
#include <iostream>

#include "str_utils.hpp"
#include "cmd_type.hpp"
#include "cmd_map.hpp"
#include "context_action_map.hpp"

namespace rvi::host
{
    bool is_ignored_ch(char ch)
    {
        return std::find(ignored_chars.begin(), ignored_chars.end(), ch) != ignored_chars.end();
    }

    std::stringstream interpreter::clean_input(std::stringstream& sstr)
    {
        std::stringstream result;
        bool dq_escape = false;
        for(char ch; sstr >> ch;) 
        {
            // Double quotes escaping
            if (ch == TXT_ESC_CHAR)
            {
                dq_escape = !dq_escape;
            }

            bool feed_flag = !dq_escape || !is_ignored_ch(ch);

            if(feed_flag)
            {
                result << ch;
            }
        }
        return result;
    }

    cmd_type interpreter::parse_command(const std::string& txt)
    {
        std::string lower_txt = txt;
        str_tolower(lower_txt);
        if(cmd_map.count(lower_txt) > 0)
        {
            return cmd_map.at(lower_txt);
        }
        else
        {
            return cmd_type::INVALID_CMD;
        }
    }

    std::vector<parsed_stmt> interpreter::read(std::stringstream& stream)
    {
        std::vector<parsed_stmt> result;
        auto clean_text = clean_input(stream).str();
        std::stringstream line;

        int def_esc = 0;
        for(auto& ch : clean_text)
        {
            if(ch == DEF_ESC_CHAR_BEGIN)
            {
                def_esc++;
            }
            else if(ch == DEF_ESC_CHAR_END)
            {
                def_esc--;
            }

            rvi_assert(def_esc >= 0, "Invalid definition end char without prior begin char!"); 

            if(ch == SEP_LINE && !def_esc)
            {
                result.push_back(parse_line(line.str()));
                line.str(std::string());
            }
            else
            {
                line << ch;
            }
        }
        return result;
    }

    parsed_stmt interpreter::parse_line(const std::string& line)
    {
        parsed_stmt result;

        auto segments = str_split_once(line, SEP_CMDARGS);
        if (segments.size() != 2)
        {            
            throw std::logic_error("Invalid line syntax: " + line);
        }
        
        result.command = parse_command(segments[0]);

        if(result.command != cmd_type::DEFINE)
        {
            auto args = str_split(segments[1], SEP_ARGS);
            std::move(args.begin(), args.end(), std::back_inserter(result.args));
        }
        else
        {
            auto args = str_split_once(segments[1], SEP_ARGS);
            rvi_assert(args.size() >= 2, "Invalid definition arg count!");

            // definition name
            result.args.push_back(args[0]); 
            // definition body
            result.args.push_back(args[1]);
        }
        return result;
    }

    void interpreter::run(const std::vector<parsed_stmt>& lines, client_context& ctx)
    {
        for (auto& line : lines)
        {
            run_line(line, ctx);
        }
    }

    void interpreter::run_line(const parsed_stmt& line, client_context& ctx)
    {
        auto cmd = context_action_map.at(line.command);
        cmd(ctx, line.args);

        // -- Debug --------------------------------
        std::stringstream ss;
        ss << get_cmd_name(line.command) << ": [";
        for (auto& a : line.args)
        {
            ss << a << ", ";
        }
        ss << "]";
        std::cout << ss.str() << std::endl;
    }
}