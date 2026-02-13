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

static void sphere_initialize_twice_aborts_operation()
{
    sphere shape(0.0, 0.0, 0.0, 1.0);

    (void)shape.initialize();
    return ;
}

static void sphere_destroy_uninitialized_aborts_operation()
{
    sphere shape;

    (void)shape.destroy();
    return ;
}

static void sphere_set_radius_uninitialized_aborts_operation()
{
    sphere shape;

    (void)shape.set_radius(1.0);
    return ;
}

static void sphere_destroy_twice_aborts_operation()
{
    sphere shape(0.0, 0.0, 0.0, 1.0);

    (void)shape.destroy();
    (void)shape.destroy();
    return ;
}

static void sphere_destructor_uninitialized_aborts_operation()
{
    sphere shape;

    (void)shape;
    return ;
}

static void sphere_move_self_uninitialized_aborts_operation()
{
    sphere shape;

    (void)shape.move(shape);
    return ;
}

static void sphere_initialize_copy_from_uninitialized_source_aborts_operation()
{
    sphere source;
    sphere destination(0.0, 0.0, 0.0, 1.0);

    (void)destination.destroy();
    (void)destination.initialize(source);
    return ;
}

static void sphere_initialize_move_from_uninitialized_source_aborts_operation()
{
    sphere source;
    sphere destination(0.0, 0.0, 0.0, 1.0);

    (void)destination.destroy();
    (void)destination.initialize(static_cast<sphere &&>(source));
    return ;
}

static void sphere_initialize_copy_from_destroyed_source_aborts_operation()
{
    sphere source;
    sphere destination(0.0, 0.0, 0.0, 1.0);

    (void)source.initialize(2.0, 3.0, 4.0, 5.0);
    (void)source.destroy();
    (void)destination.destroy();
    (void)destination.initialize(source);
    return ;
}

static void sphere_initialize_move_from_destroyed_source_aborts_operation()
{
    sphere source;
    sphere destination(0.0, 0.0, 0.0, 1.0);

    (void)source.initialize(2.0, 3.0, 4.0, 5.0);
    (void)source.destroy();
    (void)destination.destroy();
    (void)destination.initialize(static_cast<sphere &&>(source));
    return ;
}

static void sphere_get_center_x_uninitialized_aborts_operation()
{
    sphere shape;

    (void)shape.get_center_x();
    return ;
}

static void sphere_enable_thread_safety_destroyed_aborts_operation()
{
    sphere shape(0.0, 0.0, 0.0, 1.0);

    (void)shape.destroy();
    (void)shape.enable_thread_safety();
    return ;
}

static void sphere_initialize_copy_destination_initialized_aborts_operation()
{
    sphere source(0.0, 0.0, 0.0, 1.0);
    sphere destination(2.0, 2.0, 2.0, 3.0);

    (void)destination.initialize(source);
    return ;
}

static void sphere_initialize_move_destination_initialized_aborts_operation()
{
    sphere source(0.0, 0.0, 0.0, 1.0);
    sphere destination(2.0, 2.0, 2.0, 3.0);

    (void)destination.initialize(static_cast<sphere &&>(source));
    return ;
}

static void sphere_disable_thread_safety_destroyed_aborts_operation()
{
    sphere shape(0.0, 0.0, 0.0, 1.0);

    (void)shape.destroy();
    shape.disable_thread_safety();
    return ;
}

static void sphere_is_thread_safe_enabled_destroyed_aborts_operation()
{
    sphere shape(0.0, 0.0, 0.0, 1.0);

    (void)shape.destroy();
    (void)shape.is_thread_safe_enabled();
    return ;
}

static void sphere_set_center_z_uninitialized_aborts_operation()
{
    sphere shape;

    (void)shape.set_center_z(1.0);
    return ;
}

static void sphere_get_radius_uninitialized_aborts_operation()
{
    sphere shape;

    (void)shape.get_radius();
    return ;
}

static void sphere_set_center_uninitialized_aborts_operation()
{
    sphere shape;

    (void)shape.set_center(1.0, 2.0, 3.0);
    return ;
}

static void sphere_get_center_z_uninitialized_aborts_operation()
{
    sphere shape;

    (void)shape.get_center_z();
    return ;
}

static void sphere_enable_thread_safety_uninitialized_aborts_operation()
{
    sphere shape;

    (void)shape.enable_thread_safety();
    return ;
}

static void sphere_set_center_x_uninitialized_aborts_operation()
{
    sphere shape;

    (void)shape.set_center_x(1.0);
    return ;
}

static void sphere_get_center_y_uninitialized_aborts_operation()
{
    sphere shape;

    (void)shape.get_center_y();
    return ;
}

static void sphere_disable_thread_safety_uninitialized_aborts_operation()
{
    sphere shape;

    shape.disable_thread_safety();
    return ;
}

static void sphere_is_thread_safe_enabled_uninitialized_aborts_operation()
{
    sphere shape;

    (void)shape.is_thread_safe_enabled();
    return ;
}

static void sphere_set_center_y_uninitialized_aborts_operation()
{
    sphere shape;

    (void)shape.set_center_y(1.0);
    return ;
}

static void sphere_move_two_uninitialized_aborts_operation()
{
    sphere source;
    sphere destination;

    (void)destination.move(source);
    return ;
}

static void sphere_set_center_destroyed_aborts_operation()
{
    sphere shape(0.0, 0.0, 0.0, 1.0);

    (void)shape.destroy();
    (void)shape.set_center(1.0, 2.0, 3.0);
    return ;
}

static void sphere_get_center_x_destroyed_aborts_operation()
{
    sphere shape(0.0, 0.0, 0.0, 1.0);

    (void)shape.destroy();
    (void)shape.get_center_x();
    return ;
}

static void sphere_set_center_y_destroyed_aborts_operation()
{
    sphere shape(0.0, 0.0, 0.0, 1.0);

    (void)shape.destroy();
    (void)shape.set_center_y(2.0);
    return ;
}

static void sphere_get_radius_destroyed_aborts_second_operation()
{
    sphere shape(0.0, 0.0, 0.0, 1.0);

    (void)shape.destroy();
    (void)shape.get_radius();
    return ;
}

static void sphere_move_destroyed_source_aborts_operation()
{
    sphere source(0.0, 0.0, 0.0, 1.0);
    sphere destination(2.0, 2.0, 2.0, 3.0);

    (void)source.destroy();
    (void)destination.move(source);
    return ;
}

static void sphere_set_center_x_destroyed_aborts_operation()
{
    sphere shape(0.0, 0.0, 0.0, 1.0);

    (void)shape.destroy();
    (void)shape.set_center_x(1.0);
    return ;
}

static void sphere_set_center_z_destroyed_aborts_operation()
{
    sphere shape(0.0, 0.0, 0.0, 1.0);

    (void)shape.destroy();
    (void)shape.set_center_z(1.0);
    return ;
}

static void sphere_get_center_y_destroyed_aborts_operation()
{
    sphere shape(0.0, 0.0, 0.0, 1.0);

    (void)shape.destroy();
    (void)shape.get_center_y();
    return ;
}

static void sphere_get_center_z_destroyed_aborts_operation()
{
    sphere shape(0.0, 0.0, 0.0, 1.0);

    (void)shape.destroy();
    (void)shape.get_center_z();
    return ;
}

static void sphere_set_radius_destroyed_aborts_operation()
{
    sphere shape(0.0, 0.0, 0.0, 1.0);

    (void)shape.destroy();
    (void)shape.set_radius(2.0);
    return ;
}

