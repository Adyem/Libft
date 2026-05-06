#include "../test_internal.hpp"
#include "../../Modules/Template/unordered_map.hpp"
#include "../../Modules/CPP_class/class_nullptr.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

typedef ft_unordered_map<int, int> unordered_map_int_int;

FT_TEST(test_unordered_map_recursive_mutex_depth_is_balanced)
{
    unordered_map_int_int map_instance;
    ft_bool lock_acquired;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.enable_thread_safety());
    lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.lock(&lock_acquired));
    FT_ASSERT_EQ(FT_TRUE, lock_acquired);
    map_instance.unlock(lock_acquired);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.destroy());
    return (1);
}

FT_TEST(test_unordered_map_methods_leave_mutex_unlocked)
{
    unordered_map_int_int map_instance;
    ft_bool lock_acquired;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.enable_thread_safety());
    map_instance.insert(1, 10);
    map_instance.insert(2, 20);
    (void)map_instance.find(1);
    (void)map_instance.size();
    map_instance.erase(2);
    (void)map_instance.empty();
    lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.lock(&lock_acquired));
    FT_ASSERT_EQ(FT_TRUE, lock_acquired);
    map_instance.unlock(lock_acquired);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.destroy());
    return (1);
}

FT_TEST(test_unordered_map_failure_paths_leave_mutex_unlocked)
{
    unordered_map_int_int map_instance;
    ft_bool lock_acquired;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.enable_thread_safety());
    (void)map_instance.at(404);
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, map_instance.get_error());
    map_instance.insert(1, 10);
    (void)map_instance.at(2);
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, map_instance.get_error());
    lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.lock(&lock_acquired));
    FT_ASSERT_EQ(FT_TRUE, lock_acquired);
    map_instance.unlock(lock_acquired);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, map_instance.destroy());
    return (1);
}
