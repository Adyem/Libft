#include "../test_internal.hpp"
#include "../../CPP_class/class_string.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_ft_string_constructors_unlock_mutex,
        "ft_string constructors and assignments unlock mutexes")
{
    ft_string default_string;
    ft_string init_string("value");
    ft_string copy_string(init_string);
    ft_string assigned_string;
    pt_recursive_mutex *default_mutex;
    pt_recursive_mutex *init_mutex;
    pt_recursive_mutex *copy_mutex;
    pt_recursive_mutex *assigned_mutex;

    assigned_string = init_string;
    default_mutex = default_string.get_mutex_for_validation();
    init_mutex = init_string.get_mutex_for_validation();
    copy_mutex = copy_string.get_mutex_for_validation();
    assigned_mutex = assigned_string.get_mutex_for_validation();

    FT_ASSERT(default_mutex != ft_nullptr);
    FT_ASSERT(init_mutex != ft_nullptr);
    FT_ASSERT(copy_mutex != ft_nullptr);
    FT_ASSERT(assigned_mutex != ft_nullptr);

    FT_ASSERT_EQ(false, default_mutex->lockState());
    FT_ASSERT_EQ(false, init_mutex->lockState());
    FT_ASSERT_EQ(false, copy_mutex->lockState());
    FT_ASSERT_EQ(false, assigned_mutex->lockState());
    ft_errno = FT_ERR_SUCCESS;
    return (1);
}

FT_TEST(test_ft_string_mutators_unlock_mutex,
        "ft_string mutator methods unlock mutexes")
{
    ft_string string_value("start");
    ft_string erase_target("sample");
    pt_recursive_mutex *string_mutex;
    pt_recursive_mutex *erase_mutex;

    string_value.append('X');
    string_mutex = string_value.get_mutex_for_validation();
    FT_ASSERT(string_mutex != ft_nullptr);
    FT_ASSERT_EQ(false, string_mutex->lockState());

    string_value.append("YZ");
    string_mutex = string_value.get_mutex_for_validation();
    FT_ASSERT(string_mutex != ft_nullptr);
    FT_ASSERT_EQ(false, string_mutex->lockState());

    string_value.assign(3, 'a');
    string_mutex = string_value.get_mutex_for_validation();
    FT_ASSERT(string_mutex != ft_nullptr);
    FT_ASSERT_EQ(false, string_mutex->lockState());

    string_value.resize_length(2);
    string_mutex = string_value.get_mutex_for_validation();
    FT_ASSERT(string_mutex != ft_nullptr);
    FT_ASSERT_EQ(false, string_mutex->lockState());

    erase_target.erase(1, 2);
    erase_mutex = erase_target.get_mutex_for_validation();
    FT_ASSERT(erase_mutex != ft_nullptr);
    FT_ASSERT_EQ(false, erase_mutex->lockState());

    string_value.clear();
    string_mutex = string_value.get_mutex_for_validation();
    FT_ASSERT(string_mutex != ft_nullptr);
    FT_ASSERT_EQ(false, string_mutex->lockState());
    ft_errno = FT_ERR_SUCCESS;
    return (1);
}

FT_TEST(test_ft_string_observers_unlock_mutex,
        "ft_string observer methods unlock mutexes")
{
    ft_string string_value("checking");
    ft_string substring;
    pt_recursive_mutex *string_mutex;
    pt_recursive_mutex *substring_mutex;
    const char *c_string;
    char *data_pointer;
    size_t size_value;
    size_t find_result;

    c_string = string_value.c_str();
    string_mutex = string_value.get_mutex_for_validation();
    FT_ASSERT(string_mutex != ft_nullptr);
    FT_ASSERT(c_string != ft_nullptr);
    FT_ASSERT_EQ(false, string_mutex->lockState());

    data_pointer = string_value.data();
    string_mutex = string_value.get_mutex_for_validation();
    FT_ASSERT(string_mutex != ft_nullptr);
    FT_ASSERT(data_pointer != ft_nullptr);
    FT_ASSERT_EQ(false, string_mutex->lockState());

    size_value = string_value.size();
    string_mutex = string_value.get_mutex_for_validation();
    FT_ASSERT(string_mutex != ft_nullptr);
    FT_ASSERT_EQ(8, size_value);
    FT_ASSERT_EQ(false, string_mutex->lockState());

    find_result = string_value.find("ck");
    string_mutex = string_value.get_mutex_for_validation();
    FT_ASSERT(string_mutex != ft_nullptr);
    FT_ASSERT_EQ(2, find_result);
    FT_ASSERT_EQ(false, string_mutex->lockState());

    substring = string_value.substr(0, 4);
    substring_mutex = substring.get_mutex_for_validation();
    FT_ASSERT(substring_mutex != ft_nullptr);
    FT_ASSERT_EQ(false, substring_mutex->lockState());

    FT_ASSERT_EQ(false, string_value.empty());
    string_mutex = string_value.get_mutex_for_validation();
    FT_ASSERT(string_mutex != ft_nullptr);
    FT_ASSERT_EQ(false, string_mutex->lockState());
    ft_errno = FT_ERR_SUCCESS;
    return (1);
}