static int sphere_move_destroyed_destination_succeeds_operation()
{
    sphere source(0.0, 0.0, 0.0, 1.0);
    sphere destination(2.0, 2.0, 2.0, 3.0);
    int move_error;

    (void)destination.destroy();
    move_error = destination.move(source);
    if (move_error != FT_ERR_SUCCESS)
        return (0);
    if (destination.get_center_x() != 0.0 || destination.get_center_y() != 0.0
        || destination.get_center_z() != 0.0 || destination.get_radius() != 1.0)
        return (0);
    if (source.get_center_x() != 0.0 || source.get_center_y() != 0.0
        || source.get_center_z() != 0.0 || source.get_radius() != 0.0)
        return (0);
    return (1);
}

static void sphere_move_uninitialized_source_aborts_operation()
{
    sphere source;
    sphere destination(2.0, 2.0, 2.0, 3.0);

    (void)destination.move(source);
    return ;
}

static void intersect_sphere_uninitialized_first_aborts_operation()
{
    sphere first;
    sphere second(0.0, 0.0, 0.0, 1.0);

    (void)intersect_sphere(first, second);
    return ;
}

static void intersect_sphere_uninitialized_second_aborts_operation()
{
    sphere first(0.0, 0.0, 0.0, 1.0);
    sphere second;

    (void)intersect_sphere(first, second);
    return ;
}

static void intersect_sphere_uninitialized_both_aborts_operation()
{
    sphere first;
    sphere second;

    (void)intersect_sphere(first, second);
    return ;
}

static void intersect_sphere_destroyed_first_aborts_operation()
{
    sphere first(0.0, 0.0, 0.0, 1.0);
    sphere second(0.0, 0.0, 0.0, 1.0);

    (void)first.destroy();
    (void)intersect_sphere(first, second);
    return ;
}

static void intersect_sphere_destroyed_second_aborts_operation()
{
    sphere first(0.0, 0.0, 0.0, 1.0);
    sphere second(0.0, 0.0, 0.0, 1.0);

    (void)second.destroy();
    (void)intersect_sphere(first, second);
    return ;
}

static void intersect_sphere_destroyed_both_aborts_operation()
{
    sphere first(0.0, 0.0, 0.0, 1.0);
    sphere second(0.0, 0.0, 0.0, 1.0);

    (void)first.destroy();
    (void)second.destroy();
    (void)intersect_sphere(first, second);
    return ;
}

static void intersect_sphere_destroyed_and_uninitialized_aborts_operation()
{
    sphere first(0.0, 0.0, 0.0, 1.0);
    sphere second;

    (void)first.destroy();
    (void)intersect_sphere(first, second);
    return ;
}

FT_TEST(test_sphere_initialize_setters_and_getters,
    "sphere initialize, setters, and getters work in initialized lifecycle")
{
    sphere shape;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.set_center(-3.0, 4.0, -5.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.set_center_x(-2.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.set_center_y(5.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.set_center_z(-6.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.set_radius(7.5));
    FT_ASSERT_DOUBLE_EQ(-2.0, shape.get_center_x());
    FT_ASSERT_DOUBLE_EQ(5.0, shape.get_center_y());
    FT_ASSERT_DOUBLE_EQ(-6.0, shape.get_center_z());
    FT_ASSERT_DOUBLE_EQ(7.5, shape.get_radius());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_sphere_reinitialize_after_destroy,
    "sphere can be initialized again after destroy")
{
    sphere shape;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(1.0, 2.0, 3.0, 4.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(-4.0, 5.0, -6.0, 7.0));
    FT_ASSERT_DOUBLE_EQ(-4.0, shape.get_center_x());
    FT_ASSERT_DOUBLE_EQ(5.0, shape.get_center_y());
    FT_ASSERT_DOUBLE_EQ(-6.0, shape.get_center_z());
    FT_ASSERT_DOUBLE_EQ(7.0, shape.get_radius());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_sphere_negative_radius_behavior,
    "sphere currently preserves negative radius values")
{
    sphere shape;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(0.0, 0.0, 0.0, -2.0));
    FT_ASSERT_DOUBLE_EQ(-2.0, shape.get_radius());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.set_radius(-9.25));
    FT_ASSERT_DOUBLE_EQ(-9.25, shape.get_radius());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_sphere_large_finite_values_roundtrip,
    "sphere preserves large finite center and radius values")
{
    sphere shape;
    double large_value;

    large_value = std::numeric_limits<double>::max() / 8.0;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(
            large_value, -large_value / 2.0, large_value / 4.0, large_value / 16.0));
    FT_ASSERT_DOUBLE_EQ(large_value, shape.get_center_x());
    FT_ASSERT_DOUBLE_EQ(-large_value / 2.0, shape.get_center_y());
    FT_ASSERT_DOUBLE_EQ(large_value / 4.0, shape.get_center_z());
    FT_ASSERT_DOUBLE_EQ(large_value / 16.0, shape.get_radius());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_sphere_smallest_normal_radius_roundtrip,
    "sphere preserves smallest normal radius value")
{
    sphere shape;
    double small_value;

    small_value = std::numeric_limits<double>::min();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(0.0, 0.0, 0.0, small_value));
    FT_ASSERT_DOUBLE_EQ(small_value, shape.get_radius());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_sphere_infinite_radius_roundtrip,
    "sphere accepts and preserves infinite radius")
{
    sphere shape;
    double infinity_value;

    infinity_value = std::numeric_limits<double>::infinity();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(0.0, 0.0, 0.0, infinity_value));
    FT_ASSERT(std::isinf(shape.get_radius()));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_sphere_negative_zero_center_x_sign_preserved,
    "sphere preserves negative zero sign on center_x")
{
    sphere shape;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(1.0, 1.0, 1.0, 1.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.set_center_x(-0.0));
    FT_ASSERT(std::signbit(shape.get_center_x()));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_sphere_negative_zero_center_z_sign_preserved,
    "sphere preserves negative zero sign on center_z")
{
    sphere shape;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(1.0, 1.0, 1.0, 1.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.set_center_z(-0.0));
    FT_ASSERT(std::signbit(shape.get_center_z()));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_sphere_set_center_large_values,
    "sphere set_center accepts large finite values")
{
    sphere shape;
    double large_value;

    large_value = std::numeric_limits<double>::max() / 8.0;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(0.0, 0.0, 0.0, 1.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.set_center(
            large_value, -large_value, large_value / 2.0));
    FT_ASSERT_DOUBLE_EQ(large_value, shape.get_center_x());
    FT_ASSERT_DOUBLE_EQ(-large_value, shape.get_center_y());
    FT_ASSERT_DOUBLE_EQ(large_value / 2.0, shape.get_center_z());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_sphere_reinitialize_with_infinite_radius,
    "sphere can be reinitialized with infinite radius after destroy")
{
    sphere shape;
    double infinity_value;

    infinity_value = std::numeric_limits<double>::infinity();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(1.0, 2.0, 3.0, 4.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(0.0, 0.0, 0.0, infinity_value));
    FT_ASSERT(std::isinf(shape.get_radius()));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_sphere_lifecycle_multiple_reinitialize_cycles,
    "sphere remains stable over repeated destroy and initialize cycles")
{
    sphere shape;
    int iteration_index;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(1.0, 2.0, 3.0, 4.0));
    iteration_index = 0;
    while (iteration_index < 6)
    {
        FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
        FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(
                -1.0 - iteration_index,
                2.0 + iteration_index,
                -3.0 - iteration_index,
                4.0 + iteration_index));
        FT_ASSERT_DOUBLE_EQ(-1.0 - iteration_index, shape.get_center_x());
        FT_ASSERT_DOUBLE_EQ(2.0 + iteration_index, shape.get_center_y());
        FT_ASSERT_DOUBLE_EQ(-3.0 - iteration_index, shape.get_center_z());
        FT_ASSERT_DOUBLE_EQ(4.0 + iteration_index, shape.get_radius());
        iteration_index = iteration_index + 1;
    }
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_sphere_lifecycle_thread_safety_cycles,
    "sphere remains stable across initialize/thread-safety/destroy cycles")
{
    sphere shape;
    int iteration_index;

    iteration_index = 0;
    while (iteration_index < 6)
    {
        FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(
                -1.0 - iteration_index,
                2.0 + iteration_index,
                -3.0 - iteration_index,
                4.0 + iteration_index));
        FT_ASSERT_EQ(false, shape.is_thread_safe_enabled());
        FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.enable_thread_safety());
        FT_ASSERT_EQ(true, shape.is_thread_safe_enabled());
        shape.disable_thread_safety();
        FT_ASSERT_EQ(false, shape.is_thread_safe_enabled());
        FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
        iteration_index = iteration_index + 1;
    }
    return (1);
}

