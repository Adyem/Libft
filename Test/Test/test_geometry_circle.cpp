#include "../test_internal.hpp"
#include "../../Geometry/geometry.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"
#include "../../Errno/errno.hpp"
#include "../../CMA/CMA.hpp"
#include <atomic>
#include <cmath>
#include <chrono>
#include <csignal>
#include <limits>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>

#ifndef LIBFT_TEST_BUILD
#endif

static int geometry_expect_sigabrt(void (*operation)())
{
    pid_t child_process_id;
    int child_status;

    child_process_id = fork();
    if (child_process_id == 0)
    {
        operation();
        _exit(0);
    }
    if (child_process_id < 0)
        return (0);
    child_status = 0;
    if (waitpid(child_process_id, &child_status, 0) < 0)
        return (0);
    if (WIFSIGNALED(child_status) == 0)
        return (0);
    return (WTERMSIG(child_status) == SIGABRT);
}

static void circle_initialize_twice_aborts_operation()
{
    circle shape;
    (void)shape.initialize(0.0, 0.0, 1.0);

    (void)shape.initialize();
    return ;
}

static void circle_destroy_uninitialised_aborts_operation()
{
    circle shape;

    (void)shape.destroy();
    return ;
}

static void circle_set_radius_uninitialised_aborts_operation()
{
    circle shape;

    (void)shape.set_radius(1.0);
    return ;
}

static void circle_destroy_twice_aborts_operation()
{
    circle shape;
    (void)shape.initialize(0.0, 0.0, 1.0);

    (void)shape.destroy();
    (void)shape.destroy();
    return ;
}

static void circle_destructor_uninitialised_aborts_operation()
{
    circle shape;

    (void)shape;
    return ;
}

static void circle_move_self_uninitialised_aborts_operation()
{
    circle shape;

    (void)shape.move(shape);
    return ;
}

static void circle_initialize_copy_from_uninitialised_source_aborts_operation()
{
    circle source;
    circle destination;
    (void)destination.initialize(0.0, 0.0, 1.0);

    (void)destination.destroy();
    (void)destination.initialize(source);
    return ;
}

static void circle_initialize_move_from_uninitialised_source_aborts_operation()
{
    circle source;
    circle destination;
    (void)destination.initialize(0.0, 0.0, 1.0);

    (void)destination.destroy();
    (void)destination.initialize(static_cast<circle &&>(source));
    return ;
}

static void circle_initialize_copy_from_destroyed_source_aborts_operation()
{
    circle source;
    circle destination;
    (void)destination.initialize(0.0, 0.0, 1.0);

    (void)source.initialize(2.0, 3.0, 4.0);
    (void)source.destroy();
    (void)destination.destroy();
    (void)destination.initialize(source);
    return ;
}

static void circle_initialize_move_from_destroyed_source_aborts_operation()
{
    circle source;
    circle destination;
    (void)destination.initialize(0.0, 0.0, 1.0);

    (void)source.initialize(2.0, 3.0, 4.0);
    (void)source.destroy();
    (void)destination.destroy();
    (void)destination.initialize(static_cast<circle &&>(source));
    return ;
}

static void circle_get_center_x_uninitialised_aborts_operation()
{
    circle shape;

    (void)shape.get_center_x();
    return ;
}

static void circle_enable_thread_safety_destroyed_aborts_operation()
{
    circle shape;
    (void)shape.initialize(0.0, 0.0, 1.0);

    (void)shape.destroy();
    (void)shape.enable_thread_safety();
    return ;
}

static void circle_initialize_copy_destination_initialised_aborts_operation()
{
    circle source;
    (void)source.initialize(0.0, 0.0, 1.0);
    circle destination;
    (void)destination.initialize(2.0, 2.0, 3.0);

    (void)destination.initialize(source);
    return ;
}

static void circle_initialize_move_destination_initialised_aborts_operation()
{
    circle source;
    (void)source.initialize(0.0, 0.0, 1.0);
    circle destination;
    (void)destination.initialize(2.0, 2.0, 3.0);

    (void)destination.initialize(static_cast<circle &&>(source));
    return ;
}

static void circle_disable_thread_safety_destroyed_aborts_operation()
{
    circle shape;
    (void)shape.initialize(0.0, 0.0, 1.0);

    (void)shape.destroy();
    shape.disable_thread_safety();
    return ;
}

static void circle_is_thread_safe_enabled_destroyed_aborts_operation()
{
    circle shape;
    (void)shape.initialize(0.0, 0.0, 1.0);

    (void)shape.destroy();
    (void)shape.is_thread_safe();
    return ;
}

static void circle_set_center_x_uninitialised_aborts_operation()
{
    circle shape;

    (void)shape.set_center_x(1.0);
    return ;
}

static void circle_get_center_y_uninitialised_aborts_operation()
{
    circle shape;

    (void)shape.get_center_y();
    return ;
}

static void circle_set_center_uninitialised_aborts_operation()
{
    circle shape;

    (void)shape.set_center(1.0, 2.0);
    return ;
}

static void circle_get_radius_uninitialised_aborts_operation()
{
    circle shape;

    (void)shape.get_radius();
    return ;
}

static void circle_enable_thread_safety_uninitialised_aborts_operation()
{
    circle shape;

    (void)shape.enable_thread_safety();
    return ;
}

static void circle_set_center_y_uninitialised_aborts_operation()
{
    circle shape;

    (void)shape.set_center_y(1.0);
    return ;
}

static void circle_get_center_x_second_uninitialised_aborts_operation()
{
    circle shape;

    (void)shape.get_center_x();
    return ;
}

static void circle_disable_thread_safety_uninitialised_aborts_operation()
{
    circle shape;

    shape.disable_thread_safety();
    return ;
}

static void circle_is_thread_safe_enabled_uninitialised_aborts_operation()
{
    circle shape;

    (void)shape.is_thread_safe();
    return ;
}

static void circle_move_two_uninitialised_aborts_operation()
{
    circle source;
    circle destination;

    (void)destination.move(source);
    return ;
}

static void circle_set_center_destroyed_aborts_operation()
{
    circle shape;
    (void)shape.initialize(0.0, 0.0, 1.0);

    (void)shape.destroy();
    (void)shape.set_center(1.0, 2.0);
    return ;
}

static void circle_get_center_x_destroyed_aborts_operation()
{
    circle shape;
    (void)shape.initialize(0.0, 0.0, 1.0);

    (void)shape.destroy();
    (void)shape.get_center_x();
    return ;
}

static void circle_set_radius_destroyed_aborts_operation()
{
    circle shape;
    (void)shape.initialize(0.0, 0.0, 1.0);

    (void)shape.destroy();
    (void)shape.set_radius(2.0);
    return ;
}

static void circle_set_center_x_destroyed_aborts_operation()
{
    circle shape;
    (void)shape.initialize(0.0, 0.0, 1.0);

    (void)shape.destroy();
    (void)shape.set_center_x(2.0);
    return ;
}

static void circle_get_radius_destroyed_aborts_operation()
{
    circle shape;
    (void)shape.initialize(0.0, 0.0, 1.0);

    (void)shape.destroy();
    (void)shape.get_radius();
    return ;
}

static void circle_move_destroyed_source_aborts_operation()
{
    circle source;
    (void)source.initialize(0.0, 0.0, 1.0);
    circle destination;
    (void)destination.initialize(2.0, 2.0, 3.0);

    (void)source.destroy();
    (void)destination.move(source);
    return ;
}

static void circle_set_center_y_destroyed_aborts_operation()
{
    circle shape;
    (void)shape.initialize(0.0, 0.0, 1.0);

    (void)shape.destroy();
    (void)shape.set_center_y(1.0);
    return ;
}

static void circle_get_center_y_destroyed_aborts_operation()
{
    circle shape;
    (void)shape.initialize(0.0, 0.0, 1.0);

    (void)shape.destroy();
    (void)shape.get_center_y();
    return ;
}

