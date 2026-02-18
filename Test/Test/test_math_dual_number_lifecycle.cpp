#include "../test_internal.hpp"
#include "../../Math/math_autodiff.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../CMA/CMA.hpp"
#include "../../System_utils/test_runner.hpp"
#include <cmath>
#include <csignal>
#include <cstring>
#include <sys/wait.h>
#include <unistd.h>

#ifndef LIBFT_TEST_BUILD
#endif

static int dual_number_expect_sigabrt(void (*operation)())
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

static bool dual_number_almost_equal(double left_value, double right_value,
    double tolerance)
{
    return (std::fabs(left_value - right_value) <= tolerance);
}

static ft_size_t dual_number_current_allocated_bytes(void)
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

static int dual_number_expect_recursive_mutex_usable(pt_recursive_mutex *mutex_pointer)
{
    int lock_error;
    int unlock_error;

    FT_ASSERT(mutex_pointer != ft_nullptr);
    FT_ASSERT_EQ(false, mutex_pointer->lockState());
    lock_error = mutex_pointer->lock();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, lock_error);
    lock_error = mutex_pointer->lock();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, lock_error);
    FT_ASSERT_EQ(true, mutex_pointer->lockState());
    unlock_error = mutex_pointer->unlock();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, unlock_error);
    FT_ASSERT_EQ(true, mutex_pointer->lockState());
    unlock_error = mutex_pointer->unlock();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, unlock_error);
    FT_ASSERT_EQ(false, mutex_pointer->lockState());
    return (1);
}

static void dual_number_call_initialize_twice(void)
{
    ft_dual_number dual_number_value;

    (void)dual_number_value.initialize();
    (void)dual_number_value.initialize();
    return ;
}

static void dual_number_call_destroy_uninitialized(void)
{
    ft_dual_number dual_number_value;

    (void)dual_number_value.destroy();
    return ;
}

static void dual_number_call_enable_thread_safety_uninitialized(void)
{
    ft_dual_number dual_number_value;

    (void)dual_number_value.enable_thread_safety();
    return ;
}

static void dual_number_call_disable_thread_safety_uninitialized(void)
{
    ft_dual_number dual_number_value;

    dual_number_value.disable_thread_safety();
    return ;
}

static void dual_number_call_is_thread_safe_enabled_uninitialized(void)
{
    ft_dual_number dual_number_value;

    (void)dual_number_value.is_thread_safe_enabled();
    return ;
}

static void dual_number_call_value_uninitialized(void)
{
    ft_dual_number dual_number_value;

    (void)dual_number_value.value();
    return ;
}

static void dual_number_call_derivative_uninitialized(void)
{
    ft_dual_number dual_number_value;

    (void)dual_number_value.derivative();
    return ;
}

static void dual_number_call_move_uninitialized(void)
{
    ft_dual_number dual_number_value;

    (void)dual_number_value.move(dual_number_value);
    return ;
}

static void dual_number_call_copy_from_uninitialized_source(void)
{
    ft_dual_number destination_value;
    ft_dual_number source_value;

    (void)destination_value.initialize();
    (void)destination_value.initialize(source_value);
    return ;
}

static void dual_number_call_move_from_uninitialized_source(void)
{
    ft_dual_number destination_value;
    ft_dual_number source_value;

    (void)destination_value.initialize();
    (void)destination_value.initialize(static_cast<ft_dual_number &&>(source_value));
    return ;
}

static void dual_number_call_copy_from_destroyed_source(void)
{
    ft_dual_number destination_value;
    ft_dual_number source_value;

    (void)destination_value.initialize();
    (void)source_value.initialize(5.0, 6.0);
    (void)source_value.destroy();
    (void)destination_value.initialize(source_value);
    return ;
}

static void dual_number_call_move_from_destroyed_source(void)
{
    ft_dual_number destination_value;
    ft_dual_number source_value;

    (void)destination_value.initialize();
    (void)source_value.initialize(5.0, 6.0);
    (void)source_value.destroy();
    (void)destination_value.initialize(static_cast<ft_dual_number &&>(source_value));
    return ;
}

