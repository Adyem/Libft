#include "../test_internal.hpp"
#include "../../Math/math_polynomial.hpp"
#include "../../Math/math.hpp"
#include "../../CMA/CMA.hpp"
#include "../../System_utils/test_runner.hpp"
#include <sys/wait.h>
#include <unistd.h>
#include <csignal>
#include <cstring>

#ifndef LIBFT_TEST_BUILD
#endif

static int spline_expect_sigabrt(void (*operation)())
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

static ft_size_t spline_current_allocated_bytes(void)
{
    ft_size_t allocation_count;
    ft_size_t free_count;
    ft_size_t current_bytes;
    ft_size_t peak_bytes;
    int32_t status;

    allocation_count = 0;
    free_count = 0;
    current_bytes = 0;
    peak_bytes = 0;
    status = cma_get_extended_stats(&allocation_count, &free_count,
            &current_bytes, &peak_bytes);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, status);
    return (current_bytes);
}

static void spline_fill_sample_points(ft_vector<double> &x_values,
    ft_vector<double> &y_values)
{
    x_values.push_back(0.0);
    x_values.push_back(1.0);
    x_values.push_back(2.0);
    x_values.push_back(3.0);
    y_values.push_back(0.0);
    y_values.push_back(1.0);
    y_values.push_back(8.0);
    y_values.push_back(27.0);
    return ;
}

static void spline_call_initialize_twice()
{
    ft_cubic_spline spline_value;

    (void)spline_value.initialize();
    (void)spline_value.initialize();
    return ;
}

static void spline_call_destroy_uninitialized()
{
    ft_cubic_spline spline_value;

    (void)spline_value.destroy();
    return ;
}

static void spline_call_copy_from_uninitialized_source()
{
    ft_cubic_spline destination_value;
    ft_cubic_spline source_value;

    (void)destination_value.initialize();
    (void)destination_value.initialize(source_value);
    return ;
}

static void spline_call_initialize_move_from_uninitialized_source()
{
    ft_cubic_spline destination_value;
    ft_cubic_spline source_value;

    (void)destination_value.initialize();
    (void)destination_value.initialize(static_cast<ft_cubic_spline &&>(source_value));
    return ;
}

static void spline_call_initialize_copy_from_destroyed_source()
{
    ft_vector<double> x_values;
    ft_vector<double> y_values;
    ft_cubic_spline source_value;
    ft_cubic_spline destination_value;

    spline_fill_sample_points(x_values, y_values);
    (void)source_value.initialize(ft_cubic_spline_build(x_values, y_values));
    (void)source_value.destroy();
    (void)destination_value.initialize(source_value);
    return ;
}

static void spline_call_initialize_move_from_destroyed_source()
{
    ft_vector<double> x_values;
    ft_vector<double> y_values;
    ft_cubic_spline source_value;
    ft_cubic_spline destination_value;

    spline_fill_sample_points(x_values, y_values);
    (void)source_value.initialize(ft_cubic_spline_build(x_values, y_values));
    (void)source_value.destroy();
    (void)destination_value.initialize(static_cast<ft_cubic_spline &&>(source_value));
    return ;
}

static void spline_call_move_self_uninitialized()
{
    ft_cubic_spline spline_value;

    (void)spline_value.move(spline_value);
    return ;
}

static void spline_call_enable_thread_safety_uninitialized()
{
    ft_cubic_spline spline_value;

    (void)spline_value.enable_thread_safety();
    return ;
}

static void spline_call_disable_thread_safety_uninitialized()
{
    ft_cubic_spline spline_value;

    spline_value.disable_thread_safety();
    return ;
}

static void spline_call_is_thread_safe_enabled_uninitialized()
{
    ft_cubic_spline spline_value;

    (void)spline_value.is_thread_safe_enabled();
    return ;
}

static void spline_call_destroy_after_destroy()
{
    ft_cubic_spline spline_value;

    (void)spline_value.initialize();
    (void)spline_value.destroy();
    (void)spline_value.destroy();
    return ;
}

