#include "../test_internal.hpp"
#include "../../Geometry/geometry.hpp"
#include "../../System_utils/test_runner.hpp"
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

static void aabb_initialize_twice_aborts_operation()
{
    aabb box(0.0, 0.0, 1.0, 1.0);

    (void)box.initialize();
    return ;
}

static void aabb_destroy_uninitialized_aborts_operation()
{
    aabb box;

    (void)box.destroy();
    return ;
}

static void aabb_set_bounds_uninitialized_aborts_operation()
{
    aabb box;

    (void)box.set_bounds(0.0, 0.0, 1.0, 1.0);
    return ;
}

static void aabb_destroy_twice_aborts_operation()
{
    aabb box(0.0, 0.0, 1.0, 1.0);

    (void)box.destroy();
    (void)box.destroy();
    return ;
}

static void aabb_destructor_uninitialized_aborts_operation()
{
    aabb box;

    (void)box;
    return ;
}

static void aabb_move_self_uninitialized_aborts_operation()
{
    aabb box;

    (void)box.move(box);
    return ;
}

static void aabb_initialize_copy_from_uninitialized_source_aborts_operation()
{
    aabb source;
    aabb destination(0.0, 0.0, 1.0, 1.0);

    (void)destination.destroy();
    (void)destination.initialize(source);
    return ;
}

static void aabb_initialize_move_from_uninitialized_source_aborts_operation()
{
    aabb source;
    aabb destination(0.0, 0.0, 1.0, 1.0);

    (void)destination.destroy();
    (void)destination.initialize(static_cast<aabb &&>(source));
    return ;
}

static void aabb_initialize_copy_from_destroyed_source_aborts_operation()
{
    aabb source;
    aabb destination(0.0, 0.0, 1.0, 1.0);

    (void)source.initialize(2.0, 3.0, 4.0, 5.0);
    (void)source.destroy();
    (void)destination.destroy();
    (void)destination.initialize(source);
    return ;
}

static void aabb_initialize_move_from_destroyed_source_aborts_operation()
{
    aabb source;
    aabb destination(0.0, 0.0, 1.0, 1.0);

    (void)source.initialize(2.0, 3.0, 4.0, 5.0);
    (void)source.destroy();
    (void)destination.destroy();
    (void)destination.initialize(static_cast<aabb &&>(source));
    return ;
}

static void aabb_get_minimum_x_uninitialized_aborts_operation()
{
    aabb box;

    (void)box.get_minimum_x();
    return ;
}

static void aabb_enable_thread_safety_destroyed_aborts_operation()
{
    aabb box(0.0, 0.0, 1.0, 1.0);

    (void)box.destroy();
    (void)box.enable_thread_safety();
    return ;
}

static void aabb_initialize_copy_destination_initialized_aborts_operation()
{
    aabb source(0.0, 0.0, 1.0, 1.0);
    aabb destination(2.0, 2.0, 3.0, 3.0);

    (void)destination.initialize(source);
    return ;
}

static void aabb_initialize_move_destination_initialized_aborts_operation()
{
    aabb source(0.0, 0.0, 1.0, 1.0);
    aabb destination(2.0, 2.0, 3.0, 3.0);

    (void)destination.initialize(static_cast<aabb &&>(source));
    return ;
}

static void aabb_disable_thread_safety_destroyed_aborts_operation()
{
    aabb box(0.0, 0.0, 1.0, 1.0);

    (void)box.destroy();
    box.disable_thread_safety();
    return ;
}

static void aabb_is_thread_safe_enabled_destroyed_aborts_operation()
{
    aabb box(0.0, 0.0, 1.0, 1.0);

    (void)box.destroy();
    (void)box.is_thread_safe_enabled();
    return ;
}

static void aabb_set_minimum_x_uninitialized_aborts_operation()
{
    aabb box;

    (void)box.set_minimum_x(-1.0);
    return ;
}

static void aabb_get_maximum_y_uninitialized_aborts_operation()
{
    aabb box;

    (void)box.get_maximum_y();
    return ;
}

static void aabb_set_maximum_x_uninitialized_aborts_operation()
{
    aabb box;

    (void)box.set_maximum_x(1.0);
    return ;
}

static void aabb_get_minimum_y_uninitialized_aborts_operation()
{
    aabb box;

    (void)box.get_minimum_y();
    return ;
}

static void aabb_enable_thread_safety_uninitialized_aborts_operation()
{
    aabb box;

    (void)box.enable_thread_safety();
    return ;
}

static void aabb_set_maximum_y_uninitialized_aborts_operation()
{
    aabb box;

    (void)box.set_maximum_y(1.0);
    return ;
}

static void aabb_get_maximum_x_uninitialized_aborts_operation()
{
    aabb box;

    (void)box.get_maximum_x();
    return ;
}

static void aabb_disable_thread_safety_uninitialized_aborts_operation()
{
    aabb box;

    box.disable_thread_safety();
    return ;
}

static void aabb_is_thread_safe_enabled_uninitialized_aborts_operation()
{
    aabb box;

    (void)box.is_thread_safe_enabled();
    return ;
}

static void aabb_set_minimum_uninitialized_aborts_operation()
{
    aabb box;

    (void)box.set_minimum(0.0, 0.0);
    return ;
}

static void aabb_set_minimum_y_uninitialized_aborts_operation()
{
    aabb box;

    (void)box.set_minimum_y(0.0);
    return ;
}

static void aabb_set_maximum_uninitialized_aborts_operation()
{
    aabb box;

    (void)box.set_maximum(1.0, 1.0);
    return ;
}

static void aabb_move_two_uninitialized_aborts_operation()
{
    aabb source;
    aabb destination;

    (void)destination.move(source);
    return ;
}

static void aabb_set_bounds_destroyed_aborts_operation()
{
    aabb box(0.0, 0.0, 1.0, 1.0);

    (void)box.destroy();
    (void)box.set_bounds(0.0, 0.0, 1.0, 1.0);
    return ;
}

static void aabb_get_minimum_x_destroyed_aborts_operation()
{
    aabb box(0.0, 0.0, 1.0, 1.0);

    (void)box.destroy();
    (void)box.get_minimum_x();
    return ;
}

static void aabb_move_destroyed_source_aborts_operation()
{
    aabb source(0.0, 0.0, 1.0, 1.0);
    aabb destination(2.0, 2.0, 3.0, 3.0);

    (void)source.destroy();
    (void)destination.move(source);
    return ;
}

static void aabb_set_minimum_destroyed_aborts_operation()
{
    aabb box(0.0, 0.0, 1.0, 1.0);

    (void)box.destroy();
    (void)box.set_minimum(0.0, 0.0);
    return ;
}

static void aabb_set_minimum_x_destroyed_aborts_operation()
{
    aabb box(0.0, 0.0, 1.0, 1.0);

    (void)box.destroy();
    (void)box.set_minimum_x(0.0);
    return ;
}

static void aabb_set_minimum_y_destroyed_aborts_operation()
{
    aabb box(0.0, 0.0, 1.0, 1.0);

    (void)box.destroy();
    (void)box.set_minimum_y(0.0);
    return ;
}

static void aabb_set_maximum_destroyed_aborts_operation()
{
    aabb box(0.0, 0.0, 1.0, 1.0);

    (void)box.destroy();
    (void)box.set_maximum(1.0, 1.0);
    return ;
}

static void aabb_set_maximum_x_destroyed_aborts_operation()
{
    aabb box(0.0, 0.0, 1.0, 1.0);

    (void)box.destroy();
    (void)box.set_maximum_x(1.0);
    return ;
}

static void aabb_set_maximum_y_destroyed_aborts_operation()
{
    aabb box(0.0, 0.0, 1.0, 1.0);

    (void)box.destroy();
    (void)box.set_maximum_y(1.0);
    return ;
}

static void aabb_get_minimum_y_destroyed_aborts_operation()
{
    aabb box(0.0, 0.0, 1.0, 1.0);

    (void)box.destroy();
    (void)box.get_minimum_y();
    return ;
}

static void aabb_get_maximum_x_destroyed_aborts_operation()
{
    aabb box(0.0, 0.0, 1.0, 1.0);

    (void)box.destroy();
    (void)box.get_maximum_x();
    return ;
}

static void aabb_get_maximum_y_destroyed_aborts_operation()
{
    aabb box(0.0, 0.0, 1.0, 1.0);

    (void)box.destroy();
    (void)box.get_maximum_y();
    return ;
}

static int aabb_move_destroyed_destination_succeeds_operation()
{
    aabb source(0.0, 0.0, 1.0, 1.0);
    aabb destination(2.0, 2.0, 3.0, 3.0);
    int move_error;

    (void)destination.destroy();
    move_error = destination.move(source);
    if (move_error != FT_ERR_SUCCESS)
        return (0);
    if (destination.get_minimum_x() != 0.0 || destination.get_minimum_y() != 0.0
        || destination.get_maximum_x() != 1.0 || destination.get_maximum_y() != 1.0)
        return (0);
    if (source.get_minimum_x() != 0.0 || source.get_minimum_y() != 0.0
        || source.get_maximum_x() != 0.0 || source.get_maximum_y() != 0.0)
        return (0);
    return (1);
}