static void dual_number_call_move_assignment_from_uninitialized_source(void)
{
    ft_dual_number destination_value;
    alignas(ft_dual_number) unsigned char storage[sizeof(ft_dual_number)];
    ft_dual_number *source_value;

    (void)destination_value.initialize(3.0, 4.0);
    std::memset(storage, 0, sizeof(storage));
    source_value = reinterpret_cast<ft_dual_number *>(storage);
    destination_value = static_cast<ft_dual_number &&>(*source_value);
    return ;
}

static void dual_number_call_copy_assignment_from_uninitialized_source(void)
{
    ft_dual_number destination_value;
    alignas(ft_dual_number) unsigned char storage[sizeof(ft_dual_number)];
    ft_dual_number *source_value;

    (void)destination_value.initialize(3.0, 4.0);
    std::memset(storage, 0, sizeof(storage));
    source_value = reinterpret_cast<ft_dual_number *>(storage);
    destination_value = *source_value;
    return ;
}

static void dual_number_call_copy_assignment_from_destroyed_source(void)
{
    ft_dual_number destination_value;
    ft_dual_number source_value;

    (void)destination_value.initialize(3.0, 4.0);
    (void)source_value.initialize(5.0, 6.0);
    (void)source_value.destroy();
    destination_value = source_value;
    return ;
}

static void dual_number_call_move_assignment_from_destroyed_source(void)
{
    ft_dual_number destination_value;
    ft_dual_number source_value;

    (void)destination_value.initialize(3.0, 4.0);
    (void)source_value.initialize(5.0, 6.0);
    (void)source_value.destroy();
    destination_value = static_cast<ft_dual_number &&>(source_value);
    return ;
}

static void dual_number_call_destructor_uninitialized_raw_memory(void)
{
    alignas(ft_dual_number) unsigned char storage[sizeof(ft_dual_number)];
    ft_dual_number *dual_number_pointer;

    std::memset(storage, 0, sizeof(storage));
    dual_number_pointer = reinterpret_cast<ft_dual_number *>(storage);
    dual_number_pointer->~ft_dual_number();
    return ;
}

static ft_dual_number dual_number_univariate_sample(const ft_dual_number &input,
    void *user_data)
{
    ft_dual_number three_value;
    ft_dual_number result;

    (void)user_data;
    three_value = ft_dual_number::constant(3.0);
    result = (input * input) + (three_value * input);
    return (result);
}

static ft_dual_number dual_number_multivariate_sample(
    const ft_vector<ft_dual_number> &input, void *user_data)
{
    ft_dual_number result;

    (void)user_data;
    if (input.size() < 2)
        return (ft_dual_number::constant(0.0));
    result = (input[0] * input[0]) + (input[0] * input[1]) + input[1];
    return (result);
}

FT_TEST(test_dual_number_initialize_twice_aborts,
    "ft_dual_number initialize aborts on already initialized object")
{
    FT_ASSERT_EQ(1, dual_number_expect_sigabrt(dual_number_call_initialize_twice));
    return (1);
}

FT_TEST(test_dual_number_destroy_uninitialized_aborts,
    "ft_dual_number destroy aborts on uninitialized object")
{
    FT_ASSERT_EQ(1, dual_number_expect_sigabrt(dual_number_call_destroy_uninitialized));
    return (1);
}

FT_TEST(test_dual_number_enable_thread_safety_uninitialized_aborts,
    "ft_dual_number enable_thread_safety aborts on uninitialized object")
{
    FT_ASSERT_EQ(1,
        dual_number_expect_sigabrt(dual_number_call_enable_thread_safety_uninitialized));
    return (1);
}

FT_TEST(test_dual_number_disable_thread_safety_uninitialized_aborts,
    "ft_dual_number disable_thread_safety aborts on uninitialized object")
{
    FT_ASSERT_EQ(1,
        dual_number_expect_sigabrt(dual_number_call_disable_thread_safety_uninitialized));
    return (1);
}

