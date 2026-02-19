#include "../test_internal.hpp"
#include "../../Template/circular_buffer.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_ft_circular_buffer_push_pop_cycle, "ft_circular_buffer preserves order across wrap around")
{
    ft_circular_buffer<int> buffer_instance(3);

    FT_ASSERT_EQ(false, buffer_instance.is_full());
    FT_ASSERT_EQ(true, buffer_instance.is_empty());
    FT_ASSERT_EQ(0UL, buffer_instance.size());
    FT_ASSERT_EQ(3UL, buffer_instance.capacity());

    buffer_instance.push(1);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_circular_buffer<int>::last_operation_error());
    buffer_instance.push(2);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_circular_buffer<int>::last_operation_error());
    buffer_instance.push(3);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_circular_buffer<int>::last_operation_error());

    FT_ASSERT(buffer_instance.is_full());
    FT_ASSERT_EQ(false, buffer_instance.is_empty());

    int first_value = buffer_instance.pop();
    FT_ASSERT_EQ(1, first_value);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_circular_buffer<int>::last_operation_error());

    buffer_instance.push(4);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_circular_buffer<int>::last_operation_error());

    int second_value = buffer_instance.pop();
    FT_ASSERT_EQ(2, second_value);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_circular_buffer<int>::last_operation_error());

    int third_value = buffer_instance.pop();
    FT_ASSERT_EQ(3, third_value);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_circular_buffer<int>::last_operation_error());

    int fourth_value = buffer_instance.pop();
    FT_ASSERT_EQ(4, fourth_value);
    FT_ASSERT(buffer_instance.is_empty());
    FT_ASSERT_EQ(0UL, buffer_instance.size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_circular_buffer<int>::last_operation_error());
    return (1);
}

FT_TEST(test_ft_circular_buffer_error_states, "ft_circular_buffer surfaces full and empty error codes")
{
    ft_circular_buffer<int> buffer_instance(2);

    buffer_instance.push(10);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_circular_buffer<int>::last_operation_error());
    buffer_instance.push(20);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_circular_buffer<int>::last_operation_error());

    buffer_instance.push(30);
    FT_ASSERT_EQ(FT_ERR_FULL, ft_circular_buffer<int>::last_operation_error());

    buffer_instance.clear();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_circular_buffer<int>::last_operation_error());
    FT_ASSERT(buffer_instance.is_empty());

    int empty_value = buffer_instance.pop();
    FT_ASSERT_EQ(0, empty_value);
    FT_ASSERT_EQ(FT_ERR_EMPTY, ft_circular_buffer<int>::last_operation_error());

    buffer_instance.push(50);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_circular_buffer<int>::last_operation_error());
    buffer_instance.push(60);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_circular_buffer<int>::last_operation_error());

    buffer_instance.pop();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_circular_buffer<int>::last_operation_error());
    buffer_instance.pop();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_circular_buffer<int>::last_operation_error());
    return (1);
}

FT_TEST(test_ft_circular_buffer_wraparound_preserves_order,
        "ft_circular_buffer pop returns values in FIFO order across wrap")
{
    ft_circular_buffer<int> buffer_instance(2);
    buffer_instance.push(9);
    buffer_instance.push(8);
    FT_ASSERT_EQ(9, buffer_instance.pop());
    buffer_instance.push(7);
    FT_ASSERT_EQ(8, buffer_instance.pop());
    FT_ASSERT_EQ(7, buffer_instance.pop());
    FT_ASSERT(buffer_instance.is_empty());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_circular_buffer<int>::last_operation_error());
    return (1);
}