static void aabb_move_uninitialized_source_aborts_operation()
{
    aabb source;
    aabb destination(2.0, 2.0, 3.0, 3.0);

    (void)destination.move(source);
    return ;
}

static void intersect_aabb_uninitialized_first_aborts_operation()
{
    aabb first;
    aabb second(0.0, 0.0, 1.0, 1.0);

    (void)intersect_aabb(first, second);
    return ;
}

static void intersect_aabb_uninitialized_second_aborts_operation()
{
    aabb first(0.0, 0.0, 1.0, 1.0);
    aabb second;

    (void)intersect_aabb(first, second);
    return ;
}

static void intersect_aabb_uninitialized_both_aborts_operation()
{
    aabb first;
    aabb second;

    (void)intersect_aabb(first, second);
    return ;
}

static void intersect_aabb_destroyed_first_aborts_operation()
{
    aabb first(0.0, 0.0, 1.0, 1.0);
    aabb second(0.0, 0.0, 1.0, 1.0);

    (void)first.destroy();
    (void)intersect_aabb(first, second);
    return ;
}

static void intersect_aabb_destroyed_second_aborts_operation()
{
    aabb first(0.0, 0.0, 1.0, 1.0);
    aabb second(0.0, 0.0, 1.0, 1.0);

    (void)second.destroy();
    (void)intersect_aabb(first, second);
    return ;
}

static void intersect_aabb_destroyed_both_aborts_operation()
{
    aabb first(0.0, 0.0, 1.0, 1.0);
    aabb second(0.0, 0.0, 1.0, 1.0);

    (void)first.destroy();
    (void)second.destroy();
    (void)intersect_aabb(first, second);
    return ;
}

static void intersect_aabb_destroyed_and_uninitialized_aborts_operation()
{
    aabb first(0.0, 0.0, 1.0, 1.0);
    aabb second;

    (void)first.destroy();
    (void)intersect_aabb(first, second);
    return ;
}

FT_TEST(test_aabb_initialize_setters_and_getters,
    "aabb initialize, setters, and getters work in initialized lifecycle")
{
    aabb box;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.set_bounds(-3.0, -2.0, 6.0, 9.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.set_minimum(-2.0, -1.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.set_maximum(7.0, 10.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.set_minimum_x(-1.5));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.set_minimum_y(-0.5));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.set_maximum_x(8.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.set_maximum_y(11.0));
    FT_ASSERT_DOUBLE_EQ(-1.5, box.get_minimum_x());
    FT_ASSERT_DOUBLE_EQ(-0.5, box.get_minimum_y());
    FT_ASSERT_DOUBLE_EQ(8.0, box.get_maximum_x());
    FT_ASSERT_DOUBLE_EQ(11.0, box.get_maximum_y());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.destroy());
    return (1);
}

FT_TEST(test_aabb_invalid_bounds_preserve_state,
    "aabb invalid set_bounds returns error and preserves existing bounds")
{
    aabb box;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.set_bounds(-2.0, -3.0, 4.0, 6.0));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, box.set_bounds(8.0, -3.0, 4.0, 6.0));
    FT_ASSERT_DOUBLE_EQ(-2.0, box.get_minimum_x());
    FT_ASSERT_DOUBLE_EQ(-3.0, box.get_minimum_y());
    FT_ASSERT_DOUBLE_EQ(4.0, box.get_maximum_x());
    FT_ASSERT_DOUBLE_EQ(6.0, box.get_maximum_y());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.destroy());
    return (1);
}

FT_TEST(test_aabb_invalid_minimum_maximum_setters_preserve_state,
    "aabb min/max setter validation rejects invalid updates and keeps bounds")
{
    aabb box;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.initialize(-2.0, -3.0, 4.0, 6.0));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, box.set_minimum_x(5.0));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, box.set_minimum_y(7.0));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, box.set_maximum_x(-3.0));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, box.set_maximum_y(-4.0));
    FT_ASSERT_DOUBLE_EQ(-2.0, box.get_minimum_x());
    FT_ASSERT_DOUBLE_EQ(-3.0, box.get_minimum_y());
    FT_ASSERT_DOUBLE_EQ(4.0, box.get_maximum_x());
    FT_ASSERT_DOUBLE_EQ(6.0, box.get_maximum_y());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.destroy());
    return (1);
}

FT_TEST(test_aabb_reinitialize_after_destroy,
    "aabb can be initialized again after destroy")
{
    aabb box;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.initialize(1.0, 2.0, 3.0, 4.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.initialize(-7.0, -8.0, 9.0, 10.0));
    FT_ASSERT_DOUBLE_EQ(-7.0, box.get_minimum_x());
    FT_ASSERT_DOUBLE_EQ(-8.0, box.get_minimum_y());
    FT_ASSERT_DOUBLE_EQ(9.0, box.get_maximum_x());
    FT_ASSERT_DOUBLE_EQ(10.0, box.get_maximum_y());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.destroy());
    return (1);
}

FT_TEST(test_aabb_constructor_invalid_bounds_allows_reinitialize,
    "aabb ctor with invalid bounds leaves object recoverable via initialize")
{
    aabb box(5.0, 0.0, 1.0, 1.0);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.initialize(0.0, 0.0, 2.0, 2.0));
    FT_ASSERT_DOUBLE_EQ(0.0, box.get_minimum_x());
    FT_ASSERT_DOUBLE_EQ(0.0, box.get_minimum_y());
    FT_ASSERT_DOUBLE_EQ(2.0, box.get_maximum_x());
    FT_ASSERT_DOUBLE_EQ(2.0, box.get_maximum_y());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.destroy());
    return (1);
}

FT_TEST(test_aabb_large_finite_bounds_roundtrip,
    "aabb preserves large finite bounds values")
{
    aabb box;
    double large_value;

    large_value = std::numeric_limits<double>::max() / 4.0;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.initialize(
            -large_value, -large_value / 2.0, large_value / 2.0, large_value));
    FT_ASSERT_DOUBLE_EQ(-large_value, box.get_minimum_x());
    FT_ASSERT_DOUBLE_EQ(-large_value / 2.0, box.get_minimum_y());
    FT_ASSERT_DOUBLE_EQ(large_value / 2.0, box.get_maximum_x());
    FT_ASSERT_DOUBLE_EQ(large_value, box.get_maximum_y());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.destroy());
    return (1);
}

FT_TEST(test_aabb_smallest_normal_bounds_roundtrip,
    "aabb preserves smallest normal bounds values")
{
    aabb box;
    double small_value;

    small_value = std::numeric_limits<double>::min();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.initialize(
            -small_value, -small_value, small_value, small_value));
    FT_ASSERT_DOUBLE_EQ(-small_value, box.get_minimum_x());
    FT_ASSERT_DOUBLE_EQ(-small_value, box.get_minimum_y());
    FT_ASSERT_DOUBLE_EQ(small_value, box.get_maximum_x());
    FT_ASSERT_DOUBLE_EQ(small_value, box.get_maximum_y());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.destroy());
    return (1);
}

FT_TEST(test_aabb_infinite_maximum_roundtrip,
    "aabb supports infinite maximum bounds values")
{
    aabb box;
    double infinity_value;

    infinity_value = std::numeric_limits<double>::infinity();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.initialize(0.0, 0.0, infinity_value, infinity_value));
    FT_ASSERT(std::isinf(box.get_maximum_x()));
    FT_ASSERT(std::isinf(box.get_maximum_y()));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.destroy());
    return (1);
}

FT_TEST(test_aabb_set_minimum_x_negative_infinity,
    "aabb accepts negative infinity for minimum_x when max is finite")
{
    aabb box;
    double negative_infinity;

    negative_infinity = -std::numeric_limits<double>::infinity();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.initialize(-1.0, -1.0, 1.0, 1.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.set_minimum_x(negative_infinity));
    FT_ASSERT(std::isinf(box.get_minimum_x()));
    FT_ASSERT(std::signbit(box.get_minimum_x()));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.destroy());
    return (1);
}

FT_TEST(test_aabb_set_maximum_x_infinity,
    "aabb accepts positive infinity for maximum_x")
{
    aabb box;
    double infinity_value;

    infinity_value = std::numeric_limits<double>::infinity();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.initialize(-1.0, -1.0, 1.0, 1.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.set_maximum_x(infinity_value));
    FT_ASSERT(std::isinf(box.get_maximum_x()));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.destroy());
    return (1);
}

FT_TEST(test_aabb_negative_zero_minimum_x_sign_preserved,
    "aabb preserves negative zero sign on minimum_x")
{
    aabb box;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.initialize(-1.0, -1.0, 1.0, 1.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.set_minimum_x(-0.0));
    FT_ASSERT(std::signbit(box.get_minimum_x()));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.destroy());
    return (1);
}

FT_TEST(test_aabb_negative_zero_maximum_y_sign_preserved,
    "aabb preserves negative zero sign on maximum_y")
{
    aabb box;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.initialize(-1.0, -1.0, 1.0, 1.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.set_maximum_y(-0.0));
    FT_ASSERT(std::signbit(box.get_maximum_y()));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.destroy());
    return (1);
}