FT_TEST(test_dual_number_is_thread_safe_uninitialized_aborts,
    "ft_dual_number is_thread_safe_enabled aborts on uninitialized object")
{
    FT_ASSERT_EQ(1,
        dual_number_expect_sigabrt(dual_number_call_is_thread_safe_enabled_uninitialized));
    return (1);
}

FT_TEST(test_dual_number_value_uninitialized_aborts,
    "ft_dual_number value aborts on uninitialized object")
{
    FT_ASSERT_EQ(1, dual_number_expect_sigabrt(dual_number_call_value_uninitialized));
    return (1);
}

FT_TEST(test_dual_number_derivative_uninitialized_aborts,
    "ft_dual_number derivative aborts on uninitialized object")
{
    FT_ASSERT_EQ(1, dual_number_expect_sigabrt(dual_number_call_derivative_uninitialized));
    return (1);
}

FT_TEST(test_dual_number_move_uninitialized_aborts,
    "ft_dual_number move aborts on uninitialized object")
{
    FT_ASSERT_EQ(1, dual_number_expect_sigabrt(dual_number_call_move_uninitialized));
    return (1);
}

FT_TEST(test_dual_number_initialize_copy_uninitialized_source_aborts,
    "ft_dual_number initialize(copy) aborts with uninitialized source")
{
    FT_ASSERT_EQ(1, dual_number_expect_sigabrt(dual_number_call_copy_from_uninitialized_source));
    return (1);
}

FT_TEST(test_dual_number_initialize_move_uninitialized_source_aborts,
    "ft_dual_number initialize(move) aborts with uninitialized source")
{
    FT_ASSERT_EQ(1, dual_number_expect_sigabrt(dual_number_call_move_from_uninitialized_source));
    return (1);
}

FT_TEST(test_dual_number_initialize_copy_destroyed_source_aborts,
    "ft_dual_number initialize(copy) aborts with destroyed source")
{
    FT_ASSERT_EQ(1, dual_number_expect_sigabrt(dual_number_call_copy_from_destroyed_source));
    return (1);
}

FT_TEST(test_dual_number_initialize_move_destroyed_source_aborts,
    "ft_dual_number initialize(move) aborts with destroyed source")
{
    FT_ASSERT_EQ(1, dual_number_expect_sigabrt(dual_number_call_move_from_destroyed_source));
    return (1);
}

FT_TEST(test_dual_number_copy_assignment_uninitialized_source_aborts,
    "ft_dual_number copy assignment aborts with uninitialized source")
{
    FT_ASSERT_EQ(1,
        dual_number_expect_sigabrt(dual_number_call_copy_assignment_from_uninitialized_source));
    return (1);
}

FT_TEST(test_dual_number_move_assignment_uninitialized_source_aborts,
    "ft_dual_number move assignment aborts with uninitialized source")
{
    FT_ASSERT_EQ(1,
        dual_number_expect_sigabrt(dual_number_call_move_assignment_from_uninitialized_source));
    return (1);
}

FT_TEST(test_dual_number_copy_assignment_destroyed_source_aborts,
    "ft_dual_number copy assignment aborts with destroyed source")
{
    FT_ASSERT_EQ(1,
        dual_number_expect_sigabrt(dual_number_call_copy_assignment_from_destroyed_source));
    return (1);
}

FT_TEST(test_dual_number_move_assignment_destroyed_source_aborts,
    "ft_dual_number move assignment aborts with destroyed source")
{
    FT_ASSERT_EQ(1,
        dual_number_expect_sigabrt(dual_number_call_move_assignment_from_destroyed_source));
    return (1);
}

FT_TEST(test_dual_number_uninitialized_raw_destructor_aborts,
    "ft_dual_number destructor aborts on raw uninitialized memory")
{
    FT_ASSERT_EQ(1, dual_number_expect_sigabrt(dual_number_call_destructor_uninitialized_raw_memory));
    return (1);
}

