#include "../test_internal.hpp"
#include "../../Game/game_pathfinding.hpp"
#include "../../Template/move.hpp"
#include "../../PThread/pthread.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include "../../CPP_class/class_nullptr.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_path_step_recursive_mutex_reentrant_lock,
    "ft_path_step recursive mutex allows reentrant locking")
{
    ft_path_step step;
    int test_failed;
    const char *failure_expression;
    int failure_line;

    test_failed = 0;
    failure_expression = ft_nullptr;
    failure_line = 0;
    if (ft_path_step_test_helper::lock(step) != FT_ERR_SUCCESS)
    {
        test_failed = 1;
        failure_expression = "ft_path_step_test_helper::lock(step) == FT_ERR_SUCCESS";
        failure_line = __LINE__;
    }
    if (test_failed == 0 && ft_path_step_test_helper::lock(step) != FT_ERR_SUCCESS)
    {
        test_failed = 1;
        failure_expression = "ft_path_step_test_helper::lock(step) == FT_ERR_SUCCESS";
        failure_line = __LINE__;
    }
    if (test_failed == 0 && !ft_path_step_test_helper::is_locked(step))
    {
        test_failed = 1;
        failure_expression = "ft_path_step_test_helper::is_locked(step)";
        failure_line = __LINE__;
    }
    if (test_failed == 0 && ft_path_step_test_helper::unlock(step) != FT_ERR_SUCCESS)
    {
        test_failed = 1;
        failure_expression = "ft_path_step_test_helper::unlock(step) == FT_ERR_SUCCESS";
        failure_line = __LINE__;
    }
    if (test_failed == 0 && ft_path_step_test_helper::unlock(step) != FT_ERR_SUCCESS)
    {
        test_failed = 1;
        failure_expression = "ft_path_step_test_helper::unlock(step) == FT_ERR_SUCCESS";
        failure_line = __LINE__;
    }
    if (test_failed == 0 && ft_path_step_test_helper::is_locked(step))
    {
        test_failed = 1;
        failure_expression = "!ft_path_step_test_helper::is_locked(step)";
        failure_line = __LINE__;
    }
    if (test_failed != 0)
    {
        ft_test_fail(failure_expression, __FILE__, failure_line);
        return (0);
    }
    return (1);
}

FT_TEST(test_path_step_recursive_mutex_release_depth,
    "ft_path_step recursive mutex releases fully after matching unlocks")
{
    ft_path_step step;
    int test_failed;
    const char *failure_expression;
    int failure_line;

    test_failed = 0;
    failure_expression = ft_nullptr;
    failure_line = 0;
    if (ft_path_step_test_helper::lock(step) != FT_ERR_SUCCESS)
    {
        test_failed = 1;
        failure_expression = "ft_path_step_test_helper::lock(step) == FT_ERR_SUCCESS";
        failure_line = __LINE__;
    }
    if (test_failed == 0 && ft_path_step_test_helper::lock(step) != FT_ERR_SUCCESS)
    {
        test_failed = 1;
        failure_expression = "ft_path_step_test_helper::lock(step) == FT_ERR_SUCCESS";
        failure_line = __LINE__;
    }
    if (test_failed == 0 && ft_path_step_test_helper::unlock(step) != FT_ERR_SUCCESS)
    {
        test_failed = 1;
        failure_expression = "ft_path_step_test_helper::unlock(step) == FT_ERR_SUCCESS";
        failure_line = __LINE__;
    }
    if (test_failed == 0 && !ft_path_step_test_helper::is_locked(step))
    {
        test_failed = 1;
        failure_expression = "ft_path_step_test_helper::is_locked(step)";
        failure_line = __LINE__;
    }
    if (test_failed == 0 && ft_path_step_test_helper::unlock(step) != FT_ERR_SUCCESS)
    {
        test_failed = 1;
        failure_expression = "ft_path_step_test_helper::unlock(step) == FT_ERR_SUCCESS";
        failure_line = __LINE__;
    }
    if (test_failed == 0 && ft_path_step_test_helper::is_locked(step))
    {
        test_failed = 1;
        failure_expression = "!ft_path_step_test_helper::is_locked(step)";
        failure_line = __LINE__;
    }
    if (test_failed != 0)
    {
        ft_test_fail(failure_expression, __FILE__, failure_line);
        return (0);
    }
    return (1);
}

