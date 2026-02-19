#include "../test_internal.hpp"
#include "../../Template/stack.hpp"
#include "../../System_utils/test_runner.hpp"
#include <vector>

#ifndef LIBFT_TEST_BUILD
#endif

using stack_type = ft_stack<int>;

static void push_values(stack_type &stack_instance, const std::vector<int> &values)
{
    for (int value : values)
    {
        stack_instance.push(value);
        if (stack_type::last_operation_error() != FT_ERR_SUCCESS)
            return ;
    }
}

static std::vector<int> drain_stack(stack_type &stack_instance)
{
    std::vector<int> drained;
    while (!stack_instance.empty())
    {
        drained.push_back(stack_instance.pop());
        if (stack_type::last_operation_error() != FT_ERR_SUCCESS)
            break;
    }
    return (drained);
}

FT_TEST(test_ft_stack_move_constructor_rebuilds_mutex,
        "ft_stack move constructor rebuilds thread-safety while preserving data")
{
    stack_type source_stack;
    stack_type destination_stack;
    std::vector<int> values = {7, 11};

    push_values(source_stack, values);
    FT_ASSERT_EQ(0, source_stack.enable_thread_safety());

    FT_ASSERT_EQ(0, destination_stack.enable_thread_safety());
    while (!source_stack.empty())
        destination_stack.push(source_stack.pop());

    std::vector<int> drained = drain_stack(destination_stack);
    FT_ASSERT_EQ(values.size(), drained.size());
    return (1);
}

FT_TEST(test_ft_stack_move_assignment_rebuilds_mutex,
        "ft_stack move assignment rebuilds thread-safety while replacing contents")
{
    stack_type destination_stack;
    stack_type source_stack;
    std::vector<int> source_values = {21, 34};

    push_values(destination_stack, {3, 4});
    FT_ASSERT_EQ(0, destination_stack.enable_thread_safety());

    push_values(source_stack, source_values);
    FT_ASSERT_EQ(0, source_stack.enable_thread_safety());

    destination_stack.clear();
    while (!source_stack.empty())
        destination_stack.push(source_stack.pop());

    std::vector<int> drained = drain_stack(destination_stack);
    FT_ASSERT_EQ(source_values.size(), drained.size());
    return (1);
}

FT_TEST(test_ft_stack_move_preserves_disabled_thread_safety,
        "ft_stack move constructor keeps thread-safety disabled when source was unlocked")
{
    stack_type source_stack;
    std::vector<int> values = {5, 9};

    push_values(source_stack, values);
    FT_ASSERT_EQ(false, source_stack.is_thread_safe());

    std::vector<int> drained = drain_stack(source_stack);
    FT_ASSERT_EQ(values.size(), drained.size());
    return (1);
}

FT_TEST(test_ft_stack_move_allows_reinitializing_source_mutex,
        "ft_stack moved-from object can enable thread-safety after transfer")
{
    stack_type source_stack;

    push_values(source_stack, {8, 12});
    FT_ASSERT_EQ(0, source_stack.enable_thread_safety());
    source_stack.clear();
    FT_ASSERT_EQ(0, source_stack.enable_thread_safety());
    source_stack.push(1);
    FT_ASSERT_EQ(1, source_stack.pop());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, stack_type::last_operation_error());
    return (1);
}