FT_TEST(test_dual_number_initialize_destroy_reinitialize_success,
    "ft_dual_number can initialize destroy and initialize again")
{
    ft_dual_number dual_number_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, dual_number_value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, dual_number_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, dual_number_value.initialize(-1.0, 7.0));
    FT_ASSERT_EQ(-1.0, dual_number_value.value());
    FT_ASSERT_EQ(7.0, dual_number_value.derivative());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, dual_number_value.destroy());
    return (1);
}

FT_TEST(test_dual_number_constant_and_variable_helpers,
    "ft_dual_number constant and variable set derivative correctly")
{
    ft_dual_number constant_value;
    ft_dual_number variable_value;

    constant_value = ft_dual_number::constant(8.0);
    variable_value = ft_dual_number::variable(8.0);
    FT_ASSERT_EQ(8.0, constant_value.value());
    FT_ASSERT_EQ(0.0, constant_value.derivative());
    FT_ASSERT_EQ(8.0, variable_value.value());
    FT_ASSERT_EQ(1.0, variable_value.derivative());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, constant_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, variable_value.destroy());
    return (1);
}

FT_TEST(test_dual_number_initialize_copy_success,
    "ft_dual_number initialize(copy) copies value derivative and error state")
{
    ft_dual_number source_value;
    ft_dual_number destination_value;
    ft_dual_number zero_value;
    ft_dual_number division_result;

    zero_value = ft_dual_number::constant(0.0);
    source_value = ft_dual_number::variable(3.0);
    division_result = source_value / zero_value;
    FT_ASSERT_EQ(FT_ERR_DIVIDE_BY_ZERO, ft_dual_number::last_operation_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.initialize(source_value));
    FT_ASSERT_EQ(FT_ERR_DIVIDE_BY_ZERO, ft_dual_number::last_operation_error());
    FT_ASSERT_EQ(3.0, destination_value.value());
    FT_ASSERT_EQ(1.0, destination_value.derivative());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_dual_number::last_operation_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, zero_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, division_result.destroy());
    return (1);
}

FT_TEST(test_dual_number_initialize_move_success,
    "ft_dual_number initialize(move) transfers value and clears source")
{
    ft_dual_number source_value;
    ft_dual_number destination_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.initialize(5.0, 2.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS,
        destination_value.initialize(static_cast<ft_dual_number &&>(source_value)));
    FT_ASSERT_EQ(5.0, destination_value.value());
    FT_ASSERT_EQ(2.0, destination_value.derivative());
    FT_ASSERT_EQ(0.0, source_value.value());
    FT_ASSERT_EQ(0.0, source_value.derivative());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.destroy());
    return (1);
}

