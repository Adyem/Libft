#include "../../CPP_class/class_big_number.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

FT_TEST(test_big_number_mutex_construction_and_assignment_unlocks, "ft_big_number constructors and assignments unlock mutexes")
{
    ft_big_number default_number;
    ft_big_number source_number;
    ft_big_number copy_number(source_number);
    ft_big_number moved_number(ft_move(source_number));
    ft_big_number assigned_number;
    ft_big_number move_assigned_number;
    pt_recursive_mutex *default_mutex;
    pt_recursive_mutex *copy_mutex;
    pt_recursive_mutex *moved_mutex;
    pt_recursive_mutex *assigned_mutex;
    pt_recursive_mutex *move_assigned_mutex;

    source_number.assign("42");
    assigned_number = source_number;
    move_assigned_number = ft_move(source_number);
    default_mutex = default_number.get_mutex_for_testing();
    copy_mutex = copy_number.get_mutex_for_testing();
    moved_mutex = moved_number.get_mutex_for_testing();
    assigned_mutex = assigned_number.get_mutex_for_testing();
    move_assigned_mutex = move_assigned_number.get_mutex_for_testing();

    FT_ASSERT(default_mutex != ft_nullptr);
    FT_ASSERT(copy_mutex != ft_nullptr);
    FT_ASSERT(moved_mutex != ft_nullptr);
    FT_ASSERT(assigned_mutex != ft_nullptr);
    FT_ASSERT(move_assigned_mutex != ft_nullptr);

    FT_ASSERT_EQ(false, default_mutex->lockState());
    FT_ASSERT_EQ(false, copy_mutex->lockState());
    FT_ASSERT_EQ(false, moved_mutex->lockState());
    FT_ASSERT_EQ(false, assigned_mutex->lockState());
    FT_ASSERT_EQ(false, move_assigned_mutex->lockState());
    ft_errno = FT_ERR_SUCCESSS;
    return (1);
}

FT_TEST(test_big_number_mutex_arithmetic_unlocks, "ft_big_number arithmetic operators unlock mutexes")
{
    ft_big_number left_number;
    ft_big_number right_number;
    ft_big_number sum_number;
    ft_big_number difference_number;
    ft_big_number product_number;
    ft_big_number quotient_number;
    ft_big_number remainder_number;
    pt_recursive_mutex *left_mutex;
    pt_recursive_mutex *right_mutex;
    pt_recursive_mutex *sum_mutex;
    pt_recursive_mutex *difference_mutex;
    pt_recursive_mutex *product_mutex;
    pt_recursive_mutex *quotient_mutex;
    pt_recursive_mutex *remainder_mutex;

    left_number.assign("144");
    right_number.assign("12");
    sum_number = left_number + right_number;
    difference_number = left_number - right_number;
    product_number = left_number * right_number;
    quotient_number = left_number / right_number;
    remainder_number = left_number % right_number;
    left_mutex = left_number.get_mutex_for_testing();
    right_mutex = right_number.get_mutex_for_testing();
    sum_mutex = sum_number.get_mutex_for_testing();
    difference_mutex = difference_number.get_mutex_for_testing();
    product_mutex = product_number.get_mutex_for_testing();
    quotient_mutex = quotient_number.get_mutex_for_testing();
    remainder_mutex = remainder_number.get_mutex_for_testing();

    FT_ASSERT(left_mutex != ft_nullptr);
    FT_ASSERT(right_mutex != ft_nullptr);
    FT_ASSERT(sum_mutex != ft_nullptr);
    FT_ASSERT(difference_mutex != ft_nullptr);
    FT_ASSERT(product_mutex != ft_nullptr);
    FT_ASSERT(quotient_mutex != ft_nullptr);
    FT_ASSERT(remainder_mutex != ft_nullptr);

    FT_ASSERT_EQ(false, left_mutex->lockState());
    FT_ASSERT_EQ(false, right_mutex->lockState());
    FT_ASSERT_EQ(false, sum_mutex->lockState());
    FT_ASSERT_EQ(false, difference_mutex->lockState());
    FT_ASSERT_EQ(false, product_mutex->lockState());
    FT_ASSERT_EQ(false, quotient_mutex->lockState());
    FT_ASSERT_EQ(false, remainder_mutex->lockState());
    ft_errno = FT_ERR_SUCCESSS;
    return (1);
}