FT_TEST(test_aabb_lifecycle_multiple_reinitialize_cycles,
    "aabb remains stable over repeated destroy and initialize cycles")
{
    aabb box;
    int iteration_index;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.initialize(1.0, 2.0, 3.0, 4.0));
    iteration_index = 0;
    while (iteration_index < 6)
    {
        FT_ASSERT_EQ(FT_ERR_SUCCESS, box.destroy());
        FT_ASSERT_EQ(FT_ERR_SUCCESS, box.initialize(
                -1.0 - iteration_index,
                -2.0 - iteration_index,
                3.0 + iteration_index,
                4.0 + iteration_index));
        FT_ASSERT_DOUBLE_EQ(-1.0 - iteration_index, box.get_minimum_x());
        FT_ASSERT_DOUBLE_EQ(-2.0 - iteration_index, box.get_minimum_y());
        FT_ASSERT_DOUBLE_EQ(3.0 + iteration_index, box.get_maximum_x());
        FT_ASSERT_DOUBLE_EQ(4.0 + iteration_index, box.get_maximum_y());
        iteration_index = iteration_index + 1;
    }
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.destroy());
    return (1);
}

FT_TEST(test_aabb_lifecycle_thread_safety_cycles,
    "aabb remains stable across initialize/thread-safety/destroy cycles")
{
    aabb box;
    int iteration_index;

    iteration_index = 0;
    while (iteration_index < 6)
    {
        FT_ASSERT_EQ(FT_ERR_SUCCESS, box.initialize(
                -1.0 - iteration_index,
                -2.0 - iteration_index,
                3.0 + iteration_index,
                4.0 + iteration_index));
        FT_ASSERT_EQ(false, box.is_thread_safe_enabled());
        FT_ASSERT_EQ(FT_ERR_SUCCESS, box.enable_thread_safety());
        FT_ASSERT_EQ(true, box.is_thread_safe_enabled());
        box.disable_thread_safety();
        FT_ASSERT_EQ(false, box.is_thread_safe_enabled());
        FT_ASSERT_EQ(FT_ERR_SUCCESS, box.destroy());
        iteration_index = iteration_index + 1;
    }
    return (1);
}

FT_TEST(test_aabb_enable_thread_safety_failure_then_recovery,
    "aabb thread safety can recover after allocation failure")
{
    aabb box;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.initialize(0.0, 0.0, 1.0, 1.0));
    cma_set_alloc_limit(1);
    error_code = box.enable_thread_safety();
    cma_set_alloc_limit(0);
    FT_ASSERT_NEQ(FT_ERR_SUCCESS, error_code);
    FT_ASSERT_EQ(false, box.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.enable_thread_safety());
    FT_ASSERT_EQ(true, box.is_thread_safe_enabled());
    box.disable_thread_safety();
    FT_ASSERT_EQ(false, box.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.destroy());
    return (1);
}

FT_TEST(test_aabb_enable_thread_safety_repeated_alloc_failures,
    "aabb repeated allocation failures keep thread safety disabled")
{
    aabb box;
    int error_code;
    int iteration_index;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.initialize(0.0, 0.0, 1.0, 1.0));
    cma_set_alloc_limit(1);
    iteration_index = 0;
    while (iteration_index < 3)
    {
        error_code = box.enable_thread_safety();
        FT_ASSERT_NEQ(FT_ERR_SUCCESS, error_code);
        FT_ASSERT_EQ(false, box.is_thread_safe_enabled());
        iteration_index = iteration_index + 1;
    }
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.destroy());
    return (1);
}

FT_TEST(test_aabb_enable_thread_safety_alloc_failure_then_destroy,
    "aabb destroy succeeds after thread safety allocation failure")
{
    aabb box;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.initialize(-2.0, -2.0, 2.0, 2.0));
    cma_set_alloc_limit(1);
    error_code = box.enable_thread_safety();
    cma_set_alloc_limit(0);
    FT_ASSERT_NEQ(FT_ERR_SUCCESS, error_code);
    FT_ASSERT_EQ(false, box.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.destroy());
    return (1);
}

FT_TEST(test_aabb_enable_thread_safety_alloc_failure_after_disable,
    "aabb can fail enabling thread safety again after disable")
{
    aabb box;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.initialize(-1.0, -1.0, 1.0, 1.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.enable_thread_safety());
    box.disable_thread_safety();
    FT_ASSERT_EQ(false, box.is_thread_safe_enabled());
    cma_set_alloc_limit(1);
    error_code = box.enable_thread_safety();
    cma_set_alloc_limit(0);
    FT_ASSERT_NEQ(FT_ERR_SUCCESS, error_code);
    FT_ASSERT_EQ(false, box.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.destroy());
    return (1);
}

FT_TEST(test_aabb_get_mutex_for_testing_alloc_failure_returns_null,
    "aabb test mutex accessor returns null under allocation failure")
{
    aabb box;
    pt_recursive_mutex *mutex_pointer;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.initialize(0.0, 0.0, 1.0, 1.0));
    cma_set_alloc_limit(1);
    mutex_pointer = box.get_mutex_for_testing();
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(ft_nullptr, mutex_pointer);
    FT_ASSERT_EQ(false, box.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.destroy());
    return (1);
}

FT_TEST(test_aabb_get_mutex_for_testing_recovers_after_alloc_failure,
    "aabb test mutex accessor recovers once allocation limit resets")
{
    aabb box;
    pt_recursive_mutex *mutex_pointer;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.initialize(0.0, 0.0, 1.0, 1.0));
    cma_set_alloc_limit(1);
    mutex_pointer = box.get_mutex_for_testing();
    FT_ASSERT_EQ(ft_nullptr, mutex_pointer);
    cma_set_alloc_limit(0);
    mutex_pointer = box.get_mutex_for_testing();
    FT_ASSERT_NEQ(ft_nullptr, mutex_pointer);
    FT_ASSERT_EQ(true, box.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.destroy());
    return (1);
}

FT_TEST(test_aabb_enable_thread_safety_alloc_failure_reinit_cycle,
    "aabb allocation failure path remains stable across reinitialize cycle")
{
    aabb box;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.initialize(-3.0, -3.0, 3.0, 3.0));
    cma_set_alloc_limit(1);
    error_code = box.enable_thread_safety();
    cma_set_alloc_limit(0);
    FT_ASSERT_NEQ(FT_ERR_SUCCESS, error_code);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.initialize(-4.0, -4.0, 4.0, 4.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.enable_thread_safety());
    FT_ASSERT_EQ(true, box.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.destroy());
    return (1);
}

FT_TEST(test_aabb_enable_thread_safety_alloc_failure_preserves_bounds,
    "aabb bounds stay unchanged when enabling thread safety fails")
{
    aabb box;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.initialize(-5.0, -6.0, 7.0, 8.0));
    cma_set_alloc_limit(1);
    error_code = box.enable_thread_safety();
    cma_set_alloc_limit(0);
    FT_ASSERT_NEQ(FT_ERR_SUCCESS, error_code);
    FT_ASSERT_DOUBLE_EQ(-5.0, box.get_minimum_x());
    FT_ASSERT_DOUBLE_EQ(-6.0, box.get_minimum_y());
    FT_ASSERT_DOUBLE_EQ(7.0, box.get_maximum_x());
    FT_ASSERT_DOUBLE_EQ(8.0, box.get_maximum_y());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.destroy());
    return (1);
}

FT_TEST(test_aabb_enable_thread_safety_alloc_limit_two_state_consistent,
    "aabb alloc limit 2 keeps thread safety state consistent")
{
    aabb box;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.initialize(-1.0, -1.0, 1.0, 1.0));
    cma_set_alloc_limit(2);
    error_code = box.enable_thread_safety();
    cma_set_alloc_limit(0);
    if (error_code == FT_ERR_SUCCESS)
        FT_ASSERT_EQ(true, box.is_thread_safe_enabled());
    else
        FT_ASSERT_EQ(false, box.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.destroy());
    return (1);
}

FT_TEST(test_aabb_enable_thread_safety_alloc_limit_three_state_consistent,
    "aabb alloc limit 3 keeps thread safety state consistent")
{
    aabb box;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.initialize(-1.0, -1.0, 1.0, 1.0));
    cma_set_alloc_limit(3);
    error_code = box.enable_thread_safety();
    cma_set_alloc_limit(0);
    if (error_code == FT_ERR_SUCCESS)
        FT_ASSERT_EQ(true, box.is_thread_safe_enabled());
    else
        FT_ASSERT_EQ(false, box.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.destroy());
    return (1);
}

FT_TEST(test_aabb_enable_thread_safety_alloc_limit_two_then_recover,
    "aabb recovers after alloc limit 2 attempt")
{
    aabb box;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.initialize(-2.0, -2.0, 2.0, 2.0));
    cma_set_alloc_limit(2);
    error_code = box.enable_thread_safety();
    cma_set_alloc_limit(0);
    if (error_code == FT_ERR_SUCCESS)
    {
        FT_ASSERT_EQ(true, box.is_thread_safe_enabled());
        box.disable_thread_safety();
    }
    FT_ASSERT_EQ(false, box.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.enable_thread_safety());
    FT_ASSERT_EQ(true, box.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.destroy());
    return (1);
}

