#include "../test_internal.hpp"
#include "../../Template/unordered_map.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

typedef ft_unordered_map<int, int> unordered_map_int_int;

static int unordered_map_expect_recursive_mutex_usable(pt_recursive_mutex *mutex_pointer)
{
    int lock_error;
    int unlock_error;

    FT_ASSERT(mutex_pointer != ft_nullptr);
    FT_ASSERT_EQ(false, mutex_pointer->lockState());
    lock_error = mutex_pointer->lock();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, lock_error);
    lock_error = mutex_pointer->lock();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, lock_error);
    FT_ASSERT_EQ(true, mutex_pointer->lockState());
    unlock_error = mutex_pointer->unlock();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, unlock_error);
    FT_ASSERT_EQ(true, mutex_pointer->lockState());
    unlock_error = mutex_pointer->unlock();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, unlock_error);
    FT_ASSERT_EQ(false, mutex_pointer->lockState());
    return (1);
}

FT_TEST(test_unordered_map_recursive_mutex_depth_is_balanced,
    "unordered_map recursive mutex supports nested lock and unlock")
{
    unordered_map_int_int map_instance;
    pt_recursive_mutex *mutex_pointer;

    mutex_pointer = map_instance.get_mutex_for_validation();
    FT_ASSERT_EQ(1, unordered_map_expect_recursive_mutex_usable(mutex_pointer));
    return (1);
}

FT_TEST(test_unordered_map_methods_leave_mutex_unlocked,
    "unordered_map methods leave recursive mutex unlocked on return")
{
    unordered_map_int_int map_instance;
    pt_recursive_mutex *mutex_pointer;

    map_instance.insert(1, 10);
    map_instance.insert(2, 20);
    (void)map_instance.find(1);
    (void)map_instance.size();
    map_instance.erase(2);
    (void)map_instance.empty();
    mutex_pointer = map_instance.get_mutex_for_validation();
    FT_ASSERT_EQ(1, unordered_map_expect_recursive_mutex_usable(mutex_pointer));
    return (1);
}

FT_TEST(test_unordered_map_failure_paths_leave_mutex_unlocked,
    "unordered_map failure paths leave recursive mutex unlocked")
{
    unordered_map_int_int map_instance;
    pt_recursive_mutex *mutex_pointer;

    (void)map_instance.at(404);
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, map_instance.last_operation_error());
    map_instance.insert(1, 10);
    (void)map_instance.at(2);
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, map_instance.last_operation_error());
    mutex_pointer = map_instance.get_mutex_for_validation();
    FT_ASSERT_EQ(1, unordered_map_expect_recursive_mutex_usable(mutex_pointer));
    return (1);
}