static int circle_move_destroyed_destination_succeeds_operation()
{
    circle source;
    (void)source.initialize(0.0, 0.0, 1.0);
    circle destination;
    (void)destination.initialize(2.0, 2.0, 3.0);
    int move_error;

    (void)destination.destroy();
    move_error = destination.move(source);
    if (move_error != FT_ERR_SUCCESS)
        return (0);
    if (destination.get_center_x() != 0.0 || destination.get_center_y() != 0.0
        || destination.get_radius() != 1.0)
        return (0);
    if (source.get_center_x() != 0.0 || source.get_center_y() != 0.0
        || source.get_radius() != 0.0)
        return (0);
    return (1);
}

static void circle_move_uninitialised_source_aborts_operation()
{
    circle source;
    circle destination;
    (void)destination.initialize(2.0, 2.0, 3.0);

    (void)destination.move(source);
    return ;
}

static void circle_set_center_y_uninitialised_second_aborts_operation()
{
    circle shape;

    (void)shape.set_center_y(2.0);
    return ;
}

static void intersect_circle_uninitialised_first_aborts_operation()
{
    circle first;
    circle second;
    (void)second.initialize(0.0, 0.0, 1.0);

    (void)intersect_circle(first, second);
    return ;
}

static void intersect_circle_uninitialised_second_aborts_operation()
{
    circle first;
    (void)first.initialize(0.0, 0.0, 1.0);
    circle second;

    (void)intersect_circle(first, second);
    return ;
}

static void intersect_circle_uninitialised_both_aborts_operation()
{
    circle first;
    circle second;

    (void)intersect_circle(first, second);
    return ;
}

static void intersect_circle_destroyed_first_aborts_operation()
{
    circle first;
    (void)first.initialize(0.0, 0.0, 1.0);
    circle second;
    (void)second.initialize(0.0, 0.0, 1.0);

    (void)first.destroy();
    (void)intersect_circle(first, second);
    return ;
}

static void intersect_circle_destroyed_second_aborts_operation()
{
    circle first;
    (void)first.initialize(0.0, 0.0, 1.0);
    circle second;
    (void)second.initialize(0.0, 0.0, 1.0);

    (void)second.destroy();
    (void)intersect_circle(first, second);
    return ;
}

static void intersect_circle_destroyed_both_aborts_operation()
{
    circle first;
    (void)first.initialize(0.0, 0.0, 1.0);
    circle second;
    (void)second.initialize(0.0, 0.0, 1.0);

    (void)first.destroy();
    (void)second.destroy();
    (void)intersect_circle(first, second);
    return ;
}

static void intersect_circle_destroyed_and_uninitialised_aborts_operation()
{
    circle first;
    (void)first.initialize(0.0, 0.0, 1.0);
    circle second;

    (void)first.destroy();
    (void)intersect_circle(first, second);
    return ;
}

FT_TEST(test_circle_initialize_setters_and_getters)
{
    circle shape;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.set_center(-3.0, 4.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.set_center_x(-2.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.set_center_y(5.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.set_radius(6.5));
    FT_ASSERT_DOUBLE_EQ(-2.0, shape.get_center_x());
    FT_ASSERT_DOUBLE_EQ(5.0, shape.get_center_y());
    FT_ASSERT_DOUBLE_EQ(6.5, shape.get_radius());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_circle_reinitialize_after_destroy)
{
    circle shape;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(1.0, 2.0, 3.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(-4.0, 5.0, 6.0));
    FT_ASSERT_DOUBLE_EQ(-4.0, shape.get_center_x());
    FT_ASSERT_DOUBLE_EQ(5.0, shape.get_center_y());
    FT_ASSERT_DOUBLE_EQ(6.0, shape.get_radius());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_circle_negative_radius_behavior)
{
    circle shape;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(0.0, 0.0, -3.0));
    FT_ASSERT_DOUBLE_EQ(-3.0, shape.get_radius());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.set_radius(-7.5));
    FT_ASSERT_DOUBLE_EQ(-7.5, shape.get_radius());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_circle_large_finite_values_roundtrip)
{
    circle shape;
    double large_value;

    large_value = std::numeric_limits<double>::max() / 8.0;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(
            large_value, -large_value / 2.0, large_value / 4.0));
    FT_ASSERT_DOUBLE_EQ(large_value, shape.get_center_x());
    FT_ASSERT_DOUBLE_EQ(-large_value / 2.0, shape.get_center_y());
    FT_ASSERT_DOUBLE_EQ(large_value / 4.0, shape.get_radius());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_circle_smallest_normal_radius_roundtrip)
{
    circle shape;
    double small_value;

    small_value = std::numeric_limits<double>::min();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(0.0, 0.0, small_value));
    FT_ASSERT_DOUBLE_EQ(small_value, shape.get_radius());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_circle_infinite_radius_roundtrip)
{
    circle shape;
    double infinity_value;

    infinity_value = std::numeric_limits<double>::infinity();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(0.0, 0.0, infinity_value));
    FT_ASSERT(std::isinf(shape.get_radius()));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_circle_negative_zero_center_x_sign_preserved)
{
    circle shape;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(1.0, 1.0, 1.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.set_center_x(-0.0));
    FT_ASSERT(std::signbit(shape.get_center_x()));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_circle_negative_zero_center_y_sign_preserved)
{
    circle shape;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(1.0, 1.0, 1.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.set_center_y(-0.0));
    FT_ASSERT(std::signbit(shape.get_center_y()));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_circle_set_center_large_values)
{
    circle shape;
    double large_value;

    large_value = std::numeric_limits<double>::max() / 8.0;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(0.0, 0.0, 1.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.set_center(large_value, -large_value));
    FT_ASSERT_DOUBLE_EQ(large_value, shape.get_center_x());
    FT_ASSERT_DOUBLE_EQ(-large_value, shape.get_center_y());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_circle_reinitialize_with_infinite_radius)
{
    circle shape;
    double infinity_value;

    infinity_value = std::numeric_limits<double>::infinity();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(1.0, 2.0, 3.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(0.0, 0.0, infinity_value));
    FT_ASSERT(std::isinf(shape.get_radius()));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_circle_lifecycle_multiple_reinitialize_cycles)
{
    circle shape;
    int iteration_index;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(1.0, 2.0, 3.0));
    iteration_index = 0;
    while (iteration_index < 6)
    {
        FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
        FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(
                -1.0 - iteration_index,
                2.0 + iteration_index,
                3.0 + iteration_index));
        FT_ASSERT_DOUBLE_EQ(-1.0 - iteration_index, shape.get_center_x());
        FT_ASSERT_DOUBLE_EQ(2.0 + iteration_index, shape.get_center_y());
        FT_ASSERT_DOUBLE_EQ(3.0 + iteration_index, shape.get_radius());
        iteration_index = iteration_index + 1;
    }
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_circle_lifecycle_thread_safety_cycles)
{
    circle shape;
    int iteration_index;

    iteration_index = 0;
    while (iteration_index < 6)
    {
        FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(
                -1.0 - iteration_index,
                2.0 + iteration_index,
                3.0 + iteration_index));
        FT_ASSERT_EQ(false, shape.is_thread_safe());
        FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.enable_thread_safety());
        FT_ASSERT_EQ(true, shape.is_thread_safe());
        shape.disable_thread_safety();
        FT_ASSERT_EQ(false, shape.is_thread_safe());
        FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
        iteration_index = iteration_index + 1;
    }
    return (1);
}

