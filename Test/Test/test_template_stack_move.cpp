#include "../test_internal.hpp"
#include "../../Template/stack.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"
#include <vector>

#ifndef LIBFT_TEST_BUILD
#endif

using stack_type = ft_stack<int>;

static void push_values(stack_type &stack_instance, const std::vector<int> &values)
{
    ft_size_t index;

    index = 0;
    while (index < values.size())
    {
        stack_instance.push(values[index]);
        if (stack_instance.get_error() != FT_ERR_SUCCESS)
            return ;
        index += 1;
    }
    return ;
}

static std::vector<int> drain_stack(stack_type &stack_instance)
{
    std::vector<int> drained;
    while (!stack_instance.empty())
    {
        drained.push_back(stack_instance.pop());
        if (stack_instance.get_error() != FT_ERR_SUCCESS)
            break;
    }
    return (drained);
}

FT_TEST(test_ft_stack_move_constructor_rebuilds_mutex)
{
    stack_type source_stack;
    stack_type destination_stack;
    std::vector<int> values = {7, 11};

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_stack.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_stack.initialize());
    push_values(source_stack, values);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_stack.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_stack.enable_thread_safety());
    while (!source_stack.empty())
        destination_stack.push(source_stack.pop());

    std::vector<int> drained = drain_stack(destination_stack);
    FT_ASSERT_EQ(values.size(), drained.size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_stack.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_stack.destroy());
    return (1);
}

FT_TEST(test_ft_stack_move_assignment_rebuilds_mutex)
{
    stack_type destination_stack;
    stack_type source_stack;
    std::vector<int> source_values = {21, 34};

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_stack.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_stack.initialize());
    push_values(destination_stack, {3, 4});
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_stack.enable_thread_safety());

    push_values(source_stack, source_values);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_stack.enable_thread_safety());

    destination_stack.clear();
    while (!source_stack.empty())
        destination_stack.push(source_stack.pop());

    std::vector<int> drained = drain_stack(destination_stack);
    FT_ASSERT_EQ(source_values.size(), drained.size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_stack.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_stack.destroy());
    return (1);
}

FT_TEST(test_ft_stack_move_preserves_disabled_thread_safety)
{
    stack_type source_stack;
    std::vector<int> values = {5, 9};

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_stack.initialize());
    push_values(source_stack, values);
    FT_ASSERT_EQ(false, source_stack.is_thread_safe());

    std::vector<int> drained = drain_stack(source_stack);
    FT_ASSERT_EQ(values.size(), drained.size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_stack.destroy());
    return (1);
}

FT_TEST(test_ft_stack_move_allows_reinitializing_source_mutex)
{
    stack_type source_stack;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_stack.initialize());
    push_values(source_stack, {8, 12});
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_stack.enable_thread_safety());
    source_stack.clear();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_stack.enable_thread_safety());
    source_stack.push(1);
    FT_ASSERT_EQ(1, source_stack.pop());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_stack.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_stack.destroy());
    return (1);
}
