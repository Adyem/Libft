#include "../test_internal.hpp"
#include "../../Basic/basic.hpp"
#include "../../Math/math.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_validate_int_ok, "validate int ok")
{
    FT_ASSERT_EQ(FT_SUCCESS, ft_validate_int("123"));
    FT_ASSERT_EQ(FT_SUCCESS, math_validate_int("456"));
    return (1);
}

FT_TEST(test_validate_int_empty, "validate int empty")
{
    FT_ASSERT_EQ(FT_FAILURE, ft_validate_int("+"));
    FT_ASSERT_EQ(FT_FAILURE, math_validate_int("-"));
    return (1);
}

FT_TEST(test_validate_int_range, "validate int range")
{
    FT_ASSERT_EQ(FT_FAILURE, ft_validate_int("2147483648"));
    FT_ASSERT_EQ(FT_FAILURE, math_validate_int("-2147483649"));
    return (1);
}

FT_TEST(test_validate_int_invalid, "validate int invalid")
{
    FT_ASSERT_EQ(FT_FAILURE, ft_validate_int("12a3"));
    FT_ASSERT_EQ(FT_FAILURE, math_validate_int("123b"));
    return (1);
}

FT_TEST(test_validate_int_nullptr, "validate int nullptr")
{
    FT_ASSERT_EQ(FT_FAILURE, ft_validate_int(ft_nullptr));
    return (1);
}

FT_TEST(test_validate_int_leading_whitespace_fails, "validate int rejects leading whitespace")
{
    FT_ASSERT_EQ(FT_FAILURE, ft_validate_int(" 123"));
    FT_ASSERT_EQ(FT_FAILURE, ft_validate_int("\t456"));
    return (1);
}

FT_TEST(test_validate_int_trailing_whitespace_fails, "validate int rejects trailing whitespace")
{
    FT_ASSERT_EQ(FT_FAILURE, ft_validate_int("789 "));
    FT_ASSERT_EQ(FT_FAILURE, ft_validate_int("321\n"));
    return (1);
}

FT_TEST(test_validate_int_accepts_signed_limits, "validate int accepts signed limits")
{
    FT_ASSERT_EQ(FT_SUCCESS, ft_validate_int("2147483647"));
    FT_ASSERT_EQ(FT_SUCCESS, ft_validate_int("-2147483648"));
    return (1);
}

FT_TEST(test_validate_int_allows_leading_plus_and_zeroes, "validate int accepts plus sign and leading zeroes")
{
    FT_ASSERT_EQ(FT_SUCCESS, ft_validate_int("+42"));
    FT_ASSERT_EQ(FT_SUCCESS, math_validate_int("00099"));
    return (1);
}

FT_TEST(test_validate_int_allows_plus_signed_limits, "validate int accepts plus signed limits")
{
    FT_ASSERT_EQ(FT_SUCCESS, ft_validate_int("+2147483647"));
    FT_ASSERT_EQ(FT_FAILURE, ft_validate_int("+2147483648"));
    return (1);
}

FT_TEST(test_validate_int_accepts_zero_variants, "validate int accepts zero and negative zero")
{
    FT_ASSERT_EQ(FT_SUCCESS, ft_validate_int("0"));
    FT_ASSERT_EQ(FT_SUCCESS, math_validate_int("-0"));
    return (1);
}

FT_TEST(test_validate_int_rejects_embedded_spacing_and_signs, "validate int rejects embedded spacing and signs")
{
    FT_ASSERT_EQ(FT_FAILURE, ft_validate_int("1 23"));
    FT_ASSERT_EQ(FT_FAILURE, math_validate_int("12-3"));
    return (1);
}

FT_TEST(test_validate_int_rejects_empty_string, "validate int rejects empty string")
{
    FT_ASSERT_EQ(FT_FAILURE, ft_validate_int(""));
    FT_ASSERT_EQ(FT_FAILURE, math_validate_int(""));
    return (1);
}

FT_TEST(test_validate_int_rejects_double_signs, "validate int rejects double signs")
{
    FT_ASSERT_EQ(FT_FAILURE, ft_validate_int("++1"));
    FT_ASSERT_EQ(FT_FAILURE, math_validate_int("--2"));
    return (1);
}

FT_TEST(test_validate_int_rejects_mixed_signs_and_trailing_sign, "validate int rejects mixed sign placement")
{
    FT_ASSERT_EQ(FT_FAILURE, ft_validate_int("+-3"));
    FT_ASSERT_EQ(FT_FAILURE, math_validate_int("-+4"));
    FT_ASSERT_EQ(FT_FAILURE, ft_validate_int("123+"));
    FT_ASSERT_EQ(FT_FAILURE, math_validate_int("-456-"));
    return (1);
}

FT_TEST(test_math_validate_int_accepts_signed_limits, "math validate int accepts signed limits")
{
    FT_ASSERT_EQ(FT_SUCCESS, math_validate_int("2147483647"));
    FT_ASSERT_EQ(FT_SUCCESS, math_validate_int("-2147483648"));
    return (1);
}

FT_TEST(test_math_validate_int_rejects_overflow, "math validate int rejects overflow")
{
    FT_ASSERT_EQ(FT_FAILURE, math_validate_int("2147483648"));
    FT_ASSERT_EQ(FT_FAILURE, math_validate_int("-2147483649"));
    FT_ASSERT_EQ(FT_FAILURE, math_validate_int("+2147483648"));
    return (1);
}