FT_TEST(test_aabb_enable_thread_safety_alloc_limit_three_then_recover,
    "aabb recovers after alloc limit 3 attempt")
{
    aabb box;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.initialize(-2.0, -2.0, 2.0, 2.0));
    cma_set_alloc_limit(3);
    error_code = box.enable_thread_safety();
    cma_set_alloc_limit(0);
    if (error_code == FT_ERR_SUCCESS)
    {
        FT_ASSERT_EQ(true, box.is_thread_safe_enabled());
        box.disable_thread_safety();
    }
    FT_ASSERT_EQ(false, box.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.enable_thread_safety());
    FT_ASSERT_EQ(true, box.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.destroy());
    return (1);
}

FT_TEST(test_aabb_get_mutex_for_testing_alloc_limit_two_consistent,
    "aabb mutex accessor with alloc limit 2 keeps state consistent")
{
    aabb box;
    pt_recursive_mutex *mutex_pointer;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.initialize(0.0, 0.0, 1.0, 1.0));
    cma_set_alloc_limit(2);
    mutex_pointer = box.get_mutex_for_testing();
    cma_set_alloc_limit(0);
    if (mutex_pointer == ft_nullptr)
        FT_ASSERT_EQ(false, box.is_thread_safe_enabled());
    else
        FT_ASSERT_EQ(true, box.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.destroy());
    return (1);
}

FT_TEST(test_aabb_get_mutex_for_testing_alloc_limit_three_consistent,
    "aabb mutex accessor with alloc limit 3 keeps state consistent")
{
    aabb box;
    pt_recursive_mutex *mutex_pointer;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.initialize(0.0, 0.0, 1.0, 1.0));
    cma_set_alloc_limit(3);
    mutex_pointer = box.get_mutex_for_testing();
    cma_set_alloc_limit(0);
    if (mutex_pointer == ft_nullptr)
        FT_ASSERT_EQ(false, box.is_thread_safe_enabled());
    else
        FT_ASSERT_EQ(true, box.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.destroy());
    return (1);
}

FT_TEST(test_aabb_alloc_limit_two_preserves_bounds,
    "aabb alloc limit 2 attempt preserves bounds values")
{
    aabb box;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.initialize(-9.0, -8.0, 7.0, 6.0));
    cma_set_alloc_limit(2);
    (void)box.enable_thread_safety();
    cma_set_alloc_limit(0);
    FT_ASSERT_DOUBLE_EQ(-9.0, box.get_minimum_x());
    FT_ASSERT_DOUBLE_EQ(-8.0, box.get_minimum_y());
    FT_ASSERT_DOUBLE_EQ(7.0, box.get_maximum_x());
    FT_ASSERT_DOUBLE_EQ(6.0, box.get_maximum_y());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.destroy());
    return (1);
}

FT_TEST(test_aabb_enable_thread_safety_alloc_limit_four_state_consistent,
    "aabb alloc limit 4 keeps thread safety state consistent")
{
    aabb box;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.initialize(-1.0, -1.0, 1.0, 1.0));
    cma_set_alloc_limit(4);
    error_code = box.enable_thread_safety();
    cma_set_alloc_limit(0);
    if (error_code == FT_ERR_SUCCESS)
        FT_ASSERT_EQ(true, box.is_thread_safe_enabled());
    else
        FT_ASSERT_EQ(false, box.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.destroy());
    return (1);
}

FT_TEST(test_aabb_enable_thread_safety_alloc_limit_five_state_consistent,
    "aabb alloc limit 5 keeps thread safety state consistent")
{
    aabb box;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.initialize(-1.0, -1.0, 1.0, 1.0));
    cma_set_alloc_limit(5);
    error_code = box.enable_thread_safety();
    cma_set_alloc_limit(0);
    if (error_code == FT_ERR_SUCCESS)
        FT_ASSERT_EQ(true, box.is_thread_safe_enabled());
    else
        FT_ASSERT_EQ(false, box.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.destroy());
    return (1);
}

FT_TEST(test_aabb_enable_thread_safety_alloc_limit_six_state_consistent,
    "aabb alloc limit 6 keeps thread safety state consistent")
{
    aabb box;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.initialize(-1.0, -1.0, 1.0, 1.0));
    cma_set_alloc_limit(6);
    error_code = box.enable_thread_safety();
    cma_set_alloc_limit(0);
    if (error_code == FT_ERR_SUCCESS)
        FT_ASSERT_EQ(true, box.is_thread_safe_enabled());
    else
        FT_ASSERT_EQ(false, box.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.destroy());
    return (1);
}

FT_TEST(test_aabb_enable_thread_safety_alloc_limit_four_then_recover,
    "aabb recovers after alloc limit 4 attempt")
{
    aabb box;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.initialize(-2.0, -2.0, 2.0, 2.0));
    cma_set_alloc_limit(4);
    error_code = box.enable_thread_safety();
    cma_set_alloc_limit(0);
    if (error_code == FT_ERR_SUCCESS)
    {
        box.disable_thread_safety();
        FT_ASSERT_EQ(false, box.is_thread_safe_enabled());
    }
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.enable_thread_safety());
    FT_ASSERT_EQ(true, box.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.destroy());
    return (1);
}

FT_TEST(test_aabb_enable_thread_safety_alloc_limit_five_then_recover,
    "aabb recovers after alloc limit 5 attempt")
{
    aabb box;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.initialize(-2.0, -2.0, 2.0, 2.0));
    cma_set_alloc_limit(5);
    error_code = box.enable_thread_safety();
    cma_set_alloc_limit(0);
    if (error_code == FT_ERR_SUCCESS)
    {
        box.disable_thread_safety();
        FT_ASSERT_EQ(false, box.is_thread_safe_enabled());
    }
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.enable_thread_safety());
    FT_ASSERT_EQ(true, box.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.destroy());
    return (1);
}

FT_TEST(test_aabb_get_mutex_for_testing_alloc_limit_four_consistent,
    "aabb mutex accessor with alloc limit 4 keeps state consistent")
{
    aabb box;
    pt_recursive_mutex *mutex_pointer;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.initialize(0.0, 0.0, 1.0, 1.0));
    cma_set_alloc_limit(4);
    mutex_pointer = box.get_mutex_for_testing();
    cma_set_alloc_limit(0);
    if (mutex_pointer == ft_nullptr)
        FT_ASSERT_EQ(false, box.is_thread_safe_enabled());
    else
        FT_ASSERT_EQ(true, box.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.destroy());
    return (1);
}

FT_TEST(test_aabb_alloc_limit_four_preserves_bounds,
    "aabb alloc limit 4 attempt preserves bounds values")
{
    aabb box;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.initialize(-9.0, -8.0, 7.0, 6.0));
    cma_set_alloc_limit(4);
    (void)box.enable_thread_safety();
    cma_set_alloc_limit(0);
    FT_ASSERT_DOUBLE_EQ(-9.0, box.get_minimum_x());
    FT_ASSERT_DOUBLE_EQ(-8.0, box.get_minimum_y());
    FT_ASSERT_DOUBLE_EQ(7.0, box.get_maximum_x());
    FT_ASSERT_DOUBLE_EQ(6.0, box.get_maximum_y());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.destroy());
    return (1);
}

FT_TEST(test_aabb_enable_thread_safety_alloc_limit_seven_state_consistent,
    "aabb alloc limit 7 keeps thread safety state consistent")
{
    aabb box;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.initialize(-1.0, -1.0, 1.0, 1.0));
    cma_set_alloc_limit(7);
    error_code = box.enable_thread_safety();
    cma_set_alloc_limit(0);
    if (error_code == FT_ERR_SUCCESS)
        FT_ASSERT_EQ(true, box.is_thread_safe_enabled());
    else
        FT_ASSERT_EQ(false, box.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.destroy());
    return (1);
}

FT_TEST(test_aabb_enable_thread_safety_alloc_limit_eight_state_consistent,
    "aabb alloc limit 8 keeps thread safety state consistent")
{
    aabb box;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.initialize(-1.0, -1.0, 1.0, 1.0));
    cma_set_alloc_limit(8);
    error_code = box.enable_thread_safety();
    cma_set_alloc_limit(0);
    if (error_code == FT_ERR_SUCCESS)
        FT_ASSERT_EQ(true, box.is_thread_safe_enabled());
    else
        FT_ASSERT_EQ(false, box.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.destroy());
    return (1);
}

FT_TEST(test_aabb_enable_thread_safety_alloc_limit_nine_state_consistent,
    "aabb alloc limit 9 keeps thread safety state consistent")
{
    aabb box;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.initialize(-1.0, -1.0, 1.0, 1.0));
    cma_set_alloc_limit(9);
    error_code = box.enable_thread_safety();
    cma_set_alloc_limit(0);
    if (error_code == FT_ERR_SUCCESS)
        FT_ASSERT_EQ(true, box.is_thread_safe_enabled());
    else
        FT_ASSERT_EQ(false, box.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.destroy());
    return (1);
}

