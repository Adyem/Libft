#include "../test_internal.hpp"
#include "../../Game/game_experience_table.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_experience_table_generate_total_resets_zero, "Game: generate_levels_total resets table on zero count")
{
    ft_experience_table table;
    int initial_levels[3];
    int result;

    initial_levels[0] = 5;
    initial_levels[1] = 10;
    initial_levels[2] = 20;
    result = table.set_levels(initial_levels, 3);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, result);
    result = table.generate_levels_total(0, 50, 2.0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, result);
    FT_ASSERT_EQ(0, table.get_count());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    return (1);
}

FT_TEST(test_experience_table_rejects_unsorted_levels, "Game: set_levels reports configuration errors for descending values")
{
    ft_experience_table table;
    int invalid_levels[3];
    int result;

    invalid_levels[0] = 10;
    invalid_levels[1] = 5;
    invalid_levels[2] = 20;
    result = table.set_levels(invalid_levels, 3);
    FT_ASSERT_EQ(FT_ERR_CONFIGURATION, result);
    FT_ASSERT_EQ(3, table.get_count());
    FT_ASSERT_EQ(FT_ERR_CONFIGURATION, table.get_error());
    return (1);
}

FT_TEST(test_experience_table_set_value_updates_single_entry, "Game: set_value updates one level entry")
{
    ft_experience_table table;
    int base_levels[3];

    base_levels[0] = 10;
    base_levels[1] = 20;
    base_levels[2] = 40;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.set_levels(base_levels, 3));
    table.set_value(1, 30);
    FT_ASSERT_EQ(3, table.get_count());
    FT_ASSERT_EQ(30, table.get_value(1));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    return (1);
}

FT_TEST(test_experience_table_resize_grows_and_keeps_values, "Game: resize grows table and keeps existing entries")
{
    ft_experience_table table;
    int levels[2];

    levels[0] = 50;
    levels[1] = 100;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.set_levels(levels, 2));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.resize(4));
    FT_ASSERT_EQ(4, table.get_count());
    FT_ASSERT_EQ(50, table.get_value(0));
    FT_ASSERT_EQ(100, table.get_value(1));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    return (1);
}

FT_TEST(test_experience_table_get_level_boundaries, "Game: get_level returns indexes based on accumulated requirements")
{
    ft_experience_table table;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.generate_levels_scaled(3, 100, 2.0));
    FT_ASSERT_EQ(0, table.get_level(50));
    FT_ASSERT_EQ(1, table.get_level(100));
    FT_ASSERT_EQ(2, table.get_level(350));
    FT_ASSERT_EQ(3, table.get_level(800));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.get_error());
    return (1);
}

FT_TEST(test_experience_table_thread_safety_toggle, "Game: experience table thread safety toggles explicitly")
{
    ft_experience_table table;

    FT_ASSERT_EQ(false, table.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.enable_thread_safety());
    FT_ASSERT_EQ(true, table.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.disable_thread_safety());
    FT_ASSERT_EQ(false, table.is_thread_safe());
    return (1);
}