FT_TEST(test_dual_number_copy_assignment_destroyed_destination_success,
    "ft_dual_number copy assignment works when destination is destroyed")
{
    ft_dual_number source_value;
    ft_dual_number destination_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.initialize(6.0, 9.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.initialize(1.0, 1.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    destination_value = source_value;
    FT_ASSERT_EQ(6.0, destination_value.value());
    FT_ASSERT_EQ(9.0, destination_value.derivative());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_dual_number::last_operation_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.destroy());
    return (1);
}

FT_TEST(test_dual_number_move_assignment_destroyed_destination_success,
    "ft_dual_number move assignment works when destination is destroyed")
{
    ft_dual_number source_value;
    ft_dual_number destination_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.initialize(7.0, 11.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.initialize(2.0, 3.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    destination_value = static_cast<ft_dual_number &&>(source_value);
    FT_ASSERT_EQ(7.0, destination_value.value());
    FT_ASSERT_EQ(11.0, destination_value.derivative());
    FT_ASSERT_EQ(0.0, source_value.value());
    FT_ASSERT_EQ(0.0, source_value.derivative());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_dual_number::last_operation_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.destroy());
    return (1);
}

FT_TEST(test_dual_number_move_method_success_and_self_move,
    "ft_dual_number move method transfers state and self move succeeds")
{
    ft_dual_number source_value;
    ft_dual_number destination_value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.initialize(9.0, 4.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.initialize(0.0, 0.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.move(source_value));
    FT_ASSERT_EQ(9.0, destination_value.value());
    FT_ASSERT_EQ(4.0, destination_value.derivative());
    FT_ASSERT_EQ(0.0, source_value.value());
    FT_ASSERT_EQ(0.0, source_value.derivative());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.move(destination_value));
    FT_ASSERT_EQ(9.0, destination_value.value());
    FT_ASSERT_EQ(4.0, destination_value.derivative());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_value.destroy());
    return (1);
}

FT_TEST(test_dual_number_thread_safety_toggle_and_validation_mutex,
    "ft_dual_number thread safety toggles and validation mutex is usable")
{
    ft_dual_number dual_number_value;
    pt_recursive_mutex *mutex_pointer;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, dual_number_value.initialize(1.0, 1.0));
    FT_ASSERT_EQ(false, dual_number_value.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, dual_number_value.enable_thread_safety());
    FT_ASSERT_EQ(true, dual_number_value.is_thread_safe_enabled());
    mutex_pointer = dual_number_value.get_mutex_for_validation();
    FT_ASSERT_EQ(1, dual_number_expect_recursive_mutex_usable(mutex_pointer));
    dual_number_value.disable_thread_safety();
    FT_ASSERT_EQ(false, dual_number_value.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, dual_number_value.destroy());
    return (1);
}

FT_TEST(test_dual_number_get_mutex_validation_lifecycle,
    "ft_dual_number validation mutex getter is null when not initialized and lazy when initialized")
{
    ft_dual_number dual_number_value;
    pt_recursive_mutex *mutex_pointer;

    FT_ASSERT(ft_nullptr == dual_number_value.get_mutex_for_validation());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, dual_number_value.initialize());
    FT_ASSERT_EQ(false, dual_number_value.is_thread_safe_enabled());
    mutex_pointer = dual_number_value.get_mutex_for_validation();
    FT_ASSERT(mutex_pointer != ft_nullptr);
    FT_ASSERT_EQ(true, dual_number_value.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, dual_number_value.destroy());
    FT_ASSERT(ft_nullptr == dual_number_value.get_mutex_for_validation());
    return (1);
}

FT_TEST(test_dual_number_methods_leave_mutex_unlocked,
    "ft_dual_number methods leave recursive mutex unlocked on return")
{
    ft_dual_number left_value;
    ft_dual_number right_value;
    ft_dual_number result_value;
    pt_recursive_mutex *mutex_pointer;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, left_value.initialize(2.0, 1.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, right_value.initialize(3.0, 4.0));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, left_value.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, right_value.enable_thread_safety());
    result_value = left_value + right_value;
    FT_ASSERT_EQ(5.0, result_value.value());
    FT_ASSERT_EQ(5.0, result_value.derivative());
    (void)left_value.apply_exp();
    (void)right_value.apply_cos();
    mutex_pointer = left_value.get_mutex_for_validation();
    FT_ASSERT_EQ(1, dual_number_expect_recursive_mutex_usable(mutex_pointer));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, result_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, left_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, right_value.destroy());
    return (1);
}

FT_TEST(test_dual_number_arithmetic_and_last_error_behaviour,
    "ft_dual_number arithmetic reports errors and clears stale last error on success")
{
    ft_dual_number left_value;
    ft_dual_number right_value;
    ft_dual_number zero_value;
    ft_dual_number division_error_value;
    ft_dual_number success_value;

    left_value = ft_dual_number::variable(2.0);
    right_value = ft_dual_number::variable(5.0);
    zero_value = ft_dual_number::constant(0.0);
    division_error_value = left_value / zero_value;
    FT_ASSERT_EQ(FT_ERR_DIVIDE_BY_ZERO, ft_dual_number::last_operation_error());
    success_value = left_value + right_value;
    FT_ASSERT_EQ(7.0, success_value.value());
    FT_ASSERT_EQ(2.0, success_value.derivative());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_dual_number::last_operation_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, left_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, right_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, zero_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, division_error_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, success_value.destroy());
    return (1);
}