static void spline_call_initialize_copy_on_initialized_destination()
{
    ft_vector<double> x_values;
    ft_vector<double> y_values;
    ft_cubic_spline source_value;
    ft_cubic_spline destination_value;

    spline_fill_sample_points(x_values, y_values);
    (void)source_value.initialize(ft_cubic_spline_build(x_values, y_values));
    (void)destination_value.initialize();
    (void)destination_value.initialize(source_value);
    return ;
}

static void spline_call_destructor_uninitialized_raw_memory()
{
    alignas(ft_cubic_spline) unsigned char storage[sizeof(ft_cubic_spline)];
    ft_cubic_spline *spline_pointer;

    std::memset(storage, 0, sizeof(storage));
    spline_pointer = reinterpret_cast<ft_cubic_spline *>(storage);
    spline_pointer->~ft_cubic_spline();
    return ;
}

static void spline_fill_many_sample_points(ft_vector<double> &x_values,
    ft_vector<double> &y_values)
{
    x_values.push_back(0.0);
    x_values.push_back(1.0);
    x_values.push_back(2.0);
    x_values.push_back(3.0);
    x_values.push_back(4.0);
    x_values.push_back(5.0);
    x_values.push_back(6.0);
    x_values.push_back(7.0);
    y_values.push_back(0.0);
    y_values.push_back(1.0);
    y_values.push_back(8.0);
    y_values.push_back(27.0);
    y_values.push_back(64.0);
    y_values.push_back(125.0);
    y_values.push_back(216.0);
    y_values.push_back(343.0);
    return ;
}

FT_TEST(test_cubic_spline_initialize_twice_aborts,
    "ft_cubic_spline initialize aborts on already initialized object")
{
    FT_ASSERT_EQ(1, spline_expect_sigabrt(spline_call_initialize_twice));
    return (1);
}

FT_TEST(test_cubic_spline_destroy_uninitialized_aborts,
    "ft_cubic_spline destroy aborts when called on uninitialized object")
{
    FT_ASSERT_EQ(1, spline_expect_sigabrt(spline_call_destroy_uninitialized));
    return (1);
}

FT_TEST(test_cubic_spline_copy_from_uninitialized_source_aborts,
    "ft_cubic_spline initialize(copy) aborts on uninitialized source")
{
    FT_ASSERT_EQ(1, spline_expect_sigabrt(spline_call_copy_from_uninitialized_source));
    return (1);
}

FT_TEST(test_cubic_spline_initialize_move_from_uninitialized_source_aborts,
    "ft_cubic_spline initialize(move) aborts on uninitialized source")
{
    FT_ASSERT_EQ(1, spline_expect_sigabrt(spline_call_initialize_move_from_uninitialized_source));
    return (1);
}

FT_TEST(test_cubic_spline_move_self_uninitialized_aborts,
    "ft_cubic_spline move(self) aborts when object is uninitialized")
{
    FT_ASSERT_EQ(1, spline_expect_sigabrt(spline_call_move_self_uninitialized));
    return (1);
}

FT_TEST(test_cubic_spline_enable_thread_safety_uninitialized_aborts,
    "ft_cubic_spline enable_thread_safety aborts on uninitialized object")
{
    FT_ASSERT_EQ(1, spline_expect_sigabrt(spline_call_enable_thread_safety_uninitialized));
    return (1);
}

FT_TEST(test_cubic_spline_is_thread_safe_uninitialized_aborts,
    "ft_cubic_spline is_thread_safe_enabled aborts on uninitialized object")
{
    FT_ASSERT_EQ(1, spline_expect_sigabrt(spline_call_is_thread_safe_enabled_uninitialized));
    return (1);
}

FT_TEST(test_cubic_spline_disable_thread_safety_uninitialized_aborts,
    "ft_cubic_spline disable_thread_safety aborts on uninitialized object")
{
    FT_ASSERT_EQ(1, spline_expect_sigabrt(spline_call_disable_thread_safety_uninitialized));
    return (1);
}

