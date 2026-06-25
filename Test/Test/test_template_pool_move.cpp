#include "../test_internal.hpp"
#include "../../Modules/Template/pool.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Errno/errno.hpp"

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#include "../../Modules/Template/vector.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_pool_move_preserves_thread_safety_and_objects)
{
    Pool<int> source_pool;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_pool.initialize());
    source_pool.resize(1);
    FT_ASSERT_EQ(0, source_pool.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_pool.get_error());
    {
        Pool<int>::Object object = source_pool.acquire(17);

        FT_ASSERT(static_cast<bool>(object));
        FT_ASSERT_EQ(true, object.is_thread_safe());
        FT_ASSERT_EQ(17, *object.operator->());
    }
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_pool.get_error());

    Pool<int> moved_pool;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_pool.move(source_pool));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_pool.get_error());
    FT_ASSERT_EQ(true, moved_pool.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_pool.get_error());
    {
        Pool<int>::Object moved_object = moved_pool.acquire(23);

        FT_ASSERT(static_cast<bool>(moved_object));
        FT_ASSERT_EQ(true, moved_object.is_thread_safe());
        FT_ASSERT_EQ(23, *moved_object.operator->());
    }
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_pool.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_pool.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_pool.get_error());
    source_pool.resize(2);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_pool.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_pool.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_pool.destroy());
    return (1);
}

FT_TEST(test_pool_move_into_initialized_destination_preserves_source_thread_safety)
{
    Pool<int> destination_pool;
    Pool<int> source_pool;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_pool.initialize());
    destination_pool.resize(1);
    FT_ASSERT_EQ(0, destination_pool.enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_pool.initialize());
    source_pool.resize(2);
    {
        Pool<int>::Object first_object = source_pool.acquire(5);

        FT_ASSERT(static_cast<bool>(first_object));
        FT_ASSERT_EQ(5, *first_object.operator->());
    }
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_pool.get_error());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_pool.move(source_pool));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_pool.get_error());
    FT_ASSERT_EQ(false, destination_pool.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_pool.get_error());
    {
        Pool<int>::Object object = destination_pool.acquire(12);

        FT_ASSERT(static_cast<bool>(object));
        FT_ASSERT_EQ(12, *object.operator->());
    }
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_pool.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_pool.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_pool.get_error());
    source_pool.resize(1);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_pool.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_pool.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_pool.destroy());
    return (1);
}
