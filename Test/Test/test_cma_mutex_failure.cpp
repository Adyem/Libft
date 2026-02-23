#include "../test_internal.hpp"
#include "../../CMA/CMA.hpp"
#include "../../CMA/cma_internal.hpp"
#include "../../Errno/errno.hpp"
#include "../../PThread/recursive_mutex.hpp"
#include "../../System_utils/test_runner.hpp"
#include <atomic>

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_cma_lock_allocator_mutex_lock_failure,
    "cma_lock_allocator reports invalid state when the recursive mutex lock fails")
{
    bool lock_acquired = false;

    pt_recursive_mutex_lock_override_error_code.store(FT_ERR_SYS_MUTEX_LOCK_FAILED,
        std::memory_order_release);
    int32_t lock_result = cma_lock_allocator(&lock_acquired);
    pt_recursive_mutex_lock_override_error_code.store(FT_ERR_SUCCESS,
        std::memory_order_release);
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, lock_result);
    FT_ASSERT_EQ(false, lock_acquired);
    return (1);
}

FT_TEST(test_cma_malloc_mutex_lock_failure,
    "cma_malloc returns null when the allocator mutex lock fails")
{
    pt_recursive_mutex_lock_override_error_code.store(FT_ERR_SYS_MUTEX_LOCK_FAILED,
        std::memory_order_release);
    void *allocation = cma_malloc(32);
    pt_recursive_mutex_lock_override_error_code.store(FT_ERR_SUCCESS,
        std::memory_order_release);
    FT_ASSERT_EQ(ft_nullptr, allocation);
    return (1);
}

FT_TEST(test_cma_realloc_mutex_lock_failure,
    "cma_realloc returns null when allocator mutex lock fails")
{
    pt_recursive_mutex_lock_override_error_code.store(FT_ERR_SYS_MUTEX_LOCK_FAILED,
        std::memory_order_release);
    void *allocation = cma_realloc(ft_nullptr, 64);
    pt_recursive_mutex_lock_override_error_code.store(FT_ERR_SUCCESS,
        std::memory_order_release);
    FT_ASSERT_EQ(ft_nullptr, allocation);
    return (1);
}

FT_TEST(test_cma_alloc_size_mutex_lock_failure,
    "cma_alloc_size reports zero when allocator mutex lock fails")
{
    void *allocation = cma_malloc(16);
    if (allocation == ft_nullptr)
        return (0);
    pt_recursive_mutex_lock_override_error_code.store(FT_ERR_SYS_MUTEX_LOCK_FAILED,
        std::memory_order_release);
    ft_size_t block_size = cma_alloc_size(allocation);
    pt_recursive_mutex_lock_override_error_code.store(FT_ERR_SUCCESS,
        std::memory_order_release);
    FT_ASSERT_EQ(0, block_size);
    cma_free(allocation);
    return (1);
}