FT_TEST(test_sphere_enable_thread_safety_failure_then_recovery,
    "sphere thread safety can recover after allocation failure")
{
    sphere shape;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(0.0, 0.0, 0.0, 1.0));
    cma_set_alloc_limit(1);
    error_code = shape.enable_thread_safety();
    cma_set_alloc_limit(0);
    FT_ASSERT_NEQ(FT_ERR_SUCCESS, error_code);
    FT_ASSERT_EQ(false, shape.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.enable_thread_safety());
    FT_ASSERT_EQ(true, shape.is_thread_safe_enabled());
    shape.disable_thread_safety();
    FT_ASSERT_EQ(false, shape.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_sphere_enable_thread_safety_repeated_alloc_failures,
    "sphere repeated allocation failures keep thread safety disabled")
{
    sphere shape;
    int error_code;
    int iteration_index;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(0.0, 0.0, 0.0, 1.0));
    cma_set_alloc_limit(1);
    iteration_index = 0;
    while (iteration_index < 3)
    {
        error_code = shape.enable_thread_safety();
        FT_ASSERT_NEQ(FT_ERR_SUCCESS, error_code);
        FT_ASSERT_EQ(false, shape.is_thread_safe_enabled());
        iteration_index = iteration_index + 1;
    }
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_sphere_enable_thread_safety_alloc_failure_then_destroy,
    "sphere destroy succeeds after thread safety allocation failure")
{
    sphere shape;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(-2.0, -2.0, -2.0, 2.0));
    cma_set_alloc_limit(1);
    error_code = shape.enable_thread_safety();
    cma_set_alloc_limit(0);
    FT_ASSERT_NEQ(FT_ERR_SUCCESS, error_code);
    FT_ASSERT_EQ(false, shape.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_sphere_enable_thread_safety_alloc_failure_after_disable,
    "sphere can fail enabling thread safety again after disable")
{
    sphere shape;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(-1.0, -1.0, -1.0, 1.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.enable_thread_safety());
    shape.disable_thread_safety();
    FT_ASSERT_EQ(false, shape.is_thread_safe_enabled());
    cma_set_alloc_limit(1);
    error_code = shape.enable_thread_safety();
    cma_set_alloc_limit(0);
    FT_ASSERT_NEQ(FT_ERR_SUCCESS, error_code);
    FT_ASSERT_EQ(false, shape.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_sphere_get_mutex_for_testing_alloc_failure_returns_null,
    "sphere test mutex accessor returns null under allocation failure")
{
    sphere shape;
    pt_recursive_mutex *mutex_pointer;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(0.0, 0.0, 0.0, 1.0));
    cma_set_alloc_limit(1);
    mutex_pointer = shape.get_mutex_for_testing();
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(ft_nullptr, mutex_pointer);
    FT_ASSERT_EQ(false, shape.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_sphere_get_mutex_for_testing_recovers_after_alloc_failure,
    "sphere test mutex accessor recovers once allocation limit resets")
{
    sphere shape;
    pt_recursive_mutex *mutex_pointer;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(0.0, 0.0, 0.0, 1.0));
    cma_set_alloc_limit(1);
    mutex_pointer = shape.get_mutex_for_testing();
    FT_ASSERT_EQ(ft_nullptr, mutex_pointer);
    cma_set_alloc_limit(0);
    mutex_pointer = shape.get_mutex_for_testing();
    FT_ASSERT_NEQ(ft_nullptr, mutex_pointer);
    FT_ASSERT_EQ(true, shape.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_sphere_enable_thread_safety_alloc_failure_reinit_cycle,
    "sphere allocation failure path remains stable across reinitialize cycle")
{
    sphere shape;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(-3.0, -3.0, -3.0, 3.0));
    cma_set_alloc_limit(1);
    error_code = shape.enable_thread_safety();
    cma_set_alloc_limit(0);
    FT_ASSERT_NEQ(FT_ERR_SUCCESS, error_code);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(-4.0, -4.0, -4.0, 4.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.enable_thread_safety());
    FT_ASSERT_EQ(true, shape.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_sphere_enable_thread_safety_alloc_failure_preserves_values,
    "sphere values stay unchanged when enabling thread safety fails")
{
    sphere shape;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(-5.0, -6.0, -7.0, 8.0));
    cma_set_alloc_limit(1);
    error_code = shape.enable_thread_safety();
    cma_set_alloc_limit(0);
    FT_ASSERT_NEQ(FT_ERR_SUCCESS, error_code);
    FT_ASSERT_DOUBLE_EQ(-5.0, shape.get_center_x());
    FT_ASSERT_DOUBLE_EQ(-6.0, shape.get_center_y());
    FT_ASSERT_DOUBLE_EQ(-7.0, shape.get_center_z());
    FT_ASSERT_DOUBLE_EQ(8.0, shape.get_radius());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_sphere_enable_thread_safety_alloc_limit_two_state_consistent,
    "sphere alloc limit 2 keeps thread safety state consistent")
{
    sphere shape;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(-1.0, -1.0, -1.0, 1.0));
    cma_set_alloc_limit(2);
    error_code = shape.enable_thread_safety();
    cma_set_alloc_limit(0);
    if (error_code == FT_ERR_SUCCESS)
        FT_ASSERT_EQ(true, shape.is_thread_safe_enabled());
    else
        FT_ASSERT_EQ(false, shape.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_sphere_enable_thread_safety_alloc_limit_three_state_consistent,
    "sphere alloc limit 3 keeps thread safety state consistent")
{
    sphere shape;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(-1.0, -1.0, -1.0, 1.0));
    cma_set_alloc_limit(3);
    error_code = shape.enable_thread_safety();
    cma_set_alloc_limit(0);
    if (error_code == FT_ERR_SUCCESS)
        FT_ASSERT_EQ(true, shape.is_thread_safe_enabled());
    else
        FT_ASSERT_EQ(false, shape.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_sphere_enable_thread_safety_alloc_limit_two_then_recover,
    "sphere recovers after alloc limit 2 attempt")
{
    sphere shape;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(-2.0, -2.0, -2.0, 2.0));
    cma_set_alloc_limit(2);
    error_code = shape.enable_thread_safety();
    cma_set_alloc_limit(0);
    if (error_code == FT_ERR_SUCCESS)
    {
        FT_ASSERT_EQ(true, shape.is_thread_safe_enabled());
        shape.disable_thread_safety();
    }
    FT_ASSERT_EQ(false, shape.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.enable_thread_safety());
    FT_ASSERT_EQ(true, shape.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_sphere_enable_thread_safety_alloc_limit_three_then_recover,
    "sphere recovers after alloc limit 3 attempt")
{
    sphere shape;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(-2.0, -2.0, -2.0, 2.0));
    cma_set_alloc_limit(3);
    error_code = shape.enable_thread_safety();
    cma_set_alloc_limit(0);
    if (error_code == FT_ERR_SUCCESS)
    {
        FT_ASSERT_EQ(true, shape.is_thread_safe_enabled());
        shape.disable_thread_safety();
    }
    FT_ASSERT_EQ(false, shape.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.enable_thread_safety());
    FT_ASSERT_EQ(true, shape.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_sphere_get_mutex_for_testing_alloc_limit_two_consistent,
    "sphere mutex accessor with alloc limit 2 keeps state consistent")
{
    sphere shape;
    pt_recursive_mutex *mutex_pointer;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(0.0, 0.0, 0.0, 1.0));
    cma_set_alloc_limit(2);
    mutex_pointer = shape.get_mutex_for_testing();
    cma_set_alloc_limit(0);
    if (mutex_pointer == ft_nullptr)
        FT_ASSERT_EQ(false, shape.is_thread_safe_enabled());
    else
        FT_ASSERT_EQ(true, shape.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_sphere_get_mutex_for_testing_alloc_limit_three_consistent,
    "sphere mutex accessor with alloc limit 3 keeps state consistent")
{
    sphere shape;
    pt_recursive_mutex *mutex_pointer;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(0.0, 0.0, 0.0, 1.0));
    cma_set_alloc_limit(3);
    mutex_pointer = shape.get_mutex_for_testing();
    cma_set_alloc_limit(0);
    if (mutex_pointer == ft_nullptr)
        FT_ASSERT_EQ(false, shape.is_thread_safe_enabled());
    else
        FT_ASSERT_EQ(true, shape.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_sphere_alloc_limit_two_preserves_values,
    "sphere alloc limit 2 attempt preserves values")
{
    sphere shape;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(-9.0, -8.0, -7.0, 6.0));
    cma_set_alloc_limit(2);
    (void)shape.enable_thread_safety();
    cma_set_alloc_limit(0);
    FT_ASSERT_DOUBLE_EQ(-9.0, shape.get_center_x());
    FT_ASSERT_DOUBLE_EQ(-8.0, shape.get_center_y());
    FT_ASSERT_DOUBLE_EQ(-7.0, shape.get_center_z());
    FT_ASSERT_DOUBLE_EQ(6.0, shape.get_radius());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_sphere_enable_thread_safety_alloc_limit_four_state_consistent,
    "sphere alloc limit 4 keeps thread safety state consistent")
{
    sphere shape;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(-1.0, -1.0, -1.0, 1.0));
    cma_set_alloc_limit(4);
    error_code = shape.enable_thread_safety();
    cma_set_alloc_limit(0);
    if (error_code == FT_ERR_SUCCESS)
        FT_ASSERT_EQ(true, shape.is_thread_safe_enabled());
    else
        FT_ASSERT_EQ(false, shape.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_sphere_enable_thread_safety_alloc_limit_five_state_consistent,
    "sphere alloc limit 5 keeps thread safety state consistent")
{
    sphere shape;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(-1.0, -1.0, -1.0, 1.0));
    cma_set_alloc_limit(5);
    error_code = shape.enable_thread_safety();
    cma_set_alloc_limit(0);
    if (error_code == FT_ERR_SUCCESS)
        FT_ASSERT_EQ(true, shape.is_thread_safe_enabled());
    else
        FT_ASSERT_EQ(false, shape.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_sphere_enable_thread_safety_alloc_limit_six_state_consistent,
    "sphere alloc limit 6 keeps thread safety state consistent")
{
    sphere shape;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(-1.0, -1.0, -1.0, 1.0));
    cma_set_alloc_limit(6);
    error_code = shape.enable_thread_safety();
    cma_set_alloc_limit(0);
    if (error_code == FT_ERR_SUCCESS)
        FT_ASSERT_EQ(true, shape.is_thread_safe_enabled());
    else
        FT_ASSERT_EQ(false, shape.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_sphere_enable_thread_safety_alloc_limit_four_then_recover,
    "sphere recovers after alloc limit 4 attempt")
{
    sphere shape;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(-2.0, -2.0, -2.0, 2.0));
    cma_set_alloc_limit(4);
    error_code = shape.enable_thread_safety();
    cma_set_alloc_limit(0);
    if (error_code == FT_ERR_SUCCESS)
    {
        shape.disable_thread_safety();
        FT_ASSERT_EQ(false, shape.is_thread_safe_enabled());
    }
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.enable_thread_safety());
    FT_ASSERT_EQ(true, shape.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_sphere_enable_thread_safety_alloc_limit_five_then_recover,
    "sphere recovers after alloc limit 5 attempt")
{
    sphere shape;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(-2.0, -2.0, -2.0, 2.0));
    cma_set_alloc_limit(5);
    error_code = shape.enable_thread_safety();
    cma_set_alloc_limit(0);
    if (error_code == FT_ERR_SUCCESS)
    {
        shape.disable_thread_safety();
        FT_ASSERT_EQ(false, shape.is_thread_safe_enabled());
    }
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.enable_thread_safety());
    FT_ASSERT_EQ(true, shape.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_sphere_get_mutex_for_testing_alloc_limit_four_consistent,
    "sphere mutex accessor with alloc limit 4 keeps state consistent")
{
    sphere shape;
    pt_recursive_mutex *mutex_pointer;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(0.0, 0.0, 0.0, 1.0));
    cma_set_alloc_limit(4);
    mutex_pointer = shape.get_mutex_for_testing();
    cma_set_alloc_limit(0);
    if (mutex_pointer == ft_nullptr)
        FT_ASSERT_EQ(false, shape.is_thread_safe_enabled());
    else
        FT_ASSERT_EQ(true, shape.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_sphere_alloc_limit_four_preserves_values,
    "sphere alloc limit 4 attempt preserves values")
{
    sphere shape;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(-9.0, -8.0, -7.0, 6.0));
    cma_set_alloc_limit(4);
    (void)shape.enable_thread_safety();
    cma_set_alloc_limit(0);
    FT_ASSERT_DOUBLE_EQ(-9.0, shape.get_center_x());
    FT_ASSERT_DOUBLE_EQ(-8.0, shape.get_center_y());
    FT_ASSERT_DOUBLE_EQ(-7.0, shape.get_center_z());
    FT_ASSERT_DOUBLE_EQ(6.0, shape.get_radius());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_sphere_enable_thread_safety_alloc_limit_seven_state_consistent,
    "sphere alloc limit 7 keeps thread safety state consistent")
{
    sphere shape;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(-1.0, -1.0, -1.0, 1.0));
    cma_set_alloc_limit(7);
    error_code = shape.enable_thread_safety();
    cma_set_alloc_limit(0);
    if (error_code == FT_ERR_SUCCESS)
        FT_ASSERT_EQ(true, shape.is_thread_safe_enabled());
    else
        FT_ASSERT_EQ(false, shape.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_sphere_enable_thread_safety_alloc_limit_eight_state_consistent,
    "sphere alloc limit 8 keeps thread safety state consistent")
{
    sphere shape;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(-1.0, -1.0, -1.0, 1.0));
    cma_set_alloc_limit(8);
    error_code = shape.enable_thread_safety();
    cma_set_alloc_limit(0);
    if (error_code == FT_ERR_SUCCESS)
        FT_ASSERT_EQ(true, shape.is_thread_safe_enabled());
    else
        FT_ASSERT_EQ(false, shape.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_sphere_enable_thread_safety_alloc_limit_nine_state_consistent,
    "sphere alloc limit 9 keeps thread safety state consistent")
{
    sphere shape;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(-1.0, -1.0, -1.0, 1.0));
    cma_set_alloc_limit(9);
    error_code = shape.enable_thread_safety();
    cma_set_alloc_limit(0);
    if (error_code == FT_ERR_SUCCESS)
        FT_ASSERT_EQ(true, shape.is_thread_safe_enabled());
    else
        FT_ASSERT_EQ(false, shape.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_sphere_enable_thread_safety_alloc_limit_seven_then_recover,
    "sphere recovers after alloc limit 7 attempt")
{
    sphere shape;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(-2.0, -2.0, -2.0, 2.0));
    cma_set_alloc_limit(7);
    error_code = shape.enable_thread_safety();
    cma_set_alloc_limit(0);
    if (error_code == FT_ERR_SUCCESS)
    {
        shape.disable_thread_safety();
        FT_ASSERT_EQ(false, shape.is_thread_safe_enabled());
    }
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.enable_thread_safety());
    FT_ASSERT_EQ(true, shape.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_sphere_enable_thread_safety_alloc_limit_eight_then_recover,
    "sphere recovers after alloc limit 8 attempt")
{
    sphere shape;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(-2.0, -2.0, -2.0, 2.0));
    cma_set_alloc_limit(8);
    error_code = shape.enable_thread_safety();
    cma_set_alloc_limit(0);
    if (error_code == FT_ERR_SUCCESS)
    {
        shape.disable_thread_safety();
        FT_ASSERT_EQ(false, shape.is_thread_safe_enabled());
    }
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.enable_thread_safety());
    FT_ASSERT_EQ(true, shape.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_sphere_get_mutex_for_testing_alloc_limit_seven_consistent,
    "sphere mutex accessor with alloc limit 7 keeps state consistent")
{
    sphere shape;
    pt_recursive_mutex *mutex_pointer;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(0.0, 0.0, 0.0, 1.0));
    cma_set_alloc_limit(7);
    mutex_pointer = shape.get_mutex_for_testing();
    cma_set_alloc_limit(0);
    if (mutex_pointer == ft_nullptr)
        FT_ASSERT_EQ(false, shape.is_thread_safe_enabled());
    else
        FT_ASSERT_EQ(true, shape.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_sphere_alloc_limit_seven_preserves_values,
    "sphere alloc limit 7 attempt preserves values")
{
    sphere shape;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(-9.0, -8.0, -7.0, 6.0));
    cma_set_alloc_limit(7);
    (void)shape.enable_thread_safety();
    cma_set_alloc_limit(0);
    FT_ASSERT_DOUBLE_EQ(-9.0, shape.get_center_x());
    FT_ASSERT_DOUBLE_EQ(-8.0, shape.get_center_y());
    FT_ASSERT_DOUBLE_EQ(-7.0, shape.get_center_z());
    FT_ASSERT_DOUBLE_EQ(6.0, shape.get_radius());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_intersect_sphere_overlap, "intersect_sphere detects overlapping spheres")
{
    sphere first;
    sphere second;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.initialize(1.0, 2.0, 3.0, 5.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.initialize(4.0, 3.0, 5.0, 4.0));
    FT_ASSERT(intersect_sphere(first, second));
    FT_ASSERT(intersect_sphere(second, first));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.destroy());
    return (1);
}