FT_TEST(test_dual_number_apply_functions,
    "ft_dual_number apply_sin cos exp log compute values and derivatives")
{
    ft_dual_number variable_value;
    ft_dual_number sin_value;
    ft_dual_number cos_value;
    ft_dual_number exp_value;
    ft_dual_number log_value;
    ft_dual_number invalid_log_value;

    variable_value = ft_dual_number::variable(1.0);
    sin_value = variable_value.apply_sin();
    cos_value = variable_value.apply_cos();
    exp_value = variable_value.apply_exp();
    log_value = variable_value.apply_log();
    invalid_log_value = ft_dual_number::constant(0.0).apply_log();
    FT_ASSERT(dual_number_almost_equal(sin_value.value(), std::sin(1.0), 0.000001));
    FT_ASSERT(dual_number_almost_equal(sin_value.derivative(), std::cos(1.0), 0.000001));
    FT_ASSERT(dual_number_almost_equal(cos_value.value(), std::cos(1.0), 0.000001));
    FT_ASSERT(dual_number_almost_equal(cos_value.derivative(), -std::sin(1.0), 0.000001));
    FT_ASSERT(dual_number_almost_equal(exp_value.value(), std::exp(1.0), 0.000001));
    FT_ASSERT(dual_number_almost_equal(exp_value.derivative(), std::exp(1.0), 0.000001));
    FT_ASSERT(dual_number_almost_equal(log_value.value(), 0.0, 0.000001));
    FT_ASSERT(dual_number_almost_equal(log_value.derivative(), 1.0, 0.000001));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_dual_number::last_operation_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, variable_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, sin_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, cos_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, exp_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, log_value.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, invalid_log_value.destroy());
    return (1);
}

FT_TEST(test_dual_number_math_autodiff_univariate_success,
    "math_autodiff_univariate computes value and derivative")
{
    int status;
    double value;
    double derivative;

    value = 0.0;
    derivative = 0.0;
    status = math_autodiff_univariate(dual_number_univariate_sample,
            2.0, &value, &derivative, ft_nullptr);
    FT_ASSERT_EQ(0, status);
    FT_ASSERT(dual_number_almost_equal(value, 10.0, 0.000001));
    FT_ASSERT(dual_number_almost_equal(derivative, 7.0, 0.000001));
    return (1);
}

FT_TEST(test_dual_number_math_autodiff_univariate_invalid_arguments,
    "math_autodiff_univariate rejects null function and output pointers")
{
    int status;
    double value;
    double derivative;

    value = 0.0;
    derivative = 0.0;
    status = math_autodiff_univariate(ft_nullptr, 1.0, &value, &derivative, ft_nullptr);
    FT_ASSERT_EQ(-1, status);
    status = math_autodiff_univariate(dual_number_univariate_sample, 1.0,
            ft_nullptr, &derivative, ft_nullptr);
    FT_ASSERT_EQ(-1, status);
    status = math_autodiff_univariate(dual_number_univariate_sample, 1.0,
            &value, ft_nullptr, ft_nullptr);
    FT_ASSERT_EQ(-1, status);
    return (1);
}

FT_TEST(test_dual_number_math_autodiff_gradient_success,
    "math_autodiff_gradient computes gradient and output value")
{
    ft_vector<double> point_value;
    ft_vector<double> gradient_value;
    int status;
    double value;

    point_value.push_back(2.0);
    point_value.push_back(3.0);
    value = 0.0;
    status = math_autodiff_gradient(dual_number_multivariate_sample,
            point_value, gradient_value, &value, ft_nullptr);
    FT_ASSERT_EQ(0, status);
    FT_ASSERT_EQ(2, gradient_value.size());
    FT_ASSERT(dual_number_almost_equal(value, 13.0, 0.000001));
    FT_ASSERT(dual_number_almost_equal(gradient_value[0], 7.0, 0.000001));
    FT_ASSERT(dual_number_almost_equal(gradient_value[1], 3.0, 0.000001));
    return (1);
}