FT_TEST(test_cubic_spline_destroy_after_destroy_aborts,
    "ft_cubic_spline destroy aborts after object is already destroyed")
{
    FT_ASSERT_EQ(1, spline_expect_sigabrt(spline_call_destroy_after_destroy));
    return (1);
}

FT_TEST(test_cubic_spline_initialize_copy_on_initialized_destination_aborts,
    "ft_cubic_spline initialize(copy) aborts when destination is initialized")
{
    FT_ASSERT_EQ(1, spline_expect_sigabrt(spline_call_initialize_copy_on_initialized_destination));
    return (1);
}

FT_TEST(test_cubic_spline_uninitialized_raw_destructor_aborts,
    "ft_cubic_spline destructor aborts on raw uninitialized memory")
{
    FT_ASSERT_EQ(1, spline_expect_sigabrt(spline_call_destructor_uninitialized_raw_memory));
    return (1);
}

FT_TEST(test_cubic_spline_initialize_destroy_reinitialize_success,
    "ft_cubic_spline can initialize, destroy, and initialize again")
{
    ft_cubic_spline spline_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, spline_value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, spline_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, spline_value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, spline_value.destroy());
    return (1);
}

FT_TEST(test_cubic_spline_initialize_copy_success,
    "ft_cubic_spline initialize(copy) copies spline coefficients")
{
    ft_vector<double> x_values;
    ft_vector<double> y_values;
    ft_cubic_spline source_value;
    ft_cubic_spline destination_value;
    double source_evaluation;
    double destination_evaluation;

    spline_fill_sample_points(x_values, y_values);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.initialize(ft_cubic_spline_build(x_values, y_values)));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.initialize(source_value));
    source_evaluation = ft_cubic_spline_evaluate(source_value, 1.5);
    destination_evaluation = ft_cubic_spline_evaluate(destination_value, 1.5);
    FT_ASSERT(math_fabs(source_evaluation - destination_evaluation) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.destroy());
    return (1);
}

FT_TEST(test_cubic_spline_move_constructor_success,
    "ft_cubic_spline move constructor accepts build result")
{
    ft_vector<double> x_values;
    ft_vector<double> y_values;
    double evaluated_value;

    spline_fill_sample_points(x_values, y_values);
    ft_cubic_spline spline_value(ft_cubic_spline_build(x_values, y_values));
    evaluated_value = ft_cubic_spline_evaluate(spline_value, 1.5);
    FT_ASSERT(math_fabs(evaluated_value - 3.375) < 0.05);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, spline_value.destroy());
    return (1);
}

FT_TEST(test_cubic_spline_initialize_move_success,
    "ft_cubic_spline initialize(move) transfers spline data")
{
    ft_vector<double> x_values;
    ft_vector<double> y_values;
    ft_cubic_spline source_value;
    ft_cubic_spline destination_value;
    double destination_evaluation;
    double source_evaluation_after_move;

    spline_fill_sample_points(x_values, y_values);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.initialize(ft_cubic_spline_build(x_values, y_values)));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.initialize(static_cast<ft_cubic_spline &&>(source_value)));
    destination_evaluation = ft_cubic_spline_evaluate(destination_value, 1.5);
    source_evaluation_after_move = ft_cubic_spline_evaluate(source_value, 1.5);
    FT_ASSERT(math_fabs(destination_evaluation - 3.375) < 0.05);
    FT_ASSERT(math_fabs(source_evaluation_after_move) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.destroy());
    return (1);
}

FT_TEST(test_cubic_spline_move_method_success,
    "ft_cubic_spline move transfers data and clears source spline")
{
    ft_vector<double> x_values;
    ft_vector<double> y_values;
    ft_cubic_spline source_value;
    ft_cubic_spline destination_value;
    double destination_evaluation;
    double source_evaluation_after_move;

    spline_fill_sample_points(x_values, y_values);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.initialize(ft_cubic_spline_build(x_values, y_values)));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.move(source_value));
    destination_evaluation = ft_cubic_spline_evaluate(destination_value, 1.5);
    source_evaluation_after_move = ft_cubic_spline_evaluate(source_value, 1.5);
    FT_ASSERT(math_fabs(destination_evaluation - 3.375) < 0.05);
    FT_ASSERT(math_fabs(source_evaluation_after_move) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.destroy());
    return (1);
}

