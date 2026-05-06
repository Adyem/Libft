#include "../test_internal.hpp"
#include "../../Modules/Template/unordered_map.hpp"
#include "../../Modules/CPP_class/class_nullptr.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/CMA/CMA.hpp"

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

FT_TEST(test_unordered_map_late_resize_failure_preserves_entries_and_state)
{
    unordered_map_int_int map_instance;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.initialize());
    map_instance.insert(1, 10);
    map_instance.insert(2, 20);
    map_instance.insert(3, 30);
    map_instance.insert(4, 40);
    map_instance.insert(5, 50);
    FT_ASSERT_EQ(static_cast<size_t>(5), map_instance.size());
    cma_set_alloc_limit(64);
    map_instance.insert(6, 60);
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, map_instance.get_error());
    FT_ASSERT_EQ(static_cast<size_t>(5), map_instance.size());
    FT_ASSERT_EQ(10, map_instance.at(1));
    FT_ASSERT_EQ(20, map_instance.at(2));
    FT_ASSERT_EQ(30, map_instance.at(3));
    FT_ASSERT_EQ(40, map_instance.at(4));
    FT_ASSERT_EQ(50, map_instance.at(5));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.destroy());
    return (1);
}

FT_TEST(test_unordered_map_late_resize_failure_keeps_mutex_unlocked)
{
    unordered_map_int_int map_instance;
    ft_bool lock_acquired;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.initialize());
    map_instance.insert(11, 110);
    map_instance.insert(12, 120);
    map_instance.insert(13, 130);
    map_instance.insert(14, 140);
    map_instance.insert(15, 150);
    cma_set_alloc_limit(64);
    map_instance.insert(16, 160);
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, map_instance.get_error());
    lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.lock(&lock_acquired));
    FT_ASSERT_EQ(FT_TRUE, lock_acquired);
    map_instance.unlock(lock_acquired);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.destroy());
    return (1);
}

FT_TEST(test_unordered_map_operator_index_late_failure_returns_invalid_proxy)
{
    unordered_map_int_int map_instance;
    unordered_map_int_int::mapped_proxy failed_proxy;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.initialize());
    map_instance.insert(21, 210);
    map_instance.insert(22, 220);
    map_instance.insert(23, 230);
    map_instance.insert(24, 240);
    map_instance.insert(25, 250);
    cma_set_alloc_limit(64);
    failed_proxy = map_instance[26];
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, map_instance.get_error());
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, failed_proxy.get_error());
    FT_ASSERT_EQ(0, failed_proxy.is_valid());
    map_instance[21] = 211;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.get_error());
    FT_ASSERT_EQ(211, map_instance.at(21));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.destroy());
    return (1);
}

FT_TEST(test_unordered_map_late_resize_failure_releases_memory_after_scope)
{
    ft_size_t baseline_bytes;
    ft_size_t bytes_after_scope;

    baseline_bytes = unordered_map_current_allocated_bytes();
    {
        unordered_map_int_int map_instance;

        FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.initialize());
        map_instance.insert(31, 310);
        map_instance.insert(32, 320);
        map_instance.insert(33, 330);
        map_instance.insert(34, 340);
        map_instance.insert(35, 350);
        cma_set_alloc_limit(64);
        map_instance.insert(36, 360);
        cma_set_alloc_limit(0);
        FT_ASSERT_EQ(FT_ERR_NO_MEMORY, map_instance.get_error());
        FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.destroy());
    }
    bytes_after_scope = unordered_map_current_allocated_bytes();
    FT_ASSERT_EQ(baseline_bytes, bytes_after_scope);
    return (1);
}

FT_TEST(test_unordered_map_initialize_allocation_failure_sets_no_memory)
{
    unordered_map_int_int destination_map;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_map.destroy());
    cma_set_alloc_limit(1);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, destination_map.initialize());
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_map.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_map.destroy());
    return (1);
}

FT_TEST(test_unordered_map_initialize_move_allocation_failure_sets_no_memory)
{
    unordered_map_int_int destination_map;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_map.destroy());
    cma_set_alloc_limit(1);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, destination_map.initialize());
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_map.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_map.destroy());
    return (1);
}

FT_TEST(test_unordered_map_initialize_copy_allocation_failure_releases_memory)
{
    ft_size_t baseline_bytes;
    ft_size_t bytes_after_scope;

    baseline_bytes = unordered_map_current_allocated_bytes();
    {
        unordered_map_int_int source_map;
        unordered_map_int_int destination_map;

        FT_ASSERT_EQ(FT_ERR_SUCCESS, source_map.initialize());
        source_map.insert(301, 3001);
        source_map.insert(302, 3002);
        source_map.insert(303, 3003);
        source_map.insert(304, 3004);
        source_map.insert(305, 3005);
        FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_map.destroy());
        cma_set_alloc_limit(1);
        FT_ASSERT_EQ(FT_ERR_NO_MEMORY, destination_map.initialize());
        cma_set_alloc_limit(0);
        FT_ASSERT_EQ(FT_ERR_SUCCESS, source_map.destroy());
    }
    bytes_after_scope = unordered_map_current_allocated_bytes();
    FT_ASSERT_EQ(baseline_bytes, bytes_after_scope);
    return (1);
}