FT_TEST(test_dual_number_math_autodiff_gradient_null_function,
    "math_autodiff_gradient returns failure for null function")
{
    ft_vector<double> point_value;
    ft_vector<double> gradient_value;
    int status;

    point_value.push_back(1.0);
    status = math_autodiff_gradient(ft_nullptr,
            point_value, gradient_value, ft_nullptr, ft_nullptr);
    FT_ASSERT_EQ(-1, status);
    FT_ASSERT_EQ(0, gradient_value.size());
    return (1);
}

FT_TEST(test_dual_number_enable_thread_safety_allocation_failure_then_recover,
    "ft_dual_number enable_thread_safety reports no memory then recovers")
{
    ft_dual_number dual_number_value;
    int enable_error;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, dual_number_value.initialize(1.0, 1.0));
    cma_set_alloc_limit(1);
    enable_error = dual_number_value.enable_thread_safety();
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, enable_error);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, ft_dual_number::last_operation_error());
    FT_ASSERT_EQ(false, dual_number_value.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, dual_number_value.enable_thread_safety());
    FT_ASSERT_EQ(true, dual_number_value.is_thread_safe_enabled());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, dual_number_value.destroy());
    return (1);
}

FT_TEST(test_dual_number_validation_mutex_allocation_failure_then_recover,
    "ft_dual_number get_mutex_for_validation handles allocation failure and later recovers")
{
    ft_dual_number dual_number_value;
    pt_recursive_mutex *mutex_pointer;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, dual_number_value.initialize(1.0, 0.0));
    cma_set_alloc_limit(1);
    mutex_pointer = dual_number_value.get_mutex_for_validation();
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(ft_nullptr, mutex_pointer);
    FT_ASSERT_EQ(false, dual_number_value.is_thread_safe_enabled());
    mutex_pointer = dual_number_value.get_mutex_for_validation();
    FT_ASSERT(mutex_pointer != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, mutex_pointer->lock());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, mutex_pointer->unlock());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, dual_number_value.destroy());
    return (1);
}

FT_TEST(test_dual_number_math_autodiff_gradient_allocation_failure,
    "math_autodiff_gradient returns failure and clears gradient on allocation failure")
{
    ft_vector<double> point_value;
    ft_vector<double> gradient_value;
    int status;
    double output_value;

    point_value.push_back(1.0);
    point_value.push_back(2.0);
    point_value.push_back(3.0);
    output_value = 0.0;
    cma_set_alloc_limit(1);
    status = math_autodiff_gradient(dual_number_multivariate_sample,
            point_value, gradient_value, &output_value, ft_nullptr);
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(-1, status);
    FT_ASSERT_EQ(0, gradient_value.size());
    return (1);
}

FT_TEST(test_dual_number_math_autodiff_allocation_failure_no_memory_leak,
    "math_autodiff allocation failures do not leak CMA-tracked bytes")
{
    ft_size_t baseline_bytes;
    ft_size_t after_bytes;

    baseline_bytes = dual_number_current_allocated_bytes();
    {
        ft_vector<double> point_value;
        ft_vector<double> gradient_value;
        double output_value;

        point_value.push_back(1.0);
        point_value.push_back(2.0);
        point_value.push_back(3.0);
        output_value = 0.0;
        cma_set_alloc_limit(1);
        FT_ASSERT_EQ(-1, math_autodiff_gradient(dual_number_multivariate_sample,
                point_value, gradient_value, &output_value, ft_nullptr));
        cma_set_alloc_limit(0);
    }
    after_bytes = dual_number_current_allocated_bytes();
    FT_ASSERT_EQ(baseline_bytes, after_bytes);
    return (1);
}