FT_TEST(test_intersect_sphere_separated,
    "intersect_sphere returns false when centers far apart")
{
    sphere first;
    sphere second;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.initialize(-4.0, -4.0, -4.0, 1.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.initialize(4.0, 4.0, 4.0, 1.0));
    FT_ASSERT_EQ(false, intersect_sphere(first, second));
    FT_ASSERT_EQ(false, intersect_sphere(second, first));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.destroy());
    return (1);
}

FT_TEST(test_intersect_sphere_touching,
    "intersect_sphere treats tangential contact as intersection")
{
    sphere first;
    sphere second;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.initialize(0.0, 0.0, 0.0, 2.5));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.initialize(5.0, 0.0, 0.0, 2.5));
    FT_ASSERT(intersect_sphere(first, second));
    FT_ASSERT(intersect_sphere(second, first));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.destroy());
    return (1);
}

FT_TEST(test_intersect_sphere_parallel_access,
    "intersect_sphere handles concurrent calls with deterministic lock ordering")
{
    sphere first;
    sphere second;
    std::atomic<bool> worker_ready;
    std::atomic<bool> worker_failed;
    std::atomic<bool> worker_completed;
    std::thread worker_thread;
    int iteration_index;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.initialize(0.0, 0.0, 0.0, 6.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.initialize(2.0, 2.0, 2.0, 5.0));
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
            if (intersect_sphere(first, second) == false)
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
        if (intersect_sphere(second, first) == false)
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

FT_TEST(test_intersect_sphere_high_load_overlap_two_threads,
    "intersect_sphere overlap stays stable under high load with two threads")
{
    sphere first;
    sphere second;
    std::atomic<bool> worker_failed;
    std::thread worker_thread;
    int iteration_index;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.initialize(0.0, 0.0, 0.0, 6.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.initialize(2.0, 2.0, 2.0, 5.0));
    worker_failed.store(false);
    worker_thread = std::thread([&first, &second, &worker_failed]() {
        int local_iteration_index;

        local_iteration_index = 0;
        while (local_iteration_index < 8192 && worker_failed.load() == false)
        {
            if (intersect_sphere(first, second) == false)
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
        if (intersect_sphere(second, first) == false)
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

FT_TEST(test_intersect_sphere_high_load_overlap_four_threads,
    "intersect_sphere overlap stays stable under high load with four threads")
{
    sphere first;
    sphere second;
    std::atomic<bool> worker_failed;
    std::thread worker_thread_one;
    std::thread worker_thread_two;
    std::thread worker_thread_three;
    std::thread worker_thread_four;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.initialize(0.0, 0.0, 0.0, 9.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.initialize(1.0, 1.0, 1.0, 8.0));
    worker_failed.store(false);
    worker_thread_one = std::thread([&first, &second, &worker_failed]() {
        int local_iteration_index;

        local_iteration_index = 0;
        while (local_iteration_index < 4096 && worker_failed.load() == false)
        {
            if (intersect_sphere(first, second) == false)
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
            if (intersect_sphere(second, first) == false)
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
            if (intersect_sphere(first, second) == false)
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
            if (intersect_sphere(second, first) == false)
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

FT_TEST(test_intersect_sphere_high_load_touching_two_threads,
    "intersect_sphere touching case stays stable under high load")
{
    sphere first;
    sphere second;
    std::atomic<bool> worker_failed;
    std::thread worker_thread;
    int iteration_index;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.initialize(0.0, 0.0, 0.0, 2.5));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.initialize(5.0, 0.0, 0.0, 2.5));
    worker_failed.store(false);
    worker_thread = std::thread([&first, &second, &worker_failed]() {
        int local_iteration_index;

        local_iteration_index = 0;
        while (local_iteration_index < 6144 && worker_failed.load() == false)
        {
            if (intersect_sphere(first, second) == false)
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
        if (intersect_sphere(second, first) == false)
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

FT_TEST(test_intersect_sphere_high_load_separated_two_threads,
    "intersect_sphere separated case stays stable under high load")
{
    sphere first;
    sphere second;
    std::atomic<bool> worker_failed;
    std::thread worker_thread;
    int iteration_index;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.initialize(-4.0, -4.0, -4.0, 1.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.initialize(4.0, 4.0, 4.0, 1.0));
    worker_failed.store(false);
    worker_thread = std::thread([&first, &second, &worker_failed]() {
        int local_iteration_index;

        local_iteration_index = 0;
        while (local_iteration_index < 6144 && worker_failed.load() == false)
        {
            if (intersect_sphere(first, second) != false)
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
        if (intersect_sphere(second, first) != false)
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

FT_TEST(test_sphere_move_bidirectional_high_load_with_thread_safety,
    "sphere move in opposite directions stays stable under high load")
{
    sphere first;
    sphere second;
    std::atomic<bool> worker_failed;
    std::thread worker_thread;
    int iteration_index;
    int move_error;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.initialize(0.0, 0.0, 0.0, 3.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.initialize(1.0, 1.0, 1.0, 4.0));
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

FT_TEST(test_sphere_setters_getters_contention_high_load_two_threads,
    "sphere setters/getters contention stays stable under high load")
{
    sphere shape;
    std::atomic<bool> worker_failed;
    std::thread writer_thread;
    int iteration_index;
    double center_x;
    double center_y;
    double center_z;
    double radius;
    int set_error;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(0.0, 0.0, 0.0, 1.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.enable_thread_safety());
    worker_failed.store(false);
    writer_thread = std::thread([&shape, &worker_failed]() {
        int local_iteration_index;
        int local_set_error;

        local_iteration_index = 0;
        while (local_iteration_index < 4096 && worker_failed.load() == false)
        {
            if ((local_iteration_index % 2) == 0)
                local_set_error = shape.set_center(2.0, -2.0, 2.0);
            else
                local_set_error = shape.set_center(-3.0, 3.0, -3.0);
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
        center_z = shape.get_center_z();
        radius = shape.get_radius();
        if ((std::isfinite(center_x) == false) || (std::isfinite(center_y) == false)
            || (std::isfinite(center_z) == false) || (std::isfinite(radius) == false))
        {
            worker_failed.store(true);
            break;
        }
        iteration_index = iteration_index + 1;
    }
    writer_thread.join();
    FT_ASSERT_EQ(false, worker_failed.load());
    set_error = shape.set_center(0.0, 0.0, 0.0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, set_error);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_intersect_sphere_high_load_with_mutating_overlap,
    "intersect_sphere stays true under high-load overlapping mutations")
{
    sphere first;
    sphere second;
    std::atomic<bool> worker_failed;
    std::thread writer_thread;
    std::thread reader_thread_one;
    std::thread reader_thread_two;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.initialize(0.0, 0.0, 0.0, 15.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.initialize(1.0, 1.0, 1.0, 4.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.enable_thread_safety());
    worker_failed.store(false);
    writer_thread = std::thread([&second, &worker_failed]() {
        int local_iteration_index;
        int local_set_error;

        local_iteration_index = 0;
        while (local_iteration_index < 4096 && worker_failed.load() == false)
        {
            if ((local_iteration_index % 2) == 0)
                local_set_error = second.set_center(2.0, 2.0, 2.0);
            else
                local_set_error = second.set_center(-1.0, 1.0, -1.0);
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
            if (intersect_sphere(first, second) == false)
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
            if (intersect_sphere(second, first) == false)
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

FT_TEST(test_sphere_setters_getters_contention_high_load_four_threads,
    "sphere setter/getter contention stays stable with four threads")
{
    sphere shape;
    std::atomic<bool> worker_failed;
    std::thread writer_thread;
    std::thread reader_thread_one;
    std::thread reader_thread_two;
    std::thread reader_thread_three;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(0.0, 0.0, 0.0, 1.0));
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
                    static_cast<double>((local_iteration_index % 5) - 2),
                    static_cast<double>((local_iteration_index % 9) - 4));
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
            if (std::isfinite(shape.get_center_y()) == false
                || std::isfinite(shape.get_center_z()) == false)
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

FT_TEST(test_sphere_copy_and_move_initializers,
    "sphere initialize(copy), initialize(move), and move transfer state safely")
{
    sphere source;
    sphere copied_destination;
    sphere moved_destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(7.0, -3.0, 2.0, 9.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, copied_destination.initialize(source));
    FT_ASSERT_DOUBLE_EQ(7.0, copied_destination.get_center_x());
    FT_ASSERT_DOUBLE_EQ(-3.0, copied_destination.get_center_y());
    FT_ASSERT_DOUBLE_EQ(2.0, copied_destination.get_center_z());
    FT_ASSERT_DOUBLE_EQ(9.0, copied_destination.get_radius());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_destination.initialize(static_cast<sphere &&>(source)));
    FT_ASSERT_DOUBLE_EQ(7.0, moved_destination.get_center_x());
    FT_ASSERT_DOUBLE_EQ(-3.0, moved_destination.get_center_y());
    FT_ASSERT_DOUBLE_EQ(2.0, moved_destination.get_center_z());
    FT_ASSERT_DOUBLE_EQ(9.0, moved_destination.get_radius());
    FT_ASSERT_DOUBLE_EQ(0.0, source.get_center_x());
    FT_ASSERT_DOUBLE_EQ(0.0, source.get_center_y());
    FT_ASSERT_DOUBLE_EQ(0.0, source.get_center_z());
    FT_ASSERT_DOUBLE_EQ(0.0, source.get_radius());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, copied_destination.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_destination.destroy());
    return (1);
}

FT_TEST(test_sphere_move_method_transfers_values,
    "sphere move method transfers center/radius and clears source")
{
    sphere source;
    sphere destination;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize(4.0, 5.0, 6.0, 7.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.move(source));
    FT_ASSERT_DOUBLE_EQ(4.0, destination.get_center_x());
    FT_ASSERT_DOUBLE_EQ(5.0, destination.get_center_y());
    FT_ASSERT_DOUBLE_EQ(6.0, destination.get_center_z());
    FT_ASSERT_DOUBLE_EQ(7.0, destination.get_radius());
    FT_ASSERT_DOUBLE_EQ(0.0, source.get_center_x());
    FT_ASSERT_DOUBLE_EQ(0.0, source.get_center_y());
    FT_ASSERT_DOUBLE_EQ(0.0, source.get_center_z());
    FT_ASSERT_DOUBLE_EQ(0.0, source.get_radius());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.destroy());
    return (1);
}

FT_TEST(test_sphere_thread_safety_enable_disable,
    "sphere thread safety can be enabled and disabled explicitly")
{
    sphere shape;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(0.0, 0.0, 0.0, 1.0));
    FT_ASSERT_EQ(false, shape.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.enable_thread_safety());
    FT_ASSERT_EQ(true, shape.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.enable_thread_safety());
    FT_ASSERT_EQ(true, shape.is_thread_safe_enabled());
    shape.disable_thread_safety();
    FT_ASSERT_EQ(false, shape.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_sphere_initialize_from_destroyed_source_aborts,
    "sphere initialize(copy/move) from destroyed source aborts")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            sphere_initialize_copy_from_destroyed_source_aborts_operation));
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            sphere_initialize_move_from_destroyed_source_aborts_operation));
    return (1);
}

FT_TEST(test_sphere_move_self_initialized_is_noop,
    "sphere move(self) returns success and keeps values when initialized")
{
    sphere shape;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(4.0, 5.0, 6.0, 7.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.move(shape));
    FT_ASSERT_DOUBLE_EQ(4.0, shape.get_center_x());
    FT_ASSERT_DOUBLE_EQ(5.0, shape.get_center_y());
    FT_ASSERT_DOUBLE_EQ(6.0, shape.get_center_z());
    FT_ASSERT_DOUBLE_EQ(7.0, shape.get_radius());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_sphere_mutex_testing_accessor_lifecycle,
    "sphere test mutex accessor follows lifecycle and enables thread safety")
{
    sphere shape;
    pt_recursive_mutex *mutex_pointer;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(0.0, 0.0, 0.0, 1.0));
    mutex_pointer = shape.get_mutex_for_testing();
    FT_ASSERT_NEQ(ft_nullptr, mutex_pointer);
    FT_ASSERT_EQ(true, shape.is_thread_safe_enabled());
    shape.disable_thread_safety();
    FT_ASSERT_EQ(false, shape.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    FT_ASSERT_EQ(ft_nullptr, shape.get_mutex_for_testing());
    return (1);
}

FT_TEST(test_sphere_enable_thread_safety_allocation_failure,
    "sphere enable_thread_safety reports allocation failures")
{
    sphere shape;
    int error_code;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(0.0, 0.0, 0.0, 1.0));
    cma_set_alloc_limit(1);
    error_code = shape.enable_thread_safety();
    cma_set_alloc_limit(0);
    FT_ASSERT_NEQ(FT_ERR_SUCCESS, error_code);
    FT_ASSERT_EQ(false, shape.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.destroy());
    return (1);
}

FT_TEST(test_sphere_initialize_twice_aborts,
    "sphere initialize aborts when called while already initialized")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(sphere_initialize_twice_aborts_operation));
    return (1);
}

FT_TEST(test_sphere_destroy_uninitialized_aborts,
    "sphere destroy aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(sphere_destroy_uninitialized_aborts_operation));
    return (1);
}

FT_TEST(test_sphere_set_radius_uninitialized_aborts,
    "sphere set_radius aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(sphere_set_radius_uninitialized_aborts_operation));
    return (1);
}

FT_TEST(test_sphere_destroy_twice_aborts,
    "sphere destroy aborts when called on already destroyed object")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(sphere_destroy_twice_aborts_operation));
    return (1);
}

FT_TEST(test_sphere_uninitialized_destructor_aborts,
    "sphere destructor aborts when object is left uninitialized")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(sphere_destructor_uninitialized_aborts_operation));
    return (1);
}

FT_TEST(test_sphere_self_move_uninitialized_aborts,
    "sphere move(self) aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(sphere_move_self_uninitialized_aborts_operation));
    return (1);
}

FT_TEST(test_sphere_initialize_copy_from_uninitialized_source_aborts,
    "sphere initialize(copy) aborts when source object is uninitialized")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            sphere_initialize_copy_from_uninitialized_source_aborts_operation));
    return (1);
}

FT_TEST(test_sphere_initialize_move_from_uninitialized_source_aborts,
    "sphere initialize(move) aborts when source object is uninitialized")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            sphere_initialize_move_from_uninitialized_source_aborts_operation));
    return (1);
}

FT_TEST(test_sphere_get_center_x_uninitialized_aborts,
    "sphere get_center_x aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            sphere_get_center_x_uninitialized_aborts_operation));
    return (1);
}

FT_TEST(test_sphere_enable_thread_safety_destroyed_aborts,
    "sphere enable_thread_safety aborts when object is destroyed")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            sphere_enable_thread_safety_destroyed_aborts_operation));
    return (1);
}