FT_TEST(test_aabb_enable_thread_safety_alloc_limit_seven_then_recover,
    "aabb recovers after alloc limit 7 attempt")
{
    aabb box;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.initialize(-2.0, -2.0, 2.0, 2.0));
    cma_set_alloc_limit(7);
    error_code = box.enable_thread_safety();
    cma_set_alloc_limit(0);
    if (error_code == FT_ERR_SUCCESS)
    {
        box.disable_thread_safety();
        FT_ASSERT_EQ(false, box.is_thread_safe_enabled());
    }
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.enable_thread_safety());
    FT_ASSERT_EQ(true, box.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.destroy());
    return (1);
}

FT_TEST(test_aabb_enable_thread_safety_alloc_limit_eight_then_recover,
    "aabb recovers after alloc limit 8 attempt")
{
    aabb box;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.initialize(-2.0, -2.0, 2.0, 2.0));
    cma_set_alloc_limit(8);
    error_code = box.enable_thread_safety();
    cma_set_alloc_limit(0);
    if (error_code == FT_ERR_SUCCESS)
    {
        box.disable_thread_safety();
        FT_ASSERT_EQ(false, box.is_thread_safe_enabled());
    }
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.enable_thread_safety());
    FT_ASSERT_EQ(true, box.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.destroy());
    return (1);
}

FT_TEST(test_aabb_get_mutex_for_testing_alloc_limit_seven_consistent,
    "aabb mutex accessor with alloc limit 7 keeps state consistent")
{
    aabb box;
    pt_recursive_mutex *mutex_pointer;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.initialize(0.0, 0.0, 1.0, 1.0));
    cma_set_alloc_limit(7);
    mutex_pointer = box.get_mutex_for_testing();
    cma_set_alloc_limit(0);
    if (mutex_pointer == ft_nullptr)
        FT_ASSERT_EQ(false, box.is_thread_safe_enabled());
    else
        FT_ASSERT_EQ(true, box.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.destroy());
    return (1);
}

FT_TEST(test_aabb_alloc_limit_seven_preserves_bounds,
    "aabb alloc limit 7 attempt preserves bounds values")
{
    aabb box;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.initialize(-9.0, -8.0, 7.0, 6.0));
    cma_set_alloc_limit(7);
    (void)box.enable_thread_safety();
    cma_set_alloc_limit(0);
    FT_ASSERT_DOUBLE_EQ(-9.0, box.get_minimum_x());
    FT_ASSERT_DOUBLE_EQ(-8.0, box.get_minimum_y());
    FT_ASSERT_DOUBLE_EQ(7.0, box.get_maximum_x());
    FT_ASSERT_DOUBLE_EQ(6.0, box.get_maximum_y());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.destroy());
    return (1);
}

FT_TEST(test_intersect_aabb_overlap, "intersect_aabb detects overlapping boxes")
{
    aabb first;
    aabb second;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.initialize(0.0, 0.0, 5.0, 5.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.initialize(3.0, 3.0, 8.0, 8.0));
    FT_ASSERT(intersect_aabb(first, second));
    FT_ASSERT(intersect_aabb(second, first));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.destroy());
    return (1);
}

FT_TEST(test_intersect_aabb_separated,
    "intersect_aabb returns false when separated on axis")
{
    aabb first;
    aabb second;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.initialize(-2.0, -2.0, 2.0, 2.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.initialize(3.0, -1.0, 6.0, 1.0));
    FT_ASSERT_EQ(false, intersect_aabb(first, second));
    FT_ASSERT_EQ(false, intersect_aabb(second, first));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.destroy());
    return (1);
}

FT_TEST(test_intersect_aabb_touching_edge,
    "intersect_aabb treats shared boundary as collision")
{
    aabb first;
    aabb second;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.initialize(0.0, 0.0, 4.0, 4.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.initialize(4.0, 1.0, 7.0, 3.0));
    FT_ASSERT(intersect_aabb(first, second));
    FT_ASSERT(intersect_aabb(second, first));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.destroy());
    return (1);
}

