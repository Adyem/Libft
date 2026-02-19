#include "../test_internal.hpp"
#include "../../Template/deque.hpp"
#include "../../CMA/CMA.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"
#include <utility>

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_ft_deque_push_pop_order, "ft_deque preserves order across push and pop operations")
{
    ft_deque<int> deque_instance;

    deque_instance.push_back(5);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_deque<int>::last_operation_error());
    deque_instance.push_front(3);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_deque<int>::last_operation_error());
    deque_instance.push_back(7);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_deque<int>::last_operation_error());

    FT_ASSERT_EQ(false, deque_instance.empty());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_deque<int>::last_operation_error());
    FT_ASSERT_EQ(3UL, deque_instance.size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_deque<int>::last_operation_error());

    FT_ASSERT_EQ(3, deque_instance.front());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_deque<int>::last_operation_error());
    FT_ASSERT_EQ(7, deque_instance.back());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_deque<int>::last_operation_error());

    int first_value = deque_instance.pop_front();
    FT_ASSERT_EQ(3, first_value);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_deque<int>::last_operation_error());

    int last_value = deque_instance.pop_back();
    FT_ASSERT_EQ(7, last_value);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_deque<int>::last_operation_error());

    FT_ASSERT_EQ(1UL, deque_instance.size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_deque<int>::last_operation_error());
    FT_ASSERT_EQ(5, deque_instance.front());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_deque<int>::last_operation_error());

    deque_instance.clear();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_deque<int>::last_operation_error());
    FT_ASSERT(deque_instance.empty());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_deque<int>::last_operation_error());
    return (1);
}

FT_TEST(test_ft_deque_pop_on_empty_sets_error, "ft_deque reports underflow errors when popping from an empty container")
{
    ft_deque<int> deque_instance;

    int value_from_front = deque_instance.pop_front();
    FT_ASSERT_EQ(0, value_from_front);
    FT_ASSERT_EQ(FT_ERR_EMPTY, ft_deque<int>::last_operation_error());

    int value_from_back = deque_instance.pop_back();
    FT_ASSERT_EQ(0, value_from_back);
    FT_ASSERT_EQ(FT_ERR_EMPTY, ft_deque<int>::last_operation_error());

    FT_ASSERT(deque_instance.empty());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_deque<int>::last_operation_error());
    return (1);
}

FT_TEST(test_ft_deque_allocation_failure_sets_error, "ft_deque surfaces allocation failures during push operations")
{
    ft_deque<int> deque_instance;

    cma_set_alloc_limit(1);
    deque_instance.push_back(42);
    cma_set_alloc_limit(0);

    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, ft_deque<int>::last_operation_error());
    FT_ASSERT(deque_instance.empty());

    deque_instance.push_back(5);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_deque<int>::last_operation_error());
    FT_ASSERT_EQ(1UL, deque_instance.size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_deque<int>::last_operation_error());
    return (1);
}

FT_TEST(test_ft_deque_move_transfers_ownership, "ft_deque move operations transfer nodes and reset sources")
{
    ft_deque<int> source_deque;
    source_deque.push_back(1);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_deque<int>::last_operation_error());
    source_deque.push_back(2);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_deque<int>::last_operation_error());
    source_deque.push_back(3);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_deque<int>::last_operation_error());

    ft_deque<int> target_deque;
    int transferred_value = 0;

    while (!source_deque.empty())
    {
        transferred_value = source_deque.pop_front();
        target_deque.push_back(transferred_value);
    }

    FT_ASSERT(source_deque.empty());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_deque<int>::last_operation_error());
    FT_ASSERT_EQ(3UL, target_deque.size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_deque<int>::last_operation_error());
    FT_ASSERT_EQ(1, target_deque.front());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_deque<int>::last_operation_error());
    FT_ASSERT_EQ(3, target_deque.back());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_deque<int>::last_operation_error());
    return (1);
}
