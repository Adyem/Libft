#include "../test_internal.hpp"
#include "../../Template/unordered_map.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../CMA/CMA.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

typedef ft_unordered_map<int, int> unordered_map_int_int;

static ft_size_t unordered_map_current_allocated_bytes(void)
{
    ft_size_t allocation_count;
    ft_size_t free_count;
    ft_size_t current_bytes;
    ft_size_t peak_bytes;
    int32_t status;

    allocation_count = 0;
    free_count = 0;
    current_bytes = 0;
    peak_bytes = 0;
    status = cma_get_extended_stats(&allocation_count, &free_count,
            &current_bytes, &peak_bytes);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, status);
    return (current_bytes);
}

FT_TEST(test_unordered_map_late_resize_failure_preserves_entries_and_state,
    "unordered_map resize allocation failure keeps existing entries intact")
{
    unordered_map_int_int map_instance;

    map_instance.insert(1, 10);
    map_instance.insert(2, 20);
    map_instance.insert(3, 30);
    map_instance.insert(4, 40);
    map_instance.insert(5, 50);
    FT_ASSERT_EQ(static_cast<size_t>(5), map_instance.size());
    cma_set_alloc_limit(64);
    map_instance.insert(6, 60);
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, map_instance.last_operation_error());
    FT_ASSERT_EQ(static_cast<size_t>(5), map_instance.size());
    FT_ASSERT_EQ(10, map_instance.at(1));
    FT_ASSERT_EQ(20, map_instance.at(2));
    FT_ASSERT_EQ(30, map_instance.at(3));
    FT_ASSERT_EQ(40, map_instance.at(4));
    FT_ASSERT_EQ(50, map_instance.at(5));
    return (1);
}

FT_TEST(test_unordered_map_late_resize_failure_keeps_mutex_unlocked,
    "unordered_map resize failure leaves recursive mutex unlocked")
{
    unordered_map_int_int map_instance;
    pt_recursive_mutex *mutex_pointer;

    map_instance.insert(11, 110);
    map_instance.insert(12, 120);
    map_instance.insert(13, 130);
    map_instance.insert(14, 140);
    map_instance.insert(15, 150);
    mutex_pointer = map_instance.get_mutex_for_validation();
    FT_ASSERT(mutex_pointer != ft_nullptr);
    cma_set_alloc_limit(64);
    map_instance.insert(16, 160);
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, map_instance.last_operation_error());
    FT_ASSERT_EQ(false, mutex_pointer->lockState());
    return (1);
}

FT_TEST(test_unordered_map_operator_index_late_failure_returns_invalid_proxy,
    "unordered_map operator[] late allocation failure returns invalid proxy and recovers")
{
    unordered_map_int_int map_instance;
    unordered_map_int_int::mapped_proxy failed_proxy;

    map_instance.insert(21, 210);
    map_instance.insert(22, 220);
    map_instance.insert(23, 230);
    map_instance.insert(24, 240);
    map_instance.insert(25, 250);
    cma_set_alloc_limit(64);
    failed_proxy = map_instance[26];
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(0, failed_proxy.is_valid());
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, failed_proxy.get_error());
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, map_instance.last_operation_error());
    map_instance[21] = 211;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.last_operation_error());
    FT_ASSERT_EQ(211, map_instance.at(21));
    return (1);
}

FT_TEST(test_unordered_map_late_resize_failure_releases_memory_after_scope,
    "unordered_map resize failure leaves no residual memory after object destruction")
{
    ft_size_t baseline_bytes;
    ft_size_t bytes_after_scope;

    baseline_bytes = unordered_map_current_allocated_bytes();
    {
        unordered_map_int_int map_instance;

        map_instance.insert(31, 310);
        map_instance.insert(32, 320);
        map_instance.insert(33, 330);
        map_instance.insert(34, 340);
        map_instance.insert(35, 350);
        cma_set_alloc_limit(64);
        map_instance.insert(36, 360);
        cma_set_alloc_limit(0);
        FT_ASSERT_EQ(FT_ERR_NO_MEMORY, map_instance.last_operation_error());
    }
    bytes_after_scope = unordered_map_current_allocated_bytes();
    FT_ASSERT_EQ(baseline_bytes, bytes_after_scope);
    return (1);
}
