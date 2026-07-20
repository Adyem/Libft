#include "../../Modules/Game/game_world_replay.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_destroyed_state_helpers.hpp"

static void replay_destroyed_capture_snapshot(game_world_replay_session &value)
{
    game_world world;
    game_character character;
    game_inventory inventory;

    (void)world.initialize();
    (void)character.initialize();
    (void)inventory.initialize();
    (void)value.capture_snapshot(world, character, inventory);
    return ;
}

static void replay_destroyed_restore_snapshot(game_world_replay_session &value)
{
    ft_sharedptr<game_world> world_pointer;
    game_character character;
    game_inventory inventory;

    (void)character.initialize();
    (void)inventory.initialize();
    (void)value.restore_snapshot(world_pointer, character, inventory);
    return ;
}

static void replay_destroyed_replay_ticks(game_world_replay_session &value)
{
    ft_sharedptr<game_world> world_pointer;
    game_character character;
    game_inventory inventory;

    (void)character.initialize();
    (void)inventory.initialize();
    (void)value.replay_ticks(world_pointer, character, inventory, 1);
    return ;
}

static void replay_destroyed_plan_route(game_world_replay_session &value)
{
    game_world world;
    game_map3d grid;
    ft_vector<game_path_step> path;

    (void)world.initialize();
    (void)grid.initialize(1, 1, 1, 0);
    (void)path.initialize();
    (void)value.plan_route(world, grid, 0, 0, 0, 0, 0, 0, path);
    return ;
}

static void replay_destroyed_import_snapshot(game_world_replay_session &value)
{
    ft_string snapshot;

    (void)snapshot.initialize();
    (void)value.import_snapshot(snapshot);
    return ;
}

static void replay_destroyed_export_snapshot(game_world_replay_session &value)
{
    ft_string snapshot;

    (void)snapshot.initialize();
    (void)value.export_snapshot(snapshot);
    return ;
}

static void replay_destroyed_clear_snapshot(game_world_replay_session &value)
{
    value.clear_snapshot();
    return ;
}

FT_TEST(test_game_world_replay_destroyed_capture_snapshot_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_world_replay_session>(
                        replay_destroyed_capture_snapshot));
    return (1);
}

FT_TEST(test_game_world_replay_destroyed_restore_snapshot_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_world_replay_session>(
                        replay_destroyed_restore_snapshot));
    return (1);
}

FT_TEST(test_game_world_replay_destroyed_replay_ticks_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_world_replay_session>(
                        replay_destroyed_replay_ticks));
    return (1);
}

FT_TEST(test_game_world_replay_destroyed_plan_route_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_world_replay_session>(
                        replay_destroyed_plan_route));
    return (1);
}

FT_TEST(test_game_world_replay_destroyed_import_snapshot_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_world_replay_session>(
                        replay_destroyed_import_snapshot));
    return (1);
}

FT_TEST(test_game_world_replay_destroyed_export_snapshot_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_world_replay_session>(
                        replay_destroyed_export_snapshot));
    return (1);
}

FT_TEST(test_game_world_replay_destroyed_clear_snapshot_aborts)
{
    FT_ASSERT_EQ(1, expect_game_destroyed_sigabrt<game_world_replay_session>(
                        replay_destroyed_clear_snapshot));
    return (1);
}

FT_TEST(test_game_world_replay_destroyed_state_can_reinitialize)
{
    game_world_replay_session value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    return (1);
}

FT_TEST(test_game_world_replay_destroyed_destroy_is_idempotent)
{
    game_world_replay_session value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    return (1);
}

FT_TEST(test_game_world_replay_destroyed_destructor_is_non_aborting)
{
    game_world_replay_session *value;

    value = new game_world_replay_session();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value->initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value->destroy());
    delete value;
    FT_ASSERT_EQ(1, 1);
    return (1);
}