FT_TEST(test_sphere_initialize_copy_destination_initialized_aborts,
    "sphere initialize(copy) aborts when destination is already initialized")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            sphere_initialize_copy_destination_initialized_aborts_operation));
    return (1);
}

FT_TEST(test_sphere_initialize_move_destination_initialized_aborts,
    "sphere initialize(move) aborts when destination is already initialized")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            sphere_initialize_move_destination_initialized_aborts_operation));
    return (1);
}

FT_TEST(test_sphere_disable_thread_safety_destroyed_aborts,
    "sphere disable_thread_safety aborts when object is destroyed")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            sphere_disable_thread_safety_destroyed_aborts_operation));
    return (1);
}

FT_TEST(test_sphere_is_thread_safe_enabled_destroyed_aborts,
    "sphere is_thread_safe_enabled aborts when object is destroyed")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            sphere_is_thread_safe_enabled_destroyed_aborts_operation));
    return (1);
}

FT_TEST(test_sphere_set_center_z_uninitialized_aborts,
    "sphere set_center_z aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            sphere_set_center_z_uninitialized_aborts_operation));
    return (1);
}

FT_TEST(test_sphere_get_radius_uninitialized_aborts,
    "sphere get_radius aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            sphere_get_radius_uninitialized_aborts_operation));
    return (1);
}

