#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <client_context.hpp>

#include "random_gen.hpp"

using namespace rvi;

TEST(client_context, ctor_default_frame)
{
    client_context ctx;
    auto name = ctx.selected_frame().name();
    ASSERT_EQ(name, client_context::MAIN_FRAMENAME);
}

TEST(client_context, is_root_frame_selected)
{
    client_context ctx;

    ASSERT_TRUE(ctx.is_root_frame_selected());
    ctx.select_frame("test_frame");
    ASSERT_FALSE(ctx.is_root_frame_selected());
    ctx.release_frame();
    ASSERT_TRUE(ctx.is_root_frame_selected());
}

TEST(client_context, select_frame)
{
    client_context ctx;
    ASSERT_EQ(ctx.frame_count(), 1);
    auto icount = std::max(10, std::abs(get_random_int()));
    for (auto i = 0; i < icount; i++)
    {
        ctx.select_frame("test_frame" + i);
    }
    ASSERT_EQ(ctx.frame_count(), 1 + icount);
}

TEST(client_context, select_frame_dupe_noclear)
{
    client_context ctx;
    ASSERT_EQ(ctx.frame_count(), 1);
    ctx.select_frame("test_frame");
    auto icount = std::max(10, std::abs(get_random_int()));
    for (auto i = 0; i < icount; i++)
    {
        vertex vx1, vx2;
        ctx.draw_line(vx1, vx2);
    }
    ctx.release_frame();
    ctx.select_frame("test_frame");
    ASSERT_EQ(icount, ctx.selected_frame().line_count());
}

TEST(client_context, draw_line_main_frame)
{
    client_context ctx;

    auto startCount = ctx.selected_frame().line_count();
    ASSERT_EQ(startCount, 0);

    ctx.draw_line(line(vector2(1, 2), vector2(3, 4)));
    ctx.draw_line(vertex(vector2(0, 0)), vertex(vector2(0, 0)));
    ctx.draw_line(vector2(5, 6), color_rgba::RED(), vector2(7, 8), color_rgba::BLUE());
    ctx.draw_line(vector2(9, 10), vector2(11, 12));

    auto lcount = ctx.selected_frame().line_count();
    ASSERT_EQ(lcount, 4);

    line expected_ln1 = line(vector2(1, 2), vector2(3, 4));
    line expected_ln2 = line(vertex(vector2(0, 0)), vertex(vector2(0, 0)));
    line expected_ln3 = line(vertex(vector2(5, 6), color_rgba::RED()), vertex(vector2(7, 8), color_rgba::BLUE()));
    line expected_ln4 = line(vector2(9, 10), vector2(11, 12));

    auto lines = ctx.selected_frame().lines();
    ASSERT_TRUE(std::find(std::begin(lines), std::end(lines), expected_ln1) != std::end(lines));
    ASSERT_TRUE(std::find(std::begin(lines), std::end(lines), expected_ln2) != std::end(lines));
    ASSERT_TRUE(std::find(std::begin(lines), std::end(lines), expected_ln3) != std::end(lines));
    ASSERT_TRUE(std::find(std::begin(lines), std::end(lines), expected_ln4) != std::end(lines));
}

TEST(client_context, set_position)
{
    client_context ctx;
    vector2 offset(get_random_float(), get_random_float());
    ctx.set_position(offset);
    
    ASSERT_EQ(ctx.position(), offset);
}

TEST(client_context, set_scale)
{
    client_context ctx;
    vector2 scale(get_random_float(), get_random_float());
    ctx.set_scale(scale);
    
    ASSERT_EQ(ctx.scale(), scale);
}

TEST(client_context, set_rotation)
{
    client_context ctx;
    float rot = get_random_float();
    ctx.set_rotation(rot);

    ASSERT_EQ(ctx.rotation(), rot);
}

TEST(client_context, set_transform)
{
    client_context ctx;
    vector2 offset(get_random_float(), get_random_float());
    vector2 scale(get_random_float(), get_random_float());
    float rot = get_random_float();
    transform2 tform(offset, scale, rot);
    
    ctx.set_transform(tform);

    ASSERT_EQ(ctx.transform(), tform);
}

TEST(client_context, select_release_frame)
{
    client_context ctx;

    ASSERT_EQ(ctx.selected_frame().name(), client_context::MAIN_FRAMENAME);

    ctx.release_frame();
    ASSERT_EQ(ctx.selected_frame().name(), client_context::MAIN_FRAMENAME);

    ctx.select_frame("test_frame_1");
    ASSERT_EQ(ctx.selected_frame().name(), "test_frame_1");

    ctx.release_frame();
    ASSERT_EQ(ctx.selected_frame().name(), client_context::MAIN_FRAMENAME);

    ctx.select_frame("test_frame_2");
    ASSERT_EQ(ctx.selected_frame().name(), "test_frame_2");

    ctx.select_frame("test_frame_2_1");
    ASSERT_EQ(ctx.selected_frame().name(), "test_frame_2_1");

    ctx.release_frame();
    ASSERT_EQ(ctx.selected_frame().name(), "test_frame_2");

    ctx.select_frame("test_frame_2_2");
    ASSERT_EQ(ctx.selected_frame().name(), "test_frame_2_2");

    ctx.select_frame("test_frame_2_2_1");
    ASSERT_EQ(ctx.selected_frame().name(), "test_frame_2_2_1");

    ctx.select_frame("test_frame_2_2_1_1");
    ASSERT_EQ(ctx.selected_frame().name(), "test_frame_2_2_1_1");
    
}

