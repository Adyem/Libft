#include "../test_internal.hpp"
#include "../../Modules/Game/game_pathfinding.hpp"
#include "../../Modules/Game/game_map3d.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/PThread/pthread.hpp"

FT_TEST(test_game_path_step_defaults)
{
    game_path_step step;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, step.initialize());
    FT_ASSERT_EQ(static_cast<ft_size_t>(0), step.get_x());
    FT_ASSERT_EQ(static_cast<ft_size_t>(0), step.get_y());
    FT_ASSERT_EQ(static_cast<ft_size_t>(0), step.get_z());
    return (1);
}

FT_TEST(test_game_path_step_coordinates)
{
    game_path_step step;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, step.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, step.set_coordinates(4, 5, 6));
    FT_ASSERT_EQ(static_cast<ft_size_t>(4), step.get_x());
    FT_ASSERT_EQ(static_cast<ft_size_t>(5), step.get_y());
    FT_ASSERT_EQ(static_cast<ft_size_t>(6), step.get_z());
    return (1);
}

FT_TEST(test_game_path_step_set_x)
{
    game_path_step step;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, step.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, step.set_x(9));
    FT_ASSERT_EQ(static_cast<ft_size_t>(9), step.get_x());
    return (1);
}

FT_TEST(test_game_path_step_set_y)
{
    game_path_step step;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, step.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, step.set_y(9));
    FT_ASSERT_EQ(static_cast<ft_size_t>(9), step.get_y());
    return (1);
}

FT_TEST(test_game_path_step_set_z)
{
    game_path_step step;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, step.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, step.set_z(9));
    FT_ASSERT_EQ(static_cast<ft_size_t>(9), step.get_z());
    return (1);
}

FT_TEST(test_game_path_step_copy_initialization)
{
    game_path_step source;
    game_path_step copy;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.set_coordinates(1, 2, 3));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, copy.initialize(source));
    FT_ASSERT_EQ(static_cast<ft_size_t>(1), copy.get_x());
    FT_ASSERT_EQ(static_cast<ft_size_t>(2), copy.get_y());
    FT_ASSERT_EQ(static_cast<ft_size_t>(3), copy.get_z());
    return (1);
}

FT_TEST(test_game_path_step_move_initialization)
{
    game_path_step source;
    game_path_step destination;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.set_coordinates(7, 8, 9));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize(static_cast<game_path_step &&>(source)));
    FT_ASSERT_EQ(static_cast<ft_size_t>(7), destination.get_x());
    FT_ASSERT_EQ(FT_CLASS_STATE_DESTROYED, source._initialised_state);
    return (1);
}

FT_TEST(test_game_path_step_self_move)
{
    game_path_step step;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, step.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, step.set_x(5));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, step.move(step));
    FT_ASSERT_EQ(static_cast<ft_size_t>(5), step.get_x());
    return (1);
}

FT_TEST(test_game_path_step_thread_safety)
{
    game_path_step step;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, step.initialize());
    FT_ASSERT_EQ(FT_FALSE, step.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, step.enable_thread_safety());
    FT_ASSERT_EQ(FT_TRUE, step.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, step.disable_thread_safety());
    return (1);
}

FT_TEST(test_game_path_step_recursive_lock_helper)
{
    game_path_step step;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, step.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, game_path_step_test_helper::lock(step));
    FT_ASSERT_EQ(FT_TRUE, game_path_step_test_helper::is_locked(step));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, game_path_step_test_helper::unlock(step));
    FT_ASSERT_EQ(FT_FALSE, game_path_step_test_helper::is_locked(step));
    return (1);
}

FT_TEST(test_game_path_step_destroy_idempotent)
{
    game_path_step step;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, step.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, step.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, step.destroy());
    FT_ASSERT_EQ(FT_CLASS_STATE_DESTROYED, step._initialised_state);
    return (1);
}

FT_TEST(test_game_path_step_reinitialize_after_destroy)
{
    game_path_step step;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, step.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, step.set_x(3));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, step.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, step.initialize());
    FT_ASSERT_EQ(static_cast<ft_size_t>(0), step.get_x());
    return (1);
}

FT_TEST(test_game_path_step_error_string)
{
    game_path_step step;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, step.initialize());
    FT_ASSERT_STR_EQ("Success", step.get_error_str());
    return (1);
}

FT_TEST(test_game_pathfinding_initialize)
{
    game_pathfinding pathfinding;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, pathfinding.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, pathfinding.get_error());
    return (1);
}