FT_TEST(test_ft_string_erase_out_of_range_unlocks_mutex,
        "ft_string erase unlocks mutex after out of range index")
{
    ft_string string_value("erase");
    pt_recursive_mutex *string_mutex;

    string_value.erase(10, 1);
    string_mutex = string_value.get_mutex_for_validation();

    FT_ASSERT(string_mutex != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, ft_string::last_operation_error());
    FT_ASSERT_EQ(false, string_mutex->lockState());
    ft_errno = FT_ERR_SUCCESS;
    return (1);
}

FT_TEST(test_ft_string_substr_out_of_range_unlocks_mutex,
        "ft_string substr unlocks mutex when index is out of range")
{
    ft_string source_string("short");
    ft_string substring;
    pt_recursive_mutex *source_mutex;
    pt_recursive_mutex *substring_mutex;

    substring = source_string.substr(20, 3);
    source_mutex = source_string.get_mutex_for_validation();
    substring_mutex = substring.get_mutex_for_validation();

    FT_ASSERT(source_mutex != ft_nullptr);
    FT_ASSERT(substring_mutex != ft_nullptr);

    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, ft_string::last_operation_error());
    FT_ASSERT_EQ(false, source_mutex->lockState());
    FT_ASSERT_EQ(false, substring_mutex->lockState());
    ft_errno = FT_ERR_SUCCESS;
    return (1);
}

FT_TEST(test_ft_string_at_out_of_range_unlocks_mutex,
        "ft_string at unlocks mutex when index is invalid")
{
    ft_string string_value("index");
    const char *result;
    pt_recursive_mutex *string_mutex;

    result = string_value.at(25);
    string_mutex = string_value.get_mutex_for_validation();

    FT_ASSERT(string_mutex != ft_nullptr);
    FT_ASSERT(result == ft_nullptr);
    FT_ASSERT_EQ(false, string_mutex->lockState());
    ft_errno = FT_ERR_SUCCESS;
    return (1);
}

FT_TEST(test_ft_string_back_empty_unlocks_mutex,
        "ft_string back unlocks mutex when string is empty")
{
    ft_string empty_string;
    pt_recursive_mutex *string_mutex;
    char last_character;

    last_character = empty_string.back();
    string_mutex = empty_string.get_mutex_for_validation();

    FT_ASSERT(string_mutex != ft_nullptr);
    FT_ASSERT_EQ('\0', last_character);
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, ft_string::last_operation_error());
    FT_ASSERT_EQ(false, string_mutex->lockState());
    ft_errno = FT_ERR_SUCCESS;
    return (1);
}

FT_TEST(test_ft_string_find_nullptr_unlocks_mutex,
        "ft_string find unlocks mutex when substring is null")
{
    ft_string string_value("content");
    pt_recursive_mutex *string_mutex;
    size_t find_result;

    find_result = string_value.find(ft_nullptr);
    string_mutex = string_value.get_mutex_for_validation();

    FT_ASSERT(string_mutex != ft_nullptr);
    FT_ASSERT_EQ(ft_string::npos, find_result);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_string::last_operation_error());
    FT_ASSERT_EQ(false, string_mutex->lockState());
    ft_errno = FT_ERR_SUCCESS;
    return (1);
}