FT_TEST(test_circle_enable_thread_safety_failure_then_recovery)
{
    circle shape;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(0.0, 0.0, 1.0));
    cma_set_alloc_limit(1);
    error_code = shape.enable_thread_safety();
    cma_set_alloc_limit(0);
    FT_ASSERT_NEQ(FT_ERR_SUCCESS, error_code);
    FT_ASSERT_EQ(false, shape.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.enable_thread_safety());
    FT_ASSERT_EQ(true, shape.is_thread_safe());
    shape.disable_thread_safety();
    FT_ASSERT_EQ(false, shape.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_circle_enable_thread_safety_repeated_alloc_failures)
{
    circle shape;
    int error_code;
    int iteration_index;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(0.0, 0.0, 1.0));
    cma_set_alloc_limit(1);
    iteration_index = 0;
    while (iteration_index < 3)
    {
        error_code = shape.enable_thread_safety();
        FT_ASSERT_NEQ(FT_ERR_SUCCESS, error_code);
        FT_ASSERT_EQ(false, shape.is_thread_safe());
        iteration_index = iteration_index + 1;
    }
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_circle_enable_thread_safety_alloc_failure_then_destroy)
{
    circle shape;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(-2.0, -2.0, 2.0));
    cma_set_alloc_limit(1);
    error_code = shape.enable_thread_safety();
    cma_set_alloc_limit(0);
    FT_ASSERT_NEQ(FT_ERR_SUCCESS, error_code);
    FT_ASSERT_EQ(false, shape.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_circle_enable_thread_safety_alloc_failure_after_disable)
{
    circle shape;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(-1.0, -1.0, 1.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.enable_thread_safety());
    shape.disable_thread_safety();
    FT_ASSERT_EQ(false, shape.is_thread_safe());
    cma_set_alloc_limit(1);
    error_code = shape.enable_thread_safety();
    cma_set_alloc_limit(0);
    FT_ASSERT_NEQ(FT_ERR_SUCCESS, error_code);
    FT_ASSERT_EQ(false, shape.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_circle_get_mutex_for_testing_alloc_failure_returns_null)
{
    circle shape;
    pt_recursive_mutex *mutex_pointer;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(0.0, 0.0, 1.0));
    cma_set_alloc_limit(1);
    mutex_pointer = shape._mutex;
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(ft_nullptr, mutex_pointer);
    FT_ASSERT_EQ(false, shape.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_circle_get_mutex_for_testing_recovers_after_alloc_failure)
{
    circle shape;
    pt_recursive_mutex *mutex_pointer;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(0.0, 0.0, 1.0));
    cma_set_alloc_limit(1);
    mutex_pointer = shape._mutex;
    FT_ASSERT_EQ(ft_nullptr, mutex_pointer);
    cma_set_alloc_limit(0);
    mutex_pointer = shape._mutex;
    FT_ASSERT_NEQ(ft_nullptr, mutex_pointer);
    FT_ASSERT_EQ(true, shape.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_circle_enable_thread_safety_alloc_failure_reinit_cycle)
{
    circle shape;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(-3.0, -3.0, 3.0));
    cma_set_alloc_limit(1);
    error_code = shape.enable_thread_safety();
    cma_set_alloc_limit(0);
    FT_ASSERT_NEQ(FT_ERR_SUCCESS, error_code);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(-4.0, -4.0, 4.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.enable_thread_safety());
    FT_ASSERT_EQ(true, shape.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_circle_enable_thread_safety_alloc_failure_preserves_values)
{
    circle shape;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(-5.0, -6.0, 7.0));
    cma_set_alloc_limit(1);
    error_code = shape.enable_thread_safety();
    cma_set_alloc_limit(0);
    FT_ASSERT_NEQ(FT_ERR_SUCCESS, error_code);
    FT_ASSERT_DOUBLE_EQ(-5.0, shape.get_center_x());
    FT_ASSERT_DOUBLE_EQ(-6.0, shape.get_center_y());
    FT_ASSERT_DOUBLE_EQ(7.0, shape.get_radius());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_circle_enable_thread_safety_alloc_limit_two_state_consistent)
{
    circle shape;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(-1.0, -1.0, 1.0));
    cma_set_alloc_limit(2);
    error_code = shape.enable_thread_safety();
    cma_set_alloc_limit(0);
    if (error_code == FT_ERR_SUCCESS)
        FT_ASSERT_EQ(true, shape.is_thread_safe());
    else
        FT_ASSERT_EQ(false, shape.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_circle_enable_thread_safety_alloc_limit_three_state_consistent)
{
    circle shape;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(-1.0, -1.0, 1.0));
    cma_set_alloc_limit(3);
    error_code = shape.enable_thread_safety();
    cma_set_alloc_limit(0);
    if (error_code == FT_ERR_SUCCESS)
        FT_ASSERT_EQ(true, shape.is_thread_safe());
    else
        FT_ASSERT_EQ(false, shape.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_circle_enable_thread_safety_alloc_limit_two_then_recover)
{
    circle shape;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(-2.0, -2.0, 2.0));
    cma_set_alloc_limit(2);
    error_code = shape.enable_thread_safety();
    cma_set_alloc_limit(0);
    if (error_code == FT_ERR_SUCCESS)
    {
        FT_ASSERT_EQ(true, shape.is_thread_safe());
        shape.disable_thread_safety();
    }
    FT_ASSERT_EQ(false, shape.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.enable_thread_safety());
    FT_ASSERT_EQ(true, shape.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_circle_enable_thread_safety_alloc_limit_three_then_recover)
{
    circle shape;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(-2.0, -2.0, 2.0));
    cma_set_alloc_limit(3);
    error_code = shape.enable_thread_safety();
    cma_set_alloc_limit(0);
    if (error_code == FT_ERR_SUCCESS)
    {
        FT_ASSERT_EQ(true, shape.is_thread_safe());
        shape.disable_thread_safety();
    }
    FT_ASSERT_EQ(false, shape.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.enable_thread_safety());
    FT_ASSERT_EQ(true, shape.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_circle_get_mutex_for_testing_alloc_limit_two_consistent)
{
    circle shape;
    pt_recursive_mutex *mutex_pointer;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(0.0, 0.0, 1.0));
    cma_set_alloc_limit(2);
    mutex_pointer = shape._mutex;
    cma_set_alloc_limit(0);
    if (mutex_pointer == ft_nullptr)
        FT_ASSERT_EQ(false, shape.is_thread_safe());
    else
        FT_ASSERT_EQ(true, shape.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_circle_get_mutex_for_testing_alloc_limit_three_consistent)
{
    circle shape;
    pt_recursive_mutex *mutex_pointer;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(0.0, 0.0, 1.0));
    cma_set_alloc_limit(3);
    mutex_pointer = shape._mutex;
    cma_set_alloc_limit(0);
    if (mutex_pointer == ft_nullptr)
        FT_ASSERT_EQ(false, shape.is_thread_safe());
    else
        FT_ASSERT_EQ(true, shape.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_circle_alloc_limit_two_preserves_values)
{
    circle shape;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(-9.0, -8.0, 7.0));
    cma_set_alloc_limit(2);
    (void)shape.enable_thread_safety();
    cma_set_alloc_limit(0);
    FT_ASSERT_DOUBLE_EQ(-9.0, shape.get_center_x());
    FT_ASSERT_DOUBLE_EQ(-8.0, shape.get_center_y());
    FT_ASSERT_DOUBLE_EQ(7.0, shape.get_radius());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_circle_enable_thread_safety_alloc_limit_four_state_consistent)
{
    circle shape;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(-1.0, -1.0, 1.0));
    cma_set_alloc_limit(4);
    error_code = shape.enable_thread_safety();
    cma_set_alloc_limit(0);
    if (error_code == FT_ERR_SUCCESS)
        FT_ASSERT_EQ(true, shape.is_thread_safe());
    else
        FT_ASSERT_EQ(false, shape.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_circle_enable_thread_safety_alloc_limit_five_state_consistent)
{
    circle shape;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(-1.0, -1.0, 1.0));
    cma_set_alloc_limit(5);
    error_code = shape.enable_thread_safety();
    cma_set_alloc_limit(0);
    if (error_code == FT_ERR_SUCCESS)
        FT_ASSERT_EQ(true, shape.is_thread_safe());
    else
        FT_ASSERT_EQ(false, shape.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_circle_enable_thread_safety_alloc_limit_six_state_consistent)
{
    circle shape;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(-1.0, -1.0, 1.0));
    cma_set_alloc_limit(6);
    error_code = shape.enable_thread_safety();
    cma_set_alloc_limit(0);
    if (error_code == FT_ERR_SUCCESS)
        FT_ASSERT_EQ(true, shape.is_thread_safe());
    else
        FT_ASSERT_EQ(false, shape.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_circle_enable_thread_safety_alloc_limit_four_then_recover)
{
    circle shape;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(-2.0, -2.0, 2.0));
    cma_set_alloc_limit(4);
    error_code = shape.enable_thread_safety();
    cma_set_alloc_limit(0);
    if (error_code == FT_ERR_SUCCESS)
    {
        shape.disable_thread_safety();
        FT_ASSERT_EQ(false, shape.is_thread_safe());
    }
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.enable_thread_safety());
    FT_ASSERT_EQ(true, shape.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_circle_enable_thread_safety_alloc_limit_five_then_recover)
{
    circle shape;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(-2.0, -2.0, 2.0));
    cma_set_alloc_limit(5);
    error_code = shape.enable_thread_safety();
    cma_set_alloc_limit(0);
    if (error_code == FT_ERR_SUCCESS)
    {
        shape.disable_thread_safety();
        FT_ASSERT_EQ(false, shape.is_thread_safe());
    }
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.enable_thread_safety());
    FT_ASSERT_EQ(true, shape.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_circle_get_mutex_for_testing_alloc_limit_four_consistent)
{
    circle shape;
    pt_recursive_mutex *mutex_pointer;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(0.0, 0.0, 1.0));
    cma_set_alloc_limit(4);
    mutex_pointer = shape._mutex;
    cma_set_alloc_limit(0);
    if (mutex_pointer == ft_nullptr)
        FT_ASSERT_EQ(false, shape.is_thread_safe());
    else
        FT_ASSERT_EQ(true, shape.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_circle_alloc_limit_four_preserves_values)
{
    circle shape;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(-9.0, -8.0, 7.0));
    cma_set_alloc_limit(4);
    (void)shape.enable_thread_safety();
    cma_set_alloc_limit(0);
    FT_ASSERT_DOUBLE_EQ(-9.0, shape.get_center_x());
    FT_ASSERT_DOUBLE_EQ(-8.0, shape.get_center_y());
    FT_ASSERT_DOUBLE_EQ(7.0, shape.get_radius());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_circle_enable_thread_safety_alloc_limit_seven_state_consistent)
{
    circle shape;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(-1.0, -1.0, 1.0));
    cma_set_alloc_limit(7);
    error_code = shape.enable_thread_safety();
    cma_set_alloc_limit(0);
    if (error_code == FT_ERR_SUCCESS)
        FT_ASSERT_EQ(true, shape.is_thread_safe());
    else
        FT_ASSERT_EQ(false, shape.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_circle_enable_thread_safety_alloc_limit_eight_state_consistent)
{
    circle shape;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(-1.0, -1.0, 1.0));
    cma_set_alloc_limit(8);
    error_code = shape.enable_thread_safety();
    cma_set_alloc_limit(0);
    if (error_code == FT_ERR_SUCCESS)
        FT_ASSERT_EQ(true, shape.is_thread_safe());
    else
        FT_ASSERT_EQ(false, shape.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_circle_enable_thread_safety_alloc_limit_nine_state_consistent)
{
    circle shape;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(-1.0, -1.0, 1.0));
    cma_set_alloc_limit(9);
    error_code = shape.enable_thread_safety();
    cma_set_alloc_limit(0);
    if (error_code == FT_ERR_SUCCESS)
        FT_ASSERT_EQ(true, shape.is_thread_safe());
    else
        FT_ASSERT_EQ(false, shape.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_circle_enable_thread_safety_alloc_limit_seven_then_recover)
{
    circle shape;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(-2.0, -2.0, 2.0));
    cma_set_alloc_limit(7);
    error_code = shape.enable_thread_safety();
    cma_set_alloc_limit(0);
    if (error_code == FT_ERR_SUCCESS)
    {
        shape.disable_thread_safety();
        FT_ASSERT_EQ(false, shape.is_thread_safe());
    }
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.enable_thread_safety());
    FT_ASSERT_EQ(true, shape.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_circle_enable_thread_safety_alloc_limit_eight_then_recover)
{
    circle shape;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(-2.0, -2.0, 2.0));
    cma_set_alloc_limit(8);
    error_code = shape.enable_thread_safety();
    cma_set_alloc_limit(0);
    if (error_code == FT_ERR_SUCCESS)
    {
        shape.disable_thread_safety();
        FT_ASSERT_EQ(false, shape.is_thread_safe());
    }
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.enable_thread_safety());
    FT_ASSERT_EQ(true, shape.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_circle_get_mutex_for_testing_alloc_limit_seven_consistent)
{
    circle shape;
    pt_recursive_mutex *mutex_pointer;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(0.0, 0.0, 1.0));
    cma_set_alloc_limit(7);
    mutex_pointer = shape._mutex;
    cma_set_alloc_limit(0);
    if (mutex_pointer == ft_nullptr)
        FT_ASSERT_EQ(false, shape.is_thread_safe());
    else
        FT_ASSERT_EQ(true, shape.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_circle_alloc_limit_seven_preserves_values)
{
    circle shape;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(-9.0, -8.0, 7.0));
    cma_set_alloc_limit(7);
    (void)shape.enable_thread_safety();
    cma_set_alloc_limit(0);
    FT_ASSERT_DOUBLE_EQ(-9.0, shape.get_center_x());
    FT_ASSERT_DOUBLE_EQ(-8.0, shape.get_center_y());
    FT_ASSERT_DOUBLE_EQ(7.0, shape.get_radius());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_intersect_circle_overlap)
{
    circle first;
    circle second;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.initialize(0.0, 0.0, 4.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.initialize(3.0, 0.0, 3.0));
    FT_ASSERT(intersect_circle(first, second));
    FT_ASSERT(intersect_circle(second, first));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.destroy());
    return (1);
}

