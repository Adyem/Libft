#include "../test_internal.hpp"
#include "../../CPP_class/class_string.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

static int string_expect_recursive_mutex_usable(pt_recursive_mutex *mutex_pointer)
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

FT_TEST(test_ft_string_recursive_mutex_depth_is_balanced,
    "ft_string recursive mutex supports nested lock and unlock")
{
    ft_string string_value;
    pt_recursive_mutex *mutex_pointer;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, string_value.initialize());
    mutex_pointer = string_value.get_mutex_for_validation();
    FT_ASSERT_EQ(1, string_expect_recursive_mutex_usable(mutex_pointer));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, string_value.destroy());
    return (1);
}

FT_TEST(test_ft_string_methods_leave_mutex_unlocked,
    "ft_string methods leave recursive mutex unlocked on return")
{
    ft_string string_value;
    pt_recursive_mutex *mutex_pointer;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, string_value.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, string_value.append('a'));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, string_value.assign(3, 'z'));
    (void)string_value.size();
    (void)string_value.empty();
    mutex_pointer = string_value.get_mutex_for_validation();
    FT_ASSERT_EQ(1, string_expect_recursive_mutex_usable(mutex_pointer));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, string_value.destroy());
    return (1);
}

FT_TEST(test_ft_string_failure_paths_leave_mutex_unlocked,
    "ft_string failure paths leave recursive mutex unlocked")
{
    ft_string string_value;
    pt_recursive_mutex *mutex_pointer;
    const char *null_pointer;

    null_pointer = ft_nullptr;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, string_value.initialize());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, string_value.append(null_pointer));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, string_value.assign(null_pointer, 3));
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, string_value.erase(100, 1));
    (void)string_value.at(100);
    mutex_pointer = string_value.get_mutex_for_validation();
    FT_ASSERT_EQ(1, string_expect_recursive_mutex_usable(mutex_pointer));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, string_value.destroy());
    return (1);
}
