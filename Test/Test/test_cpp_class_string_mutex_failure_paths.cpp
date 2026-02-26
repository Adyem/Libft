#include "../test_internal.hpp"
#include "../../CPP_class/class_string.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../CPP_class/class_nullptr.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_ft_string_mutex_unlocked_after_append_nullptr,
    "ft_string append nullptr leaves mutex unlocked")
{
    ft_string string_value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, string_value.initialize("data"));
    pt_recursive_mutex *mutex_pointer;

    mutex_pointer = string_value.get_mutex_for_validation();
    FT_ASSERT(mutex_pointer != ft_nullptr);
    (void)string_value.append(ft_nullptr);
    FT_ASSERT_EQ(false, mutex_pointer->lockState());
    return (1);
}

FT_TEST(test_ft_string_mutex_unlocked_after_assign_nullptr_nonzero,
    "ft_string assign nullptr non-zero length leaves mutex unlocked")
{
    ft_string string_value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, string_value.initialize("data"));
    pt_recursive_mutex *mutex_pointer;

    mutex_pointer = string_value.get_mutex_for_validation();
    FT_ASSERT(mutex_pointer != ft_nullptr);
    (void)string_value.assign(ft_nullptr, 3);
    FT_ASSERT_EQ(false, mutex_pointer->lockState());
    return (1);
}

FT_TEST(test_ft_string_mutex_unlocked_after_find_nullptr,
    "ft_string find nullptr leaves mutex unlocked")
{
    ft_string string_value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, string_value.initialize("data"));
    pt_recursive_mutex *mutex_pointer;

    mutex_pointer = string_value.get_mutex_for_validation();
    FT_ASSERT(mutex_pointer != ft_nullptr);
    FT_ASSERT_EQ(ft_string::npos, string_value.find(ft_nullptr));
    FT_ASSERT_EQ(false, mutex_pointer->lockState());
    return (1);
}

FT_TEST(test_ft_string_mutex_unlocked_after_erase_out_of_range,
    "ft_string erase out-of-range leaves mutex unlocked")
{
    ft_string string_value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, string_value.initialize("data"));
    pt_recursive_mutex *mutex_pointer;

    mutex_pointer = string_value.get_mutex_for_validation();
    FT_ASSERT(mutex_pointer != ft_nullptr);
    (void)string_value.erase(100, 2);
    FT_ASSERT_EQ(false, mutex_pointer->lockState());
    return (1);
}

FT_TEST(test_ft_string_mutex_unlocked_after_at_out_of_range,
    "ft_string at out-of-range leaves mutex unlocked")
{
    ft_string string_value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, string_value.initialize("data"));
    pt_recursive_mutex *mutex_pointer;

    mutex_pointer = string_value.get_mutex_for_validation();
    FT_ASSERT(mutex_pointer != ft_nullptr);
    FT_ASSERT(string_value.at(100) == ft_nullptr);
    FT_ASSERT_EQ(false, mutex_pointer->lockState());
    return (1);
}

FT_TEST(test_ft_string_mutex_unlocked_after_back_on_empty,
    "ft_string back on empty string leaves mutex unlocked")
{
    ft_string string_value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, string_value.initialize(""));
    pt_recursive_mutex *mutex_pointer;

    mutex_pointer = string_value.get_mutex_for_validation();
    FT_ASSERT(mutex_pointer != ft_nullptr);
    FT_ASSERT_EQ('\0', string_value.back());
    FT_ASSERT_EQ(false, mutex_pointer->lockState());
    return (1);
}

FT_TEST(test_ft_string_mutex_unlocked_after_append_string_success,
    "ft_string append string success leaves mutex unlocked")
{
    ft_string string_value;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, string_value.initialize("a"));
    ft_string suffix;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, suffix.initialize("b"));
    pt_recursive_mutex *mutex_pointer;

    mutex_pointer = string_value.get_mutex_for_validation();
    FT_ASSERT(mutex_pointer != ft_nullptr);
    (void)string_value.append(suffix);
    FT_ASSERT_EQ(false, mutex_pointer->lockState());
    FT_ASSERT(string_value == "ab");
    return (1);
}
