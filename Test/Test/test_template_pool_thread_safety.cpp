#include "../test_internal.hpp"
#include "../../Template/pool.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"
#include "../../Errno/errno.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_template_pool_thread_safety_controls)
{
    Pool<int32_t> pool;
    ft_bool lock_acquired;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, pool.initialize());
    FT_ASSERT_EQ(FT_FALSE, pool.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, pool.enable_thread_safety());
    FT_ASSERT_EQ(FT_TRUE, pool.is_thread_safe());
    pool.resize(2);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, pool.get_error());

    lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, pool.lock(&lock_acquired));
    FT_ASSERT_EQ(FT_TRUE, lock_acquired);
    pool.unlock(lock_acquired);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, pool.disable_thread_safety());
    FT_ASSERT_EQ(FT_FALSE, pool.is_thread_safe());
    return (1);
}

FT_TEST(test_template_pool_thread_safety_object_lock_unlock)
{
    Pool<int32_t> pool;
    Pool<int32_t>::Object object;
    ft_bool lock_acquired;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, pool.initialize());
    pool.resize(1);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, pool.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, pool.enable_thread_safety());

    object = pool.acquire(42);
    FT_ASSERT_EQ(FT_TRUE, static_cast<ft_bool>(object));

    lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, object.lock(&lock_acquired));
    FT_ASSERT_EQ(FT_TRUE, lock_acquired);
    object.unlock(lock_acquired);
    return (1);
}