FT_TEST(test_intersect_aabb_parallel_access,
    "intersect_aabb handles concurrent calls with deterministic lock ordering")
{
    aabb first;
    aabb second;
    std::atomic<bool> worker_ready;
    std::atomic<bool> worker_failed;
    std::atomic<bool> worker_completed;
    std::thread worker_thread;
    int iteration_index;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.initialize(0.0, 0.0, 10.0, 10.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.initialize(2.0, 2.0, 8.0, 8.0));
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
            if (intersect_aabb(first, second) == false)
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
        if (intersect_aabb(second, first) == false)
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

FT_TEST(test_intersect_aabb_high_load_overlap_two_threads,
    "intersect_aabb overlap stays stable under high load with two threads")
{
    aabb first;
    aabb second;
    std::atomic<bool> worker_failed;
    std::thread worker_thread;
    int iteration_index;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.initialize(0.0, 0.0, 10.0, 10.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.initialize(2.0, 2.0, 8.0, 8.0));
    worker_failed.store(false);
    worker_thread = std::thread([&first, &second, &worker_failed]() {
        int local_iteration_index;

        local_iteration_index = 0;
        while (local_iteration_index < 8192 && worker_failed.load() == false)
        {
            if (intersect_aabb(first, second) == false)
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
        if (intersect_aabb(second, first) == false)
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

FT_TEST(test_intersect_aabb_high_load_overlap_four_threads,
    "intersect_aabb overlap stays stable under high load with four threads")
{
    aabb first;
    aabb second;
    std::atomic<bool> worker_failed;
    std::thread worker_thread_one;
    std::thread worker_thread_two;
    std::thread worker_thread_three;
    std::thread worker_thread_four;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.initialize(0.0, 0.0, 16.0, 16.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.initialize(1.0, 1.0, 15.0, 15.0));
    worker_failed.store(false);
    worker_thread_one = std::thread([&first, &second, &worker_failed]() {
        int local_iteration_index;

        local_iteration_index = 0;
        while (local_iteration_index < 4096 && worker_failed.load() == false)
        {
            if (intersect_aabb(first, second) == false)
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
            if (intersect_aabb(second, first) == false)
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
            if (intersect_aabb(first, second) == false)
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
            if (intersect_aabb(second, first) == false)
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

FT_TEST(test_intersect_aabb_high_load_touching_two_threads,
    "intersect_aabb touching case stays stable under high load")
{
    aabb first;
    aabb second;
    std::atomic<bool> worker_failed;
    std::thread worker_thread;
    int iteration_index;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.initialize(0.0, 0.0, 4.0, 4.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.initialize(4.0, 1.0, 9.0, 3.0));
    worker_failed.store(false);
    worker_thread = std::thread([&first, &second, &worker_failed]() {
        int local_iteration_index;

        local_iteration_index = 0;
        while (local_iteration_index < 6144 && worker_failed.load() == false)
        {
            if (intersect_aabb(first, second) == false)
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
        if (intersect_aabb(second, first) == false)
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

FT_TEST(test_intersect_aabb_high_load_separated_two_threads,
    "intersect_aabb separated case stays stable under high load")
{
    aabb first;
    aabb second;
    std::atomic<bool> worker_failed;
    std::thread worker_thread;
    int iteration_index;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.initialize(-2.0, -2.0, 2.0, 2.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.initialize(3.0, -1.0, 6.0, 1.0));
    worker_failed.store(false);
    worker_thread = std::thread([&first, &second, &worker_failed]() {
        int local_iteration_index;

        local_iteration_index = 0;
        while (local_iteration_index < 6144 && worker_failed.load() == false)
        {
            if (intersect_aabb(first, second) != false)
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
        if (intersect_aabb(second, first) != false)
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

FT_TEST(test_aabb_move_bidirectional_high_load_with_thread_safety,
    "aabb move in opposite directions stays stable under high load")
{
    aabb first;
    aabb second;
    std::atomic<bool> worker_failed;
    std::thread worker_thread;
    int iteration_index;
    int move_error;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.initialize(-3.0, -3.0, 3.0, 3.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.initialize(-4.0, -4.0, 4.0, 4.0));
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

FT_TEST(test_aabb_set_bounds_getters_contention_high_load_two_threads,
    "aabb setters/getters contention stays stable under high load")
{
    aabb box;
    std::atomic<bool> worker_failed;
    std::thread writer_thread;
    int iteration_index;
    double minimum_x;
    double minimum_y;
    double maximum_x;
    double maximum_y;
    int set_error;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.initialize(-1.0, -1.0, 1.0, 1.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.enable_thread_safety());
    worker_failed.store(false);
    writer_thread = std::thread([&box, &worker_failed]() {
        int local_iteration_index;
        int local_set_error;

        local_iteration_index = 0;
        while (local_iteration_index < 4096 && worker_failed.load() == false)
        {
            if ((local_iteration_index % 2) == 0)
                local_set_error = box.set_bounds(-2.0, -2.0, 2.0, 2.0);
            else
                local_set_error = box.set_bounds(-3.0, -1.0, 3.0, 1.0);
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
        minimum_x = box.get_minimum_x();
        minimum_y = box.get_minimum_y();
        maximum_x = box.get_maximum_x();
        maximum_y = box.get_maximum_y();
        if (minimum_x > maximum_x || minimum_y > maximum_y)
        {
            worker_failed.store(true);
            break;
        }
        iteration_index = iteration_index + 1;
    }
    writer_thread.join();
    FT_ASSERT_EQ(false, worker_failed.load());
    set_error = box.set_bounds(-4.0, -4.0, 4.0, 4.0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, set_error);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.destroy());
    return (1);
}

FT_TEST(test_intersect_aabb_high_load_with_mutating_overlap,
    "intersect_aabb stays true under high-load overlapping mutations")
{
    aabb first;
    aabb second;
    std::atomic<bool> worker_failed;
    std::thread writer_thread;
    std::thread reader_thread_one;
    std::thread reader_thread_two;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.initialize(-10.0, -10.0, 10.0, 10.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.initialize(-2.0, -2.0, 2.0, 2.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.enable_thread_safety());
    worker_failed.store(false);
    writer_thread = std::thread([&second, &worker_failed]() {
        int local_iteration_index;
        int local_set_error;

        local_iteration_index = 0;
        while (local_iteration_index < 4096 && worker_failed.load() == false)
        {
            if ((local_iteration_index % 2) == 0)
                local_set_error = second.set_bounds(-3.0, -3.0, 3.0, 3.0);
            else
                local_set_error = second.set_bounds(-4.0, -1.0, 4.0, 1.0);
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
            if (intersect_aabb(first, second) == false)
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
            if (intersect_aabb(second, first) == false)
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

FT_TEST(test_aabb_move_ring_high_load_three_threads,
    "aabb ring move across three threads stays stable under high load")
{
    aabb first;
    aabb second;
    aabb third;
    std::atomic<bool> worker_failed;
    std::thread thread_one;
    std::thread thread_two;
    std::thread thread_three;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.initialize(-1.0, -1.0, 1.0, 1.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.initialize(-2.0, -2.0, 2.0, 2.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, third.initialize(-3.0, -3.0, 3.0, 3.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, third.enable_thread_safety());
    worker_failed.store(false);
    thread_one = std::thread([&first, &second, &worker_failed]() {
        int local_iteration_index;
        int local_move_error;

        local_iteration_index = 0;
        while (local_iteration_index < 2048 && worker_failed.load() == false)
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
    thread_two = std::thread([&second, &third, &worker_failed]() {
        int local_iteration_index;
        int local_move_error;

        local_iteration_index = 0;
        while (local_iteration_index < 2048 && worker_failed.load() == false)
        {
            local_move_error = second.move(third);
            if (local_move_error != FT_ERR_SUCCESS)
            {
                worker_failed.store(true);
                break;
            }
            local_iteration_index = local_iteration_index + 1;
        }
        return ;
    });
    thread_three = std::thread([&third, &first, &worker_failed]() {
        int local_iteration_index;
        int local_move_error;

        local_iteration_index = 0;
        while (local_iteration_index < 2048 && worker_failed.load() == false)
        {
            local_move_error = third.move(first);
            if (local_move_error != FT_ERR_SUCCESS)
            {
                worker_failed.store(true);
                break;
            }
            local_iteration_index = local_iteration_index + 1;
        }
        return ;
    });
    thread_one.join();
    thread_two.join();
    thread_three.join();
    FT_ASSERT_EQ(false, worker_failed.load());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, third.destroy());
    return (1);
}

FT_TEST(test_aabb_set_bounds_getters_contention_high_load_four_threads,
    "aabb setter/getter contention stays stable with four threads")
{
    aabb box;
    std::atomic<bool> worker_failed;
    std::thread writer_thread;
    std::thread reader_thread_one;
    std::thread reader_thread_two;
    std::thread reader_thread_three;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.initialize(-1.0, -1.0, 1.0, 1.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.enable_thread_safety());
    worker_failed.store(false);
    writer_thread = std::thread([&box, &worker_failed]() {
        int local_iteration_index;
        int local_set_error;

        local_iteration_index = 0;
        while (local_iteration_index < 3072 && worker_failed.load() == false)
        {
            if ((local_iteration_index % 2) == 0)
                local_set_error = box.set_bounds(-2.0, -2.0, 2.0, 2.0);
            else
                local_set_error = box.set_bounds(-3.0, -1.0, 3.0, 1.0);
            if (local_set_error != FT_ERR_SUCCESS)
            {
                worker_failed.store(true);
                break;
            }
            local_iteration_index = local_iteration_index + 1;
        }
        return ;
    });
    reader_thread_one = std::thread([&box, &worker_failed]() {
        int local_iteration_index;

        local_iteration_index = 0;
        while (local_iteration_index < 3072 && worker_failed.load() == false)
        {
            if (box.get_minimum_x() > box.get_maximum_x())
            {
                worker_failed.store(true);
                break;
            }
            local_iteration_index = local_iteration_index + 1;
        }
        return ;
    });
    reader_thread_two = std::thread([&box, &worker_failed]() {
        int local_iteration_index;

        local_iteration_index = 0;
        while (local_iteration_index < 3072 && worker_failed.load() == false)
        {
            if (box.get_minimum_y() > box.get_maximum_y())
            {
                worker_failed.store(true);
                break;
            }
            local_iteration_index = local_iteration_index + 1;
        }
        return ;
    });
    reader_thread_three = std::thread([&box, &worker_failed]() {
        int local_iteration_index;
        double minimum_x;
        double maximum_x;

        local_iteration_index = 0;
        while (local_iteration_index < 3072 && worker_failed.load() == false)
        {
            minimum_x = box.get_minimum_x();
            maximum_x = box.get_maximum_x();
            if (minimum_x > maximum_x)
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
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.destroy());
    return (1);
}

FT_TEST(test_aabb_copy_and_move_initializers,
    "aabb initialize(copy), initialize(move), and move transfer state safely")
{
    aabb source;
    aabb copied_destination;
    aabb moved_destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(-5.0, -4.0, 12.0, 14.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, copied_destination.initialize(source));
    FT_ASSERT_DOUBLE_EQ(-5.0, copied_destination.get_minimum_x());
    FT_ASSERT_DOUBLE_EQ(-4.0, copied_destination.get_minimum_y());
    FT_ASSERT_DOUBLE_EQ(12.0, copied_destination.get_maximum_x());
    FT_ASSERT_DOUBLE_EQ(14.0, copied_destination.get_maximum_y());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_destination.initialize(static_cast<aabb &&>(source)));
    FT_ASSERT_DOUBLE_EQ(-5.0, moved_destination.get_minimum_x());
    FT_ASSERT_DOUBLE_EQ(-4.0, moved_destination.get_minimum_y());
    FT_ASSERT_DOUBLE_EQ(12.0, moved_destination.get_maximum_x());
    FT_ASSERT_DOUBLE_EQ(14.0, moved_destination.get_maximum_y());
    FT_ASSERT_DOUBLE_EQ(0.0, source.get_minimum_x());
    FT_ASSERT_DOUBLE_EQ(0.0, source.get_minimum_y());
    FT_ASSERT_DOUBLE_EQ(0.0, source.get_maximum_x());
    FT_ASSERT_DOUBLE_EQ(0.0, source.get_maximum_y());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, copied_destination.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_destination.destroy());
    return (1);
}

FT_TEST(test_aabb_move_method_transfers_values,
    "aabb move method transfers bounds and clears source bounds")
{
    aabb source;
    aabb destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(-3.0, -2.0, 9.0, 13.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.move(source));
    FT_ASSERT_DOUBLE_EQ(-3.0, destination.get_minimum_x());
    FT_ASSERT_DOUBLE_EQ(-2.0, destination.get_minimum_y());
    FT_ASSERT_DOUBLE_EQ(9.0, destination.get_maximum_x());
    FT_ASSERT_DOUBLE_EQ(13.0, destination.get_maximum_y());
    FT_ASSERT_DOUBLE_EQ(0.0, source.get_minimum_x());
    FT_ASSERT_DOUBLE_EQ(0.0, source.get_minimum_y());
    FT_ASSERT_DOUBLE_EQ(0.0, source.get_maximum_x());
    FT_ASSERT_DOUBLE_EQ(0.0, source.get_maximum_y());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.destroy());
    return (1);
}

FT_TEST(test_aabb_thread_safety_enable_disable,
    "aabb thread safety can be enabled and disabled explicitly")
{
    aabb box;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.initialize(0.0, 0.0, 1.0, 1.0));
    FT_ASSERT_EQ(false, box.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.enable_thread_safety());
    FT_ASSERT_EQ(true, box.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.enable_thread_safety());
    FT_ASSERT_EQ(true, box.is_thread_safe_enabled());
    box.disable_thread_safety();
    FT_ASSERT_EQ(false, box.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.destroy());
    return (1);
}

FT_TEST(test_aabb_initialize_from_destroyed_source_aborts,
    "aabb initialize(copy/move) from destroyed source aborts")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            aabb_initialize_copy_from_destroyed_source_aborts_operation));
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            aabb_initialize_move_from_destroyed_source_aborts_operation));
    return (1);
}

FT_TEST(test_aabb_move_self_initialized_is_noop,
    "aabb move(self) returns success and keeps values when initialized")
{
    aabb box;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.initialize(-3.0, -2.0, 9.0, 13.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.move(box));
    FT_ASSERT_DOUBLE_EQ(-3.0, box.get_minimum_x());
    FT_ASSERT_DOUBLE_EQ(-2.0, box.get_minimum_y());
    FT_ASSERT_DOUBLE_EQ(9.0, box.get_maximum_x());
    FT_ASSERT_DOUBLE_EQ(13.0, box.get_maximum_y());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.destroy());
    return (1);
}

FT_TEST(test_aabb_mutex_testing_accessor_lifecycle,
    "aabb test mutex accessor follows lifecycle and enables thread safety")
{
    aabb box;
    pt_recursive_mutex *mutex_pointer;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.initialize(0.0, 0.0, 1.0, 1.0));
    mutex_pointer = box.get_mutex_for_testing();
    FT_ASSERT_NEQ(ft_nullptr, mutex_pointer);
    FT_ASSERT_EQ(true, box.is_thread_safe_enabled());
    box.disable_thread_safety();
    FT_ASSERT_EQ(false, box.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.destroy());
    FT_ASSERT_EQ(ft_nullptr, box.get_mutex_for_testing());
    return (1);
}

FT_TEST(test_aabb_enable_thread_safety_allocation_failure,
    "aabb enable_thread_safety reports allocation failures")
{
    aabb box;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.initialize(0.0, 0.0, 1.0, 1.0));
    cma_set_alloc_limit(1);
    error_code = box.enable_thread_safety();
    cma_set_alloc_limit(0);
    FT_ASSERT_NEQ(FT_ERR_SUCCESS, error_code);
    FT_ASSERT_EQ(false, box.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.destroy());
    return (1);
}

FT_TEST(test_aabb_initialize_twice_aborts,
    "aabb initialize aborts when called while already initialized")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(aabb_initialize_twice_aborts_operation));
    return (1);
}

FT_TEST(test_aabb_destroy_uninitialized_aborts,
    "aabb destroy aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(aabb_destroy_uninitialized_aborts_operation));
    return (1);
}

FT_TEST(test_aabb_set_bounds_uninitialized_aborts,
    "aabb set_bounds aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(aabb_set_bounds_uninitialized_aborts_operation));
    return (1);
}

FT_TEST(test_aabb_destroy_twice_aborts,
    "aabb destroy aborts when called on already destroyed object")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(aabb_destroy_twice_aborts_operation));
    return (1);
}

FT_TEST(test_aabb_uninitialized_destructor_aborts,
    "aabb destructor aborts when object is left uninitialized")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(aabb_destructor_uninitialized_aborts_operation));
    return (1);
}

FT_TEST(test_aabb_self_move_uninitialized_aborts,
    "aabb move(self) aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(aabb_move_self_uninitialized_aborts_operation));
    return (1);
}

FT_TEST(test_aabb_initialize_copy_from_uninitialized_source_aborts,
    "aabb initialize(copy) aborts when source object is uninitialized")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            aabb_initialize_copy_from_uninitialized_source_aborts_operation));
    return (1);
}

FT_TEST(test_aabb_initialize_move_from_uninitialized_source_aborts,
    "aabb initialize(move) aborts when source object is uninitialized")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            aabb_initialize_move_from_uninitialized_source_aborts_operation));
    return (1);
}

FT_TEST(test_aabb_get_minimum_x_uninitialized_aborts,
    "aabb get_minimum_x aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            aabb_get_minimum_x_uninitialized_aborts_operation));
    return (1);
}

FT_TEST(test_aabb_enable_thread_safety_destroyed_aborts,
    "aabb enable_thread_safety aborts when object is destroyed")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            aabb_enable_thread_safety_destroyed_aborts_operation));
    return (1);
}

FT_TEST(test_aabb_initialize_copy_destination_initialized_aborts,
    "aabb initialize(copy) aborts when destination is already initialized")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            aabb_initialize_copy_destination_initialized_aborts_operation));
    return (1);
}

