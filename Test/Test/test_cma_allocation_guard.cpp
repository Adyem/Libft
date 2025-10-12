#include "../../CMA/cma_allocation_guard.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../cma_test_helpers.hpp"

#include <utility>

FT_TEST(test_cma_allocation_guard_releases_allocation,
        "cma_allocation_guard frees the managed allocation on destruction")
{
    ft_size_t allocation_count_before;
    ft_size_t free_count_before;
    ft_size_t allocation_count_during_scope;
    ft_size_t free_count_during_scope;
    ft_size_t allocation_count_after_scope;
    ft_size_t free_count_after_scope;
    void *scoped_pointer;

    cma_set_alloc_limit(0);
    FT_ASSERT(ensure_cma_cleanup_success());
    cma_get_stats(&allocation_count_before, &free_count_before);
    scoped_pointer = cma_malloc(32);
    if (scoped_pointer == ft_nullptr)
        return (0);
    {
        cma_allocation_guard allocation_guard(scoped_pointer);

        scoped_pointer = ft_nullptr;
        cma_get_stats(&allocation_count_during_scope, &free_count_during_scope);
        FT_ASSERT_EQ(allocation_count_before + 1, allocation_count_during_scope);
        FT_ASSERT_EQ(free_count_before, free_count_during_scope);
    }
    cma_get_stats(&allocation_count_after_scope, &free_count_after_scope);
    FT_ASSERT_EQ(allocation_count_before + 1, allocation_count_after_scope);
    FT_ASSERT_EQ(free_count_before + 1, free_count_after_scope);
    return (1);
}

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
