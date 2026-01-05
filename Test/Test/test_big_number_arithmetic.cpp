#include "../../CPP_class/class_big_number.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include "../../Template/move.hpp"
#include <cstring>

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
    FT_ASSERT(!sum_number.is_negative());
    FT_ASSERT(sum_number.is_positive());
    return (1);
}

FT_TEST(test_big_number_signed_addition, "ft_big_number addition handles negative operands")
{
    ft_big_number negative_five_hundred;
    ft_big_number negative_two_hundred;
    ft_big_number positive_two_hundred;
    ft_big_number positive_five_hundred;

    negative_five_hundred.assign("-500");
    negative_two_hundred.assign("-200");
    positive_two_hundred.assign("200");
    positive_five_hundred.assign("500");

    ft_big_number sum_neg_pos = negative_five_hundred + positive_two_hundred;
    FT_ASSERT_EQ(0, sum_neg_pos.get_error());
    FT_ASSERT_EQ(0, std::strcmp(sum_neg_pos.c_str(), "300"));
    FT_ASSERT(sum_neg_pos.is_negative());
    FT_ASSERT(!sum_neg_pos.is_positive());

    ft_big_number sum_pos_neg = positive_five_hundred + negative_two_hundred;
    FT_ASSERT_EQ(0, sum_pos_neg.get_error());
    FT_ASSERT_EQ(0, std::strcmp(sum_pos_neg.c_str(), "300"));
    FT_ASSERT(!sum_pos_neg.is_negative());
    FT_ASSERT(sum_pos_neg.is_positive());

    ft_big_number sum_neg_neg = negative_five_hundred + negative_two_hundred;
    FT_ASSERT_EQ(0, sum_neg_neg.get_error());
    FT_ASSERT_EQ(0, std::strcmp(sum_neg_neg.c_str(), "700"));
    FT_ASSERT(sum_neg_neg.is_negative());
    FT_ASSERT(!sum_neg_neg.is_positive());

    ft_big_number sum_cancel = positive_two_hundred + negative_two_hundred;
    FT_ASSERT_EQ(0, sum_cancel.get_error());
    FT_ASSERT_EQ(0, std::strcmp(sum_cancel.c_str(), "0"));
    FT_ASSERT(!sum_cancel.is_negative());
    FT_ASSERT(!sum_cancel.is_positive());
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
    FT_ASSERT(!difference_number.is_negative());
    FT_ASSERT(difference_number.is_positive());
    return (1);
}

FT_TEST(test_big_number_signed_subtraction, "ft_big_number subtraction handles negative operands")
{
    ft_big_number positive_five;
    ft_big_number positive_three;
    ft_big_number negative_five;
    ft_big_number negative_three;
    ft_big_number negative_ten;

    positive_five.assign("5");
    positive_three.assign("3");
    negative_five.assign("-5");
    negative_three.assign("-3");
    negative_ten.assign("-10");

    ft_big_number diff_one = positive_five - negative_three;
    FT_ASSERT_EQ(0, diff_one.get_error());
    FT_ASSERT_EQ(0, std::strcmp(diff_one.c_str(), "8"));
    FT_ASSERT(!diff_one.is_negative());
    FT_ASSERT(diff_one.is_positive());

    ft_big_number diff_two = negative_five - positive_three;
    FT_ASSERT_EQ(0, diff_two.get_error());
    FT_ASSERT_EQ(0, std::strcmp(diff_two.c_str(), "8"));
    FT_ASSERT(diff_two.is_negative());
    FT_ASSERT(!diff_two.is_positive());

    ft_big_number diff_three = negative_ten - negative_three;
    FT_ASSERT_EQ(0, diff_three.get_error());
    FT_ASSERT_EQ(0, std::strcmp(diff_three.c_str(), "7"));
    FT_ASSERT(diff_three.is_negative());
    FT_ASSERT(!diff_three.is_positive());

    ft_big_number diff_four = negative_three - negative_ten;
    FT_ASSERT_EQ(0, diff_four.get_error());
    FT_ASSERT_EQ(0, std::strcmp(diff_four.c_str(), "7"));
    FT_ASSERT(!diff_four.is_negative());
    FT_ASSERT(diff_four.is_positive());
    return (1);
}