FT_TEST(test_path_step_recursive_mutex_accessors_while_locked,
    "ft_path_step accessors run while recursion lock is held")
{
    ft_path_step step;
    size_t value;
    int test_failed;
    const char *failure_expression;
    int failure_line;

    test_failed = 0;
    failure_expression = ft_nullptr;
    failure_line = 0;
    if (ft_path_step_test_helper::lock(step) != FT_ERR_SUCCESS)
    {
        test_failed = 1;
        failure_expression = "ft_path_step_test_helper::lock(step) == FT_ERR_SUCCESS";
        failure_line = __LINE__;
    }
    if (test_failed == 0)
    {
        value = step.get_x();
        if (step.get_error() != FT_ERR_SUCCESS)
        {
            (void)value;
            test_failed = 1;
            failure_expression = "step.get_error() == FT_ERR_SUCCESS";
            failure_line = __LINE__;
        }
    }
    if (test_failed == 0 && !ft_path_step_test_helper::is_locked(step))
    {
        test_failed = 1;
        failure_expression = "ft_path_step_test_helper::is_locked(step)";
        failure_line = __LINE__;
    }
    if (test_failed == 0 && ft_path_step_test_helper::unlock(step) != FT_ERR_SUCCESS)
    {
        test_failed = 1;
        failure_expression = "ft_path_step_test_helper::unlock(step) == FT_ERR_SUCCESS";
        failure_line = __LINE__;
    }
    if (test_failed == 0 && ft_path_step_test_helper::is_locked(step))
    {
        test_failed = 1;
        failure_expression = "!ft_path_step_test_helper::is_locked(step)";
        failure_line = __LINE__;
    }
    if (test_failed != 0)
    {
        ft_test_fail(failure_expression, __FILE__, failure_line);
        return (0);
    }
    return (1);
}

FT_TEST(test_path_step_recursive_mutex_copy_constructor_unlocks,
    "ft_path_step copy constructor leaves mutex unlocked")
{
    ft_path_step step;
    ft_path_step copy(step);
    int test_failed;
    const char *failure_expression;
    int failure_line;

    test_failed = 0;
    failure_expression = ft_nullptr;
    failure_line = 0;
    if (ft_path_step_test_helper::is_locked(step))
    {
        test_failed = 1;
        failure_expression = "!ft_path_step_test_helper::is_locked(step)";
        failure_line = __LINE__;
    }
    if (test_failed == 0 && ft_path_step_test_helper::is_locked(copy))
    {
        test_failed = 1;
        failure_expression = "!ft_path_step_test_helper::is_locked(copy)";
        failure_line = __LINE__;
    }
    if (test_failed != 0)
    {
        ft_test_fail(failure_expression, __FILE__, failure_line);
        return (0);
    }
    return (1);
}

FT_TEST(test_path_step_recursive_mutex_set_coordinates_unlocks,
    "ft_path_step set_coordinates does not leave the mutex locked")
{
    ft_path_step step;
    int test_failed;
    const char *failure_expression;
    int failure_line;

    test_failed = 0;
    failure_expression = ft_nullptr;
    failure_line = 0;
    if (step.set_coordinates(1, 2, 3) != FT_ERR_SUCCESS)
    {
        test_failed = 1;
        failure_expression = "step.set_coordinates(1, 2, 3) == FT_ERR_SUCCESS";
        failure_line = __LINE__;
    }
    if (test_failed == 0 && ft_path_step_test_helper::is_locked(step))
    {
        test_failed = 1;
        failure_expression = "!ft_path_step_test_helper::is_locked(step)";
        failure_line = __LINE__;
    }
    if (test_failed != 0)
    {
        ft_test_fail(failure_expression, __FILE__, failure_line);
        return (0);
    }
    return (1);
}

FT_TEST(test_path_step_recursive_mutex_set_axis_unlocks,
    "ft_path_step set_x/set_y/set_z leave the mutex unlocked")
{
    ft_path_step step;
    int test_failed;
    const char *failure_expression;
    int failure_line;

    test_failed = 0;
    failure_expression = ft_nullptr;
    failure_line = 0;
    if (step.set_x(9) != FT_ERR_SUCCESS)
    {
        test_failed = 1;
        failure_expression = "step.set_x(9) == FT_ERR_SUCCESS";
        failure_line = __LINE__;
    }
    if (test_failed == 0 && ft_path_step_test_helper::is_locked(step))
    {
        test_failed = 1;
        failure_expression = "!ft_path_step_test_helper::is_locked(step)";
        failure_line = __LINE__;
    }
    if (test_failed == 0 && step.set_y(10) != FT_ERR_SUCCESS)
    {
        test_failed = 1;
        failure_expression = "step.set_y(10) == FT_ERR_SUCCESS";
        failure_line = __LINE__;
    }
    if (test_failed == 0 && ft_path_step_test_helper::is_locked(step))
    {
        test_failed = 1;
        failure_expression = "!ft_path_step_test_helper::is_locked(step)";
        failure_line = __LINE__;
    }
    if (test_failed == 0 && step.set_z(11) != FT_ERR_SUCCESS)
    {
        test_failed = 1;
        failure_expression = "step.set_z(11) == FT_ERR_SUCCESS";
        failure_line = __LINE__;
    }
    if (test_failed == 0 && ft_path_step_test_helper::is_locked(step))
    {
        test_failed = 1;
        failure_expression = "!ft_path_step_test_helper::is_locked(step)";
        failure_line = __LINE__;
    }
    if (test_failed != 0)
    {
        ft_test_fail(failure_expression, __FILE__, failure_line);
        return (0);
    }
    return (1);
}