FT_TEST(test_sphere_set_center_uninitialized_aborts,
    "sphere set_center aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            sphere_set_center_uninitialized_aborts_operation));
    return (1);
}

FT_TEST(test_sphere_get_center_z_uninitialized_aborts,
    "sphere get_center_z aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            sphere_get_center_z_uninitialized_aborts_operation));
    return (1);
}

FT_TEST(test_sphere_enable_thread_safety_uninitialized_aborts,
    "sphere enable_thread_safety aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            sphere_enable_thread_safety_uninitialized_aborts_operation));
    return (1);
}

FT_TEST(test_sphere_set_center_x_uninitialized_aborts,
    "sphere set_center_x aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            sphere_set_center_x_uninitialized_aborts_operation));
    return (1);
}

FT_TEST(test_sphere_get_center_y_uninitialized_aborts,
    "sphere get_center_y aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            sphere_get_center_y_uninitialized_aborts_operation));
    return (1);
}

FT_TEST(test_sphere_disable_thread_safety_uninitialized_aborts,
    "sphere disable_thread_safety aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            sphere_disable_thread_safety_uninitialized_aborts_operation));
    return (1);
}

FT_TEST(test_sphere_is_thread_safe_enabled_uninitialized_aborts,
    "sphere is_thread_safe_enabled aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            sphere_is_thread_safe_enabled_uninitialized_aborts_operation));
    return (1);
}

