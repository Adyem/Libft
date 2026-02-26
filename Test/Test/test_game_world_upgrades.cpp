#include "../test_internal.hpp"
#include "../../Game/game_upgrade.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_upgrade_levels_boundaries, "level setters respect min/max")
{
    ft_upgrade upgrade;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, upgrade.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, upgrade.get_error());
    upgrade.set_max_level(3);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, upgrade.get_error());
    upgrade.set_current_level(5);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, upgrade.get_error());
    FT_ASSERT_EQ(3u, upgrade.get_max_level());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, upgrade.get_error());
    FT_ASSERT_EQ(3u, upgrade.get_current_level());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, upgrade.get_error());
    upgrade.set_current_level(0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, upgrade.get_error());
    FT_ASSERT_EQ(0u, upgrade.get_current_level());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, upgrade.get_error());
    return (1);
}

FT_TEST(test_upgrade_modifiers_accumulate, "modifier add/sub preserves values")
{
    ft_upgrade upgrade;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, upgrade.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, upgrade.get_error());
    upgrade.set_modifier1(10);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, upgrade.get_error());
    upgrade.add_modifier1(5);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, upgrade.get_error());
    FT_ASSERT_EQ(15, upgrade.get_modifier1());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, upgrade.get_error());
    upgrade.sub_modifier1(8);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, upgrade.get_error());
    FT_ASSERT_EQ(7, upgrade.get_modifier1());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, upgrade.get_error());
    return (1);
}