FT_TEST(test_intersect_circle_separated)
{
    circle first;
    circle second;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.initialize(-5.0, -5.0, 2.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.initialize(2.0, 2.0, 1.5));
    FT_ASSERT_EQ(false, intersect_circle(first, second));
    FT_ASSERT_EQ(false, intersect_circle(second, first));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.destroy());
    return (1);
}

FT_TEST(test_intersect_circle_touching)
{
    circle first;
    circle second;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.initialize(0.0, 0.0, 3.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.initialize(4.0, 0.0, 1.0));
    FT_ASSERT(intersect_circle(first, second));
    FT_ASSERT(intersect_circle(second, first));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.destroy());
    return (1);
}

FT_TEST(test_intersect_circle_parallel_access)
{
    circle first;
    circle second;
    std::atomic<bool> worker_ready;
    std::atomic<bool> worker_failed;
    std::atomic<bool> worker_completed;
    std::thread worker_thread;
    int iteration_index;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.initialize(0.0, 0.0, 5.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.initialize(1.0, 1.0, 4.0));
    worker_ready.store(false);
    worker_failed.store(false);
    worker_completed.store(false);
    worker_thread = std::thread([&first, &second, &worker_ready, &worker_failed,
                &worker_completed]() {
        int local_iteration_index;

        worker_ready.store(true);
        local_iteration_index = 0;
        while (local_iteration_index < 256 && worker_failed.load() == false)
        {
            if (intersect_circle(first, second) == false)
            {
                worker_failed.store(true);
                break;
            }
            local_iteration_index = local_iteration_index + 1;
        }
        worker_completed.store(true);
        return ;
    });
    while (worker_ready.load() == false)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    iteration_index = 0;
    while (iteration_index < 256)
    {
        if (intersect_circle(second, first) == false)
        {
            worker_failed.store(true);
            break;
        }
        iteration_index = iteration_index + 1;
    }
    worker_thread.join();
    FT_ASSERT(worker_completed.load());
    FT_ASSERT_EQ(false, worker_failed.load());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.destroy());
    return (1);
}

