#include "../test_internal.hpp"
#include "../../Template/pool.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_pool_manual_mutex_rebuild,
        "Pool manual rebuild reinitializes mutex after manual transfer")
{
    Pool<int> source_pool;

    source_pool.resize(1);
    FT_ASSERT_EQ(0, source_pool.enable_thread_safety());
    {
        Pool<int>::Object object = source_pool.acquire(17);

        FT_ASSERT(static_cast<bool>(object));
        FT_ASSERT_EQ(true, object.is_thread_safe());
        FT_ASSERT_EQ(17, *object.operator->());
    }

    Pool<int> manual_pool;
    manual_pool.resize(1);
    FT_ASSERT_EQ(0, manual_pool.enable_thread_safety());
    {
        Pool<int>::Object moved_object = manual_pool.acquire(23);

        FT_ASSERT(static_cast<bool>(moved_object));
        FT_ASSERT_EQ(true, moved_object.is_thread_safe());
        FT_ASSERT_EQ(23, *moved_object.operator->());
    }
    FT_ASSERT_EQ(0, source_pool.disable_thread_safety());
    FT_ASSERT_EQ(false, source_pool.is_thread_safe());
    FT_ASSERT_EQ(0, source_pool.enable_thread_safety());
    FT_ASSERT_EQ(true, source_pool.is_thread_safe());
    source_pool.resize(2);
    return (1);
}

FT_TEST(test_pool_manual_assignment_reinitializes_mutex,
        "Pool manual assignment rebuilds mutex and resets free list")
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

    FT_ASSERT_EQ(0, destination_pool.destroy());
    destination_pool.resize(2);
    FT_ASSERT_EQ(0, destination_pool.enable_thread_safety());
    {
        Pool<int>::Object object = destination_pool.acquire(12);

        FT_ASSERT(static_cast<bool>(object));
        FT_ASSERT_EQ(12, *object.operator->());
    }
    FT_ASSERT_EQ(0, source_pool.disable_thread_safety());
    FT_ASSERT_EQ(false, source_pool.is_thread_safe());
    FT_ASSERT_EQ(0, source_pool.enable_thread_safety());
    FT_ASSERT_EQ(true, source_pool.is_thread_safe());
    source_pool.resize(1);
    return (1);
}