FT_TEST(test_ft_string_append_nullptr_unlocks_mutex,
        "ft_string append unlocks mutex when input is null")
{
    ft_string string_value("base");
    pt_recursive_mutex *string_mutex;

    string_value.append(ft_nullptr);
    string_mutex = string_value.get_mutex_for_validation();

    FT_ASSERT(string_mutex != ft_nullptr);
    FT_ASSERT_EQ(false, string_mutex->lockState());
    FT_ASSERT_EQ(4, string_value.size());
    ft_errno = FT_ERR_SUCCESS;
    return (1);
}

FT_TEST(test_ft_string_push_back_unlocks_mutex,
        "ft_string push_back unlocks mutex after successful append")
{
    ft_string string_value("xy");
    pt_recursive_mutex *string_mutex;

    string_value.push_back('z');
    string_mutex = string_value.get_mutex_for_validation();

    FT_ASSERT(string_mutex != ft_nullptr);
    FT_ASSERT_EQ(false, string_mutex->lockState());
    FT_ASSERT_EQ(3, string_value.size());
    FT_ASSERT_EQ('z', string_value.back());
    ft_errno = FT_ERR_SUCCESS;
    return (1);
}

FT_TEST(test_ft_string_addition_unlocks_mutex,
        "ft_string addition unlocks mutex on all operands")
{
    ft_string left_string("left");
    ft_string right_string("right");
    ft_string combined_string;
    pt_recursive_mutex *left_mutex;
    pt_recursive_mutex *right_mutex;
    pt_recursive_mutex *combined_mutex;

    combined_string = left_string + right_string;
    left_mutex = left_string.get_mutex_for_validation();
    right_mutex = right_string.get_mutex_for_validation();
    combined_mutex = combined_string.get_mutex_for_validation();

    FT_ASSERT(left_mutex != ft_nullptr);
    FT_ASSERT(right_mutex != ft_nullptr);
    FT_ASSERT(combined_mutex != ft_nullptr);

    FT_ASSERT_EQ(false, left_mutex->lockState());
    FT_ASSERT_EQ(false, right_mutex->lockState());
    FT_ASSERT_EQ(false, combined_mutex->lockState());
    FT_ASSERT_EQ(9, combined_string.size());
    ft_errno = FT_ERR_SUCCESS;
    return (1);
}

FT_TEST(test_ft_string_assign_length_nullptr_unlocks_mutex,
        "ft_string assign unlocks mutex when input is null pointer with length")
{
    ft_string target_string;
    pt_recursive_mutex *target_mutex;

    target_string.assign(ft_nullptr, 3);
    target_mutex = target_string.get_mutex_for_validation();

    FT_ASSERT(target_mutex != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_string::last_operation_error());
    FT_ASSERT_EQ(false, target_mutex->lockState());
    ft_errno = FT_ERR_SUCCESS;
    return (1);
}

FT_TEST(test_ft_string_resize_length_expand_unlocks_mutex,
        "ft_string resize_length unlocks mutex after expansion")
{
    ft_string target_string("abc");
    pt_recursive_mutex *target_mutex;

    target_string.resize_length(5);
    target_mutex = target_string.get_mutex_for_validation();

    FT_ASSERT(target_mutex != ft_nullptr);
    FT_ASSERT_EQ(false, target_mutex->lockState());
    FT_ASSERT_EQ(5, target_string.size());
    ft_errno = FT_ERR_SUCCESS;
    return (1);
}

FT_TEST(test_ft_string_append_object_unlocks_mutex,
        "ft_string append unlocks mutex when appending another string")
{
    ft_string base_string("base");
    ft_string add_string("add");
    pt_recursive_mutex *base_mutex;

    base_string.append(add_string);
    base_mutex = base_string.get_mutex_for_validation();

    FT_ASSERT(base_mutex != ft_nullptr);
    FT_ASSERT_EQ(false, base_mutex->lockState());
    FT_ASSERT_EQ(7, base_string.size());
    ft_errno = FT_ERR_SUCCESS;
    return (1);
}

FT_TEST(test_ft_string_replace_out_of_range_unlocks_mutex,
        "ft_string assign unlocks mutex when replacing out of range")
{
    ft_string target_string("value");
    pt_recursive_mutex *target_mutex;

    target_string.erase(20, 2);
    target_mutex = target_string.get_mutex_for_validation();

    FT_ASSERT(target_mutex != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, ft_string::last_operation_error());
    FT_ASSERT_EQ(false, target_mutex->lockState());
    ft_errno = FT_ERR_SUCCESS;
    return (1);
}