FT_TEST(test_intersect_circle_high_load_overlap_two_threads)
{
    circle first;
    circle second;
    std::atomic<bool> worker_failed;
    std::thread worker_thread;
    int iteration_index;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.initialize(0.0, 0.0, 5.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.initialize(1.0, 1.0, 4.0));
    worker_failed.store(false);
    worker_thread = std::thread([&first, &second, &worker_failed]() {
        int local_iteration_index;

        local_iteration_index = 0;
        while (local_iteration_index < 8192 && worker_failed.load() == false)
        {
            if (intersect_circle(first, second) == false)
            {
                worker_failed.store(true);
                break;
            }
            local_iteration_index = local_iteration_index + 1;
        }
        return ;
    });
    iteration_index = 0;
    while (iteration_index < 8192 && worker_failed.load() == false)
    {
        if (intersect_circle(second, first) == false)
        {
            worker_failed.store(true);
            break;
        }
        iteration_index = iteration_index + 1;
    }
    worker_thread.join();
    FT_ASSERT_EQ(false, worker_failed.load());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.destroy());
    return (1);
}

FT_TEST(test_intersect_circle_high_load_overlap_four_threads)
{
    circle first;
    circle second;
    std::atomic<bool> worker_failed;
    std::thread worker_thread_one;
    std::thread worker_thread_two;
    std::thread worker_thread_three;
    std::thread worker_thread_four;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.initialize(0.0, 0.0, 10.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.initialize(1.0, 1.0, 9.0));
    worker_failed.store(false);
    worker_thread_one = std::thread([&first, &second, &worker_failed]() {
        int local_iteration_index;

        local_iteration_index = 0;
        while (local_iteration_index < 4096 && worker_failed.load() == false)
        {
            if (intersect_circle(first, second) == false)
            {
                worker_failed.store(true);
                break;
            }
            local_iteration_index = local_iteration_index + 1;
        }
        return ;
    });
    worker_thread_two = std::thread([&first, &second, &worker_failed]() {
        int local_iteration_index;

        local_iteration_index = 0;
        while (local_iteration_index < 4096 && worker_failed.load() == false)
        {
            if (intersect_circle(second, first) == false)
            {
                worker_failed.store(true);
                break;
            }
            local_iteration_index = local_iteration_index + 1;
        }
        return ;
    });
    worker_thread_three = std::thread([&first, &second, &worker_failed]() {
        int local_iteration_index;

        local_iteration_index = 0;
        while (local_iteration_index < 4096 && worker_failed.load() == false)
        {
            if (intersect_circle(first, second) == false)
            {
                worker_failed.store(true);
                break;
            }
            local_iteration_index = local_iteration_index + 1;
        }
        return ;
    });
    worker_thread_four = std::thread([&first, &second, &worker_failed]() {
        int local_iteration_index;

        local_iteration_index = 0;
        while (local_iteration_index < 4096 && worker_failed.load() == false)
        {
            if (intersect_circle(second, first) == false)
            {
                worker_failed.store(true);
                break;
            }
            local_iteration_index = local_iteration_index + 1;
        }
        return ;
    });
    worker_thread_one.join();
    worker_thread_two.join();
    worker_thread_three.join();
    worker_thread_four.join();
    FT_ASSERT_EQ(false, worker_failed.load());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.destroy());
    return (1);
}

FT_TEST(test_intersect_circle_high_load_touching_two_threads)
{
    circle first;
    circle second;
    std::atomic<bool> worker_failed;
    std::thread worker_thread;
    int iteration_index;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.initialize(0.0, 0.0, 3.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.initialize(4.0, 0.0, 1.0));
    worker_failed.store(false);
    worker_thread = std::thread([&first, &second, &worker_failed]() {
        int local_iteration_index;

        local_iteration_index = 0;
        while (local_iteration_index < 6144 && worker_failed.load() == false)
        {
            if (intersect_circle(first, second) == false)
            {
                worker_failed.store(true);
                break;
            }
            local_iteration_index = local_iteration_index + 1;
        }
        return ;
    });
    iteration_index = 0;
    while (iteration_index < 6144 && worker_failed.load() == false)
    {
        if (intersect_circle(second, first) == false)
        {
            worker_failed.store(true);
            break;
        }
        iteration_index = iteration_index + 1;
    }
    worker_thread.join();
    FT_ASSERT_EQ(false, worker_failed.load());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.destroy());
    return (1);
}

FT_TEST(test_intersect_circle_high_load_separated_two_threads)
{
    circle first;
    circle second;
    std::atomic<bool> worker_failed;
    std::thread worker_thread;
    int iteration_index;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.initialize(-5.0, -5.0, 2.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.initialize(2.0, 2.0, 1.5));
    worker_failed.store(false);
    worker_thread = std::thread([&first, &second, &worker_failed]() {
        int local_iteration_index;

        local_iteration_index = 0;
        while (local_iteration_index < 6144 && worker_failed.load() == false)
        {
            if (intersect_circle(first, second) != false)
            {
                worker_failed.store(true);
                break;
            }
            local_iteration_index = local_iteration_index + 1;
        }
        return ;
    });
    iteration_index = 0;
    while (iteration_index < 6144 && worker_failed.load() == false)
    {
        if (intersect_circle(second, first) != false)
        {
            worker_failed.store(true);
            break;
        }
        iteration_index = iteration_index + 1;
    }
    worker_thread.join();
    FT_ASSERT_EQ(false, worker_failed.load());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.destroy());
    return (1);
}

FT_TEST(test_circle_move_bidirectional_high_load_with_thread_safety)
{
    circle first;
    circle second;
    std::atomic<bool> worker_failed;
    std::thread worker_thread;
    int iteration_index;
    int move_error;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.initialize(0.0, 0.0, 3.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.initialize(1.0, 1.0, 4.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.enable_thread_safety());
    worker_failed.store(false);
    worker_thread = std::thread([&first, &second, &worker_failed]() {
        int local_iteration_index;
        int local_move_error;

        local_iteration_index = 0;
        while (local_iteration_index < 4096 && worker_failed.load() == false)
        {
            local_move_error = first.move(second);
            if (local_move_error != FT_ERR_SUCCESS)
            {
                worker_failed.store(true);
                break;
            }
            local_iteration_index = local_iteration_index + 1;
        }
        return ;
    });
    iteration_index = 0;
    while (iteration_index < 4096 && worker_failed.load() == false)
    {
        move_error = second.move(first);
        if (move_error != FT_ERR_SUCCESS)
        {
            worker_failed.store(true);
            break;
        }
        iteration_index = iteration_index + 1;
    }
    worker_thread.join();
    FT_ASSERT_EQ(false, worker_failed.load());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.destroy());
    return (1);
}

