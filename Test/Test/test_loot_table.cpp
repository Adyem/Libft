#include "../test_internal.hpp"
#include "../../RNG/loot_table.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include <climits>

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_loot_table_get_random_success_sets_success, "ft_loot_table get_random_loot success clears errors")
{
    ft_loot_table<int> table;
    int treasure_value = 42;
    Pair<int, int *> result;

    result = table.get_random_loot();
    FT_ASSERT_EQ(FT_ERR_EMPTY, result.key);
    FT_ASSERT_EQ(ft_nullptr, result.value);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.add_element(&treasure_value, 10, 0));
    result = table.get_random_loot();
    if (result.value == ft_nullptr)
        return (0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, result.key);
    FT_ASSERT_EQ(&treasure_value, result.value);
    return (1);
}

FT_TEST(test_loot_table_get_random_overflow_sets_error, "ft_loot_table get_random_loot overflow sets error state")
{
    ft_loot_table<int> table;
    int rare_gem = 100;
    int common_item = 50;
    Pair<int, int *> result;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.add_element(&rare_gem, INT_MAX, 0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.add_element(&common_item, 1, 0));

    result = table.get_random_loot();
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, result.key);
    FT_ASSERT_EQ(ft_nullptr, result.value);
    return (1);
}

FT_TEST(test_loot_table_pop_random_success_sets_success, "ft_loot_table pop_random_loot success clears errors")
{
    ft_loot_table<int> table;
    int loot_piece = 7;
    Pair<int, int *> result;

    result = table.pop_random_loot();
    FT_ASSERT_EQ(FT_ERR_EMPTY, result.key);
    FT_ASSERT_EQ(ft_nullptr, result.value);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.add_element(&loot_piece, 3, 0));
    result = table.pop_random_loot();
    if (result.value != &loot_piece)
        return (0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, result.key);
    FT_ASSERT_EQ(0u, table.size());
    return (1);
}

FT_TEST(test_loot_table_pop_random_overflow_sets_error, "ft_loot_table pop_random_loot overflow sets error state")
{
    ft_loot_table<int> table;
    int legendary_item = 5;
    int mundane_item = 6;
    Pair<int, int *> result;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.add_element(&legendary_item, INT_MAX, 0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.add_element(&mundane_item, 1, 0));

    result = table.pop_random_loot();
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, result.key);
    FT_ASSERT_EQ(ft_nullptr, result.value);
    return (1);
}
