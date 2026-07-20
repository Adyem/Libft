#include "../../Modules/Game/game_world_replay.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void
game_world_replay_session_capture_snapshot(game_world_replay_session &value)
{
    game_world world;
    game_character character;
    game_inventory inventory;

    (void)value.capture_snapshot(world, character, inventory);
    return ;
}

static void
game_world_replay_session_restore_snapshot(game_world_replay_session &value)
{
    ft_sharedptr<game_world> world_pointer;
    game_character character;
    game_inventory inventory;

    (void)value.restore_snapshot(world_pointer, character, inventory);
    return ;
}

static void
game_world_replay_session_replay_ticks(game_world_replay_session &value)
{
    ft_sharedptr<game_world> world_pointer;
    game_character character;
    game_inventory inventory;

    (void)value.replay_ticks(world_pointer, character, inventory, 1, ft_nullptr,
                             ft_nullptr);
    return ;
}

static void
game_world_replay_session_plan_route(game_world_replay_session &value)
{
    game_world world;
    game_map3d grid;
    ft_vector<game_path_step> path;

    (void)value.plan_route(world, grid, 0U, 0U, 0U, 1U, 1U, 1U, path);
    return ;
}

static void
game_world_replay_session_import_snapshot(game_world_replay_session &value)
{
    ft_string snapshot_payload;

    (void)value.import_snapshot(snapshot_payload);
    return ;
}

static void
game_world_replay_session_export_snapshot(game_world_replay_session &value)
{
    ft_string snapshot_payload;

    (void)value.export_snapshot(snapshot_payload);
    return ;
}

static void
game_world_replay_session_clear_snapshot(game_world_replay_session &value)
{
    value.clear_snapshot();
    return ;
}

FT_TEST(test_game_world_replay_session_capture_snapshot_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_world_replay_session>(
                        game_world_replay_session_capture_snapshot));
    return (1);
}

FT_TEST(test_game_world_replay_session_restore_snapshot_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_world_replay_session>(
                        game_world_replay_session_restore_snapshot));
    return (1);
}

FT_TEST(test_game_world_replay_session_replay_ticks_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_world_replay_session>(
                        game_world_replay_session_replay_ticks));
    return (1);
}

FT_TEST(test_game_world_replay_session_plan_route_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_world_replay_session>(
                        game_world_replay_session_plan_route));
    return (1);
}

FT_TEST(test_game_world_replay_session_import_snapshot_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_world_replay_session>(
                        game_world_replay_session_import_snapshot));
    return (1);
}

FT_TEST(test_game_world_replay_session_export_snapshot_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_world_replay_session>(
                        game_world_replay_session_export_snapshot));
    return (1);
}

FT_TEST(test_game_world_replay_session_clear_snapshot_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_world_replay_session>(
                        game_world_replay_session_clear_snapshot));
    return (1);
}
