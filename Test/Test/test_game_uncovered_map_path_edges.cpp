#include "../test_internal.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Game/game_map3d.hpp"
#include "../../Modules/Game/game_pathfinding.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_game_map_edge_single_cell_resize)
{
    game_map3d value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize(1, 1, 1, 4));
    value.resize(1, 1, 1, 8);
    FT_ASSERT_EQ(8, value.get(0, 0, 0));
    return (1);
}

FT_TEST(test_game_map_edge_depth_dimension)
{
    game_map3d value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize(1, 1, 3, 2));
    value.set(0, 0, 2, 19);
    FT_ASSERT_EQ(19, value.get(0, 0, 2));
    return (1);
}

FT_TEST(test_game_map_edge_width_dimension)
{
    game_map3d value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize(3, 1, 1, 2));
    value.set(2, 0, 0, 17);
    FT_ASSERT_EQ(17, value.get(2, 0, 0));
    return (1);
}

FT_TEST(test_game_map_edge_height_dimension)
{
    game_map3d value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize(1, 3, 1, 2));
    value.set(0, 2, 0, 16);
    FT_ASSERT_EQ(16, value.get(0, 2, 0));
    return (1);
}

FT_TEST(test_game_map_edge_obstacle_after_set_zero)
{
    game_map3d value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize(1, 1, 1, 1));
    value.set(0, 0, 0, 0);
    FT_ASSERT_EQ(FT_FALSE, value.is_obstacle(0, 0, 0));
    return (1);
}

FT_TEST(test_game_map_edge_toggle_twice)
{
    game_map3d value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize(1, 1, 1, 0));
    value.toggle_obstacle(0, 0, 0);
    value.toggle_obstacle(0, 0, 0);
    FT_ASSERT_EQ(FT_FALSE, value.is_obstacle(0, 0, 0));
    return (1);
}

FT_TEST(test_game_map_edge_resize_default_value)
{
    game_map3d value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize(1, 1, 1, 3));
    value.resize(2, 1, 1);
    FT_ASSERT_EQ(0, value.get(1, 0, 0));
    return (1);
}

FT_TEST(test_game_map_edge_resize_zero_depth)
{
    game_map3d value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize(1, 1, 1, 3));
    value.resize(1, 1, 0, 5);
    FT_ASSERT_EQ(static_cast<ft_size_t>(0), value.get_depth());
    return (1);
}

FT_TEST(test_game_map_edge_resize_zero_height)
{
    game_map3d value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize(1, 1, 1, 3));
    value.resize(1, 0, 1, 5);
    FT_ASSERT_EQ(static_cast<ft_size_t>(0), value.get_height());
    return (1);
}

FT_TEST(test_game_map_edge_lock_with_mutex)
{
    game_map3d value;
    ft_bool acquired;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize(1, 1, 1, 0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.enable_thread_safety());
    acquired = FT_FALSE;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.lock(&acquired));
    FT_ASSERT_EQ(FT_TRUE, acquired);
    value.unlock(acquired);
    return (1);
}

FT_TEST(test_game_path_edge_origin_step)
{
    game_path_step value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.set_coordinates(0, 0, 0));
    FT_ASSERT_EQ(static_cast<ft_size_t>(0), value.get_z());
    return (1);
}

FT_TEST(test_game_path_edge_large_coordinates)
{
    game_path_step value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.set_coordinates(100, 200, 300));
    FT_ASSERT_EQ(static_cast<ft_size_t>(200), value.get_y());
    return (1);
}

FT_TEST(test_game_path_edge_independent_y_and_z)
{
    game_path_step value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.set_coordinates(1, 2, 3));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.set_y(9));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.set_z(8));
    FT_ASSERT_EQ(static_cast<ft_size_t>(1), value.get_x());
    FT_ASSERT_EQ(static_cast<ft_size_t>(9), value.get_y());
    FT_ASSERT_EQ(static_cast<ft_size_t>(8), value.get_z());
    return (1);
}

FT_TEST(test_game_path_edge_copy_preserves_all_coordinates)
{
    game_path_step source;
    game_path_step destination;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.set_coordinates(8, 7, 6));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(source));
    FT_ASSERT_EQ(static_cast<ft_size_t>(8), destination.get_x());
    FT_ASSERT_EQ(static_cast<ft_size_t>(7), destination.get_y());
    FT_ASSERT_EQ(static_cast<ft_size_t>(6), destination.get_z());
    return (1);
}

FT_TEST(test_game_path_edge_move_preserves_z)
{
    game_path_step source;
    game_path_step destination;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.set_z(77));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(static_cast<game_path_step &&>(source)));
    FT_ASSERT_EQ(static_cast<ft_size_t>(77), destination.get_z());
    return (1);
}

FT_TEST(test_game_path_edge_disable_without_enable)
{
    game_path_step value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.disable_thread_safety());
    FT_ASSERT_EQ(FT_FALSE, value.is_thread_safe());
    return (1);
}

FT_TEST(test_game_path_edge_reinitialize_resets_coordinates)
{
    game_path_step value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.set_coordinates(4, 5, 6));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(static_cast<ft_size_t>(0), value.get_x());
    return (1);
}

FT_TEST(test_game_path_edge_astar_two_cells)
{
    game_map3d map_instance;
    game_pathfinding pathfinding;
    ft_vector<game_path_step> path;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.initialize(2, 1, 1, 0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, pathfinding.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, path.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, pathfinding.astar_grid(map_instance, 0, 0, 0, 1, 0, 0, path));
    FT_ASSERT_EQ(static_cast<ft_size_t>(2), path.size());
    return (1);
}

FT_TEST(test_game_path_edge_astar_obstacle_goal)
{
    game_map3d map_instance;
    game_pathfinding pathfinding;
    ft_vector<game_path_step> path;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.initialize(2, 1, 1, 0));
    map_instance.set(1, 0, 0, 1);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, pathfinding.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, path.initialize());
    FT_ASSERT_NEQ(FT_ERR_SUCCESS, pathfinding.astar_grid(map_instance, 0, 0, 0, 1, 0, 0, path));
    return (1);
}

FT_TEST(test_game_path_edge_update_unrelated_obstacle)
{
    game_pathfinding value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    value.update_obstacle(9, 9, 9, 1);
    FT_ASSERT_EQ(FT_FALSE, value._needs_replan);
    return (1);
}

FT_TEST(test_game_path_edge_move_pathfinding_self)
{
    game_pathfinding value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, value.move(value));
    FT_ASSERT_EQ(FT_CLASS_STATE_INITIALISED, value._initialised_state);
    return (1);
}