FT_TEST(test_game_pathfinding_thread_safety)
{
    game_pathfinding pathfinding;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, pathfinding.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, pathfinding.enable_thread_safety());
    FT_ASSERT_EQ(FT_TRUE, pathfinding.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, pathfinding.disable_thread_safety());
    FT_ASSERT_EQ(FT_FALSE, pathfinding.is_thread_safe());
    return (1);
}

FT_TEST(test_game_pathfinding_empty_grid_path)
{
    game_pathfinding pathfinding;
    game_map3d map_instance;
    ft_vector<game_path_step> path;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, pathfinding.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.initialize(1, 1, 1, 0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, path.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, pathfinding.astar_grid(map_instance, 0, 0, 0,
        0, 0, 0, path));
    FT_ASSERT_EQ(static_cast<ft_size_t>(1), path.size());
    return (1);
}

FT_TEST(test_game_pathfinding_blocked_start)
{
    game_pathfinding pathfinding;
    game_map3d map_instance;
    ft_vector<game_path_step> path;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, pathfinding.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.initialize(2, 1, 1, 0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, path.initialize());
    map_instance.set(0, 0, 0, 1);
    FT_ASSERT_NEQ(FT_ERR_SUCCESS, pathfinding.astar_grid(map_instance, 0, 0, 0,
        1, 0, 0, path));
    return (1);
}

FT_TEST(test_game_pathfinding_out_of_range_goal)
{
    game_pathfinding pathfinding;
    game_map3d map_instance;
    ft_vector<game_path_step> path;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, pathfinding.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.initialize(1, 1, 1, 0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, path.initialize());
    FT_ASSERT_NEQ(FT_ERR_SUCCESS, pathfinding.astar_grid(map_instance, 0, 0, 0,
        1, 0, 0, path));
    return (1);
}

FT_TEST(test_game_pathfinding_destroy)
{
    game_pathfinding pathfinding;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, pathfinding.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, pathfinding.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, pathfinding.destroy());
    return (1);
}

FT_TEST(test_game_path_step_setters_are_independent)
{
    game_path_step step;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, step.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, step.set_coordinates(1, 2, 3));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, step.set_x(4));
    FT_ASSERT_EQ(static_cast<ft_size_t>(4), step.get_x());
    FT_ASSERT_EQ(static_cast<ft_size_t>(2), step.get_y());
    FT_ASSERT_EQ(static_cast<ft_size_t>(3), step.get_z());
    return (1);
}

FT_TEST(test_game_path_step_mutex_error_without_mutex)
{
    game_path_step step;
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, game_path_step_test_helper::get_mutex_error(step));
    return (1);
}

FT_TEST(test_game_path_step_helper_thread_ownership)
{
    game_path_step step;
    pt_thread_id_type thread_id;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, step.initialize());
    thread_id = pt_thread_self();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, game_path_step_test_helper::lock(step));
    FT_ASSERT_EQ(FT_TRUE, game_path_step_test_helper::is_owned_by_thread(step, thread_id));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, game_path_step_test_helper::unlock(step));
    return (1);
}

FT_TEST(test_game_path_step_mutex_is_releasable)
{
    game_path_step step;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, step.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, step.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, step.disable_thread_safety());
    FT_ASSERT_EQ(FT_CLASS_STATE_INITIALISED, step._initialised_state);
    return (1);
}

FT_TEST(test_game_pathfinding_enable_is_idempotent)
{
    game_pathfinding pathfinding;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, pathfinding.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, pathfinding.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, pathfinding.enable_thread_safety());
    return (1);
}

FT_TEST(test_game_pathfinding_update_obstacle_marks_replan)
{
    game_pathfinding pathfinding;
    game_path_step step;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, pathfinding.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, step.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, step.set_coordinates(1, 2, 3));
    pathfinding._current_path.push_back(step);
    pathfinding.update_obstacle(1, 2, 3, 1);
    FT_ASSERT_EQ(FT_TRUE, pathfinding._needs_replan);
    return (1);
}

FT_TEST(test_game_pathfinding_update_obstacle_clear_replan)
{
    game_pathfinding pathfinding;
    game_path_step step;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, pathfinding.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, step.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, step.set_coordinates(1, 2, 3));
    pathfinding._current_path.push_back(step);
    pathfinding.update_obstacle(1, 2, 3, 1);
    pathfinding.update_obstacle(1, 2, 3, 0);
    FT_ASSERT_EQ(FT_TRUE, pathfinding._needs_replan);
    return (1);
}
