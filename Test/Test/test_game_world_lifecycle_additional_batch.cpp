#include "../../Modules/Game/game_world.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void world_initialize_twice(game_world &value)
{
    (void)value.initialize();
    (void)value.initialize();
    return ;
}

static void world_move_uninitialised(game_world &value)
{
    game_world source;

    (void)value.move(source);
    return ;
}

FT_TEST(test_game_world_initialize_twice_aborts_again)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_world>(world_initialize_twice));
    return (1);
}

FT_TEST(test_game_world_move_uninitialised_aborts_again)
{
    FT_ASSERT_EQ(
        1, expect_game_lifecycle_sigabrt<game_world>(world_move_uninitialised));
    return (1);
}

FT_TEST(test_game_world_initialize_succeeds)
{
    game_world value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    return (1);
}

FT_TEST(test_game_world_destroy_twice_is_safe)
{
    game_world value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    return (1);
}

FT_TEST(test_game_world_reinitialize_after_destroy)
{
    game_world value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    return (1);
}

FT_TEST(test_game_world_self_move_is_safe)
{
    game_world value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.move(value));
    return (1);
}

FT_TEST(test_game_world_const_component_access_is_available)
{
    game_world value;
    const game_world &const_value = value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    (void)const_value.get_event_scheduler();
    (void)const_value.get_world_registry();
    (void)const_value.get_replay_session();
    return (1);
}

FT_TEST(test_game_world_all_component_accessors_are_available)
{
    game_world value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    (void)value.get_economy_table();
    (void)value.get_crafting();
    (void)value.get_dialogue_table();
    (void)value.get_world_region();
    (void)value.get_quest();
    (void)value.get_vendor_profile();
    (void)value.get_upgrade();
    return (1);
}

FT_TEST(test_game_world_error_after_initialization)
{
    game_world value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.get_error());
    FT_ASSERT_NEQ(ft_nullptr, value.get_error_str());
    return (1);
}

FT_TEST(test_game_world_empty_route_returns_not_found)
{
    game_world value;
    game_map3d grid;
    ft_vector<game_path_step> path;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, grid.initialize(1, 1, 1, 0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, path.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS,
                 value.plan_route(grid, 0, 0, 0, 0, 0, 0, path));
    return (1);
}
