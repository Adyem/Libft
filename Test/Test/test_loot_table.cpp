#include "../../RNG/loot_table.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include <climits>

FT_TEST(test_loot_table_get_random_success_sets_success, "ft_loot_table getRandomLoot success clears errors")
{
    ft_loot_table<int> table;
    int treasure_value = 42;

    FT_ASSERT_EQ(ft_nullptr, table.getRandomLoot());
    FT_ASSERT_EQ(FT_ERR_EMPTY, ft_errno);
    FT_ASSERT_EQ(FT_ERR_EMPTY, table.get_error());

    table.addElement(&treasure_value, 10, 0);
    ft_errno = FT_ERR_OUT_OF_RANGE;

    int *result = table.getRandomLoot();
    if (result == ft_nullptr)
        return (0);
    FT_ASSERT_EQ(&treasure_value, result);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(ER_SUCCESS, table.get_error());
    return (1);
}

FT_TEST(test_loot_table_get_random_overflow_sets_error, "ft_loot_table getRandomLoot overflow sets error state")
{
    ft_loot_table<int> table;
    int rare_gem = 100;
    int common_item = 50;

    table.addElement(&rare_gem, INT_MAX, 0);
    table.addElement(&common_item, 1, 0);

    int *result = table.getRandomLoot();
    FT_ASSERT_EQ(ft_nullptr, result);
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, ft_errno);
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, table.get_error());
    return (1);
}

FT_TEST(test_loot_table_pop_random_success_sets_success, "ft_loot_table popRandomLoot success clears errors")
{
    ft_loot_table<int> table;
    int loot_piece = 7;

    FT_ASSERT_EQ(ft_nullptr, table.popRandomLoot());
    FT_ASSERT_EQ(FT_ERR_EMPTY, ft_errno);
    FT_ASSERT_EQ(FT_ERR_EMPTY, table.get_error());

    table.addElement(&loot_piece, 3, 0);
    ft_errno = FT_ERR_OUT_OF_RANGE;

    int *result = table.popRandomLoot();
    if (result != &loot_piece)
        return (0);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(ER_SUCCESS, table.get_error());
    FT_ASSERT_EQ(0u, table.size());
    return (1);
}

FT_TEST(test_loot_table_pop_random_overflow_sets_error, "ft_loot_table popRandomLoot overflow sets error state")
{
    ft_loot_table<int> table;
    int legendary_item = 5;
    int mundane_item = 6;

    table.addElement(&legendary_item, INT_MAX, 0);
    table.addElement(&mundane_item, 1, 0);

    int *result = table.popRandomLoot();
    FT_ASSERT_EQ(ft_nullptr, result);
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, ft_errno);
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, table.get_error());
    return (1);
}