FT_TEST(test_big_number_mutex_comparisons_unlock, "ft_big_number comparison operators unlock mutexes")
{
    ft_big_number smaller_number;
    ft_big_number larger_number;
    pt_recursive_mutex *smaller_mutex;
    pt_recursive_mutex *larger_mutex;
    bool equality_result;
    bool inequality_result;
    bool less_result;
    bool less_or_equal_result;
    bool greater_result;
    bool greater_or_equal_result;

    smaller_number.assign("7");
    larger_number.assign("21");
    equality_result = smaller_number == larger_number;
    inequality_result = smaller_number != larger_number;
    less_result = smaller_number < larger_number;
    less_or_equal_result = smaller_number <= larger_number;
    greater_result = larger_number > smaller_number;
    greater_or_equal_result = larger_number >= smaller_number;
    smaller_mutex = smaller_number.get_mutex_for_testing();
    larger_mutex = larger_number.get_mutex_for_testing();

    FT_ASSERT(smaller_mutex != ft_nullptr);
    FT_ASSERT(larger_mutex != ft_nullptr);

    FT_ASSERT_EQ(false, smaller_mutex->lockState());
    FT_ASSERT_EQ(false, larger_mutex->lockState());
    FT_ASSERT_EQ(false, equality_result);
    FT_ASSERT_EQ(true, inequality_result);
    FT_ASSERT_EQ(true, less_result);
    FT_ASSERT_EQ(true, less_or_equal_result);
    FT_ASSERT_EQ(true, greater_result);
    FT_ASSERT_EQ(true, greater_or_equal_result);
    ft_errno = FT_ERR_SUCCESSS;
    return (1);
}

FT_TEST(test_big_number_mutex_mutators_unlock, "ft_big_number mutator methods unlock mutexes")
{
    ft_big_number number;
    pt_recursive_mutex *number_mutex;

    number.assign("1000");
    number.assign_base("FF", 16);
    number.append_digit('0');
    number.append("1");
    number.append_unsigned(5);
    number.trim_leading_zeros();
    number.reduce_to(2);
    number.clear();
    number_mutex = number.get_mutex_for_testing();

    FT_ASSERT(number_mutex != ft_nullptr);

    FT_ASSERT_EQ(false, number_mutex->lockState());
    FT_ASSERT_EQ(true, number.empty());
    ft_errno = FT_ERR_SUCCESSS;
    return (1);
}

FT_TEST(test_big_number_mutex_observers_unlock, "ft_big_number observer methods unlock mutexes")
{
    ft_big_number positive_number;
    ft_big_number negative_number;
    ft_big_number modulus_number;
    ft_big_number exponent_number;
    ft_big_number base_number;
    ft_big_number mod_pow_result;
    pt_recursive_mutex *positive_mutex;
    pt_recursive_mutex *negative_mutex;
    pt_recursive_mutex *result_mutex;
    const char *string_value;
    ft_string hex_string;
    bool empty_result;
    bool negative_flag;
    bool positive_flag;
    ft_size_t size_value;

    positive_number.assign("255");
    base_number.assign("4");
    exponent_number.assign("2");
    modulus_number.assign("5");
    negative_number.assign("-9");
    mod_pow_result = base_number.mod_pow(exponent_number, modulus_number);
    string_value = positive_number.c_str();
    hex_string = positive_number.to_string_base(16);
    empty_result = positive_number.empty();
    negative_flag = negative_number.is_negative();
    positive_flag = positive_number.is_positive();
    size_value = positive_number.size();
    positive_mutex = positive_number.get_mutex_for_testing();
    negative_mutex = negative_number.get_mutex_for_testing();
    result_mutex = mod_pow_result.get_mutex_for_testing();

    FT_ASSERT(positive_mutex != ft_nullptr);
    FT_ASSERT(negative_mutex != ft_nullptr);
    FT_ASSERT(result_mutex != ft_nullptr);
    FT_ASSERT(string_value != ft_nullptr);

    FT_ASSERT_EQ(false, positive_mutex->lockState());
    FT_ASSERT_EQ(false, negative_mutex->lockState());
    FT_ASSERT_EQ(false, result_mutex->lockState());
    FT_ASSERT_EQ(false, empty_result);
    FT_ASSERT_EQ(true, negative_flag);
    FT_ASSERT_EQ(true, positive_flag);
    FT_ASSERT_EQ(3, size_value);
    FT_ASSERT_EQ('F', hex_string[0]);
    FT_ASSERT_EQ('F', hex_string[1]);
    ft_errno = FT_ERR_SUCCESSS;
    return (1);
}