FT_TEST(test_cubic_spline_thread_safety_and_validation_mutex,
    "ft_cubic_spline thread safety toggle and mutex validation work")
{
    ft_cubic_spline spline_value;
    pt_recursive_mutex *mutex_pointer;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, spline_value.initialize());
    FT_ASSERT_EQ(false, spline_value.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, spline_value.enable_thread_safety());
    FT_ASSERT_EQ(true, spline_value.is_thread_safe_enabled());
    mutex_pointer = spline_value.get_mutex_for_validation();
    FT_ASSERT(mutex_pointer != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, mutex_pointer->lock());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, mutex_pointer->unlock());
    spline_value.disable_thread_safety();
    FT_ASSERT_EQ(false, spline_value.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, spline_value.destroy());
    return (1);
}

FT_TEST(test_cubic_spline_get_mutex_for_validation_uninitialized_returns_null,
    "ft_cubic_spline validation mutex getter returns null before initialize")
{
    ft_cubic_spline spline_value;

    FT_ASSERT(ft_nullptr == spline_value.get_mutex_for_validation());
    return (1);
}

FT_TEST(test_cubic_spline_get_mutex_for_validation_lazily_creates_mutex,
    "ft_cubic_spline validation mutex getter lazily prepares recursive mutex")
{
    ft_cubic_spline spline_value;
    pt_recursive_mutex *mutex_pointer;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, spline_value.initialize());
    FT_ASSERT_EQ(false, spline_value.is_thread_safe_enabled());
    mutex_pointer = spline_value.get_mutex_for_validation();
    FT_ASSERT(mutex_pointer != ft_nullptr);
    FT_ASSERT_EQ(true, spline_value.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, mutex_pointer->lock());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, mutex_pointer->unlock());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, spline_value.destroy());
    return (1);
}

FT_TEST(test_cubic_spline_disable_thread_safety_without_enable_is_safe,
    "ft_cubic_spline disable_thread_safety is safe when mutex is not enabled")
{
    ft_cubic_spline spline_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, spline_value.initialize());
    spline_value.disable_thread_safety();
    FT_ASSERT_EQ(false, spline_value.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, spline_value.destroy());
    return (1);
}

FT_TEST(test_cubic_spline_move_self_initialized_success,
    "ft_cubic_spline move(self) succeeds when initialized")
{
    ft_vector<double> x_values;
    ft_vector<double> y_values;
    ft_cubic_spline spline_value;
    double evaluated_before;
    double evaluated_after;

    spline_fill_sample_points(x_values, y_values);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, spline_value.initialize(ft_cubic_spline_build(x_values, y_values)));
    evaluated_before = ft_cubic_spline_evaluate(spline_value, 1.5);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, spline_value.move(spline_value));
    evaluated_after = ft_cubic_spline_evaluate(spline_value, 1.5);
    FT_ASSERT(math_fabs(evaluated_before - evaluated_after) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, spline_value.destroy());
    return (1);
}

FT_TEST(test_cubic_spline_initialize_copy_from_destroyed_source_aborts,
    "ft_cubic_spline initialize(copy) aborts for destroyed source")
{
    FT_ASSERT_EQ(1, spline_expect_sigabrt(spline_call_initialize_copy_from_destroyed_source));
    return (1);
}

FT_TEST(test_cubic_spline_initialize_move_from_destroyed_source_aborts,
    "ft_cubic_spline initialize(move) aborts for destroyed source")
{
    FT_ASSERT_EQ(1, spline_expect_sigabrt(spline_call_initialize_move_from_destroyed_source));
    return (1);
}

