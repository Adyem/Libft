#include "../../Modules/Game/game_experience_table.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

FT_TEST(test_game_experience_table_zero_count_clears_levels)
{
    game_experience_table value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.generate_levels_total(0, 10, 2.0));
    FT_ASSERT_EQ(0, value.get_count());
    return (1);
}

FT_TEST(test_game_experience_table_negative_count_clears_levels)
{
    game_experience_table value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.generate_levels_scaled(-1, 10, 2.0));
    FT_ASSERT_EQ(0, value.get_count());
    return (1);
}

FT_TEST(test_game_experience_table_null_levels_clears_levels)
{
    game_experience_table value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.set_levels(ft_nullptr, 3));
    FT_ASSERT_EQ(0, value.get_count());
    return (1);
}

FT_TEST(test_game_experience_table_resize_negative_count_clears_levels)
{
    game_experience_table value;
    int32_t levels[2];

    levels[0] = 10;
    levels[1] = 20;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.set_levels(levels, 2));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.resize(-1));
    FT_ASSERT_EQ(0, value.get_count());
    return (1);
}

FT_TEST(test_game_experience_table_get_value_below_zero_reports_range)
{
    game_experience_table value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.generate_levels_total(2, 10, 2.0));
    FT_ASSERT_EQ(0, value.get_value(-1));
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, value.get_error());
    return (1);
}

FT_TEST(test_game_experience_table_get_value_at_count_reports_range)
{
    game_experience_table value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.generate_levels_total(2, 10, 2.0));
    FT_ASSERT_EQ(0, value.get_value(2));
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, value.get_error());
    return (1);
}

FT_TEST(test_game_experience_table_set_value_below_zero_reports_range)
{
    game_experience_table value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.generate_levels_total(2, 10, 2.0));
    value.set_value(-1, 30);
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, value.get_error());
    return (1);
}

FT_TEST(test_game_experience_table_set_value_at_count_reports_range)
{
    game_experience_table value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.generate_levels_total(2, 10, 2.0));
    value.set_value(2, 30);
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, value.get_error());
    return (1);
}

FT_TEST(test_game_experience_table_resize_down_preserves_prefix)
{
    game_experience_table value;
    int32_t levels[4];

    levels[0] = 10;
    levels[1] = 20;
    levels[2] = 40;
    levels[3] = 80;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.set_levels(levels, 4));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.resize(2));
    FT_ASSERT_EQ(2, value.get_count());
    FT_ASSERT_EQ(10, value.get_value(0));
    FT_ASSERT_EQ(20, value.get_value(1));
    return (1);
}

FT_TEST(test_game_experience_table_resize_up_initializes_new_values)
{
    game_experience_table value;
    int32_t levels[2];

    levels[0] = 10;
    levels[1] = 20;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.set_levels(levels, 2));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.resize(4));
    FT_ASSERT_EQ(4, value.get_count());
    FT_ASSERT_EQ(10, value.get_value(0));
    FT_ASSERT_EQ(20, value.get_value(1));
    FT_ASSERT_EQ(0, value.get_value(2));
    return (1);
}