FT_TEST(test_big_number_mutex_error_helpers_unlock, "ft_big_number error helpers unlock mutexes")
{
    ft_big_number number;
    pt_recursive_mutex *number_mutex;
    int error_value;
    const char *error_string;

    number.append_digit('X');
    error_value = ft_big_number::last_operation_error();
    error_string = ft_big_number::last_operation_error_str();
    number.reset_system_error();
    number_mutex = number.get_mutex_for_testing();

    FT_ASSERT(number_mutex != ft_nullptr);
    FT_ASSERT(error_string != ft_nullptr);

    FT_ASSERT_EQ(false, number_mutex->lockState());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, error_value);
    ft_errno = FT_ERR_SUCCESSS;
    return (1);
}

FT_TEST(test_big_number_mutex_division_unlocks_on_divide_by_zero, "ft_big_number division unlocks mutexes after divide by zero")
{
    ft_big_number dividend_number;
    ft_big_number divisor_number;
    pt_recursive_mutex *dividend_mutex;
    pt_recursive_mutex *divisor_mutex;
    ft_big_number quotient_result;

    dividend_number.assign("100");
    dividend_mutex = dividend_number.get_mutex_for_testing();
    divisor_mutex = divisor_number.get_mutex_for_testing();

    FT_ASSERT(dividend_mutex != ft_nullptr);
    FT_ASSERT(divisor_mutex != ft_nullptr);

    quotient_result = dividend_number / divisor_number;

    FT_ASSERT_EQ(FT_ERR_DIVIDE_BY_ZERO, ft_big_number::last_operation_error());
    FT_ASSERT_EQ(false, dividend_mutex->lockState());
    FT_ASSERT_EQ(false, divisor_mutex->lockState());
    ft_errno = FT_ERR_SUCCESSS;
    return (1);
}

FT_TEST(test_big_number_mutex_modulus_unlocks_on_operand_error, "ft_big_number modulus unlocks mutexes when operands are already in error")
{
    ft_big_number invalid_left_number;
    ft_big_number right_number;
    pt_recursive_mutex *left_mutex;
    pt_recursive_mutex *right_mutex;
    ft_big_number remainder_result;

    right_number.assign("2");
    invalid_left_number.append_digit('x');
    left_mutex = invalid_left_number.get_mutex_for_testing();
    right_mutex = right_number.get_mutex_for_testing();

    FT_ASSERT(left_mutex != ft_nullptr);
    FT_ASSERT(right_mutex != ft_nullptr);

    remainder_result = invalid_left_number % right_number;

    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_big_number::last_operation_error());
    FT_ASSERT_EQ(false, left_mutex->lockState());
    FT_ASSERT_EQ(false, right_mutex->lockState());
    ft_errno = FT_ERR_SUCCESSS;
    return (1);
}

FT_TEST(test_big_number_mutex_to_string_invalid_base_unlocks,
        "ft_big_number to_string_base unlocks mutex on invalid base")
{
    ft_big_number number;
    ft_string result_string;
    pt_recursive_mutex *number_mutex;

    number.assign("10");
    result_string = number.to_string_base(1);
    number_mutex = number.get_mutex_for_testing();

    FT_ASSERT(number_mutex != ft_nullptr);

    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_string::last_operation_error());
    FT_ASSERT_EQ(false, number_mutex->lockState());
    ft_errno = FT_ERR_SUCCESSS;
    return (1);
}

FT_TEST(test_big_number_mutex_assign_base_invalid_digits_unlocks,
        "ft_big_number assign_base unlocks mutex when digits are invalid")
{
    ft_big_number number;
    pt_recursive_mutex *number_mutex;

    number.assign_base("1Z", 10);
    number_mutex = number.get_mutex_for_testing();

    FT_ASSERT(number_mutex != ft_nullptr);

    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_big_number::last_operation_error());
    FT_ASSERT_EQ(false, number_mutex->lockState());
    ft_errno = FT_ERR_SUCCESSS;
    return (1);
}

FT_TEST(test_big_number_mutex_append_digit_invalid_unlocks,
        "ft_big_number append_digit unlocks mutex on invalid character")
{
    ft_big_number number;
    pt_recursive_mutex *number_mutex;

    number.append_digit('x');
    number_mutex = number.get_mutex_for_testing();

    FT_ASSERT(number_mutex != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_big_number::last_operation_error());
    FT_ASSERT_EQ(false, number_mutex->lockState());
    ft_errno = FT_ERR_SUCCESSS;
    return (1);
}