FT_TEST(test_aabb_initialize_move_destination_initialized_aborts,
    "aabb initialize(move) aborts when destination is already initialized")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            aabb_initialize_move_destination_initialized_aborts_operation));
    return (1);
}

FT_TEST(test_aabb_disable_thread_safety_destroyed_aborts,
    "aabb disable_thread_safety aborts when object is destroyed")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            aabb_disable_thread_safety_destroyed_aborts_operation));
    return (1);
}

FT_TEST(test_aabb_is_thread_safe_enabled_destroyed_aborts,
    "aabb is_thread_safe_enabled aborts when object is destroyed")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            aabb_is_thread_safe_enabled_destroyed_aborts_operation));
    return (1);
}

FT_TEST(test_aabb_set_minimum_x_uninitialized_aborts,
    "aabb set_minimum_x aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            aabb_set_minimum_x_uninitialized_aborts_operation));
    return (1);
}

FT_TEST(test_aabb_get_maximum_y_uninitialized_aborts,
    "aabb get_maximum_y aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            aabb_get_maximum_y_uninitialized_aborts_operation));
    return (1);
}

FT_TEST(test_aabb_set_maximum_x_uninitialized_aborts,
    "aabb set_maximum_x aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            aabb_set_maximum_x_uninitialized_aborts_operation));
    return (1);
}

FT_TEST(test_aabb_get_minimum_y_uninitialized_aborts,
    "aabb get_minimum_y aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            aabb_get_minimum_y_uninitialized_aborts_operation));
    return (1);
}

FT_TEST(test_aabb_enable_thread_safety_uninitialized_aborts,
    "aabb enable_thread_safety aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            aabb_enable_thread_safety_uninitialized_aborts_operation));
    return (1);
}

FT_TEST(test_aabb_set_maximum_y_uninitialized_aborts,
    "aabb set_maximum_y aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            aabb_set_maximum_y_uninitialized_aborts_operation));
    return (1);
}

FT_TEST(test_aabb_get_maximum_x_uninitialized_aborts,
    "aabb get_maximum_x aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            aabb_get_maximum_x_uninitialized_aborts_operation));
    return (1);
}

FT_TEST(test_aabb_disable_thread_safety_uninitialized_aborts,
    "aabb disable_thread_safety aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            aabb_disable_thread_safety_uninitialized_aborts_operation));
    return (1);
}

FT_TEST(test_aabb_is_thread_safe_enabled_uninitialized_aborts,
    "aabb is_thread_safe_enabled aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            aabb_is_thread_safe_enabled_uninitialized_aborts_operation));
    return (1);
}

FT_TEST(test_aabb_set_minimum_uninitialized_aborts,
    "aabb set_minimum aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            aabb_set_minimum_uninitialized_aborts_operation));
    return (1);
}

FT_TEST(test_aabb_set_minimum_y_uninitialized_aborts,
    "aabb set_minimum_y aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            aabb_set_minimum_y_uninitialized_aborts_operation));
    return (1);
}

FT_TEST(test_aabb_set_maximum_uninitialized_aborts,
    "aabb set_maximum aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            aabb_set_maximum_uninitialized_aborts_operation));
    return (1);
}

FT_TEST(test_aabb_move_two_uninitialized_aborts,
    "aabb move aborts when both objects are uninitialized")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            aabb_move_two_uninitialized_aborts_operation));
    return (1);
}

FT_TEST(test_aabb_set_bounds_destroyed_aborts,
    "aabb set_bounds aborts when object is destroyed")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            aabb_set_bounds_destroyed_aborts_operation));
    return (1);
}

FT_TEST(test_aabb_get_minimum_x_destroyed_aborts,
    "aabb get_minimum_x aborts when object is destroyed")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            aabb_get_minimum_x_destroyed_aborts_operation));
    return (1);
}

FT_TEST(test_aabb_move_destroyed_source_aborts,
    "aabb move aborts when source object is destroyed")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            aabb_move_destroyed_source_aborts_operation));
    return (1);
}

FT_TEST(test_aabb_set_minimum_destroyed_aborts,
    "aabb set_minimum aborts when object is destroyed")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            aabb_set_minimum_destroyed_aborts_operation));
    return (1);
}

FT_TEST(test_aabb_set_minimum_x_destroyed_aborts,
    "aabb set_minimum_x aborts when object is destroyed")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            aabb_set_minimum_x_destroyed_aborts_operation));
    return (1);
}

FT_TEST(test_aabb_set_minimum_y_destroyed_aborts,
    "aabb set_minimum_y aborts when object is destroyed")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            aabb_set_minimum_y_destroyed_aborts_operation));
    return (1);
}

FT_TEST(test_aabb_set_maximum_destroyed_aborts,
    "aabb set_maximum aborts when object is destroyed")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            aabb_set_maximum_destroyed_aborts_operation));
    return (1);
}

FT_TEST(test_aabb_set_maximum_x_destroyed_aborts,
    "aabb set_maximum_x aborts when object is destroyed")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            aabb_set_maximum_x_destroyed_aborts_operation));
    return (1);
}

FT_TEST(test_aabb_set_maximum_y_destroyed_aborts,
    "aabb set_maximum_y aborts when object is destroyed")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            aabb_set_maximum_y_destroyed_aborts_operation));
    return (1);
}

