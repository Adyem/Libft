#include "../../Modules/Game/game_character.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

FT_TEST(test_game_character_zero_physical_damage_is_safe)
{
    game_character value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_hit_points(20);
    value.take_damage(0, FT_DAMAGE_PHYSICAL);
    FT_ASSERT_EQ(20, value.get_hit_points());
    return (1);
}

FT_TEST(test_game_character_negative_physical_damage_is_safe)
{
    game_character value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_hit_points(20);
    value.take_damage(-5, FT_DAMAGE_PHYSICAL);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.get_error());
    return (1);
}

FT_TEST(test_game_character_zero_magical_damage_is_safe)
{
    game_character value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_hit_points(20);
    value.take_damage(0, FT_DAMAGE_MAGICAL);
    FT_ASSERT_EQ(20, value.get_hit_points());
    return (1);
}

FT_TEST(test_game_character_armor_can_be_zero)
{
    game_character value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_physical_armor(0);
    value.set_magic_armor(0);
    FT_ASSERT_EQ(0, value.get_physical_armor());
    FT_ASSERT_EQ(0, value.get_magic_armor());
    return (1);
}

FT_TEST(test_game_character_armor_can_exceed_default_value)
{
    game_character value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_physical_armor(1000);
    value.set_magic_armor(1000);
    FT_ASSERT_EQ(1000, value.get_physical_armor());
    FT_ASSERT_EQ(1000, value.get_magic_armor());
    return (1);
}

FT_TEST(test_game_character_restore_armor_resets_current_values)
{
    game_character value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_physical_armor(30);
    value.set_magic_armor(40);
    value.set_current_physical_armor(0);
    value.set_current_magic_armor(0);
    value.restore_armor();
    FT_ASSERT_EQ(30, value.get_current_physical_armor());
    FT_ASSERT_EQ(40, value.get_current_magic_armor());
    return (1);
}