FT_TEST(test_big_number_mutex_append_unsigned_unlocks,
        "ft_big_number append_unsigned unlocks mutex on success")
{
    ft_big_number number;
    pt_recursive_mutex *number_mutex;

    number.append_unsigned(12345);
    number_mutex = number.get_mutex_for_testing();

    FT_ASSERT(number_mutex != ft_nullptr);
    FT_ASSERT_EQ(false, number_mutex->lockState());
    FT_ASSERT_EQ(false, number.empty());
    ft_errno = FT_ERR_SUCCESSS;
    return (1);
}

FT_TEST(test_big_number_mutex_mod_pow_zero_modulus_unlocks,
        "ft_big_number mod_pow unlocks mutex when modulus is zero")
{
    ft_big_number base_number;
    ft_big_number exponent_number;
    ft_big_number modulus_number;
    ft_big_number result_number;
    pt_recursive_mutex *base_mutex;
    pt_recursive_mutex *exponent_mutex;
    pt_recursive_mutex *modulus_mutex;
    pt_recursive_mutex *result_mutex;

    base_number.assign("5");
    exponent_number.assign("3");
    modulus_number.append_digit('0');
    result_number = base_number.mod_pow(exponent_number, modulus_number);
    base_mutex = base_number.get_mutex_for_testing();
    exponent_mutex = exponent_number.get_mutex_for_testing();
    modulus_mutex = modulus_number.get_mutex_for_testing();
    result_mutex = result_number.get_mutex_for_testing();

    FT_ASSERT(base_mutex != ft_nullptr);
    FT_ASSERT(exponent_mutex != ft_nullptr);
    FT_ASSERT(modulus_mutex != ft_nullptr);
    FT_ASSERT(result_mutex != ft_nullptr);

    FT_ASSERT_EQ(FT_ERR_DIVIDE_BY_ZERO, ft_big_number::last_operation_error());
    FT_ASSERT_EQ(false, base_mutex->lockState());
    FT_ASSERT_EQ(false, exponent_mutex->lockState());
    FT_ASSERT_EQ(false, modulus_mutex->lockState());
    FT_ASSERT_EQ(false, result_mutex->lockState());
    ft_errno = FT_ERR_SUCCESSS;
    return (1);
}

FT_TEST(test_big_number_mutex_addition_with_error_operand_unlocks,
        "ft_big_number addition unlocks mutex when left operand has error")
{
    ft_big_number invalid_number;
    ft_big_number valid_number;
    ft_big_number sum_number;
    pt_recursive_mutex *invalid_mutex;
    pt_recursive_mutex *valid_mutex;
    pt_recursive_mutex *sum_mutex;

    invalid_number.append_digit('q');
    valid_number.assign("9");
    sum_number = invalid_number + valid_number;
    invalid_mutex = invalid_number.get_mutex_for_testing();
    valid_mutex = valid_number.get_mutex_for_testing();
    sum_mutex = sum_number.get_mutex_for_testing();

    FT_ASSERT(invalid_mutex != ft_nullptr);
    FT_ASSERT(valid_mutex != ft_nullptr);
    FT_ASSERT(sum_mutex != ft_nullptr);

    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_big_number::last_operation_error());
    FT_ASSERT_EQ(false, invalid_mutex->lockState());
    FT_ASSERT_EQ(false, valid_mutex->lockState());
    FT_ASSERT_EQ(false, sum_mutex->lockState());
    ft_errno = FT_ERR_SUCCESSS;
    return (1);
}

FT_TEST(test_big_number_mutex_clear_after_error_unlocks,
        "ft_big_number clear unlocks mutex after error state")
{
    ft_big_number number;
    pt_recursive_mutex *number_mutex;

    number.append_digit('m');
    number.clear();
    number_mutex = number.get_mutex_for_testing();

    FT_ASSERT(number_mutex != ft_nullptr);
    FT_ASSERT_EQ(false, number_mutex->lockState());
    FT_ASSERT_EQ(true, number.empty());
    ft_errno = FT_ERR_SUCCESSS;
    return (1);
}

