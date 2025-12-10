#include "../../Libft/libft.hpp"
#include "../../Math/math.hpp"
#include "../../CMA/CMA.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_validate_int_ok, "validate int ok")
{
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(FT_SUCCESS, ft_validate_int("123"));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(FT_SUCCESS, math_validate_int("456"));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_validate_int_empty, "validate int empty")
{
    ft_errno = FT_ER_SUCCESSS;
    FT_ASSERT_EQ(FT_FAILURE, ft_validate_int("+"));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    ft_errno = FT_ER_SUCCESSS;
    FT_ASSERT_EQ(FT_FAILURE, math_validate_int("-"));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_validate_int_range, "validate int range")
{
    ft_errno = FT_ER_SUCCESSS;
    FT_ASSERT_EQ(FT_FAILURE, ft_validate_int("2147483648"));
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, ft_errno);
    ft_errno = FT_ER_SUCCESSS;
    FT_ASSERT_EQ(FT_FAILURE, math_validate_int("-2147483649"));
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, ft_errno);
    return (1);
}

FT_TEST(test_validate_int_invalid, "validate int invalid")
{
    ft_errno = FT_ER_SUCCESSS;
    FT_ASSERT_EQ(FT_FAILURE, ft_validate_int("12a3"));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    ft_errno = FT_ER_SUCCESSS;
    FT_ASSERT_EQ(FT_FAILURE, math_validate_int("123b"));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_validate_int_nullptr, "validate int nullptr")
{
    ft_errno = FT_ER_SUCCESSS;
    FT_ASSERT_EQ(FT_FAILURE, ft_validate_int(ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_validate_int_leading_whitespace_fails, "validate int rejects leading whitespace")
{
    ft_errno = FT_ER_SUCCESSS;
    FT_ASSERT_EQ(FT_FAILURE, ft_validate_int(" 123"));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    ft_errno = FT_ER_SUCCESSS;
    FT_ASSERT_EQ(FT_FAILURE, ft_validate_int("\t456"));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_validate_int_trailing_whitespace_fails, "validate int rejects trailing whitespace")
{
    ft_errno = FT_ER_SUCCESSS;
    FT_ASSERT_EQ(FT_FAILURE, ft_validate_int("789 "));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    ft_errno = FT_ER_SUCCESSS;
    FT_ASSERT_EQ(FT_FAILURE, ft_validate_int("321\n"));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_validate_int_accepts_signed_limits, "validate int accepts signed limits")
{
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(FT_SUCCESS, ft_validate_int("2147483647"));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(FT_SUCCESS, ft_validate_int("-2147483648"));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_validate_int_allows_leading_plus_and_zeroes, "validate int accepts plus sign and leading zeroes")
{
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(FT_SUCCESS, ft_validate_int("+42"));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(FT_SUCCESS, math_validate_int("00099"));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_validate_int_allows_plus_signed_limits, "validate int accepts plus signed limits")
{
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(FT_SUCCESS, ft_validate_int("+2147483647"));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(FT_FAILURE, ft_validate_int("+2147483648"));
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, ft_errno);
    return (1);
}

FT_TEST(test_validate_int_accepts_zero_variants, "validate int accepts zero and negative zero")
{
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(FT_SUCCESS, ft_validate_int("0"));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(FT_SUCCESS, math_validate_int("-0"));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_validate_int_rejects_embedded_spacing_and_signs, "validate int rejects embedded spacing and signs")
{
    ft_errno = FT_ER_SUCCESSS;
    FT_ASSERT_EQ(FT_FAILURE, ft_validate_int("1 23"));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    ft_errno = FT_ER_SUCCESSS;
    FT_ASSERT_EQ(FT_FAILURE, math_validate_int("12-3"));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_validate_int_rejects_empty_string, "validate int rejects empty string")
{
    ft_errno = FT_ER_SUCCESSS;
    FT_ASSERT_EQ(FT_FAILURE, ft_validate_int(""));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    ft_errno = FT_ER_SUCCESSS;
    FT_ASSERT_EQ(FT_FAILURE, math_validate_int(""));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_validate_int_rejects_double_signs, "validate int rejects double signs")
{
    ft_errno = FT_ER_SUCCESSS;
    FT_ASSERT_EQ(FT_FAILURE, ft_validate_int("++1"));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    ft_errno = FT_ER_SUCCESSS;
    FT_ASSERT_EQ(FT_FAILURE, math_validate_int("--2"));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_validate_int_rejects_mixed_signs_and_trailing_sign, "validate int rejects mixed sign placement")
{
    ft_errno = FT_ER_SUCCESSS;
    FT_ASSERT_EQ(FT_FAILURE, ft_validate_int("+-3"));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    ft_errno = FT_ER_SUCCESSS;
    FT_ASSERT_EQ(FT_FAILURE, math_validate_int("-+4"));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    ft_errno = FT_ER_SUCCESSS;
    FT_ASSERT_EQ(FT_FAILURE, ft_validate_int("123+"));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    ft_errno = FT_ER_SUCCESSS;
    FT_ASSERT_EQ(FT_FAILURE, math_validate_int("-456-"));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_math_validate_int_accepts_signed_limits, "math validate int accepts signed limits")
{
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(FT_SUCCESS, math_validate_int("2147483647"));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(FT_SUCCESS, math_validate_int("-2147483648"));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_math_validate_int_rejects_overflow, "math validate int rejects overflow")
{
    ft_errno = FT_ER_SUCCESSS;
    FT_ASSERT_EQ(FT_FAILURE, math_validate_int("2147483648"));
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, ft_errno);
    ft_errno = FT_ER_SUCCESSS;
    FT_ASSERT_EQ(FT_FAILURE, math_validate_int("-2147483649"));
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, ft_errno);
    ft_errno = FT_ER_SUCCESSS;
    FT_ASSERT_EQ(FT_FAILURE, math_validate_int("+2147483648"));
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, ft_errno);
    return (1);
}

FT_TEST(test_cma_atoi_ok, "cma atoi ok")
{
    int *number;
    int test_ok;

    number = cma_atoi("789");
    if (number == ft_nullptr)
        return (0);
    test_ok = (*number == 789);
    cma_free(number);
    return (test_ok);
}

FT_TEST(test_cma_atoi_invalid, "cma atoi invalid")
{
    FT_ASSERT_EQ(ft_nullptr, cma_atoi("99x"));
    return (1);
}