FT_TEST(test_ft_string_operator_plus_char_unlocks_mutex,
        "ft_string operator plus with character unlocks mutexes")
{
    ft_string base_string("ok");
    ft_string combined_string;
    pt_recursive_mutex *base_mutex;
    pt_recursive_mutex *combined_mutex;

    combined_string = base_string + '!';
    base_mutex = base_string.get_mutex_for_validation();
    combined_mutex = combined_string.get_mutex_for_validation();

    FT_ASSERT(base_mutex != ft_nullptr);
    FT_ASSERT(combined_mutex != ft_nullptr);

    FT_ASSERT_EQ(false, base_mutex->lockState());
    FT_ASSERT_EQ(false, combined_mutex->lockState());
    FT_ASSERT_EQ(3, combined_string.size());
    FT_ASSERT_EQ('!', combined_string.back());
    ft_errno = FT_ERR_SUCCESS;
    return (1);
}

FT_TEST(test_ft_string_assignment_from_nullptr_unlocks_mutex,
        "ft_string operator assignment unlocks mutex on null pointer input")
{
    ft_string target_string("value");
    pt_recursive_mutex *target_mutex;

    target_string = ft_nullptr;
    target_mutex = target_string.get_mutex_for_validation();

    FT_ASSERT(target_mutex != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_string::last_operation_error());
    FT_ASSERT_EQ(false, target_mutex->lockState());
    ft_errno = FT_ERR_SUCCESS;
    return (1);
}

FT_TEST(test_ft_string_operator_plus_equals_cstr_unlocks_mutex,
        "ft_string operator plus equals unlocks mutex when c-string is null")
{
    ft_string base_string("base");
    pt_recursive_mutex *base_mutex;

    base_string += ft_nullptr;
    base_mutex = base_string.get_mutex_for_validation();

    FT_ASSERT(base_mutex != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_string::last_operation_error());
    FT_ASSERT_EQ(false, base_mutex->lockState());
    ft_errno = FT_ERR_SUCCESS;
    return (1);
}

FT_TEST(test_ft_string_operator_plus_equals_object_unlocks_mutex,
        "ft_string operator plus equals unlocks mutex when adding object")
{
    ft_string base_string("hi");
    ft_string append_string("there");
    pt_recursive_mutex *base_mutex;

    base_string += append_string;
    base_mutex = base_string.get_mutex_for_validation();

    FT_ASSERT(base_mutex != ft_nullptr);
    FT_ASSERT_EQ(false, base_mutex->lockState());
    FT_ASSERT_EQ(7, base_string.size());
    ft_errno = FT_ERR_SUCCESS;
    return (1);
}

FT_TEST(test_ft_string_conversion_operator_unlocks_mutex,
        "ft_string conversion operator unlocks mutex after use")
{
    ft_string base_string("convert");
    const char *c_string;
    pt_recursive_mutex *base_mutex;

    c_string = static_cast<const char *>(base_string);
    base_mutex = base_string.get_mutex_for_validation();

    FT_ASSERT(base_mutex != ft_nullptr);
    FT_ASSERT(c_string != ft_nullptr);
    FT_ASSERT_EQ(false, base_mutex->lockState());
    ft_errno = FT_ERR_SUCCESS;
    return (1);
}

FT_TEST(test_ft_string_operator_plus_cstring_left_unlocks_mutex,
        "ft_string operator plus with c-string left operand unlocks mutexes")
{
    ft_string right_string("right");
    ft_string combined_string;
    pt_recursive_mutex *right_mutex;
    pt_recursive_mutex *combined_mutex;

    combined_string = "left" + right_string;
    right_mutex = right_string.get_mutex_for_validation();
    combined_mutex = combined_string.get_mutex_for_validation();

    FT_ASSERT(right_mutex != ft_nullptr);
    FT_ASSERT(combined_mutex != ft_nullptr);

    FT_ASSERT_EQ(false, right_mutex->lockState());
    FT_ASSERT_EQ(false, combined_mutex->lockState());
    FT_ASSERT_EQ(9, combined_string.size());
    ft_errno = FT_ERR_SUCCESS;
    return (1);
}
