#include "../test_internal.hpp"
#include "../../RNG/loot_table.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include <climits>

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_loot_table_get_random_success_sets_success, "ft_loot_table getRandomLoot success clears errors")
{
    ft_loot_table<int> table;
    int treasure_value = 42;
    Pair<int, int *> result;

    result = table.getRandomLoot();
    FT_ASSERT_EQ(FT_ERR_EMPTY, result.key);
    FT_ASSERT_EQ(ft_nullptr, result.value);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.addElement(&treasure_value, 10, 0));
    result = table.getRandomLoot();
    if (result.value == ft_nullptr)
        return (0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, result.key);
    FT_ASSERT_EQ(&treasure_value, result.value);
    return (1);
}

FT_TEST(test_loot_table_get_random_overflow_sets_error, "ft_loot_table getRandomLoot overflow sets error state")
{
    ft_loot_table<int> table;
    int rare_gem = 100;
    int common_item = 50;
    Pair<int, int *> result;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.addElement(&rare_gem, INT_MAX, 0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.addElement(&common_item, 1, 0));

    result = table.getRandomLoot();
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, result.key);
    FT_ASSERT_EQ(ft_nullptr, result.value);
    return (1);
}

FT_TEST(test_loot_table_pop_random_success_sets_success, "ft_loot_table popRandomLoot success clears errors")
{
    ft_loot_table<int> table;
    int loot_piece = 7;
    Pair<int, int *> result;

    result = table.popRandomLoot();
    FT_ASSERT_EQ(FT_ERR_EMPTY, result.key);
    FT_ASSERT_EQ(ft_nullptr, result.value);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.addElement(&loot_piece, 3, 0));
    result = table.popRandomLoot();
    if (result.value != &loot_piece)
        return (0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, result.key);
    FT_ASSERT_EQ(0u, table.size());
    return (1);
}

FT_TEST(test_loot_table_pop_random_overflow_sets_error, "ft_loot_table popRandomLoot overflow sets error state")
{
    ft_loot_table<int> table;
    int legendary_item = 5;
    int mundane_item = 6;
    Pair<int, int *> result;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.addElement(&legendary_item, INT_MAX, 0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, table.addElement(&mundane_item, 1, 0));

    result = table.popRandomLoot();
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, result.key);
    FT_ASSERT_EQ(ft_nullptr, result.value);
    return (1);
}