FT_TEST(test_big_number_mutex_assign_base_zero_unlocks,
        "ft_big_number assign_base unlocks mutex when base is zero")
{
    ft_big_number number;
    pt_recursive_mutex *number_mutex;

    number.assign_base("10", 0);
    number_mutex = number.get_mutex_for_testing();

    FT_ASSERT(number_mutex != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_big_number::last_operation_error());
    FT_ASSERT_EQ(false, number_mutex->lockState());
    ft_errno = FT_ERR_SUCCESSS;
    return (1);
}

FT_TEST(test_big_number_mutex_multiplication_with_error_operand_unlocks,
        "ft_big_number multiplication unlocks mutex when operand has error")
{
    ft_big_number invalid_number;
    ft_big_number valid_number;
    ft_big_number product_number;
    pt_recursive_mutex *invalid_mutex;
    pt_recursive_mutex *valid_mutex;
    pt_recursive_mutex *product_mutex;

    invalid_number.append_digit('Z');
    valid_number.assign("4");
    product_number = invalid_number * valid_number;
    invalid_mutex = invalid_number.get_mutex_for_testing();
    valid_mutex = valid_number.get_mutex_for_testing();
    product_mutex = product_number.get_mutex_for_testing();

    FT_ASSERT(invalid_mutex != ft_nullptr);
    FT_ASSERT(valid_mutex != ft_nullptr);
    FT_ASSERT(product_mutex != ft_nullptr);

    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_big_number::last_operation_error());
    FT_ASSERT_EQ(false, invalid_mutex->lockState());
    FT_ASSERT_EQ(false, valid_mutex->lockState());
    FT_ASSERT_EQ(false, product_mutex->lockState());
    ft_errno = FT_ERR_SUCCESSS;
    return (1);
}

FT_TEST(test_big_number_mutex_to_string_valid_unlocks,
        "ft_big_number to_string_base unlocks mutex on success")
{
    ft_big_number number;
    ft_string decimal_string;
    pt_recursive_mutex *number_mutex;
    pt_recursive_mutex *string_mutex;

    number.assign("256");
    decimal_string = number.to_string_base(10);
    number_mutex = number.get_mutex_for_testing();
    string_mutex = decimal_string.get_mutex_for_testing();

    FT_ASSERT(number_mutex != ft_nullptr);
    FT_ASSERT(string_mutex != ft_nullptr);

    FT_ASSERT_EQ(false, number_mutex->lockState());
    FT_ASSERT_EQ(false, string_mutex->lockState());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_string::last_operation_error());
    FT_ASSERT_EQ(3, decimal_string.size());
    ft_errno = FT_ERR_SUCCESSS;
    return (1);
}

FT_TEST(test_big_number_mutex_mod_pow_with_error_operand_unlocks,
        "ft_big_number mod_pow unlocks mutex when operands have errors")
{
    ft_big_number base_number;
    ft_big_number exponent_number;
    ft_big_number modulus_number;
    ft_big_number result_number;
    pt_recursive_mutex *base_mutex;
    pt_recursive_mutex *exponent_mutex;
    pt_recursive_mutex *modulus_mutex;
    pt_recursive_mutex *result_mutex;

    base_number.append_digit('x');
    exponent_number.assign("2");
    modulus_number.assign("3");
    result_number = base_number.mod_pow(exponent_number, modulus_number);
    base_mutex = base_number.get_mutex_for_testing();
    exponent_mutex = exponent_number.get_mutex_for_testing();
    modulus_mutex = modulus_number.get_mutex_for_testing();
    result_mutex = result_number.get_mutex_for_testing();

    FT_ASSERT(base_mutex != ft_nullptr);
    FT_ASSERT(exponent_mutex != ft_nullptr);
    FT_ASSERT(modulus_mutex != ft_nullptr);
    FT_ASSERT(result_mutex != ft_nullptr);

    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_big_number::last_operation_error());
    FT_ASSERT_EQ(false, base_mutex->lockState());
    FT_ASSERT_EQ(false, exponent_mutex->lockState());
    FT_ASSERT_EQ(false, modulus_mutex->lockState());
    FT_ASSERT_EQ(false, result_mutex->lockState());
    ft_errno = FT_ERR_SUCCESSS;
    return (1);
}