FT_TEST(test_big_number_subtraction_negative_result, "ft_big_number subtraction returns negative results")
{
    ft_big_number small_number;
    ft_big_number large_number;

    small_number.assign("5");
    large_number.assign("10");
    ft_big_number result_number = small_number - large_number;
    FT_ASSERT_EQ(0, result_number.get_error());
    FT_ASSERT(result_number.is_negative());
    FT_ASSERT(!result_number.is_positive());
    FT_ASSERT_EQ(0, std::strcmp(result_number.c_str(), "5"));
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

FT_TEST(test_big_number_signed_multiplication, "ft_big_number multiplication applies sign")
{
    ft_big_number positive_twelve;
    ft_big_number negative_three;
    ft_big_number negative_four;

    positive_twelve.assign("12");
    negative_three.assign("-3");
    negative_four.assign("-4");

    ft_big_number product_one = positive_twelve * negative_three;
    FT_ASSERT_EQ(0, product_one.get_error());
    FT_ASSERT_EQ(0, std::strcmp(product_one.c_str(), "36"));
    FT_ASSERT(product_one.is_negative());
    FT_ASSERT(!product_one.is_positive());

    ft_big_number product_two = negative_three * negative_four;
    FT_ASSERT_EQ(0, product_two.get_error());
    FT_ASSERT_EQ(0, std::strcmp(product_two.c_str(), "12"));
    FT_ASSERT(!product_two.is_negative());
    FT_ASSERT(product_two.is_positive());

    ft_big_number product_three = negative_four * positive_twelve;
    FT_ASSERT_EQ(0, product_three.get_error());
    FT_ASSERT_EQ(0, std::strcmp(product_three.c_str(), "48"));
    FT_ASSERT(product_three.is_negative());
    FT_ASSERT(!product_three.is_positive());
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

FT_TEST(test_big_number_signed_division, "ft_big_number division applies sign")
{
    ft_big_number numerator_positive;
    ft_big_number numerator_negative;
    ft_big_number denominator_positive;
    ft_big_number denominator_negative;

    numerator_positive.assign("100");
    numerator_negative.assign("-100");
    denominator_positive.assign("5");
    denominator_negative.assign("-5");

    ft_big_number quotient_one = numerator_negative / denominator_positive;
    FT_ASSERT_EQ(0, quotient_one.get_error());
    FT_ASSERT_EQ(0, std::strcmp(quotient_one.c_str(), "20"));
    FT_ASSERT(quotient_one.is_negative());
    FT_ASSERT(!quotient_one.is_positive());

    ft_big_number quotient_two = numerator_positive / denominator_negative;
    FT_ASSERT_EQ(0, quotient_two.get_error());
    FT_ASSERT_EQ(0, std::strcmp(quotient_two.c_str(), "20"));
    FT_ASSERT(quotient_two.is_negative());
    FT_ASSERT(!quotient_two.is_positive());

    ft_big_number quotient_three = numerator_negative / denominator_negative;
    FT_ASSERT_EQ(0, quotient_three.get_error());
    FT_ASSERT_EQ(0, std::strcmp(quotient_three.c_str(), "20"));
    FT_ASSERT(!quotient_three.is_negative());
    FT_ASSERT(quotient_three.is_positive());
    return (1);
}

FT_TEST(test_big_number_division_by_zero_error, "ft_big_number division reports divide by zero")
{
    ft_big_number numerator_number;
    ft_big_number zero_number;

    ft_errno = 0;
    ft_sys_errno = FT_SYS_ERR_INVALID_STATE;
    zero_number.assign("0");
    numerator_number.assign("12345");
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    ft_big_number result_number = numerator_number / zero_number;
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    FT_ASSERT_EQ(FT_ERR_DIVIDE_BY_ZERO, result_number.get_error());
    FT_ASSERT_EQ(FT_ERR_DIVIDE_BY_ZERO, ft_big_number::last_error());
    FT_ASSERT_EQ(FT_ERR_DIVIDE_BY_ZERO, ft_big_number::error_for(ft_big_number::last_op_id()));
    ft_errno = 0;
    return (1);
}

FT_TEST(test_big_number_modulus_basic, "ft_big_number modulus returns signed remainders")
{
    ft_big_number positive_dividend;
    ft_big_number divisor;

    positive_dividend.assign("100");
    divisor.assign("7");

    ft_big_number positive_remainder = positive_dividend % divisor;
    FT_ASSERT_EQ(0, positive_remainder.get_error());
    FT_ASSERT_EQ(0, std::strcmp(positive_remainder.c_str(), "2"));
    FT_ASSERT(!positive_remainder.is_negative());
    FT_ASSERT(positive_remainder.is_positive());

    ft_big_number negative_dividend;

    negative_dividend.assign("-100");
    ft_big_number negative_remainder = negative_dividend % divisor;
    FT_ASSERT_EQ(0, negative_remainder.get_error());
    FT_ASSERT_EQ(0, std::strcmp(negative_remainder.c_str(), "2"));
    FT_ASSERT(negative_remainder.is_negative());
    FT_ASSERT(!negative_remainder.is_positive());

    ft_big_number small_dividend;
    ft_big_number large_divisor;

    small_dividend.assign("3");
    large_divisor.assign("10");
    ft_big_number small_remainder = small_dividend % large_divisor;
    FT_ASSERT_EQ(0, small_remainder.get_error());
    FT_ASSERT_EQ(0, std::strcmp(small_remainder.c_str(), "3"));
    FT_ASSERT(!small_remainder.is_negative());
    FT_ASSERT(small_remainder.is_positive());

    ft_big_number zero_divisor;

    ft_errno = 0;
    ft_sys_errno = FT_SYS_ERR_INVALID_STATE;
    zero_divisor.assign("0");
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    ft_big_number error_remainder = positive_dividend % zero_divisor;
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    FT_ASSERT_EQ(FT_ERR_DIVIDE_BY_ZERO, error_remainder.get_error());
    FT_ASSERT_EQ(FT_ERR_DIVIDE_BY_ZERO, ft_big_number::last_error());
    FT_ASSERT_EQ(FT_ERR_DIVIDE_BY_ZERO, ft_big_number::error_for(ft_big_number::last_op_id()));
    ft_errno = 0;
    return (1);
}

FT_TEST(test_big_number_modular_exponentiation, "ft_big_number mod_pow performs repeated modular multiplication")
{
    ft_big_number base_number;
    ft_big_number exponent_number;
    ft_big_number modulus_number;

    base_number.assign("7");
    exponent_number.assign("4");
    modulus_number.assign("13");
    ft_big_number power_result = base_number.mod_pow(exponent_number, modulus_number);
    FT_ASSERT_EQ(0, power_result.get_error());
    FT_ASSERT_EQ(0, std::strcmp(power_result.c_str(), "9"));
    FT_ASSERT(!power_result.is_negative());
    FT_ASSERT(power_result.is_positive());

    ft_big_number zero_exponent;

    zero_exponent.assign("0");
    ft_big_number identity_result = base_number.mod_pow(zero_exponent, modulus_number);
    FT_ASSERT_EQ(0, identity_result.get_error());
    FT_ASSERT_EQ(0, std::strcmp(identity_result.c_str(), "1"));
    FT_ASSERT(!identity_result.is_negative());
    FT_ASSERT(identity_result.is_positive());

    ft_big_number negative_base;
    ft_big_number cube_exponent;
    ft_big_number modulus_five;

    negative_base.assign("-2");
    cube_exponent.assign("3");
    modulus_five.assign("5");
    ft_big_number wrapped_result = negative_base.mod_pow(cube_exponent, modulus_five);
    FT_ASSERT_EQ(0, wrapped_result.get_error());
    FT_ASSERT_EQ(0, std::strcmp(wrapped_result.c_str(), "2"));
    FT_ASSERT(!wrapped_result.is_negative());
    FT_ASSERT(wrapped_result.is_positive());

    ft_big_number zero_modulus;
    ft_big_number exponent_one;

    zero_modulus.assign("0");
    exponent_one.assign("1");
    ft_errno = 0;
    ft_sys_errno = FT_SYS_ERR_INVALID_STATE;
    ft_big_number modulus_error = base_number.mod_pow(exponent_one, zero_modulus);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(FT_ERR_DIVIDE_BY_ZERO, modulus_error.get_error());
    FT_ASSERT_EQ(FT_ERR_DIVIDE_BY_ZERO, ft_big_number::last_error());
    FT_ASSERT_EQ(FT_ERR_DIVIDE_BY_ZERO, ft_big_number::error_for(ft_big_number::last_op_id()));
    ft_errno = 0;

    ft_big_number negative_exponent;

    negative_exponent.assign("-1");
    ft_errno = 0;
    ft_sys_errno = FT_SYS_ERR_INVALID_STATE;
    ft_big_number exponent_error
        = base_number.mod_pow(negative_exponent, modulus_number);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, exponent_error.get_error());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_big_number::last_error());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_big_number::error_for(ft_big_number::last_op_id()));
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

FT_TEST(test_big_number_signed_comparisons, "ft_big_number comparisons handle signs")
{
    ft_big_number negative_large;
    ft_big_number negative_small;
    ft_big_number positive_small;
    ft_big_number positive_large;
    ft_big_number negative_small_copy;

    negative_large.assign("-200");
    negative_small.assign("-50");
    positive_small.assign("50");
    positive_large.assign("200");
    negative_small_copy.assign("-50");

    FT_ASSERT(negative_large.is_negative());
    FT_ASSERT(!negative_large.is_positive());
    FT_ASSERT(positive_large.is_positive());
    FT_ASSERT(!positive_large.is_negative());

    FT_ASSERT(negative_large < negative_small);
    FT_ASSERT(!(negative_large > negative_small));
    FT_ASSERT(negative_small < positive_small);
    FT_ASSERT(!(positive_small < negative_large));
    FT_ASSERT(positive_large > negative_small);
    FT_ASSERT(positive_small >= negative_small);
    FT_ASSERT(negative_small <= positive_small);
    FT_ASSERT(negative_small == negative_small_copy);
    FT_ASSERT(negative_small != positive_small);
    return (1);
}
