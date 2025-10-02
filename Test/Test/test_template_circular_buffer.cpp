#include "../../Template/circular_buffer.hpp"
#include "../../Template/move.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_ft_circular_buffer_push_pop_cycle, "ft_circular_buffer preserves order across wrap around")
{
    ft_circular_buffer<int> buffer_instance(3);

    FT_ASSERT_EQ(false, buffer_instance.is_full());
    FT_ASSERT_EQ(true, buffer_instance.is_empty());
    FT_ASSERT_EQ(0UL, buffer_instance.size());
    FT_ASSERT_EQ(3UL, buffer_instance.capacity());

    buffer_instance.push(1);
    FT_ASSERT_EQ(ER_SUCCESS, buffer_instance.get_error());
    buffer_instance.push(2);
    FT_ASSERT_EQ(ER_SUCCESS, buffer_instance.get_error());
    buffer_instance.push(3);
    FT_ASSERT_EQ(ER_SUCCESS, buffer_instance.get_error());

    FT_ASSERT(buffer_instance.is_full());
    FT_ASSERT_EQ(false, buffer_instance.is_empty());

    int first_value = buffer_instance.pop();
    FT_ASSERT_EQ(1, first_value);
    FT_ASSERT_EQ(ER_SUCCESS, buffer_instance.get_error());

    buffer_instance.push(4);
    FT_ASSERT_EQ(ER_SUCCESS, buffer_instance.get_error());

    int second_value = buffer_instance.pop();
    FT_ASSERT_EQ(2, second_value);
    FT_ASSERT_EQ(ER_SUCCESS, buffer_instance.get_error());

    int third_value = buffer_instance.pop();
    FT_ASSERT_EQ(3, third_value);
    FT_ASSERT_EQ(ER_SUCCESS, buffer_instance.get_error());

    int fourth_value = buffer_instance.pop();
    FT_ASSERT_EQ(4, fourth_value);
    FT_ASSERT(buffer_instance.is_empty());
    FT_ASSERT_EQ(0UL, buffer_instance.size());
    FT_ASSERT_EQ(ER_SUCCESS, buffer_instance.get_error());
    return (1);
}

FT_TEST(test_ft_circular_buffer_error_states, "ft_circular_buffer surfaces full and empty error codes")
{
    ft_circular_buffer<int> buffer_instance(2);

    buffer_instance.push(10);
    FT_ASSERT_EQ(ER_SUCCESS, buffer_instance.get_error());
    buffer_instance.push(20);
    FT_ASSERT_EQ(ER_SUCCESS, buffer_instance.get_error());

    buffer_instance.push(30);
    FT_ASSERT_EQ(CIRCULAR_BUFFER_FULL, buffer_instance.get_error());

    buffer_instance.clear();
    FT_ASSERT_EQ(ER_SUCCESS, buffer_instance.get_error());
    FT_ASSERT(buffer_instance.is_empty());

    int empty_value = buffer_instance.pop();
    FT_ASSERT_EQ(0, empty_value);
    FT_ASSERT_EQ(CIRCULAR_BUFFER_EMPTY, buffer_instance.get_error());

    buffer_instance.push(50);
    FT_ASSERT_EQ(ER_SUCCESS, buffer_instance.get_error());
    buffer_instance.push(60);
    FT_ASSERT_EQ(ER_SUCCESS, buffer_instance.get_error());

    buffer_instance.pop();
    FT_ASSERT_EQ(ER_SUCCESS, buffer_instance.get_error());
    buffer_instance.pop();
    FT_ASSERT_EQ(ER_SUCCESS, buffer_instance.get_error());
    return (1);
}

FT_TEST(test_ft_circular_buffer_move_semantics, "ft_circular_buffer move constructor and assignment transfer storage")
{
    ft_circular_buffer<int> original_buffer(2);
    original_buffer.push(5);
    original_buffer.push(7);

    ft_circular_buffer<int> moved_buffer(ft_move(original_buffer));

    FT_ASSERT(original_buffer.is_empty());
    FT_ASSERT_EQ(ER_SUCCESS, original_buffer.get_error());

    FT_ASSERT_EQ(false, moved_buffer.is_empty());
    FT_ASSERT_EQ(2UL, moved_buffer.size());
    FT_ASSERT_EQ(5, moved_buffer.pop());
    FT_ASSERT_EQ(ER_SUCCESS, moved_buffer.get_error());

    ft_circular_buffer<int> assigned_buffer(3);
    assigned_buffer = ft_move(moved_buffer);

    FT_ASSERT(moved_buffer.is_empty());
    FT_ASSERT_EQ(ER_SUCCESS, moved_buffer.get_error());

    FT_ASSERT_EQ(1UL, assigned_buffer.size());
    FT_ASSERT_EQ(7, assigned_buffer.pop());
    FT_ASSERT_EQ(ER_SUCCESS, assigned_buffer.get_error());
    FT_ASSERT(assigned_buffer.is_empty());
    return (1);
}