FT_TEST(test_circle_setters_getters_contention_high_load_two_threads)
{
    circle shape;
    std::atomic<bool> worker_failed;
    std::thread writer_thread;
    int iteration_index;
    double center_x;
    double center_y;
    double radius;
    int set_error;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(0.0, 0.0, 1.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.enable_thread_safety());
    worker_failed.store(false);
    writer_thread = std::thread([&shape, &worker_failed]() {
        int local_iteration_index;
        int local_set_error;

        local_iteration_index = 0;
        while (local_iteration_index < 4096 && worker_failed.load() == false)
        {
            if ((local_iteration_index % 2) == 0)
                local_set_error = shape.set_center(2.0, -2.0);
            else
                local_set_error = shape.set_center(-3.0, 3.0);
            if (local_set_error != FT_ERR_SUCCESS)
            {
                worker_failed.store(true);
                break;
            }
            local_set_error = shape.set_radius(5.0 + (local_iteration_index % 3));
            if (local_set_error != FT_ERR_SUCCESS)
            {
                worker_failed.store(true);
                break;
            }
            local_iteration_index = local_iteration_index + 1;
        }
        return ;
    });
    iteration_index = 0;
    while (iteration_index < 4096 && worker_failed.load() == false)
    {
        center_x = shape.get_center_x();
        center_y = shape.get_center_y();
        radius = shape.get_radius();
        if ((std::isfinite(center_x) == false) || (std::isfinite(center_y) == false)
            || (std::isfinite(radius) == false))
        {
            worker_failed.store(true);
            break;
        }
        iteration_index = iteration_index + 1;
    }
    writer_thread.join();
    FT_ASSERT_EQ(false, worker_failed.load());
    set_error = shape.set_center(0.0, 0.0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, set_error);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_intersect_circle_high_load_with_mutating_overlap)
{
    circle first;
    circle second;
    std::atomic<bool> worker_failed;
    std::thread writer_thread;
    std::thread reader_thread_one;
    std::thread reader_thread_two;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.initialize(0.0, 0.0, 12.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.initialize(1.0, 1.0, 4.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.enable_thread_safety());
    worker_failed.store(false);
    writer_thread = std::thread([&second, &worker_failed]() {
        int local_iteration_index;
        int local_set_error;

        local_iteration_index = 0;
        while (local_iteration_index < 4096 && worker_failed.load() == false)
        {
            if ((local_iteration_index % 2) == 0)
                local_set_error = second.set_center(2.0, 2.0);
            else
                local_set_error = second.set_center(-1.0, 1.0);
            if (local_set_error != FT_ERR_SUCCESS)
            {
                worker_failed.store(true);
                break;
            }
            local_set_error = second.set_radius(4.0 + (local_iteration_index % 2));
            if (local_set_error != FT_ERR_SUCCESS)
            {
                worker_failed.store(true);
                break;
            }
            local_iteration_index = local_iteration_index + 1;
        }
        return ;
    });
    reader_thread_one = std::thread([&first, &second, &worker_failed]() {
        int local_iteration_index;

        local_iteration_index = 0;
        while (local_iteration_index < 4096 && worker_failed.load() == false)
        {
            if (intersect_circle(first, second) == false)
            {
                worker_failed.store(true);
                break;
            }
            local_iteration_index = local_iteration_index + 1;
        }
        return ;
    });
    reader_thread_two = std::thread([&first, &second, &worker_failed]() {
        int local_iteration_index;

        local_iteration_index = 0;
        while (local_iteration_index < 4096 && worker_failed.load() == false)
        {
            if (intersect_circle(second, first) == false)
            {
                worker_failed.store(true);
                break;
            }
            local_iteration_index = local_iteration_index + 1;
        }
        return ;
    });
    writer_thread.join();
    reader_thread_one.join();
    reader_thread_two.join();
    FT_ASSERT_EQ(false, worker_failed.load());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.destroy());
    return (1);
}

FT_TEST(test_circle_setters_getters_contention_high_load_four_threads)
{
    circle shape;
    std::atomic<bool> worker_failed;
    std::thread writer_thread;
    std::thread reader_thread_one;
    std::thread reader_thread_two;
    std::thread reader_thread_three;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(0.0, 0.0, 1.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.enable_thread_safety());
    worker_failed.store(false);
    writer_thread = std::thread([&shape, &worker_failed]() {
        int local_iteration_index;
        int local_error_code;

        local_iteration_index = 0;
        while (local_iteration_index < 3072 && worker_failed.load() == false)
        {
            local_error_code = shape.set_center(
                    static_cast<double>((local_iteration_index % 7) - 3),
                    static_cast<double>((local_iteration_index % 5) - 2));
            if (local_error_code != FT_ERR_SUCCESS)
            {
                worker_failed.store(true);
                break;
            }
            local_error_code = shape.set_radius(4.0 + (local_iteration_index % 4));
            if (local_error_code != FT_ERR_SUCCESS)
            {
                worker_failed.store(true);
                break;
            }
            local_iteration_index = local_iteration_index + 1;
        }
        return ;
    });
    reader_thread_one = std::thread([&shape, &worker_failed]() {
        int local_iteration_index;

        local_iteration_index = 0;
        while (local_iteration_index < 3072 && worker_failed.load() == false)
        {
            if (std::isfinite(shape.get_center_x()) == false)
            {
                worker_failed.store(true);
                break;
            }
            local_iteration_index = local_iteration_index + 1;
        }
        return ;
    });
    reader_thread_two = std::thread([&shape, &worker_failed]() {
        int local_iteration_index;

        local_iteration_index = 0;
        while (local_iteration_index < 3072 && worker_failed.load() == false)
        {
            if (std::isfinite(shape.get_center_y()) == false)
            {
                worker_failed.store(true);
                break;
            }
            local_iteration_index = local_iteration_index + 1;
        }
        return ;
    });
    reader_thread_three = std::thread([&shape, &worker_failed]() {
        int local_iteration_index;

        local_iteration_index = 0;
        while (local_iteration_index < 3072 && worker_failed.load() == false)
        {
            if (std::isfinite(shape.get_radius()) == false)
            {
                worker_failed.store(true);
                break;
            }
            local_iteration_index = local_iteration_index + 1;
        }
        return ;
    });
    writer_thread.join();
    reader_thread_one.join();
    reader_thread_two.join();
    reader_thread_three.join();
    FT_ASSERT_EQ(false, worker_failed.load());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_circle_copy_and_move_initializers)
{
    circle source;
    circle copied_destination;
    circle moved_destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(-4.0, 5.0, 6.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, copied_destination.initialize(source));
    FT_ASSERT_DOUBLE_EQ(-4.0, copied_destination.get_center_x());
    FT_ASSERT_DOUBLE_EQ(5.0, copied_destination.get_center_y());
    FT_ASSERT_DOUBLE_EQ(6.0, copied_destination.get_radius());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_destination.initialize(static_cast<circle &&>(source)));
    FT_ASSERT_DOUBLE_EQ(-4.0, moved_destination.get_center_x());
    FT_ASSERT_DOUBLE_EQ(5.0, moved_destination.get_center_y());
    FT_ASSERT_DOUBLE_EQ(6.0, moved_destination.get_radius());
    FT_ASSERT_DOUBLE_EQ(0.0, source.get_center_x());
    FT_ASSERT_DOUBLE_EQ(0.0, source.get_center_y());
    FT_ASSERT_DOUBLE_EQ(0.0, source.get_radius());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, copied_destination.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_destination.destroy());
    return (1);
}

FT_TEST(test_circle_move_method_transfers_values)
{
    circle source;
    circle destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(7.0, -2.0, 9.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.move(source));
    FT_ASSERT_DOUBLE_EQ(7.0, destination.get_center_x());
    FT_ASSERT_DOUBLE_EQ(-2.0, destination.get_center_y());
    FT_ASSERT_DOUBLE_EQ(9.0, destination.get_radius());
    FT_ASSERT_DOUBLE_EQ(0.0, source.get_center_x());
    FT_ASSERT_DOUBLE_EQ(0.0, source.get_center_y());
    FT_ASSERT_DOUBLE_EQ(0.0, source.get_radius());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.destroy());
    return (1);
}

FT_TEST(test_circle_thread_safety_enable_disable)
{
    circle shape;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(0.0, 0.0, 1.0));
    FT_ASSERT_EQ(false, shape.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.enable_thread_safety());
    FT_ASSERT_EQ(true, shape.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.enable_thread_safety());
    FT_ASSERT_EQ(true, shape.is_thread_safe());
    shape.disable_thread_safety();
    FT_ASSERT_EQ(false, shape.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_circle_initialize_from_destroyed_source_aborts)
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            circle_initialize_copy_from_destroyed_source_aborts_operation));
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            circle_initialize_move_from_destroyed_source_aborts_operation));
    return (1);
}

FT_TEST(test_circle_move_self_initialised_is_noop)
{
    circle shape;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(7.0, -2.0, 9.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.move(shape));
    FT_ASSERT_DOUBLE_EQ(7.0, shape.get_center_x());
    FT_ASSERT_DOUBLE_EQ(-2.0, shape.get_center_y());
    FT_ASSERT_DOUBLE_EQ(9.0, shape.get_radius());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_circle_mutex_testing_accessor_lifecycle)
{
    circle shape;
    pt_recursive_mutex *mutex_pointer;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(0.0, 0.0, 1.0));
    mutex_pointer = shape._mutex;
    FT_ASSERT_NEQ(ft_nullptr, mutex_pointer);
    FT_ASSERT_EQ(true, shape.is_thread_safe());
    shape.disable_thread_safety();
    FT_ASSERT_EQ(false, shape.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    FT_ASSERT_EQ(ft_nullptr, shape._mutex);
    return (1);
}

