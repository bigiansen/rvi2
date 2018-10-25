#pragma once

#include <unordered_map>
#include <functional>
#include <fstream>
#include <iostream>

#include <client_context.hpp>
#include <instruction_generator.hpp>

#include "cmd_type.hpp"
#include "cmd_map.hpp"
#include "interpreter.hpp"
#include "str_utils.hpp"
#include "data_reader.hpp"
#include "runtime.hpp"

namespace rvi::host
{
    typedef std::function<void(runtime&, client_context&, const std::vector<std::string>&)> ctx_action;

    extern void expect_argc(const std::vector<std::string>& args, size_t argc, cmd_type ct);

    static constexpr void no_expect_args() { return; }

    extern std::vector<definition_inst> parse_definition_body(const std::string& body);

    static std::unordered_map<cmd_type, ctx_action> context_action_map = 
    {
        { 
            cmd_type::SELECT_FRAME , [](runtime&, client_context& ctx, const std::vector<std::string>& args)
            {
                expect_argc(args, 1, cmd_type::SELECT_FRAME);
                std::string frame_name = args[0];
                ctx.select_frame(frame_name);
            }
        },
        {
            cmd_type::RELEASE_FRAME, [](runtime&, client_context& ctx, const std::vector<std::string>&)
            {
                no_expect_args();
                ctx.release_frame();
            }
        },
        {
            cmd_type::DELETE_FRAME, [](runtime&, client_context& ctx, const std::vector<std::string>& args)
            {
                expect_argc(args, 1, cmd_type::DELETE_FRAME);
                std::string frame_name = args[0];
                ctx.delete_frame(frame_name);
            }
        },
        {
            cmd_type::DRAW_LINE, [](runtime&, client_context& ctx, const std::vector<std::string>& args)
            {
                expect_argc(args, 4, cmd_type::DRAW_LINE);
                float fx = std::stof(args[0]);
                float fy = std::stof(args[1]);
                float tx = std::stof(args[2]);
                float ty = std::stof(args[3]);
                vector2 from(fx, fy);
                vector2 to(tx, ty);
                ctx.draw_line(from, to);
            }
        },
        {
            cmd_type::SET_TRANSFORM, [](runtime&, client_context& ctx, const std::vector<std::string>& args)
            {
                expect_argc(args, 5, cmd_type::SET_TRANSFORM);
                float pos_x = std::stof(args[0]);
                float pos_y = std::stof(args[1]);
                float scl_x = std::stof(args[2]);
                float scl_y = std::stof(args[3]);
                float rottn = std::stof(args[4]);
                transform2 tform(vector2(pos_x, pos_y), vector2(scl_x, scl_y), rottn);
                ctx.set_transform(tform);
            }
        },
        {
            cmd_type::SET_POSITION, [](runtime&, client_context& ctx, const std::vector<std::string>& args)
            {
                expect_argc(args, 2, cmd_type::SET_POSITION);
                float pos_x = std::stof(args[0]);
                float pos_y = std::stof(args[1]);
                ctx.set_position(vector2(pos_x, pos_y));
            },
        },
        {
            cmd_type::SET_SCALE, [](runtime&, client_context& ctx, const std::vector<std::string>& args)
            {
                expect_argc(args, 2, cmd_type::SET_SCALE);
                float scl_x = std::stof(args[0]);
                float scl_y = std::stof(args[1]);
                ctx.set_scale(vector2(scl_x, scl_y));
            }
        },
        {
            cmd_type::SET_ROTATION, [](runtime&, client_context& ctx, const std::vector<std::string>& args)
            {
                expect_argc(args, 1, cmd_type::SET_ROTATION);
                float rottn = std::stof(args[0]);
                ctx.set_rotation(rottn);
            }
        },
        {
            cmd_type::SET_COLOR, [](runtime&, client_context& ctx, const std::vector<std::string>& args)
            {
                expect_argc(args, 4, cmd_type::SET_COLOR);
                uint8_t r = static_cast<uint8_t>(std::stoi(args[0]));
                uint8_t g = static_cast<uint8_t>(std::stoi(args[1]));
                uint8_t b = static_cast<uint8_t>(std::stoi(args[2]));
                uint8_t a = static_cast<uint8_t>(std::stoi(args[3]));
                ctx.set_color(color_rgba(r, b, g, a));
            }
        },
        {
            cmd_type::DEFINE, [](runtime&, client_context& ctx, const std::vector<std::string>& args)
            {
                expect_argc(args, 2, cmd_type::DEFINE);
                std::string name = args[0];
                std::string body = str_split(args[1], '{')[1];
                body = str_split(body, '}')[0];
                auto instructions = parse_definition_body(body);
                definition def(name);
                std::for_each(instructions.begin(), instructions.end(), [&](auto&& inst)
                { 
                    def.add_instruction(std::move(inst));
                });
                ctx.add_definition(std::move(def));
            }
        },
        {
            cmd_type::UNDEFINE, [](runtime&, client_context& ctx, const std::vector<std::string>& args)
            {
                expect_argc(args, 1, cmd_type::UNDEFINE);
                std::string fname = args[0];
                ctx.delete_definition(fname);
            }
        },
        {
            cmd_type::CALL, [](runtime&, client_context& ctx, const std::vector<std::string>& args)
            {
                expect_argc(args, 1, cmd_type::CALL);
                std::string fname = args[0];
                ctx.execute_definition(fname);
            }
        },
        {
            cmd_type::INCLUDE, [](runtime& rtime, client_context& ctx, const std::vector<std::string>& args)
            {
                expect_argc(args, 1, cmd_type::INCLUDE);
                const std::string& fname = args[0];
                if(rtime.can_include(fname))
                {
                    std::string text = data_reader::get_include_text(fname);
                    std::stringstream ss;
                    ss << text;
                    auto stmt_col = interpreter::read(ss);

                    std::string aux = rtime.current_include();
                    rtime.push_include(fname);
                    interpreter::run(rtime, stmt_col, ctx);
                    rtime.pop_include();
                }
                else
                {
                    std::cout << "Skipping re-inclusion of file: [ " << fname << " ]" << std::endl;
                }
            }
        },
        {
            cmd_type::NO_REINCLUDE, [](runtime& rtime, client_context&, const std::vector<std::string>&)
            {
                no_expect_args();
                rtime.mark_include_once();
            }
        },
        {
            cmd_type::EXEC_BIND, [](runtime& rtime, client_context& ctx, const std::vector<std::string>& args)
            {
                expect_argc(args, 1, cmd_type::EXEC_BIND);
                std::string name = args[0];
                arglist_t b_args(args.begin() + 1, args.end());
                rtime.exec_binding(name, ctx, b_args);
            }
        }
    };
}