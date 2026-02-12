#include "../test_internal.hpp"
#include "../../Template/pool.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_pool_move_constructor_rebuilds_mutex,
        "Pool move constructor rebuilds thread safety while keeping storage")
{
    Pool<int> source_pool;

    source_pool.resize(1);
    FT_ASSERT_EQ(0, source_pool.enable_thread_safety());
    {
        Pool<int>::Object object = source_pool.acquire(17);

        FT_ASSERT(static_cast<bool>(object));
        FT_ASSERT_EQ(true, object.is_thread_safe_enabled());
        FT_ASSERT_EQ(17, *object.operator->());
    }

    Pool<int> moved_pool(ft_move(source_pool));

    FT_ASSERT_EQ(true, moved_pool.is_thread_safe_enabled());
    FT_ASSERT_EQ(false, source_pool.is_thread_safe_enabled());
    {
        Pool<int>::Object moved_object = moved_pool.acquire(23);

        FT_ASSERT(static_cast<bool>(moved_object));
        FT_ASSERT_EQ(true, moved_object.is_thread_safe_enabled());
        FT_ASSERT_EQ(23, *moved_object.operator->());
    }
    FT_ASSERT_EQ(0, source_pool.enable_thread_safety());
    FT_ASSERT_EQ(true, source_pool.is_thread_safe_enabled());
    source_pool.resize(2);
    return (1);
}

FT_TEST(test_pool_move_assignment_rebuilds_mutex,
        "Pool move assignment reinitializes mutex and transfers free list")
{
    Pool<int> destination_pool;
    Pool<int> source_pool;

    destination_pool.resize(1);
    FT_ASSERT_EQ(0, destination_pool.enable_thread_safety());
    source_pool.resize(2);
    FT_ASSERT_EQ(0, source_pool.enable_thread_safety());
    {
        Pool<int>::Object first_object = source_pool.acquire(5);

        FT_ASSERT(static_cast<bool>(first_object));
        FT_ASSERT_EQ(5, *first_object.operator->());
    }

    destination_pool = ft_move(source_pool);

    FT_ASSERT_EQ(true, destination_pool.is_thread_safe_enabled());
    FT_ASSERT_EQ(false, source_pool.is_thread_safe_enabled());
    {
        Pool<int>::Object object = destination_pool.acquire(12);

        FT_ASSERT(static_cast<bool>(object));
        FT_ASSERT_EQ(12, *object.operator->());
    }
    FT_ASSERT_EQ(0, source_pool.enable_thread_safety());
    FT_ASSERT_EQ(true, source_pool.is_thread_safe_enabled());
    source_pool.resize(1);
    return (1);
}