FT_TEST(test_big_number_mutex_trim_leading_zeros_after_error_unlocks,
        "ft_big_number trim_leading_zeros unlocks mutex after error state")
{
    ft_big_number number;
    pt_recursive_mutex *number_mutex;

    number.append_digit('y');
    number.trim_leading_zeros();
    number_mutex = number.get_mutex_for_testing();

    FT_ASSERT(number_mutex != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_big_number::last_operation_error());
    FT_ASSERT_EQ(false, number_mutex->lockState());
    ft_errno = FT_ERR_SUCCESSS;
    return (1);
}

FT_TEST(test_big_number_mutex_assign_nullptr_unlocks,
        "ft_big_number assign unlocks mutex when input is null")
{
    ft_big_number number;
    pt_recursive_mutex *number_mutex;

    number.assign(ft_nullptr);
    number_mutex = number.get_mutex_for_testing();

    FT_ASSERT(number_mutex != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_big_number::last_operation_error());
    FT_ASSERT_EQ(false, number_mutex->lockState());
    ft_errno = FT_ERR_SUCCESSS;
    return (1);
}

FT_TEST(test_big_number_mutex_append_nullptr_unlocks,
        "ft_big_number append unlocks mutex when input is null")
{
    ft_big_number number;
    pt_recursive_mutex *number_mutex;

    number.append(ft_nullptr);
    number_mutex = number.get_mutex_for_testing();

    FT_ASSERT(number_mutex != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_big_number::last_operation_error());
    FT_ASSERT_EQ(false, number_mutex->lockState());
    ft_errno = FT_ERR_SUCCESSS;
    return (1);
}

FT_TEST(test_big_number_mutex_mod_pow_invalid_exponent_unlocks,
        "ft_big_number mod_pow unlocks mutex when exponent is invalid")
{
    ft_big_number base_number;
    ft_big_number exponent_number;
    ft_big_number modulus_number;
    ft_big_number result_number;
    pt_recursive_mutex *base_mutex;
    pt_recursive_mutex *exponent_mutex;
    pt_recursive_mutex *modulus_mutex;
    pt_recursive_mutex *result_mutex;

    base_number.assign("2");
    exponent_number.append_digit('g');
    modulus_number.assign("5");
    result_number = base_number.mod_pow(exponent_number, modulus_number);
    base_mutex = base_number.get_mutex_for_testing();
    exponent_mutex = exponent_number.get_mutex_for_testing();
    modulus_mutex = modulus_number.get_mutex_for_testing();
    result_mutex = result_number.get_mutex_for_testing();

    FT_ASSERT(base_mutex != ft_nullptr);
    FT_ASSERT(exponent_mutex != ft_nullptr);
    FT_ASSERT(modulus_mutex != ft_nullptr);
    FT_ASSERT(result_mutex != ft_nullptr);

    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_big_number::last_operation_error());
    FT_ASSERT_EQ(false, base_mutex->lockState());
    FT_ASSERT_EQ(false, exponent_mutex->lockState());
    FT_ASSERT_EQ(false, modulus_mutex->lockState());
    FT_ASSERT_EQ(false, result_mutex->lockState());
    ft_errno = FT_ERR_SUCCESSS;
    return (1);
}

FT_TEST(test_big_number_mutex_to_string_negative_base_unlocks,
        "ft_big_number to_string_base unlocks mutex when base is negative")
{
    ft_big_number number;
    ft_string result_string;
    pt_recursive_mutex *number_mutex;
    pt_recursive_mutex *string_mutex;

    number.assign("10");
    result_string = number.to_string_base(-2);
    number_mutex = number.get_mutex_for_testing();
    string_mutex = result_string.get_mutex_for_testing();

    FT_ASSERT(number_mutex != ft_nullptr);
    FT_ASSERT(string_mutex != ft_nullptr);

    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_string::last_operation_error());
    FT_ASSERT_EQ(false, number_mutex->lockState());
    FT_ASSERT_EQ(false, string_mutex->lockState());
    ft_errno = FT_ERR_SUCCESSS;
    return (1);
}

FT_TEST(test_big_number_mutex_from_hex_string_null_unlocks,
        "big_number_from_hex_string unlocks mutex when input is null")
{
    ft_big_number result_number;
    pt_recursive_mutex *result_mutex;

    result_number = big_number_from_hex_string(ft_nullptr);
    result_mutex = result_number.get_mutex_for_testing();

    FT_ASSERT(result_mutex != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_big_number::last_operation_error());
    FT_ASSERT_EQ(false, result_mutex->lockState());
    ft_errno = FT_ERR_SUCCESSS;
    return (1);
}
