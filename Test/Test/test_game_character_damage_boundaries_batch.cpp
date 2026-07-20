#include "../../Modules/Game/game_character.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

FT_TEST(test_game_character_flat_physical_damage_equal_to_armor_is_absorbed)
{
    game_character value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_hit_points(20);
    value.set_physical_armor(5);
    value.take_damage_flat(5, FT_DAMAGE_PHYSICAL);
    FT_ASSERT_EQ(20, value.get_hit_points());
    return (1);
}

FT_TEST(test_game_character_flat_physical_damage_above_armor_reduces_hit_points)
{
    game_character value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_hit_points(20);
    value.set_physical_armor(5);
    value.take_damage_flat(8, FT_DAMAGE_PHYSICAL);
    FT_ASSERT_EQ(17, value.get_hit_points());
    return (1);
}

FT_TEST(test_game_character_flat_magical_damage_equal_to_armor_is_absorbed)
{
    game_character value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_hit_points(20);
    value.set_magic_armor(5);
    value.take_damage_flat(5, FT_DAMAGE_MAGICAL);
    FT_ASSERT_EQ(20, value.get_hit_points());
    return (1);
}

FT_TEST(test_game_character_flat_magical_damage_above_armor_reduces_hit_points)
{
    game_character value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_hit_points(20);
    value.set_magic_armor(5);
    value.take_damage_flat(8, FT_DAMAGE_MAGICAL);
    FT_ASSERT_EQ(17, value.get_hit_points());
    return (1);
}

FT_TEST(test_game_character_buffer_physical_damage_equal_to_armor_is_absorbed)
{
    game_character value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_hit_points(20);
    value.set_current_physical_armor(5);
    value.take_damage_buffer(5, FT_DAMAGE_PHYSICAL);
    FT_ASSERT_EQ(20, value.get_hit_points());
    FT_ASSERT_EQ(0, value.get_current_physical_armor());
    return (1);
}

FT_TEST(test_game_character_buffer_physical_damage_above_armor_spills)
{
    game_character value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_hit_points(20);
    value.set_current_physical_armor(5);
    value.take_damage_buffer(8, FT_DAMAGE_PHYSICAL);
    FT_ASSERT_EQ(17, value.get_hit_points());
    FT_ASSERT_EQ(0, value.get_current_physical_armor());
    return (1);
}

FT_TEST(test_game_character_buffer_magical_damage_equal_to_armor_is_absorbed)
{
    game_character value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_hit_points(20);
    value.set_current_magic_armor(5);
    value.take_damage_buffer(5, FT_DAMAGE_MAGICAL);
    FT_ASSERT_EQ(20, value.get_hit_points());
    FT_ASSERT_EQ(0, value.get_current_magic_armor());
    return (1);
}

FT_TEST(test_game_character_buffer_magical_damage_above_armor_spills)
{
    game_character value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_hit_points(20);
    value.set_current_magic_armor(5);
    value.take_damage_buffer(8, FT_DAMAGE_MAGICAL);
    FT_ASSERT_EQ(17, value.get_hit_points());
    FT_ASSERT_EQ(0, value.get_current_magic_armor());
    return (1);
}

FT_TEST(
    test_game_character_magic_shield_physical_damage_uses_current_magic_armor)
{
    game_character value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_hit_points(20);
    value.set_current_magic_armor(5);
    value.take_damage_magic_shield(5, FT_DAMAGE_PHYSICAL);
    FT_ASSERT_EQ(20, value.get_hit_points());
    FT_ASSERT_EQ(0, value.get_current_magic_armor());
    return (1);
}

FT_TEST(
    test_game_character_magic_shield_magical_damage_uses_current_magic_armor)
{
    game_character value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_hit_points(20);
    value.set_current_magic_armor(5);
    value.take_damage_magic_shield(5, FT_DAMAGE_MAGICAL);
    FT_ASSERT_EQ(20, value.get_hit_points());
    FT_ASSERT_EQ(0, value.get_current_magic_armor());
    return (1);
}

FT_TEST(test_game_character_buffer_damage_with_zero_armor_reaches_hit_points)
{
    game_character value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_hit_points(20);
    value.set_current_physical_armor(0);
    value.take_damage_buffer(3, FT_DAMAGE_PHYSICAL);
    FT_ASSERT_EQ(17, value.get_hit_points());
    return (1);
}

FT_TEST(
    test_game_character_magic_shield_damage_with_zero_armor_reaches_hit_points)
{
    game_character value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_hit_points(20);
    value.set_current_magic_armor(0);
    value.take_damage_magic_shield(3, FT_DAMAGE_MAGICAL);
    FT_ASSERT_EQ(17, value.get_hit_points());
    return (1);
}

FT_TEST(test_game_character_remove_missing_skill_preserves_empty_collection)
{
    game_character value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.remove_skill(404);
    FT_ASSERT_EQ(0, value.get_skills().size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.get_error());
    return (1);
}

FT_TEST(test_game_character_restore_physical_armor_restores_current_value)
{
    game_character value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_physical_armor(9);
    value.set_current_physical_armor(0);
    value.restore_physical_armor();
    FT_ASSERT_EQ(9, value.get_current_physical_armor());
    return (1);
}

FT_TEST(test_game_character_restore_magic_armor_restores_current_value)
{
    game_character value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_magic_armor(9);
    value.set_current_magic_armor(0);
    value.restore_magic_armor();
    FT_ASSERT_EQ(9, value.get_current_magic_armor());
    return (1);
}

FT_TEST(test_game_character_damage_cannot_reduce_hit_points_below_zero)
{
    game_character value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_hit_points(2);
    value.take_damage_flat(100, FT_DAMAGE_PHYSICAL);
    FT_ASSERT_EQ(0, value.get_hit_points());
    FT_ASSERT_EQ(FT_FALSE, value.is_alive());
    return (1);
}
