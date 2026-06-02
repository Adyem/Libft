#include "../test_internal.hpp"
#include "../../Modules/Template/circular_buffer.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_ft_circular_buffer_push_pop_cycle)
{
    ft_circular_buffer<int> buffer_instance(3);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer_instance.initialize());

    FT_ASSERT_EQ(false, buffer_instance.is_full());
    FT_ASSERT_EQ(true, buffer_instance.is_empty());
    FT_ASSERT_EQ(0UL, buffer_instance.size());
    FT_ASSERT_EQ(3UL, buffer_instance.capacity());

    buffer_instance.push(1);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer_instance.get_error());
    buffer_instance.push(2);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer_instance.get_error());
    buffer_instance.push(3);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer_instance.get_error());

    FT_ASSERT(buffer_instance.is_full());
    FT_ASSERT_EQ(false, buffer_instance.is_empty());

    int first_value = buffer_instance.pop();
    FT_ASSERT_EQ(1, first_value);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer_instance.get_error());

    buffer_instance.push(4);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer_instance.get_error());

    int second_value = buffer_instance.pop();
    FT_ASSERT_EQ(2, second_value);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer_instance.get_error());

    int third_value = buffer_instance.pop();
    FT_ASSERT_EQ(3, third_value);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer_instance.get_error());

    int fourth_value = buffer_instance.pop();
    FT_ASSERT_EQ(4, fourth_value);
    FT_ASSERT(buffer_instance.is_empty());
    FT_ASSERT_EQ(0UL, buffer_instance.size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer_instance.get_error());
    return (1);
}

FT_TEST(test_ft_circular_buffer_error_states)
{
    ft_circular_buffer<int> buffer_instance(2);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer_instance.initialize());

    buffer_instance.push(10);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer_instance.get_error());
    buffer_instance.push(20);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer_instance.get_error());

    buffer_instance.push(30);
    FT_ASSERT_EQ(FT_ERR_FULL, buffer_instance.get_error());

    buffer_instance.clear();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer_instance.get_error());
    FT_ASSERT(buffer_instance.is_empty());

    int empty_value = buffer_instance.pop();
    FT_ASSERT_EQ(0, empty_value);
    FT_ASSERT_EQ(FT_ERR_EMPTY, buffer_instance.get_error());

    buffer_instance.push(50);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer_instance.get_error());
    buffer_instance.push(60);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer_instance.get_error());

    buffer_instance.pop();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer_instance.get_error());
    buffer_instance.pop();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer_instance.get_error());
    return (1);
}

FT_TEST(test_ft_circular_buffer_wraparound_preserves_order)
{
    ft_circular_buffer<int> buffer_instance(2);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer_instance.initialize());
    buffer_instance.push(9);
    buffer_instance.push(8);
    FT_ASSERT_EQ(9, buffer_instance.pop());
    buffer_instance.push(7);
    FT_ASSERT_EQ(8, buffer_instance.pop());
    FT_ASSERT_EQ(7, buffer_instance.pop());
    FT_ASSERT(buffer_instance.is_empty());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, buffer_instance.get_error());
    return (1);
}