FT_TEST(test_path_step_recursive_mutex_getters_unlock,
    "ft_path_step getters do not leave the mutex locked")
{
    ft_path_step step;
    size_t value;
    int test_failed;
    const char *failure_expression;
    int failure_line;

    test_failed = 0;
    failure_expression = ft_nullptr;
    failure_line = 0;
    value = step.get_x();
    if (step.get_error() != FT_ERR_SUCCESS)
    {
        (void)value;
        test_failed = 1;
        failure_expression = "step.get_error() == FT_ERR_SUCCESS";
        failure_line = __LINE__;
    }
    if (test_failed == 0 && ft_path_step_test_helper::is_locked(step))
    {
        test_failed = 1;
        failure_expression = "!ft_path_step_test_helper::is_locked(step)";
        failure_line = __LINE__;
    }
    if (test_failed == 0)
    {
        value = step.get_y();
        if (step.get_error() != FT_ERR_SUCCESS)
        {
            (void)value;
            test_failed = 1;
            failure_expression = "step.get_error() == FT_ERR_SUCCESS";
            failure_line = __LINE__;
        }
    }
    if (test_failed == 0 && ft_path_step_test_helper::is_locked(step))
    {
        test_failed = 1;
        failure_expression = "!ft_path_step_test_helper::is_locked(step)";
        failure_line = __LINE__;
    }
    if (test_failed == 0)
    {
        value = step.get_z();
        if (step.get_error() != FT_ERR_SUCCESS)
        {
            (void)value;
            test_failed = 1;
            failure_expression = "step.get_error() == FT_ERR_SUCCESS";
            failure_line = __LINE__;
        }
    }
    if (test_failed == 0 && ft_path_step_test_helper::is_locked(step))
    {
        test_failed = 1;
        failure_expression = "!ft_path_step_test_helper::is_locked(step)";
        failure_line = __LINE__;
    }
    if (test_failed != 0)
    {
        ft_test_fail(failure_expression, __FILE__, failure_line);
        return (0);
    }
    return (1);
}

FT_TEST(test_path_step_recursive_mutex_get_error_unlocks,
    "ft_path_step get_error does not leave the mutex locked")
{
    ft_path_step step;
    int test_failed;
    const char *failure_expression;
    int failure_line;

    test_failed = 0;
    failure_expression = ft_nullptr;
    failure_line = 0;
    if (step.get_error() != FT_ERR_SUCCESS)
    {
        test_failed = 1;
        failure_expression = "step.get_error() == FT_ERR_SUCCESS";
        failure_line = __LINE__;
    }
    if (test_failed == 0 && ft_path_step_test_helper::is_locked(step))
    {
        test_failed = 1;
        failure_expression = "!ft_path_step_test_helper::is_locked(step)";
        failure_line = __LINE__;
    }
    if (test_failed != 0)
    {
        ft_test_fail(failure_expression, __FILE__, failure_line);
        return (0);
    }
    return (1);
}

FT_TEST(test_path_step_recursive_mutex_assignment_unlocks,
    "ft_path_step assignment leaves both mutexes unlocked")
{
    ft_path_step step;
    ft_path_step target;
    int test_failed;
    const char *failure_expression;
    int failure_line;

    test_failed = 0;
    failure_expression = ft_nullptr;
    failure_line = 0;
    target = step;
    if (target.get_error() != FT_ERR_SUCCESS)
    {
        test_failed = 1;
        failure_expression = "target.get_error() == FT_ERR_SUCCESS";
        failure_line = __LINE__;
    }
    if (test_failed == 0 && ft_path_step_test_helper::is_locked(step))
    {
        test_failed = 1;
        failure_expression = "!ft_path_step_test_helper::is_locked(step)";
        failure_line = __LINE__;
    }
    if (test_failed == 0 && ft_path_step_test_helper::is_locked(target))
    {
        test_failed = 1;
        failure_expression = "!ft_path_step_test_helper::is_locked(target)";
        failure_line = __LINE__;
    }
    if (test_failed != 0)
    {
        ft_test_fail(failure_expression, __FILE__, failure_line);
        return (0);
    }
    return (1);
}