FT_TEST(test_sphere_set_center_y_uninitialized_aborts,
    "sphere set_center_y aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            sphere_set_center_y_uninitialized_aborts_operation));
    return (1);
}

FT_TEST(test_sphere_move_two_uninitialized_aborts,
    "sphere move aborts when both objects are uninitialized")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            sphere_move_two_uninitialized_aborts_operation));
    return (1);
}

FT_TEST(test_sphere_set_center_destroyed_aborts,
    "sphere set_center aborts when object is destroyed")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            sphere_set_center_destroyed_aborts_operation));
    return (1);
}

FT_TEST(test_sphere_get_center_x_destroyed_aborts,
    "sphere get_center_x aborts when object is destroyed")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            sphere_get_center_x_destroyed_aborts_operation));
    return (1);
}

FT_TEST(test_sphere_set_center_y_destroyed_aborts,
    "sphere set_center_y aborts when object is destroyed")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            sphere_set_center_y_destroyed_aborts_operation));
    return (1);
}

FT_TEST(test_sphere_get_radius_destroyed_aborts_second,
    "sphere get_radius aborts when object is destroyed")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            sphere_get_radius_destroyed_aborts_second_operation));
    return (1);
}

FT_TEST(test_sphere_move_destroyed_source_aborts,
    "sphere move aborts when source object is destroyed")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            sphere_move_destroyed_source_aborts_operation));
    return (1);
}

FT_TEST(test_sphere_set_center_x_destroyed_aborts,
    "sphere set_center_x aborts when object is destroyed")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            sphere_set_center_x_destroyed_aborts_operation));
    return (1);
}

FT_TEST(test_sphere_set_center_z_destroyed_aborts,
    "sphere set_center_z aborts when object is destroyed")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            sphere_set_center_z_destroyed_aborts_operation));
    return (1);
}

FT_TEST(test_sphere_get_center_y_destroyed_aborts,
    "sphere get_center_y aborts when object is destroyed")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            sphere_get_center_y_destroyed_aborts_operation));
    return (1);
}

FT_TEST(test_sphere_get_center_z_destroyed_aborts,
    "sphere get_center_z aborts when object is destroyed")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            sphere_get_center_z_destroyed_aborts_operation));
    return (1);
}

FT_TEST(test_sphere_set_radius_destroyed_aborts,
    "sphere set_radius aborts when object is destroyed")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            sphere_set_radius_destroyed_aborts_operation));
    return (1);
}

