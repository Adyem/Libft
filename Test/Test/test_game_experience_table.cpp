#include "../test_internal.hpp"
#include "../../Game/game_experience_table.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_experience_table_generate_total_resets_zero, "Game: generate_levels_total resets table on zero count")
{
    ft_experience_table table(3);
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

FT_TEST(test_experience_table_copy_isolation, "Game: copy construction duplicates level values")
{
    ft_experience_table table;
    int base_levels[3];

    base_levels[0] = 10;
    base_levels[1] = 20;
    base_levels[2] = 40;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.set_levels(base_levels, 3));
    ft_experience_table duplicate(table);
    table.set_value(1, 30);
    FT_ASSERT_EQ(3, duplicate.get_count());
    FT_ASSERT_EQ(20, duplicate.get_value(1));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, duplicate.get_error());
    return (1);
}

FT_TEST(test_experience_table_move_clears_source, "Game: move construction transfers arrays and clears origin")
{
    ft_experience_table source;
    ft_experience_table destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.generate_levels_total(2, 50, 2.0));
    destination = ft_move(source);
    FT_ASSERT_EQ(2, destination.get_count());
    FT_ASSERT_EQ(0, source.get_count());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());
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
