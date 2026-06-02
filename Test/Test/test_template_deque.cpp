#include "../test_internal.hpp"
#include "../../Modules/Template/deque.hpp"
#include "../../Modules/CMA/CMA.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include <utility>

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_ft_deque_push_pop_order)
{
    ft_deque<int> deque_instance;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, deque_instance.initialize());

    deque_instance.push_back(5);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, deque_instance.get_error());
    deque_instance.push_front(3);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, deque_instance.get_error());
    deque_instance.push_back(7);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, deque_instance.get_error());

    FT_ASSERT_EQ(false, deque_instance.empty());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, deque_instance.get_error());
    FT_ASSERT_EQ(3UL, deque_instance.size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, deque_instance.get_error());

    FT_ASSERT_EQ(3, deque_instance.front());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, deque_instance.get_error());
    FT_ASSERT_EQ(7, deque_instance.back());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, deque_instance.get_error());

    int first_value = deque_instance.pop_front();
    FT_ASSERT_EQ(3, first_value);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, deque_instance.get_error());

    int last_value = deque_instance.pop_back();
    FT_ASSERT_EQ(7, last_value);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, deque_instance.get_error());

    FT_ASSERT_EQ(1UL, deque_instance.size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, deque_instance.get_error());
    FT_ASSERT_EQ(5, deque_instance.front());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, deque_instance.get_error());

    deque_instance.clear();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, deque_instance.get_error());
    FT_ASSERT(deque_instance.empty());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, deque_instance.get_error());
    return (1);
}

FT_TEST(test_ft_deque_pop_on_empty_sets_error)
{
    ft_deque<int> deque_instance;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, deque_instance.initialize());

    int value_from_front = deque_instance.pop_front();
    FT_ASSERT_EQ(0, value_from_front);
    FT_ASSERT_EQ(FT_ERR_EMPTY, deque_instance.get_error());

    int value_from_back = deque_instance.pop_back();
    FT_ASSERT_EQ(0, value_from_back);
    FT_ASSERT_EQ(FT_ERR_EMPTY, deque_instance.get_error());

    FT_ASSERT(deque_instance.empty());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, deque_instance.get_error());
    return (1);
}

FT_TEST(test_ft_deque_allocation_failure_sets_error)
{
    ft_deque<int> deque_instance;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, deque_instance.initialize());

    cma_set_alloc_limit(1);
    deque_instance.push_back(42);
    cma_set_alloc_limit(0);

    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, deque_instance.get_error());
    FT_ASSERT(deque_instance.empty());

    deque_instance.push_back(5);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, deque_instance.get_error());
    FT_ASSERT_EQ(1UL, deque_instance.size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, deque_instance.get_error());
    return (1);
}

FT_TEST(test_ft_deque_move_transfers_ownership)
{
    ft_deque<int> source_deque;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_deque.initialize());
    source_deque.push_back(1);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_deque.get_error());
    source_deque.push_back(2);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_deque.get_error());
    source_deque.push_back(3);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_deque.get_error());

    ft_deque<int> target_deque;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, target_deque.initialize());
    int transferred_value = 0;

    while (!source_deque.empty())
    {
        transferred_value = source_deque.pop_front();
        target_deque.push_back(transferred_value);
    }

    FT_ASSERT(source_deque.empty());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_deque.get_error());
    FT_ASSERT_EQ(3UL, target_deque.size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, target_deque.get_error());
    FT_ASSERT_EQ(1, target_deque.front());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, target_deque.get_error());
    FT_ASSERT_EQ(3, target_deque.back());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, target_deque.get_error());
    return (1);
}