FT_TEST(test_circle_enable_thread_safety_allocation_failure)
{
    circle shape;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(0.0, 0.0, 1.0));
    cma_set_alloc_limit(1);
    error_code = shape.enable_thread_safety();
    cma_set_alloc_limit(0);
    FT_ASSERT_NEQ(FT_ERR_SUCCESS, error_code);
    FT_ASSERT_EQ(false, shape.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_circle_initialize_twice_aborts)
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(circle_initialize_twice_aborts_operation));
    return (1);
}

FT_TEST(test_circle_destroy_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(circle_destroy_uninitialised_aborts_operation));
    return (1);
}

FT_TEST(test_circle_set_radius_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(circle_set_radius_uninitialised_aborts_operation));
    return (1);
}

FT_TEST(test_circle_destroy_twice_aborts)
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(circle_destroy_twice_aborts_operation));
    return (1);
}

FT_TEST(test_circle_uninitialised_destructor_aborts)
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(circle_destructor_uninitialised_aborts_operation));
    return (1);
}

FT_TEST(test_circle_self_move_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(circle_move_self_uninitialised_aborts_operation));
    return (1);
}

FT_TEST(test_circle_initialize_copy_from_uninitialised_source_aborts)
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            circle_initialize_copy_from_uninitialised_source_aborts_operation));
    return (1);
}

FT_TEST(test_circle_initialize_move_from_uninitialised_source_aborts)
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            circle_initialize_move_from_uninitialised_source_aborts_operation));
    return (1);
}

FT_TEST(test_circle_get_center_x_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            circle_get_center_x_uninitialised_aborts_operation));
    return (1);
}

FT_TEST(test_circle_enable_thread_safety_destroyed_aborts)
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            circle_enable_thread_safety_destroyed_aborts_operation));
    return (1);
}

FT_TEST(test_circle_initialize_copy_destination_initialised_aborts)
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            circle_initialize_copy_destination_initialised_aborts_operation));
    return (1);
}

FT_TEST(test_circle_initialize_move_destination_initialised_aborts)
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            circle_initialize_move_destination_initialised_aborts_operation));
    return (1);
}

FT_TEST(test_circle_disable_thread_safety_destroyed_aborts)
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            circle_disable_thread_safety_destroyed_aborts_operation));
    return (1);
}

FT_TEST(test_circle_is_thread_safe_enabled_destroyed_aborts)
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            circle_is_thread_safe_enabled_destroyed_aborts_operation));
    return (1);
}

FT_TEST(test_circle_set_center_x_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            circle_set_center_x_uninitialised_aborts_operation));
    return (1);
}

FT_TEST(test_circle_get_center_y_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            circle_get_center_y_uninitialised_aborts_operation));
    return (1);
}

FT_TEST(test_circle_set_center_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            circle_set_center_uninitialised_aborts_operation));
    return (1);
}

FT_TEST(test_circle_get_radius_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            circle_get_radius_uninitialised_aborts_operation));
    return (1);
}

FT_TEST(test_circle_enable_thread_safety_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            circle_enable_thread_safety_uninitialised_aborts_operation));
    return (1);
}

FT_TEST(test_circle_set_center_y_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            circle_set_center_y_uninitialised_aborts_operation));
    return (1);
}

FT_TEST(test_circle_get_center_x_uninitialised_aborts_additional)
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            circle_get_center_x_second_uninitialised_aborts_operation));
    return (1);
}

FT_TEST(test_circle_disable_thread_safety_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            circle_disable_thread_safety_uninitialised_aborts_operation));
    return (1);
}

FT_TEST(test_circle_is_thread_safe_enabled_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            circle_is_thread_safe_enabled_uninitialised_aborts_operation));
    return (1);
}

FT_TEST(test_circle_move_two_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            circle_move_two_uninitialised_aborts_operation));
    return (1);
}

FT_TEST(test_circle_set_center_destroyed_aborts)
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            circle_set_center_destroyed_aborts_operation));
    return (1);
}

FT_TEST(test_circle_get_center_x_destroyed_aborts)
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            circle_get_center_x_destroyed_aborts_operation));
    return (1);
}

FT_TEST(test_circle_set_radius_destroyed_aborts)
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            circle_set_radius_destroyed_aborts_operation));
    return (1);
}

FT_TEST(test_circle_set_center_x_destroyed_aborts)
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            circle_set_center_x_destroyed_aborts_operation));
    return (1);
}

FT_TEST(test_circle_get_radius_destroyed_aborts)
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            circle_get_radius_destroyed_aborts_operation));
    return (1);
}

FT_TEST(test_circle_move_destroyed_source_aborts)
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            circle_move_destroyed_source_aborts_operation));
    return (1);
}

FT_TEST(test_circle_set_center_y_destroyed_aborts)
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            circle_set_center_y_destroyed_aborts_operation));
    return (1);
}

FT_TEST(test_circle_get_center_y_destroyed_aborts)
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            circle_get_center_y_destroyed_aborts_operation));
    return (1);
}

FT_TEST(test_circle_move_destroyed_destination_succeeds)
{
    FT_ASSERT_EQ(1, circle_move_destroyed_destination_succeeds_operation());
    return (1);
}

FT_TEST(test_circle_move_uninitialised_source_aborts)
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            circle_move_uninitialised_source_aborts_operation));
    return (1);
}

FT_TEST(test_circle_set_center_y_uninitialised_second_aborts)
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            circle_set_center_y_uninitialised_second_aborts_operation));
    return (1);
}

FT_TEST(test_intersect_circle_uninitialised_first_aborts)
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            intersect_circle_uninitialised_first_aborts_operation));
    return (1);
}

FT_TEST(test_intersect_circle_uninitialised_second_aborts)
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            intersect_circle_uninitialised_second_aborts_operation));
    return (1);
}

FT_TEST(test_intersect_circle_uninitialised_both_aborts)
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            intersect_circle_uninitialised_both_aborts_operation));
    return (1);
}

FT_TEST(test_intersect_circle_destroyed_first_aborts)
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            intersect_circle_destroyed_first_aborts_operation));
    return (1);
}

FT_TEST(test_intersect_circle_destroyed_second_aborts)
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            intersect_circle_destroyed_second_aborts_operation));
    return (1);
}

FT_TEST(test_intersect_circle_destroyed_both_aborts)
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            intersect_circle_destroyed_both_aborts_operation));
    return (1);
}

FT_TEST(test_intersect_circle_destroyed_and_uninitialised_aborts)
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            intersect_circle_destroyed_and_uninitialised_aborts_operation));
    return (1);
}

FT_TEST(test_intersect_circle_high_load_overlap_two_threads_soak_rounds)
{
    circle first;
    circle second;
    std::atomic<bool> worker_failed;
    std::thread worker_thread;
    int round_index;
    int iteration_index;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.initialize(0.0, 0.0, 5.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.initialize(1.0, 1.0, 4.0));
    round_index = 0;
    while (round_index < 4)
    {
        worker_failed.store(false);
        worker_thread = std::thread([&first, &second, &worker_failed]() {
            int local_iteration_index;

            local_iteration_index = 0;
            while (local_iteration_index < 4096 && worker_failed.load() == false)
            {
                if (intersect_circle(first, second) == false)
                    worker_failed.store(true);
                local_iteration_index = local_iteration_index + 1;
            }
            return ;
        });
        iteration_index = 0;
        while (iteration_index < 4096 && worker_failed.load() == false)
        {
            if (intersect_circle(second, first) == false)
                worker_failed.store(true);
            iteration_index = iteration_index + 1;
        }
        worker_thread.join();
        FT_ASSERT_EQ(false, worker_failed.load());
        round_index = round_index + 1;
    }
    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.destroy());
    return (1);
}