FT_TEST(test_path_step_recursive_mutex_move_assignment_unlocks,
    "ft_path_step move assignment leaves mutexes unlocked")
{
    ft_path_step source;
    ft_path_step target;
    int test_failed;
    const char *failure_expression;
    int failure_line;

    test_failed = 0;
    failure_expression = ft_nullptr;
    failure_line = 0;
    target = ft_move(source);
    if (target.get_error() != FT_ERR_SUCCESS)
    {
        test_failed = 1;
        failure_expression = "target.get_error() == FT_ERR_SUCCESS";
        failure_line = __LINE__;
    }
    if (test_failed == 0 && ft_path_step_test_helper::is_locked(target))
    {
        test_failed = 1;
        failure_expression = "!ft_path_step_test_helper::is_locked(target)";
        failure_line = __LINE__;
    }
    if (test_failed == 0 && ft_path_step_test_helper::is_locked(source))
    {
        test_failed = 1;
        failure_expression = "!ft_path_step_test_helper::is_locked(source)";
        failure_line = __LINE__;
    }
    if (test_failed != 0)
    {
        ft_test_fail(failure_expression, __FILE__, failure_line);
        return (0);
    }
    return (1);
}

FT_TEST(test_path_step_recursive_mutex_get_error_str_unlocks,
    "ft_path_step get_error_str does not leave the mutex locked")
{
    ft_path_step step;
    const char *message;
    int test_failed;
    const char *failure_expression;
    int failure_line;

    test_failed = 0;
    failure_expression = ft_nullptr;
    failure_line = 0;
    message = step.get_error_str();
    if (message == ft_nullptr)
    {
        test_failed = 1;
        failure_expression = "step.get_error_str() != ft_nullptr";
        failure_line = __LINE__;
    }
    if (test_failed == 0 && ft_path_step_test_helper::is_locked(step))
    {
        test_failed = 1;
        failure_expression = "!ft_path_step_test_helper::is_locked(step)";
        failure_line = __LINE__;
    }
    if (test_failed != 0)
    {
        ft_test_fail(failure_expression, __FILE__, failure_line);
        return (0);
    }
    return (1);
}

FT_TEST(test_path_step_recursive_mutex_owned_state,
    "ft_path_step recursive mutex reports ownership while locked")
{
    ft_path_step step;
    int test_failed;
    const char *failure_expression;
    int failure_line;

    test_failed = 0;
    failure_expression = ft_nullptr;
    failure_line = 0;
    if (ft_path_step_test_helper::is_owned_by_thread(step, THREAD_ID))
    {
        test_failed = 1;
        failure_expression = "!ft_path_step_test_helper::is_owned_by_thread(step, THREAD_ID)";
        failure_line = __LINE__;
    }
    if (test_failed == 0 && ft_path_step_test_helper::lock(step) != FT_ERR_SUCCESS)
    {
        test_failed = 1;
        failure_expression = "ft_path_step_test_helper::lock(step) == FT_ERR_SUCCESS";
        failure_line = __LINE__;
    }
    if (test_failed == 0 && !ft_path_step_test_helper::is_owned_by_thread(step, THREAD_ID))
    {
        test_failed = 1;
        failure_expression = "ft_path_step_test_helper::is_owned_by_thread(step, THREAD_ID)";
        failure_line = __LINE__;
    }
    if (test_failed == 0 && ft_path_step_test_helper::unlock(step) != FT_ERR_SUCCESS)
    {
        test_failed = 1;
        failure_expression = "ft_path_step_test_helper::unlock(step) == FT_ERR_SUCCESS";
        failure_line = __LINE__;
    }
    if (test_failed == 0 && ft_path_step_test_helper::is_owned_by_thread(step, THREAD_ID))
    {
        test_failed = 1;
        failure_expression = "!ft_path_step_test_helper::is_owned_by_thread(step, THREAD_ID)";
        failure_line = __LINE__;
    }
    if (test_failed != 0)
    {
        ft_test_fail(failure_expression, __FILE__, failure_line);
        return (0);
    }
    return (1);
}
