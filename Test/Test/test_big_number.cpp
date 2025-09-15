#include "../../CPP_class/class_big_number.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include <cstring>

FT_TEST(test_big_number_default_state, "ft_big_number default state is zero")
{
    ft_big_number number;

    FT_ASSERT(number.empty());
    FT_ASSERT_EQ(static_cast<ft_size_t>(0), number.size());
    FT_ASSERT_EQ(0, std::strcmp(number.c_str(), "0"));
    FT_ASSERT_EQ(0, number.get_error());
    return (1);
}

FT_TEST(test_big_number_assign_trim, "ft_big_number assign trims leading zeros")
{
    ft_big_number number;

    number.assign("0001234500");
    FT_ASSERT_EQ(0, number.get_error());
    FT_ASSERT_EQ(static_cast<ft_size_t>(7), number.size());
    FT_ASSERT_EQ(0, std::strcmp(number.c_str(), "1234500"));
    return (1);
}

FT_TEST(test_big_number_append_helpers, "ft_big_number append_digit and append_unsigned extend number")
{
    ft_big_number number;

    number.append_digit('4');
    FT_ASSERT_EQ(0, number.get_error());
    number.append_digit('2');
    FT_ASSERT_EQ(0, number.get_error());
    number.append_unsigned(12345);
    FT_ASSERT_EQ(0, number.get_error());
    FT_ASSERT_EQ(static_cast<ft_size_t>(7), number.size());
    FT_ASSERT_EQ(0, std::strcmp(number.c_str(), "4212345"));
    return (1);
}

FT_TEST(test_big_number_assign_invalid_digit, "ft_big_number assign rejects invalid characters")
{
    ft_big_number number;

    ft_errno = 0;
    number.assign("12a3");
    FT_ASSERT_EQ(BIG_NUMBER_INVALID_DIGIT, number.get_error());
    FT_ASSERT_EQ(BIG_NUMBER_INVALID_DIGIT, ft_errno);
    FT_ASSERT_EQ(static_cast<ft_size_t>(0), number.size());
    FT_ASSERT_EQ(0, std::strcmp(number.c_str(), "0"));
    ft_errno = 0;
    return (1);
}

FT_TEST(test_big_number_append_invalid_digit, "ft_big_number append_digit rejects invalid characters")
{
    ft_big_number number;

    ft_errno = 0;
    number.append_digit('x');
    FT_ASSERT_EQ(BIG_NUMBER_INVALID_DIGIT, number.get_error());
    FT_ASSERT_EQ(BIG_NUMBER_INVALID_DIGIT, ft_errno);
    ft_errno = 0;
    return (1);
}

FT_TEST(test_big_number_addition_large_values, "ft_big_number addition handles large operands")
{
    ft_big_number left_number;
    ft_big_number right_number;

    left_number.assign("12345678901234567890");
    right_number.assign("98765432109876543210");
    FT_ASSERT_EQ(0, left_number.get_error());
    FT_ASSERT_EQ(0, right_number.get_error());
    ft_big_number sum_number = left_number + right_number;
    FT_ASSERT_EQ(0, sum_number.get_error());
    FT_ASSERT_EQ(0, std::strcmp(sum_number.c_str(), "111111111011111111100"));
    return (1);
}

FT_TEST(test_big_number_subtraction_basic, "ft_big_number subtraction removes digits")
{
    ft_big_number left_number;
    ft_big_number right_number;

    left_number.assign("1000");
    right_number.assign("1");
    ft_big_number difference_number = left_number - right_number;
    FT_ASSERT_EQ(0, difference_number.get_error());
    FT_ASSERT_EQ(0, std::strcmp(difference_number.c_str(), "999"));
    return (1);
}

FT_TEST(test_big_number_subtraction_negative_error, "ft_big_number subtraction reports negative results")
{
    ft_big_number small_number;
    ft_big_number large_number;

    ft_errno = 0;
    small_number.assign("5");
    large_number.assign("10");
    ft_big_number result_number = small_number - large_number;
    FT_ASSERT_EQ(BIG_NUMBER_NEGATIVE_RESULT, result_number.get_error());
    FT_ASSERT_EQ(BIG_NUMBER_NEGATIVE_RESULT, ft_errno);
    ft_errno = 0;
    return (1);
}

FT_TEST(test_big_number_multiplication_large_values, "ft_big_number multiplication handles carries")
{
    ft_big_number left_number;
    ft_big_number right_number;

    left_number.assign("99999");
    right_number.assign("88888");
    ft_big_number product_number = left_number * right_number;
    FT_ASSERT_EQ(0, product_number.get_error());
    FT_ASSERT_EQ(0, std::strcmp(product_number.c_str(), "8888711112"));
    return (1);
}

FT_TEST(test_big_number_division_even, "ft_big_number division returns quotient")
{
    ft_big_number numerator_number;
    ft_big_number denominator_number;

    numerator_number.assign("99999999999999999999");
    denominator_number.assign("3");
    ft_big_number quotient_number = numerator_number / denominator_number;
    FT_ASSERT_EQ(0, quotient_number.get_error());
    FT_ASSERT_EQ(0, std::strcmp(quotient_number.c_str(), "33333333333333333333"));
    return (1);
}

FT_TEST(test_big_number_division_by_zero_error, "ft_big_number division reports divide by zero")
{
    ft_big_number numerator_number;
    ft_big_number zero_number;

    ft_errno = 0;
    numerator_number.assign("12345");
    ft_big_number result_number = numerator_number / zero_number;
    FT_ASSERT_EQ(BIG_NUMBER_DIVIDE_BY_ZERO, result_number.get_error());
    FT_ASSERT_EQ(BIG_NUMBER_DIVIDE_BY_ZERO, ft_errno);
    ft_errno = 0;
    return (1);
}

FT_TEST(test_big_number_comparisons, "ft_big_number comparison operators compare magnitude")
{
    ft_big_number first_number;
    ft_big_number second_number;
    ft_big_number third_number;
    ft_big_number zero_number;

    first_number.assign("12345");
    second_number.assign("12345");
    third_number.assign("12346");
    zero_number.assign("0");
    FT_ASSERT(first_number == second_number);
    FT_ASSERT(first_number <= second_number);
    FT_ASSERT(first_number >= second_number);
    FT_ASSERT(first_number != third_number);
    FT_ASSERT(first_number < third_number);
    FT_ASSERT(third_number > first_number);
    FT_ASSERT(zero_number <= first_number);
    FT_ASSERT(first_number >= zero_number);
    return (1);
}