TEST(client_context, frame_count)
{
    client_context ctx;
    ctx.select_frame("test_frame_1");
    ctx.release_frame();
    ctx.select_frame("test_frame_2");
    ctx.select_frame("test_frame_2_1");
    ctx.release_frame();
    ctx.select_frame("test_frame_2_2");
    ctx.select_frame("test_frame_2_2_1");
    ctx.select_frame("test_frame_2_2_1_1");

    ASSERT_EQ(ctx.frame_count(), 7);
}

TEST(client_context, contains_definition)
{
    client_context ctx;
    definition def("test_def");

    ASSERT_FALSE(ctx.contains_definition(def.name()));

    ctx.add_definition(def);
    ASSERT_TRUE(ctx.contains_definition(def.name()));
}

TEST(client_context, add_definition)
{
    client_context ctx;
    definition def("test_def");

    ASSERT_FALSE(ctx.contains_definition(def.name()));

    ctx.add_definition(def);
    ASSERT_TRUE(ctx.contains_definition(def.name()));

    definition def2("test_def2");
    ctx.add_definition(def2);
    ASSERT_TRUE(ctx.contains_definition(def2.name()));
}

TEST(client_context, delete_definition)
{
    client_context ctx;
    definition def("test_def");

    ASSERT_FALSE(ctx.contains_definition(def.name()));

    ctx.add_definition(def);
    ASSERT_TRUE(ctx.contains_definition(def.name()));

    definition def2("test_def2");
    ctx.add_definition(def2);
    ASSERT_TRUE(ctx.contains_definition(def2.name()));

    ctx.delete_definition(def.name());
    ASSERT_FALSE(ctx.contains_definition(def.name()));
    ASSERT_TRUE(ctx.contains_definition(def2.name()));

    ctx.delete_definition(def2.name());
    ASSERT_FALSE(ctx.contains_definition(def.name()));
    ASSERT_FALSE(ctx.contains_definition(def2.name()));
}

TEST(client_context, execute_definition)
{
    client_context ctx;
    definition def("test_def");

    bool execOk = false;
    def.add_instruction([&execOk](client_context& ctx) { execOk = true; });

    ctx.add_definition(def);

    ASSERT_FALSE(execOk);
    ctx.execute_definition(def.name());
    ASSERT_TRUE(execOk);
}

TEST(client_context, get_fpath)
{
    client_context ctx;

    ASSERT_EQ(ctx.get_fpath(), client_context::MAIN_FRAMENAME);
    ctx.select_frame("A");
    auto expected = client_context::MAIN_FRAMENAME 
        + client_context::FRAMEPATH_SEPARATOR 
        + "A";
    ASSERT_EQ(ctx.get_fpath(), expected);

    ctx.select_frame("B");
    expected = client_context::MAIN_FRAMENAME 
        + client_context::FRAMEPATH_SEPARATOR 
        + "A"
        + client_context::FRAMEPATH_SEPARATOR 
        + "B";
    ASSERT_EQ(ctx.get_fpath(), expected);
}

TEST(client_context, extract_fpath_with_transform_1lvl)
{
    client_context ctx;

    transform2 tform(vector2(1, 3), vector2(3, 2), 45.0F);
    ctx.set_transform(tform);

    auto rootPath = ctx.get_fpath();
    auto rootPair = ctx.extract_fpath_with_transform(rootPath);

    ASSERT_EQ(rootPair.first, tform);
    ASSERT_EQ(rootPair.second.name(), client_context::MAIN_FRAMENAME);
}

TEST(client_context, extract_fpath_with_transform_2lvl)
{
    client_context ctx;

    transform2 tform(
        vector2(get_random_float(), get_random_float()), 
        vector2(get_random_float(), get_random_float()), 
        get_random_float());

    transform2 ch_tform(
        vector2(get_random_float(), get_random_float()), 
        vector2(get_random_float(), get_random_float()), 
        get_random_float());
    ctx.set_transform(tform);

    ctx.select_frame("childframe");
    ctx.set_transform(ch_tform);

    auto childPath = ctx.get_fpath();
    auto childPair = ctx.extract_fpath_with_transform(childPath);

    transform2 expectedTform(
        (tform.position + ch_tform.position),
        (tform.scale * ch_tform.scale),
        math::clamp_angle_deg(tform.rotation + ch_tform.rotation));

    ASSERT_EQ(childPair.first, expectedTform);
    ASSERT_EQ(childPair.second.name(), "childframe");
}