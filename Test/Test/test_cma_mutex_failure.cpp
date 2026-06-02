#include "../test_internal.hpp"
#include "../../Modules/CMA/CMA.hpp"
#include "../../Modules/CMA/cma_internal.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include <atomic>

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/PThread/mutex.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_cma_lock_allocator_mutex_lock_failure)
{
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;

    pt_recursive_mutex_lock_override_error_code.store(FT_ERR_SYS_MUTEX_LOCK_FAILED,
        std::memory_order_release);
    int32_t lock_result = cma_lock_allocator(&lock_acquired);
    pt_recursive_mutex_lock_override_error_code.store(FT_ERR_SUCCESS,
        std::memory_order_release);
    FT_ASSERT_EQ(FT_ERR_INVALID_STATE, lock_result);
    FT_ASSERT_EQ(FT_FALSE, lock_acquired);
    return (1);
}

FT_TEST(test_cma_malloc_mutex_lock_failure)
{
    pt_recursive_mutex_lock_override_error_code.store(FT_ERR_SYS_MUTEX_LOCK_FAILED,
        std::memory_order_release);
    void *allocation = cma_malloc(32);
    pt_recursive_mutex_lock_override_error_code.store(FT_ERR_SUCCESS,
        std::memory_order_release);
    FT_ASSERT_EQ(ft_nullptr, allocation);
    return (1);
}

FT_TEST(test_cma_realloc_mutex_lock_failure)
{
    pt_recursive_mutex_lock_override_error_code.store(FT_ERR_SYS_MUTEX_LOCK_FAILED,
        std::memory_order_release);
    void *allocation = cma_realloc(ft_nullptr, 64);
    pt_recursive_mutex_lock_override_error_code.store(FT_ERR_SUCCESS,
        std::memory_order_release);
    FT_ASSERT_EQ(ft_nullptr, allocation);
    return (1);
}

FT_TEST(test_cma_alloc_size_mutex_lock_failure)
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
