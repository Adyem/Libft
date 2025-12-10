#include "../../CMA/cma_allocation_guard.hpp"
#include "../../PThread/thread.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include "../../CPP_class/class_nullptr.hpp"

#include <utility>

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

FT_TEST(test_cma_allocation_guard_destructor_resets_errno,
        "cma_allocation_guard destructor sets ft_errno to success")
{
    void *allocation_pointer;

    cma_set_alloc_limit(0);
    allocation_pointer = cma_malloc(40);
    if (allocation_pointer == ft_nullptr)
        return (0);
    ft_errno = FT_ERR_INVALID_OPERATION;
    {
        cma_allocation_guard allocation_guard(allocation_pointer);

        ft_errno = FT_ERR_INVALID_OPERATION;
    }
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_cma_allocation_guard_reset_resets_errno,
        "cma_allocation_guard reset sets ft_errno to success on success")
{
    void *first_pointer;
    void *second_pointer;
    cma_allocation_guard allocation_guard;

    cma_set_alloc_limit(0);
    first_pointer = cma_malloc(32);
    if (first_pointer == ft_nullptr)
        return (0);
    allocation_guard = cma_allocation_guard(first_pointer);
    second_pointer = cma_malloc(48);
    if (second_pointer == ft_nullptr)
        return (0);
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    allocation_guard.reset(second_pointer);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    allocation_guard.reset(ft_nullptr);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}
