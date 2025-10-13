#include "../../CMA/cma_allocation_guard.hpp"
#include "../../PThread/thread.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../cma_test_helpers.hpp"

#include <utility>

/*
static void thread_allocation_worker(ft_size_t iteration_count, ft_size_t allocation_size)
{
    ft_size_t iteration_index;
    void *allocation_pointer;

    iteration_index = 0;
    while (iteration_index < iteration_count)
    {
        allocation_pointer = cma_malloc(allocation_size);
        if (allocation_pointer != ft_nullptr)
        {
            cma_free(allocation_pointer);
        }
        iteration_index = iteration_index + 1;
    }
    return ;
}

FT_TEST(test_cma_allocation_guard_releases_allocation,
        "cma_allocation_guard concurrent allocations do not hang")
{
    ft_size_t thread_count;
    ft_size_t allocation_iterations;
    ft_size_t allocation_size_base;
    ft_size_t thread_index;
    ft_size_t join_index;
    ft_thread threads[4];

    thread_count = 2;
    allocation_iterations = 64;
    allocation_size_base = 32;
    cma_set_alloc_limit(0);
    FT_ASSERT(ensure_cma_cleanup_success());
    thread_index = 0;
    while (thread_index < thread_count)
    {
        threads[thread_index] = ft_thread(thread_allocation_worker,
            allocation_iterations, allocation_size_base + (thread_index * 8));
        FT_ASSERT_EQ(ER_SUCCESS, threads[thread_index].get_error());
        FT_ASSERT(threads[thread_index].joinable());
        thread_index = thread_index + 1;
    }
    join_index = 0;
    while (join_index < thread_count)
    {
        threads[join_index].join();
        FT_ASSERT_EQ(ER_SUCCESS, threads[join_index].get_error());
        join_index = join_index + 1;
    }
    FT_ASSERT(ensure_cma_cleanup_success());
    return (1);
}
*/

FT_TEST(test_cma_allocation_guard_release_transfers_ownership,
        "cma_allocation_guard release prevents automatic free")
{
    ft_size_t allocation_count_before;
    ft_size_t free_count_before;
    ft_size_t allocation_count_after_release;
    ft_size_t free_count_after_release;
    ft_size_t allocation_count_after_manual_free;
    ft_size_t free_count_after_manual_free;
    void *released_pointer;

    cma_set_alloc_limit(0);
    FT_ASSERT(ensure_cma_cleanup_success());
    cma_get_stats(&allocation_count_before, &free_count_before);
    released_pointer = cma_malloc(48);
    if (released_pointer == ft_nullptr)
        return (0);
    {
        cma_allocation_guard allocation_guard(released_pointer);

        released_pointer = allocation_guard.release();
        FT_ASSERT_EQ(ft_nullptr, allocation_guard.get());
    }
    cma_get_stats(&allocation_count_after_release, &free_count_after_release);
    FT_ASSERT_EQ(allocation_count_before + 1, allocation_count_after_release);
    FT_ASSERT_EQ(free_count_before, free_count_after_release);
    cma_free(released_pointer);
    cma_get_stats(&allocation_count_after_manual_free, &free_count_after_manual_free);
    FT_ASSERT_EQ(allocation_count_before + 1, allocation_count_after_manual_free);
    FT_ASSERT_EQ(free_count_before + 1, free_count_after_manual_free);
    return (1);
}

FT_TEST(test_cma_allocation_guard_move_transfers_ownership,
        "cma_allocation_guard move operations transfer managed state")
{
    void *initial_pointer;
    cma_allocation_guard destination_guard;

    cma_set_alloc_limit(0);
    initial_pointer = cma_malloc(24);
    if (initial_pointer == ft_nullptr)
        return (0);
    {
        cma_allocation_guard source_guard(initial_pointer);

        destination_guard = std::move(source_guard);
        FT_ASSERT_EQ(ft_nullptr, source_guard.get());
        FT_ASSERT(destination_guard.owns_allocation());
    }
    return (1);
}
