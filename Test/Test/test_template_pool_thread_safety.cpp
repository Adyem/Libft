#include "../../Template/pool.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

FT_TEST(test_pool_thread_safety_controls,
        "Pool installs optional mutex guards for shared state")
{
    Pool<int> pool;
    bool lock_acquired;

    FT_ASSERT_EQ(false, pool.is_thread_safe_enabled());
    FT_ASSERT_EQ(0, pool.enable_thread_safety());
    FT_ASSERT_EQ(true, pool.is_thread_safe_enabled());
    pool.resize(2);
    lock_acquired = false;
    FT_ASSERT_EQ(0, pool.lock(&lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    pool.unlock(lock_acquired);
    pool.disable_thread_safety();
    FT_ASSERT_EQ(false, pool.is_thread_safe_enabled());
    return (1);
}

FT_TEST(test_pool_object_thread_safety_management,
        "Pool::Object inherits and transfers guard state")
{
    Pool<int> pool;

    pool.resize(1);
    FT_ASSERT_EQ(0, pool.enable_thread_safety());
    Pool<int>::Object object = pool.acquire(42);
    FT_ASSERT_EQ(true, static_cast<bool>(object));
    FT_ASSERT_EQ(true, object.is_thread_safe_enabled());

    bool lock_acquired;

    lock_acquired = false;
    FT_ASSERT_EQ(0, object.lock(&lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    object.unlock(lock_acquired);

    Pool<int>::Object moved(ft_move(object));
    FT_ASSERT_EQ(true, moved.is_thread_safe_enabled());
    Pool<int>::Object assigned;
    FT_ASSERT_EQ(0, assigned.enable_thread_safety());
    assigned = ft_move(moved);
    FT_ASSERT_EQ(true, assigned.is_thread_safe_enabled());
    FT_ASSERT_EQ(false, moved.is_thread_safe_enabled());
    return (1);
}
