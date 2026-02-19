#include "../test_internal.hpp"
#include "../../Template/circular_buffer.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_ft_circular_buffer_thread_safe_ops, "ft_circular_buffer guards pushes/pops with mutex")
{
    ft_circular_buffer<int> buffer_instance(3);
    bool lock_acquired = false;

    FT_ASSERT_EQ(0, buffer_instance.enable_thread_safety());
    FT_ASSERT(buffer_instance.is_thread_safe());
    FT_ASSERT_EQ(0, buffer_instance.lock(&lock_acquired));
    FT_ASSERT(lock_acquired);
    buffer_instance.push(1);
    FT_ASSERT_EQ(1, buffer_instance.pop());
    buffer_instance.unlock(lock_acquired);
    FT_ASSERT_EQ(false, buffer_instance.is_full());
    FT_ASSERT_EQ(false, buffer_instance.is_empty());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_circular_buffer<int>::last_operation_error());
    buffer_instance.disable_thread_safety();
    FT_ASSERT_EQ(false, buffer_instance.is_thread_safe());
    return (1);
}

FT_TEST(test_ft_circular_buffer_dynamic_reset_after_errors,
        "ft_circular_buffer clears errors after manual reset")
{
    ft_circular_buffer<int> buffer_instance(2);

    buffer_instance.push(1);
    buffer_instance.push(2);
    buffer_instance.push(3); // triggers full error
    FT_ASSERT_EQ(FT_ERR_FULL, ft_circular_buffer<int>::last_operation_error());
    buffer_instance.clear();
    FT_ASSERT_EQ(true, buffer_instance.is_empty());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_circular_buffer<int>::last_operation_error());
    buffer_instance.pop(); // empty underflow
    FT_ASSERT_EQ(FT_ERR_EMPTY, ft_circular_buffer<int>::last_operation_error());
    buffer_instance.push(4);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_circular_buffer<int>::last_operation_error());
    return (1);
}
