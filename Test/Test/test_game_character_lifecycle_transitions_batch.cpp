#include "../../Modules/Game/game_character.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void character_move_from_uninitialised(game_character &value)
{
    game_character source;

    (void)value.move(source);
    return ;
}

FT_TEST(test_game_character_move_copies_initialized_values)
{
    game_character source;
    game_character destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize());
    source.set_hit_points(41);
    source.set_coins(12);
    source.set_x(7);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.move(source));
    FT_ASSERT_EQ(41, destination.get_hit_points());
    FT_ASSERT_EQ(12, destination.get_coins());
    FT_ASSERT_EQ(7, destination.get_x());
    return (1);
}

FT_TEST(test_game_character_move_replaces_initialized_destination)
{
    game_character source;
    game_character destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize());
    source.set_hit_points(31);
    destination.set_hit_points(99);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.move(source));
    FT_ASSERT_EQ(31, destination.get_hit_points());
    return (1);
}

FT_TEST(
    test_game_character_move_from_destroyed_source_leaves_destination_destroyed)
{
    game_character source;
    game_character destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.move(source));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.destroy());
    return (1);
}

FT_TEST(test_game_character_move_from_uninitialised_source_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_character>(
                        character_move_from_uninitialised));
    return (1);
}

FT_TEST(test_game_character_move_self_preserves_values)
{
    game_character value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.set_hit_points(24);
    value.set_y(8);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.move(value));
    FT_ASSERT_EQ(24, value.get_hit_points());
    FT_ASSERT_EQ(8, value.get_y());
    return (1);
}

FT_TEST(test_game_character_move_transfers_thread_safe_source_data)
{
    game_character source;
    game_character destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.enable_thread_safety());
    source.set_valor(19);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.move(source));
    FT_ASSERT_EQ(19, destination.get_valor());
    return (1);
}

FT_TEST(test_game_character_move_source_can_be_reinitialized)
{
    game_character source;
    game_character destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    source.set_experience(55);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.move(source));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(55, source.get_experience());
    return (1);
}

FT_TEST(
    test_game_character_move_destination_can_be_reinitialized_after_destroyed_source)
{
    game_character source;
    game_character destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.move(source));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize());
    FT_ASSERT_EQ(FT_FALSE, destination.is_alive());
    return (1);
}

FT_TEST(test_game_character_move_preserves_coordinates)
{
    game_character source;
    game_character destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    source.set_x(-4);
    source.set_y(5);
    source.set_z(9);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.move(source));
    FT_ASSERT_EQ(-4, destination.get_x());
    FT_ASSERT_EQ(5, destination.get_y());
    FT_ASSERT_EQ(9, destination.get_z());
    return (1);
}

FT_TEST(test_game_character_move_cleanup_is_idempotent)
{
    game_character source;
    game_character destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.move(source));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.destroy());
    return (1);
}