FT_TEST(test_cubic_spline_initialize_copy_on_destroyed_destination_success,
    "ft_cubic_spline initialize(copy) succeeds when destination is destroyed")
{
    ft_vector<double> x_values;
    ft_vector<double> y_values;
    ft_cubic_spline source_value;
    ft_cubic_spline destination_value;

    spline_fill_sample_points(x_values, y_values);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.initialize(ft_cubic_spline_build(x_values, y_values)));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.initialize(source_value));
    FT_ASSERT(math_fabs(ft_cubic_spline_evaluate(destination_value, 1.5) - 3.375) < 0.05);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.destroy());
    return (1);
}

FT_TEST(test_cubic_spline_initialize_move_on_destroyed_destination_success,
    "ft_cubic_spline initialize(move) succeeds when destination is destroyed")
{
    ft_vector<double> x_values;
    ft_vector<double> y_values;
    ft_cubic_spline source_value;
    ft_cubic_spline destination_value;

    spline_fill_sample_points(x_values, y_values);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.initialize(ft_cubic_spline_build(x_values, y_values)));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS,
        destination_value.initialize(static_cast<ft_cubic_spline &&>(source_value)));
    FT_ASSERT(math_fabs(ft_cubic_spline_evaluate(destination_value, 1.5) - 3.375) < 0.05);
    FT_ASSERT(math_fabs(ft_cubic_spline_evaluate(source_value, 1.5)) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.destroy());
    return (1);
}

FT_TEST(test_cubic_spline_enable_thread_safety_allocation_failure,
    "ft_cubic_spline enable_thread_safety reports no memory under CMA limit")
{
    ft_cubic_spline spline_value;
    int enable_error;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, spline_value.initialize());
    cma_set_alloc_limit(1);
    enable_error = spline_value.enable_thread_safety();
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, enable_error);
    FT_ASSERT_EQ(false, spline_value.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, spline_value.destroy());
    return (1);
}

FT_TEST(test_cubic_spline_enable_thread_safety_failure_then_recover,
    "ft_cubic_spline enable_thread_safety recovers after allocation limit reset")
{
    ft_cubic_spline spline_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, spline_value.initialize());
    cma_set_alloc_limit(1);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, spline_value.enable_thread_safety());
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, spline_value.enable_thread_safety());
    FT_ASSERT_EQ(true, spline_value.is_thread_safe_enabled());
    spline_value.disable_thread_safety();
    FT_ASSERT_EQ(false, spline_value.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, spline_value.destroy());
    return (1);
}

FT_TEST(test_cubic_spline_enable_thread_safety_idempotent_success,
    "ft_cubic_spline enable_thread_safety succeeds when called twice")
{
    ft_cubic_spline spline_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, spline_value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, spline_value.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, spline_value.enable_thread_safety());
    FT_ASSERT_EQ(true, spline_value.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, spline_value.destroy());
    return (1);
}

FT_TEST(test_cubic_spline_initialize_copy_allocation_failure,
    "ft_cubic_spline initialize(copy) returns no memory when allocation fails")
{
    ft_vector<double> x_values;
    ft_vector<double> y_values;
    ft_cubic_spline source_value;
    ft_cubic_spline destination_value;
    int initialize_error;

    spline_fill_many_sample_points(x_values, y_values);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.initialize(ft_cubic_spline_build(x_values, y_values)));
    cma_set_alloc_limit(1);
    initialize_error = destination_value.initialize(source_value);
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, initialize_error);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.destroy());
    return (1);
}

FT_TEST(test_cubic_spline_initialize_move_allocation_failure,
    "ft_cubic_spline initialize(move) returns no memory when allocation fails")
{
    ft_vector<double> x_values;
    ft_vector<double> y_values;
    ft_cubic_spline source_value;
    ft_cubic_spline destination_value;
    int initialize_error;

    spline_fill_many_sample_points(x_values, y_values);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.initialize(ft_cubic_spline_build(x_values, y_values)));
    cma_set_alloc_limit(1);
    initialize_error = destination_value.initialize(static_cast<ft_cubic_spline &&>(source_value));
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, initialize_error);
    FT_ASSERT(math_fabs(ft_cubic_spline_evaluate(source_value, 1.5) - 3.375) < 0.1);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.destroy());
    return (1);
}