FT_TEST(test_aabb_get_minimum_y_destroyed_aborts,
    "aabb get_minimum_y aborts when object is destroyed")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            aabb_get_minimum_y_destroyed_aborts_operation));
    return (1);
}

FT_TEST(test_aabb_get_maximum_x_destroyed_aborts,
    "aabb get_maximum_x aborts when object is destroyed")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            aabb_get_maximum_x_destroyed_aborts_operation));
    return (1);
}

FT_TEST(test_aabb_get_maximum_y_destroyed_aborts,
    "aabb get_maximum_y aborts when object is destroyed")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            aabb_get_maximum_y_destroyed_aborts_operation));
    return (1);
}

FT_TEST(test_aabb_move_destroyed_destination_succeeds,
    "aabb move succeeds when destination object is destroyed")
{
    FT_ASSERT_EQ(1, aabb_move_destroyed_destination_succeeds_operation());
    return (1);
}

FT_TEST(test_aabb_move_uninitialized_source_aborts,
    "aabb move aborts when source object is uninitialized")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            aabb_move_uninitialized_source_aborts_operation));
    return (1);
}

FT_TEST(test_intersect_aabb_uninitialized_first_aborts,
    "intersect_aabb aborts when first box is uninitialized")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            intersect_aabb_uninitialized_first_aborts_operation));
    return (1);
}

FT_TEST(test_intersect_aabb_uninitialized_second_aborts,
    "intersect_aabb aborts when second box is uninitialized")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            intersect_aabb_uninitialized_second_aborts_operation));
    return (1);
}

FT_TEST(test_intersect_aabb_uninitialized_both_aborts,
    "intersect_aabb aborts when both boxes are uninitialized")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            intersect_aabb_uninitialized_both_aborts_operation));
    return (1);
}

FT_TEST(test_intersect_aabb_destroyed_first_aborts,
    "intersect_aabb aborts when first box is destroyed")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            intersect_aabb_destroyed_first_aborts_operation));
    return (1);
}

FT_TEST(test_intersect_aabb_destroyed_second_aborts,
    "intersect_aabb aborts when second box is destroyed")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            intersect_aabb_destroyed_second_aborts_operation));
    return (1);
}

FT_TEST(test_intersect_aabb_destroyed_both_aborts,
    "intersect_aabb aborts when both boxes are destroyed")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            intersect_aabb_destroyed_both_aborts_operation));
    return (1);
}

FT_TEST(test_intersect_aabb_destroyed_and_uninitialized_aborts,
    "intersect_aabb aborts when first is destroyed and second uninitialized")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            intersect_aabb_destroyed_and_uninitialized_aborts_operation));
    return (1);
}

FT_TEST(test_intersect_aabb_high_load_overlap_two_threads_soak_rounds,
    "intersect_aabb overlap stays stable across repeated high-load rounds")
{
    aabb first;
    aabb second;
    std::atomic<bool> worker_failed;
    std::thread worker_thread;
    int round_index;
    int iteration_index;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.initialize(0.0, 0.0, 10.0, 10.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.initialize(2.0, 2.0, 8.0, 8.0));
    round_index = 0;
    while (round_index < 4)
    {
        worker_failed.store(false);
        worker_thread = std::thread([&first, &second, &worker_failed]() {
            int local_iteration_index;

            local_iteration_index = 0;
            while (local_iteration_index < 4096 && worker_failed.load() == false)
            {
                if (intersect_aabb(first, second) == false)
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
            if (intersect_aabb(second, first) == false)
            {
                worker_failed.store(true);
                break;
            }
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

FT_TEST(test_intersect_aabb_high_load_overlap_four_threads_soak_rounds,
    "intersect_aabb four-thread overlap stays stable across soak rounds")
{
    aabb first;
    aabb second;
    std::atomic<bool> worker_failed;
    std::thread thread_one;
    std::thread thread_two;
    std::thread thread_three;
    std::thread thread_four;
    int round_index;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.initialize(0.0, 0.0, 12.0, 12.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.initialize(1.0, 1.0, 11.0, 11.0));
    round_index = 0;
    while (round_index < 3)
    {
        worker_failed.store(false);
        thread_one = std::thread([&first, &second, &worker_failed]() {
            int index;

            index = 0;
            while (index < 3072 && worker_failed.load() == false)
            {
                if (intersect_aabb(first, second) == false)
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
                if (intersect_aabb(second, first) == false)
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
                if (intersect_aabb(first, second) == false)
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
                if (intersect_aabb(second, first) == false)
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

FT_TEST(test_intersect_aabb_high_load_touching_two_threads_soak_rounds,
    "intersect_aabb touching case stays stable across soak rounds")
{
    aabb first;
    aabb second;
    std::atomic<bool> worker_failed;
    std::thread worker_thread;
    int round_index;
    int index;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.initialize(0.0, 0.0, 4.0, 4.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.initialize(4.0, 1.0, 7.0, 3.0));
    round_index = 0;
    while (round_index < 4)
    {
        worker_failed.store(false);
        worker_thread = std::thread([&first, &second, &worker_failed]() {
            int iteration_index;

            iteration_index = 0;
            while (iteration_index < 3072 && worker_failed.load() == false)
            {
                if (intersect_aabb(first, second) == false)
                    worker_failed.store(true);
                iteration_index = iteration_index + 1;
            }
            return ;
        });
        index = 0;
        while (index < 3072 && worker_failed.load() == false)
        {
            if (intersect_aabb(second, first) == false)
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

FT_TEST(test_intersect_aabb_high_load_separated_two_threads_soak_rounds,
    "intersect_aabb separated case stays stable across soak rounds")
{
    aabb first;
    aabb second;
    std::atomic<bool> worker_failed;
    std::thread worker_thread;
    int round_index;
    int index;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.initialize(-2.0, -2.0, 2.0, 2.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.initialize(3.0, -1.0, 6.0, 1.0));
    round_index = 0;
    while (round_index < 4)
    {
        worker_failed.store(false);
        worker_thread = std::thread([&first, &second, &worker_failed]() {
            int iteration_index;

            iteration_index = 0;
            while (iteration_index < 3072 && worker_failed.load() == false)
            {
                if (intersect_aabb(first, second) != false)
                    worker_failed.store(true);
                iteration_index = iteration_index + 1;
            }
            return ;
        });
        index = 0;
        while (index < 3072 && worker_failed.load() == false)
        {
            if (intersect_aabb(second, first) != false)
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

FT_TEST(test_aabb_move_bidirectional_high_load_soak_rounds,
    "aabb bidirectional move stays stable across soak rounds")
{
    aabb first;
    aabb second;
    std::atomic<bool> worker_failed;
    std::thread worker_thread;
    int round_index;
    int index;
    int move_error;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.initialize(-3.0, -3.0, 3.0, 3.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.initialize(-4.0, -4.0, 4.0, 4.0));
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

FT_TEST(test_aabb_set_bounds_getters_contention_high_load_soak_rounds,
    "aabb setter/getter contention stays stable across soak rounds")
{
    aabb box;
    std::atomic<bool> worker_failed;
    std::thread writer_thread;
    int round_index;
    int index;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.initialize(-1.0, -1.0, 1.0, 1.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.enable_thread_safety());
    round_index = 0;
    while (round_index < 3)
    {
        worker_failed.store(false);
        writer_thread = std::thread([&box, &worker_failed]() {
            int iteration_index;
            int local_set_error;

            iteration_index = 0;
            while (iteration_index < 3072 && worker_failed.load() == false)
            {
                if ((iteration_index % 2) == 0)
                    local_set_error = box.set_bounds(-2.0, -2.0, 2.0, 2.0);
                else
                    local_set_error = box.set_bounds(-3.0, -1.0, 3.0, 1.0);
                if (local_set_error != FT_ERR_SUCCESS)
                    worker_failed.store(true);
                iteration_index = iteration_index + 1;
            }
            return ;
        });
        index = 0;
        while (index < 3072 && worker_failed.load() == false)
        {
            if (box.get_minimum_x() > box.get_maximum_x()
                || box.get_minimum_y() > box.get_maximum_y())
                worker_failed.store(true);
            index = index + 1;
        }
        writer_thread.join();
        FT_ASSERT_EQ(false, worker_failed.load());
        round_index = round_index + 1;
    }
    FT_ASSERT_EQ(FT_ERR_SUCCESS, box.destroy());
    return (1);
}

FT_TEST(test_intersect_aabb_high_load_mutating_overlap_soak_rounds,
    "intersect_aabb mutating overlap stays stable across soak rounds")
{
    aabb first;
    aabb second;
    std::atomic<bool> worker_failed;
    std::thread writer_thread;
    std::thread reader_thread;
    int round_index;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.initialize(-10.0, -10.0, 10.0, 10.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.initialize(-2.0, -2.0, 2.0, 2.0));
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
                    local_set_error = second.set_bounds(-3.0, -3.0, 3.0, 3.0);
                else
                    local_set_error = second.set_bounds(-4.0, -1.0, 4.0, 1.0);
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
                if (intersect_aabb(first, second) == false
                    || intersect_aabb(second, first) == false)
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
