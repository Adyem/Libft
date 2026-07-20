#include "../../Modules/Game/game_pathfinding.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "test_game_lifecycle_helpers.hpp"

static void pathfinding_initialize_twice(game_pathfinding &value)
{
    (void)value.initialize();
    (void)value.initialize();
    return ;
}

static void pathfinding_move_uninitialised(game_pathfinding &value)
{
    game_pathfinding source;

    (void)value.move(source);
    return ;
}

static void pathfinding_update_obstacle(game_pathfinding &value)
{
    value.update_obstacle(0, 0, 0, 1);
    return ;
}

static void pathfinding_astar_grid(game_pathfinding &value)
{
    game_map3d grid;
    ft_vector<game_path_step> path;

    (void)value.astar_grid(grid, 0, 0, 0, 1, 1, 1, path);
    return ;
}

static void pathfinding_recalculate_path(game_pathfinding &value)
{
    game_map3d grid;
    ft_vector<game_path_step> path;

    (void)value.recalculate_path(grid, 0, 0, 0, 1, 1, 1, path);
    return ;
}

static void pathfinding_dijkstra_graph(game_pathfinding &value)
{
    ft_graph<int32_t> graph;
    ft_vector<ft_size_t> path;

    (void)value.dijkstra_graph(graph, 0, 1, path);
    return ;
}

FT_TEST(test_game_pathfinding_initialize_twice_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_pathfinding>(
                        pathfinding_initialize_twice));
    return (1);
}

FT_TEST(test_game_pathfinding_move_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_pathfinding>(
                        pathfinding_move_uninitialised));
    return (1);
}

FT_TEST(test_game_pathfinding_update_obstacle_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_pathfinding>(
                        pathfinding_update_obstacle));
    return (1);
}

FT_TEST(test_game_pathfinding_astar_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_pathfinding>(
                        pathfinding_astar_grid));
    return (1);
}

FT_TEST(test_game_pathfinding_recalculate_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_pathfinding>(
                        pathfinding_recalculate_path));
    return (1);
}

FT_TEST(test_game_pathfinding_dijkstra_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, expect_game_lifecycle_sigabrt<game_pathfinding>(
                        pathfinding_dijkstra_graph));
    return (1);
}

FT_TEST(test_game_pathfinding_initialize_default_state)
{
    game_pathfinding value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_FALSE, value.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.get_error());
    return (1);
}

FT_TEST(test_game_pathfinding_empty_grid_returns_error)
{
    game_pathfinding value;
    game_map3d grid;
    ft_vector<game_path_step> path;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, grid.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, path.initialize());
    FT_ASSERT_NEQ(FT_ERR_SUCCESS,
                  value.astar_grid(grid, 0, 0, 0, 1, 1, 1, path));
    return (1);
}

FT_TEST(test_game_pathfinding_update_obstacle_initialised)
{
    game_pathfinding value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.update_obstacle(1, 2, 3, 1);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.get_error());
    return (1);
}

FT_TEST(test_game_pathfinding_thread_safety_enable_disable_cycle)
{
    game_pathfinding value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.enable_thread_safety());
    FT_ASSERT_EQ(FT_TRUE, value.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.disable_thread_safety());
    FT_ASSERT_EQ(FT_FALSE, value.is_thread_safe());
    return (1);
}

FT_TEST(test_game_pathfinding_lock_unlock_initialised)
{
    game_pathfinding value;
    ft_bool lock_acquired = FT_FALSE;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.lock(&lock_acquired));
    FT_ASSERT_EQ(FT_TRUE, lock_acquired);
    value.unlock(lock_acquired);
    return (1);
}

FT_TEST(test_game_pathfinding_destroy_twice_is_safe)
{
    game_pathfinding value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    return (1);
}

FT_TEST(test_game_pathfinding_reinitialize_after_destroy)
{
    game_pathfinding value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.get_error());
    return (1);
}

FT_TEST(test_game_pathfinding_self_move_is_safe)
{
    game_pathfinding value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.move(value));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.get_error());
    return (1);
}

FT_TEST(test_game_pathfinding_error_string_after_success)
{
    game_pathfinding value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_NEQ(ft_nullptr, value.get_error_str());
    return (1);
}

FT_TEST(test_game_pathfinding_multiple_obstacle_updates)
{
    game_pathfinding value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.update_obstacle(0, 0, 0, 1);
    value.update_obstacle(1, 1, 1, 0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.get_error());
    return (1);
}

FT_TEST(test_game_pathfinding_empty_path_is_destroyed_cleanly)
{
    game_pathfinding value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    return (1);
}

FT_TEST(test_game_pathfinding_thread_safety_reenable)
{
    game_pathfinding value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.disable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.enable_thread_safety());
    FT_ASSERT_EQ(FT_TRUE, value.is_thread_safe());
    return (1);
}

FT_TEST(test_game_pathfinding_second_destroy_is_safe)
{
    game_pathfinding value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    return (1);
}

FT_TEST(test_game_pathfinding_empty_astar_path_is_initialised)
{
    game_pathfinding value;
    game_map3d grid;
    ft_vector<game_path_step> path;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, grid.initialize(1, 1, 1, 0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, path.initialize());
    (void)value.astar_grid(grid, 0, 0, 0, 0, 0, 0, path);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, path.get_error());
    return (1);
}
