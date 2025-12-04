#include "../../Game/game_upgrade.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

FT_TEST(test_game_upgrade_rejects_level_above_max, "Game upgrade blocks setting a level greater than max")
{
    ft_upgrade upgrade;

    upgrade.set_max_level(3);
    upgrade.set_current_level(5);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, upgrade.get_error());
    FT_ASSERT_EQ((uint16_t)0, upgrade.get_current_level());
    return (1);
}


FT_TEST(test_game_upgrade_add_level_clamps_to_max, "Game upgrade clamps added levels to maximum")
{
    ft_upgrade upgrade;

    upgrade.set_max_level(4);
    upgrade.add_level(6);
    FT_ASSERT_EQ((uint16_t)4, upgrade.get_current_level());
    FT_ASSERT_EQ(ER_SUCCESS, upgrade.get_error());
    return (1);
}


FT_TEST(test_game_upgrade_sub_level_clamps_to_zero, "Game upgrade prevents negative current level when subtracting")
{
    ft_upgrade upgrade;

    upgrade.set_current_level(2);
    upgrade.sub_level(5);
    FT_ASSERT_EQ((uint16_t)0, upgrade.get_current_level());
    FT_ASSERT_EQ(ER_SUCCESS, upgrade.get_error());
    return (1);
}


FT_TEST(test_game_upgrade_sets_all_modifiers, "Game upgrade stores modifier values independently")
{
    ft_upgrade upgrade;

    upgrade.set_modifier1(3);
    upgrade.set_modifier2(-4);
    upgrade.set_modifier3(7);
    upgrade.set_modifier4(0);
    FT_ASSERT_EQ(3, upgrade.get_modifier1());
    FT_ASSERT_EQ(-4, upgrade.get_modifier2());
    FT_ASSERT_EQ(7, upgrade.get_modifier3());
    FT_ASSERT_EQ(0, upgrade.get_modifier4());
    FT_ASSERT_EQ(ER_SUCCESS, upgrade.get_error());
    return (1);
}


FT_TEST(test_game_upgrade_modifier_add_and_subtract_accumulate, "Game upgrade accumulates modifier additions and subtractions")
{
    ft_upgrade upgrade;

    upgrade.set_modifier1(1);
    upgrade.add_modifier1(5);
    upgrade.sub_modifier1(2);
    FT_ASSERT_EQ(4, upgrade.get_modifier1());
    FT_ASSERT_EQ(ER_SUCCESS, upgrade.get_error());
    return (1);
}


FT_TEST(test_game_upgrade_copy_preserves_modifiers, "Game upgrade copy constructor retains modifiers")
{
    ft_upgrade original;

    original.set_modifier2(6);
    original.set_modifier3(-2);
    original.set_modifier4(9);
    ft_upgrade copied(original);
    FT_ASSERT_EQ(6, copied.get_modifier2());
    FT_ASSERT_EQ(-2, copied.get_modifier3());
    FT_ASSERT_EQ(9, copied.get_modifier4());
    FT_ASSERT_EQ(ER_SUCCESS, copied.get_error());
    return (1);
}


FT_TEST(test_game_upgrade_move_resets_modifiers_in_source, "Game upgrade move constructor clears modifier fields in source")
{
    ft_upgrade source;

    source.set_modifier1(8);
    source.set_modifier2(4);
    source.set_modifier3(1);
    source.set_modifier4(5);
    ft_upgrade moved(ft_move(source));
    FT_ASSERT_EQ(8, moved.get_modifier1());
    FT_ASSERT_EQ(4, moved.get_modifier2());
    FT_ASSERT_EQ(1, moved.get_modifier3());
    FT_ASSERT_EQ(5, moved.get_modifier4());
    FT_ASSERT_EQ(0, source.get_modifier1());
    FT_ASSERT_EQ(0, source.get_modifier2());
    FT_ASSERT_EQ(0, source.get_modifier3());
    FT_ASSERT_EQ(0, source.get_modifier4());
    FT_ASSERT_EQ(ER_SUCCESS, moved.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, source.get_error());
    return (1);
}


FT_TEST(test_game_upgrade_move_assignment_transfers_modifiers, "Game upgrade move assignment transfers modifiers and clears origin")
{
    ft_upgrade source;
    ft_upgrade destination;

    source.set_modifier1(11);
    source.set_modifier2(-7);
    source.set_modifier3(13);
    source.set_modifier4(-9);
    destination.set_modifier1(1);
    destination.set_modifier2(2);
    destination.set_modifier3(3);
    destination.set_modifier4(4);
    destination = ft_move(source);
    FT_ASSERT_EQ(11, destination.get_modifier1());
    FT_ASSERT_EQ(-7, destination.get_modifier2());
    FT_ASSERT_EQ(13, destination.get_modifier3());
    FT_ASSERT_EQ(-9, destination.get_modifier4());
    FT_ASSERT_EQ(0, source.get_modifier1());
    FT_ASSERT_EQ(0, source.get_modifier2());
    FT_ASSERT_EQ(0, source.get_modifier3());
    FT_ASSERT_EQ(0, source.get_modifier4());
    FT_ASSERT_EQ(ER_SUCCESS, destination.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, source.get_error());
    return (1);
}


FT_TEST(test_game_upgrade_clears_error_after_valid_level_set, "Game upgrade resets error after applying valid bounds")
{
    ft_upgrade upgrade;

    upgrade.set_max_level(2);
    upgrade.set_current_level(4);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, upgrade.get_error());
    upgrade.set_max_level(5);
    upgrade.set_current_level(4);
    FT_ASSERT_EQ((uint16_t)4, upgrade.get_current_level());
    FT_ASSERT_EQ(ER_SUCCESS, upgrade.get_error());
    return (1);
}