FT_TEST(test_intersect_circle_high_load_overlap_four_threads_soak_rounds)
{
    circle first;
    circle second;
    std::atomic<bool> worker_failed;
    std::thread thread_one;
    std::thread thread_two;
    std::thread thread_three;
    std::thread thread_four;
    int round_index;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.initialize(0.0, 0.0, 10.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.initialize(1.0, 1.0, 9.0));
    round_index = 0;
    while (round_index < 3)
    {
        worker_failed.store(false);
        thread_one = std::thread([&first, &second, &worker_failed]() {
            int index;
            index = 0;
            while (index < 3072 && worker_failed.load() == false)
            {
                if (intersect_circle(first, second) == false)
                    worker_failed.store(true);
                index = index + 1;
            }
            return ;
        });
        thread_two = std::thread([&first, &second, &worker_failed]() {
            int index;
            index = 0;
            while (index < 3072 && worker_failed.load() == false)
            {
                if (intersect_circle(second, first) == false)
                    worker_failed.store(true);
                index = index + 1;
            }
            return ;
        });
        thread_three = std::thread([&first, &second, &worker_failed]() {
            int index;
            index = 0;
            while (index < 3072 && worker_failed.load() == false)
            {
                if (intersect_circle(first, second) == false)
                    worker_failed.store(true);
                index = index + 1;
            }
            return ;
        });
        thread_four = std::thread([&first, &second, &worker_failed]() {
            int index;
            index = 0;
            while (index < 3072 && worker_failed.load() == false)
            {
                if (intersect_circle(second, first) == false)
                    worker_failed.store(true);
                index = index + 1;
            }
            return ;
        });
        thread_one.join();
        thread_two.join();
        thread_three.join();
        thread_four.join();
        FT_ASSERT_EQ(false, worker_failed.load());
        round_index = round_index + 1;
    }
    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.destroy());
    return (1);
}

FT_TEST(test_intersect_circle_high_load_touching_two_threads_soak_rounds)
{
    circle first;
    circle second;
    std::atomic<bool> worker_failed;
    std::thread worker_thread;
    int round_index;
    int index;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.initialize(0.0, 0.0, 3.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.initialize(4.0, 0.0, 1.0));
    round_index = 0;
    while (round_index < 4)
    {
        worker_failed.store(false);
        worker_thread = std::thread([&first, &second, &worker_failed]() {
            int iteration_index;
            iteration_index = 0;
            while (iteration_index < 3072 && worker_failed.load() == false)
            {
                if (intersect_circle(first, second) == false)
                    worker_failed.store(true);
                iteration_index = iteration_index + 1;
            }
            return ;
        });
        index = 0;
        while (index < 3072 && worker_failed.load() == false)
        {
            if (intersect_circle(second, first) == false)
                worker_failed.store(true);
            index = index + 1;
        }
        worker_thread.join();
        FT_ASSERT_EQ(false, worker_failed.load());
        round_index = round_index + 1;
    }
    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.destroy());
    return (1);
}

FT_TEST(test_intersect_circle_high_load_separated_two_threads_soak_rounds)
{
    circle first;
    circle second;
    std::atomic<bool> worker_failed;
    std::thread worker_thread;
    int round_index;
    int index;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.initialize(-5.0, -5.0, 2.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.initialize(2.0, 2.0, 1.5));
    round_index = 0;
    while (round_index < 4)
    {
        worker_failed.store(false);
        worker_thread = std::thread([&first, &second, &worker_failed]() {
            int iteration_index;
            iteration_index = 0;
            while (iteration_index < 3072 && worker_failed.load() == false)
            {
                if (intersect_circle(first, second) != false)
                    worker_failed.store(true);
                iteration_index = iteration_index + 1;
            }
            return ;
        });
        index = 0;
        while (index < 3072 && worker_failed.load() == false)
        {
            if (intersect_circle(second, first) != false)
                worker_failed.store(true);
            index = index + 1;
        }
        worker_thread.join();
        FT_ASSERT_EQ(false, worker_failed.load());
        round_index = round_index + 1;
    }
    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.destroy());
    return (1);
}

FT_TEST(test_circle_move_bidirectional_high_load_soak_rounds)
{
    circle first;
    circle second;
    std::atomic<bool> worker_failed;
    std::thread worker_thread;
    int round_index;
    int index;
    int move_error;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.initialize(0.0, 0.0, 3.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.initialize(1.0, 1.0, 4.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.enable_thread_safety());
    round_index = 0;
    while (round_index < 3)
    {
        worker_failed.store(false);
        worker_thread = std::thread([&first, &second, &worker_failed]() {
            int iteration_index;
            int local_move_error;

            iteration_index = 0;
            while (iteration_index < 2048 && worker_failed.load() == false)
            {
                local_move_error = first.move(second);
                if (local_move_error != FT_ERR_SUCCESS)
                    worker_failed.store(true);
                iteration_index = iteration_index + 1;
            }
            return ;
        });
        index = 0;
        while (index < 2048 && worker_failed.load() == false)
        {
            move_error = second.move(first);
            if (move_error != FT_ERR_SUCCESS)
                worker_failed.store(true);
            index = index + 1;
        }
        worker_thread.join();
        FT_ASSERT_EQ(false, worker_failed.load());
        round_index = round_index + 1;
    }
    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.destroy());
    return (1);
}

FT_TEST(test_circle_setters_getters_contention_high_load_soak_rounds)
{
    circle shape;
    std::atomic<bool> worker_failed;
    std::thread writer_thread;
    int round_index;
    int index;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(0.0, 0.0, 1.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.enable_thread_safety());
    round_index = 0;
    while (round_index < 3)
    {
        worker_failed.store(false);
        writer_thread = std::thread([&shape, &worker_failed]() {
            int iteration_index;
            int local_error_code;

            iteration_index = 0;
            while (iteration_index < 3072 && worker_failed.load() == false)
            {
                local_error_code = shape.set_center(
                        static_cast<double>((iteration_index % 7) - 3),
                        static_cast<double>((iteration_index % 5) - 2));
                if (local_error_code != FT_ERR_SUCCESS)
                    worker_failed.store(true);
                local_error_code = shape.set_radius(4.0 + (iteration_index % 4));
                if (local_error_code != FT_ERR_SUCCESS)
                    worker_failed.store(true);
                iteration_index = iteration_index + 1;
            }
            return ;
        });
        index = 0;
        while (index < 3072 && worker_failed.load() == false)
        {
            if (std::isfinite(shape.get_center_x()) == false
                || std::isfinite(shape.get_center_y()) == false
                || std::isfinite(shape.get_radius()) == false)
                worker_failed.store(true);
            index = index + 1;
        }
        writer_thread.join();
        FT_ASSERT_EQ(false, worker_failed.load());
        round_index = round_index + 1;
    }
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_intersect_circle_high_load_mutating_overlap_soak_rounds)
{
    circle first;
    circle second;
    std::atomic<bool> worker_failed;
    std::thread writer_thread;
    std::thread reader_thread;
    int round_index;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.initialize(0.0, 0.0, 12.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.initialize(1.0, 1.0, 4.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.enable_thread_safety());
    round_index = 0;
    while (round_index < 3)
    {
        worker_failed.store(false);
        writer_thread = std::thread([&second, &worker_failed]() {
            int iteration_index;
            int local_set_error;

            iteration_index = 0;
            while (iteration_index < 3072 && worker_failed.load() == false)
            {
                if ((iteration_index % 2) == 0)
                    local_set_error = second.set_center(2.0, 2.0);
                else
                    local_set_error = second.set_center(-1.0, 1.0);
                if (local_set_error != FT_ERR_SUCCESS)
                    worker_failed.store(true);
                local_set_error = second.set_radius(4.0 + (iteration_index % 2));
                if (local_set_error != FT_ERR_SUCCESS)
                    worker_failed.store(true);
                iteration_index = iteration_index + 1;
            }
            return ;
        });
        reader_thread = std::thread([&first, &second, &worker_failed]() {
            int iteration_index;

            iteration_index = 0;
            while (iteration_index < 3072 && worker_failed.load() == false)
            {
                if (intersect_circle(first, second) == false
                    || intersect_circle(second, first) == false)
                    worker_failed.store(true);
                iteration_index = iteration_index + 1;
            }
            return ;
        });
        writer_thread.join();
        reader_thread.join();
        FT_ASSERT_EQ(false, worker_failed.load());
        round_index = round_index + 1;
    }
    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.destroy());
    return (1);
}
