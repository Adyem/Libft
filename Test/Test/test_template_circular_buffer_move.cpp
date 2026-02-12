#include "../test_internal.hpp"
#include "../../Template/circular_buffer.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_ft_circular_buffer_move_constructor_rebuilds_mutex,
        "ft_circular_buffer move constructor recreates mutex and preserves data")
{
    ft_circular_buffer<int> source_buffer(3);
    int first_value;
    int second_value;
    bool moved_lock_acquired;
    bool source_lock_acquired;

    moved_lock_acquired = false;
    source_lock_acquired = false;
    FT_ASSERT_EQ(0, source_buffer.enable_thread_safety());
    FT_ASSERT(source_buffer.is_thread_safe());
    source_buffer.push(5);
    source_buffer.push(7);
    ft_circular_buffer<int> moved_buffer(ft_move(source_buffer));
    FT_ASSERT(moved_buffer.is_thread_safe());
    FT_ASSERT_EQ(false, source_buffer.is_thread_safe());
    FT_ASSERT_EQ(0, moved_buffer.lock(&moved_lock_acquired));
    FT_ASSERT(moved_lock_acquired);
    first_value = moved_buffer.pop();
    second_value = moved_buffer.pop();
    moved_buffer.unlock(moved_lock_acquired);
    FT_ASSERT_EQ(5, first_value);
    FT_ASSERT_EQ(7, second_value);
    FT_ASSERT_EQ(0, source_buffer.enable_thread_safety());
    FT_ASSERT(source_buffer.is_thread_safe());
    FT_ASSERT_EQ(0, source_buffer.lock(&source_lock_acquired));
    FT_ASSERT(source_lock_acquired);
    source_buffer.unlock(source_lock_acquired);
    return (1);
}

FT_TEST(test_ft_circular_buffer_move_assignment_rebuilds_mutex,
        "ft_circular_buffer move assignment reinstates mutex and keeps moved-from reusable")
{
    ft_circular_buffer<int> source_buffer(2);
    ft_circular_buffer<int> destination_buffer(2);
    int popped_value;
    bool destination_lock_acquired;
    bool source_lock_acquired;

    destination_lock_acquired = false;
    source_lock_acquired = false;
    FT_ASSERT_EQ(0, source_buffer.enable_thread_safety());
    FT_ASSERT_EQ(0, destination_buffer.enable_thread_safety());
    source_buffer.push(9);
    source_buffer.push(11);
    destination_buffer = ft_move(source_buffer);
    FT_ASSERT(destination_buffer.is_thread_safe());
    FT_ASSERT_EQ(false, source_buffer.is_thread_safe());
    FT_ASSERT_EQ(0, destination_buffer.lock(&destination_lock_acquired));
    FT_ASSERT(destination_lock_acquired);
    popped_value = destination_buffer.pop();
    destination_buffer.unlock(destination_lock_acquired);
    FT_ASSERT_EQ(9, popped_value);
    FT_ASSERT_EQ(0, source_buffer.enable_thread_safety());
    FT_ASSERT(source_buffer.is_thread_safe());
    FT_ASSERT_EQ(0, source_buffer.lock(&source_lock_acquired));
    FT_ASSERT(source_lock_acquired);
    source_buffer.unlock(source_lock_acquired);
    return (1);
}