FT_TEST(test_cubic_spline_initialize_copy_alloc_failure_then_recover,
    "ft_cubic_spline initialize(copy) recovers after allocation limit reset")
{
    ft_vector<double> x_values;
    ft_vector<double> y_values;
    ft_cubic_spline source_value;
    ft_cubic_spline destination_value;

    spline_fill_many_sample_points(x_values, y_values);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.initialize(ft_cubic_spline_build(x_values, y_values)));
    cma_set_alloc_limit(1);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, destination_value.initialize(source_value));
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.initialize(source_value));
    FT_ASSERT(math_fabs(ft_cubic_spline_evaluate(destination_value, 1.5) - 3.375) < 0.1);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.destroy());
    return (1);
}

FT_TEST(test_cubic_spline_move_allocation_failure_keeps_source,
    "ft_cubic_spline move failure keeps source data unchanged")
{
    ft_vector<double> x_values;
    ft_vector<double> y_values;
    ft_cubic_spline source_value;
    ft_cubic_spline destination_value;
    int move_error;

    spline_fill_many_sample_points(x_values, y_values);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.initialize(ft_cubic_spline_build(x_values, y_values)));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.initialize());
    cma_set_alloc_limit(1);
    move_error = destination_value.move(source_value);
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, move_error);
    FT_ASSERT(math_fabs(ft_cubic_spline_evaluate(source_value, 1.5) - 3.375) < 0.1);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.destroy());
    return (1);
}

FT_TEST(test_cubic_spline_copy_failure_no_memory_leak_after_scope,
    "ft_cubic_spline copy allocation failure leaves no residual allocations")
{
    ft_size_t baseline_bytes;
    ft_size_t bytes_after_scope;

    baseline_bytes = spline_current_allocated_bytes();
    {
        ft_vector<double> x_values;
        ft_vector<double> y_values;
        ft_cubic_spline source_value;
        ft_cubic_spline destination_value;

        spline_fill_many_sample_points(x_values, y_values);
        FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.initialize(ft_cubic_spline_build(x_values, y_values)));
        cma_set_alloc_limit(1);
        FT_ASSERT_EQ(FT_ERR_NO_MEMORY, destination_value.initialize(source_value));
        cma_set_alloc_limit(0);
        FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.destroy());
    }
    bytes_after_scope = spline_current_allocated_bytes();
    FT_ASSERT_EQ(baseline_bytes, bytes_after_scope);
    return (1);
}

FT_TEST(test_cubic_spline_build_invalid_mismatched_sizes_returns_empty_spline,
    "ft_cubic_spline_build with mismatched vectors returns empty spline result")
{
    ft_vector<double> x_values;
    ft_vector<double> y_values;
    ft_cubic_spline spline_value;

    x_values.push_back(0.0);
    x_values.push_back(1.0);
    y_values.push_back(0.0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, spline_value.initialize(ft_cubic_spline_build(x_values, y_values)));
    FT_ASSERT(math_fabs(ft_cubic_spline_evaluate(spline_value, 0.5)) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, spline_value.destroy());
    return (1);
}

FT_TEST(test_cubic_spline_build_invalid_non_increasing_x_returns_empty_spline,
    "ft_cubic_spline_build with non-increasing x values returns empty spline result")
{
    ft_vector<double> x_values;
    ft_vector<double> y_values;
    ft_cubic_spline spline_value;

    x_values.push_back(0.0);
    x_values.push_back(1.0);
    x_values.push_back(1.0);
    y_values.push_back(0.0);
    y_values.push_back(1.0);
    y_values.push_back(1.0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, spline_value.initialize(ft_cubic_spline_build(x_values, y_values)));
    FT_ASSERT(math_fabs(ft_cubic_spline_evaluate(spline_value, 0.5)) < 0.000001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, spline_value.destroy());
    return (1);
}
