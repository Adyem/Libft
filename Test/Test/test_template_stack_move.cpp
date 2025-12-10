#include "../../Template/stack.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

FT_TEST(test_ft_stack_move_constructor_rebuilds_mutex,
        "ft_stack move constructor rebuilds thread-safety while preserving data")
{
    ft_stack<int> source_stack;
    int first_value;
    int second_value;

    source_stack.push(7);
    source_stack.push(11);
    FT_ASSERT_EQ(0, source_stack.enable_thread_safety());
    FT_ASSERT(source_stack.is_thread_safe());

    ft_stack<int> moved_stack(ft_move(source_stack));

    FT_ASSERT(moved_stack.is_thread_safe());
    FT_ASSERT_EQ(false, source_stack.is_thread_safe());
    FT_ASSERT_EQ(2UL, moved_stack.size());

    first_value = moved_stack.pop();
    FT_ASSERT_EQ(11, first_value);
    second_value = moved_stack.pop();
    FT_ASSERT_EQ(7, second_value);
    FT_ASSERT(moved_stack.empty());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, moved_stack.get_error());
    return (1);
}

FT_TEST(test_ft_stack_move_assignment_rebuilds_mutex,
        "ft_stack move assignment rebuilds thread-safety while replacing contents")
{
    ft_stack<int> destination_stack;
    ft_stack<int> source_stack;
    int popped_value;

    destination_stack.push(3);
    destination_stack.push(4);
    FT_ASSERT_EQ(0, destination_stack.enable_thread_safety());
    FT_ASSERT(destination_stack.is_thread_safe());

    source_stack.push(21);
    source_stack.push(34);
    FT_ASSERT_EQ(0, source_stack.enable_thread_safety());
    FT_ASSERT(source_stack.is_thread_safe());

    destination_stack = ft_move(source_stack);

    FT_ASSERT(destination_stack.is_thread_safe());
    FT_ASSERT_EQ(false, source_stack.is_thread_safe());
    FT_ASSERT_EQ(2UL, destination_stack.size());

    popped_value = destination_stack.pop();
    FT_ASSERT_EQ(34, popped_value);
    popped_value = destination_stack.pop();
    FT_ASSERT_EQ(21, popped_value);
    FT_ASSERT(destination_stack.empty());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, destination_stack.get_error());
    return (1);
}

FT_TEST(test_ft_stack_move_preserves_disabled_thread_safety,
        "ft_stack move constructor keeps thread-safety disabled when source was unlocked")
{
    ft_stack<int> source_stack;

    source_stack.push(5);
    source_stack.push(9);
    FT_ASSERT_EQ(false, source_stack.is_thread_safe());

    ft_stack<int> moved_stack(ft_move(source_stack));

    FT_ASSERT_EQ(false, moved_stack.is_thread_safe());
    FT_ASSERT_EQ(false, source_stack.is_thread_safe());
    FT_ASSERT_EQ(2UL, moved_stack.size());
    FT_ASSERT_EQ(9, moved_stack.pop());
    FT_ASSERT_EQ(5, moved_stack.pop());
    return (1);
}

FT_TEST(test_ft_stack_move_allows_reinitializing_source_mutex,
        "ft_stack moved-from object can enable thread-safety after transfer")
{
    ft_stack<int> source_stack;
    ft_stack<int> moved_stack;

    source_stack.push(8);
    source_stack.push(12);
    FT_ASSERT_EQ(0, source_stack.enable_thread_safety());
    FT_ASSERT(source_stack.is_thread_safe());

    moved_stack = ft_move(source_stack);

    FT_ASSERT(moved_stack.is_thread_safe());
    FT_ASSERT_EQ(false, source_stack.is_thread_safe());
    FT_ASSERT_EQ(12, moved_stack.pop());
    FT_ASSERT_EQ(8, moved_stack.pop());
    FT_ASSERT_EQ(0, source_stack.enable_thread_safety());
    FT_ASSERT(source_stack.is_thread_safe());
    source_stack.push(1);
    FT_ASSERT_EQ(1, source_stack.pop());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, source_stack.get_error());
    return (1);
}