FT_TEST(test_sphere_move_destroyed_destination_succeeds,
    "sphere move succeeds when destination object is destroyed")
{
    FT_ASSERT_EQ(1, sphere_move_destroyed_destination_succeeds_operation());
    return (1);
}

FT_TEST(test_sphere_move_uninitialized_source_aborts,
    "sphere move aborts when source object is uninitialized")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            sphere_move_uninitialized_source_aborts_operation));
    return (1);
}

FT_TEST(test_intersect_sphere_uninitialized_first_aborts,
    "intersect_sphere aborts when first sphere is uninitialized")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            intersect_sphere_uninitialized_first_aborts_operation));
    return (1);
}

FT_TEST(test_intersect_sphere_uninitialized_second_aborts,
    "intersect_sphere aborts when second sphere is uninitialized")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            intersect_sphere_uninitialized_second_aborts_operation));
    return (1);
}

FT_TEST(test_intersect_sphere_uninitialized_both_aborts,
    "intersect_sphere aborts when both spheres are uninitialized")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            intersect_sphere_uninitialized_both_aborts_operation));
    return (1);
}

FT_TEST(test_intersect_sphere_destroyed_first_aborts,
    "intersect_sphere aborts when first sphere is destroyed")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            intersect_sphere_destroyed_first_aborts_operation));
    return (1);
}

FT_TEST(test_intersect_sphere_destroyed_second_aborts,
    "intersect_sphere aborts when second sphere is destroyed")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            intersect_sphere_destroyed_second_aborts_operation));
    return (1);
}

FT_TEST(test_intersect_sphere_destroyed_both_aborts,
    "intersect_sphere aborts when both spheres are destroyed")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            intersect_sphere_destroyed_both_aborts_operation));
    return (1);
}

FT_TEST(test_intersect_sphere_destroyed_and_uninitialized_aborts,
    "intersect_sphere aborts when first is destroyed and second uninitialized")
{
    FT_ASSERT_EQ(1, geometry_expect_sigabrt(
            intersect_sphere_destroyed_and_uninitialized_aborts_operation));
    return (1);
}

FT_TEST(test_intersect_sphere_high_load_overlap_two_threads_soak_rounds,
    "intersect_sphere overlap stays stable across repeated high-load rounds")
{
    sphere first;
    sphere second;
    std::atomic<bool> worker_failed;
    std::thread worker_thread;
    int round_index;
    int iteration_index;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.initialize(0.0, 0.0, 0.0, 6.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.initialize(2.0, 2.0, 2.0, 5.0));
    round_index = 0;
    while (round_index < 4)
    {
        worker_failed.store(false);
        worker_thread = std::thread([&first, &second, &worker_failed]() {
            int local_iteration_index;

            local_iteration_index = 0;
            while (local_iteration_index < 4096 && worker_failed.load() == false)
            {
                if (intersect_sphere(first, second) == false)
                    worker_failed.store(true);
                local_iteration_index = local_iteration_index + 1;
            }
            return ;
        });
        iteration_index = 0;
        while (iteration_index < 4096 && worker_failed.load() == false)
        {
            if (intersect_sphere(second, first) == false)
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

FT_TEST(test_intersect_sphere_high_load_overlap_four_threads_soak_rounds,
    "intersect_sphere four-thread overlap stays stable across soak rounds")
{
    sphere first;
    sphere second;
    std::atomic<bool> worker_failed;
    std::thread thread_one;
    std::thread thread_two;
    std::thread thread_three;
    std::thread thread_four;
    int round_index;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.initialize(0.0, 0.0, 0.0, 9.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.initialize(1.0, 1.0, 1.0, 8.0));
    round_index = 0;
    while (round_index < 3)
    {
        worker_failed.store(false);
        thread_one = std::thread([&first, &second, &worker_failed]() {
            int index;
            index = 0;
            while (index < 3072 && worker_failed.load() == false)
            {
                if (intersect_sphere(first, second) == false)
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
                if (intersect_sphere(second, first) == false)
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
                if (intersect_sphere(first, second) == false)
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
                if (intersect_sphere(second, first) == false)
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

FT_TEST(test_intersect_sphere_high_load_touching_two_threads_soak_rounds,
    "intersect_sphere touching case stays stable across soak rounds")
{
    sphere first;
    sphere second;
    std::atomic<bool> worker_failed;
    std::thread worker_thread;
    int round_index;
    int index;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.initialize(0.0, 0.0, 0.0, 2.5));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.initialize(5.0, 0.0, 0.0, 2.5));
    round_index = 0;
    while (round_index < 4)
    {
        worker_failed.store(false);
        worker_thread = std::thread([&first, &second, &worker_failed]() {
            int iteration_index;
            iteration_index = 0;
            while (iteration_index < 3072 && worker_failed.load() == false)
            {
                if (intersect_sphere(first, second) == false)
                    worker_failed.store(true);
                iteration_index = iteration_index + 1;
            }
            return ;
        });
        index = 0;
        while (index < 3072 && worker_failed.load() == false)
        {
            if (intersect_sphere(second, first) == false)
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

FT_TEST(test_intersect_sphere_high_load_separated_two_threads_soak_rounds,
    "intersect_sphere separated case stays stable across soak rounds")
{
    sphere first;
    sphere second;
    std::atomic<bool> worker_failed;
    std::thread worker_thread;
    int round_index;
    int index;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.initialize(-4.0, -4.0, -4.0, 1.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.initialize(4.0, 4.0, 4.0, 1.0));
    round_index = 0;
    while (round_index < 4)
    {
        worker_failed.store(false);
        worker_thread = std::thread([&first, &second, &worker_failed]() {
            int iteration_index;
            iteration_index = 0;
            while (iteration_index < 3072 && worker_failed.load() == false)
            {
                if (intersect_sphere(first, second) != false)
                    worker_failed.store(true);
                iteration_index = iteration_index + 1;
            }
            return ;
        });
        index = 0;
        while (index < 3072 && worker_failed.load() == false)
        {
            if (intersect_sphere(second, first) != false)
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

FT_TEST(test_sphere_move_bidirectional_high_load_soak_rounds,
    "sphere bidirectional move stays stable across soak rounds")
{
    sphere first;
    sphere second;
    std::atomic<bool> worker_failed;
    std::thread worker_thread;
    int round_index;
    int index;
    int move_error;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.initialize(0.0, 0.0, 0.0, 3.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.initialize(1.0, 1.0, 1.0, 4.0));
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

FT_TEST(test_sphere_setters_getters_contention_high_load_soak_rounds,
    "sphere setter/getter contention stays stable across soak rounds")
{
    sphere shape;
    std::atomic<bool> worker_failed;
    std::thread writer_thread;
    int round_index;
    int index;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, shape.initialize(0.0, 0.0, 0.0, 1.0));
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
                        static_cast<double>((iteration_index % 5) - 2),
                        static_cast<double>((iteration_index % 9) - 4));
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
                || std::isfinite(shape.get_center_z()) == false
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

FT_TEST(test_intersect_sphere_high_load_mutating_overlap_soak_rounds,
    "intersect_sphere mutating overlap stays stable across soak rounds")
{
    sphere first;
    sphere second;
    std::atomic<bool> worker_failed;
    std::thread writer_thread;
    std::thread reader_thread;
    int round_index;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, first.initialize(0.0, 0.0, 0.0, 15.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second.initialize(1.0, 1.0, 1.0, 4.0));
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
                    local_set_error = second.set_center(2.0, 2.0, 2.0);
                else
                    local_set_error = second.set_center(-1.0, 1.0, -1.0);
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
                if (intersect_sphere(first, second) == false
                    || intersect_sphere(second, first) == false)
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
