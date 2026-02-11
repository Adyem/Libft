#include "../../Basic/basic.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Basic/limits.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"

FT_TEST(test_atoi_simple, "ft_atoi simple")
{
    ft_errno = FT_ERR_OUT_OF_RANGE;
    FT_ASSERT_EQ(42, ft_atoi("42", ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_atoi_negative, "ft_atoi negative")
{
    ft_errno = FT_ERR_OUT_OF_RANGE;
    FT_ASSERT_EQ(-13, ft_atoi("-13", ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_atoi_intmax, "ft_atoi INT_MAX")
{
    ft_string integer_string;

    integer_string = ft_to_string(FT_INT_MAX);
    ft_errno = FT_ERR_OUT_OF_RANGE;
    FT_ASSERT_EQ(FT_INT_MAX, ft_atoi(integer_string.c_str(), ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_atoi_intmin, "ft_atoi INT_MIN")
{
    ft_string integer_string;

    integer_string = ft_to_string(FT_INT_MIN);
    ft_errno = FT_ERR_OUT_OF_RANGE;
    FT_ASSERT_EQ(FT_INT_MIN, ft_atoi(integer_string.c_str(), ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_atoi_overflow_sets_erange, "ft_atoi clamps overflow to INT_MAX")
{
    ft_string overflow_string;

    overflow_string = ft_to_string(FT_INT_MAX);
    overflow_string += "9";
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(FT_INT_MAX, ft_atoi(overflow_string.c_str(), ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, ft_errno);
    return (1);
}

FT_TEST(test_atoi_underflow_sets_erange, "ft_atoi clamps underflow to INT_MIN")
{
    ft_string underflow_string;

    underflow_string = ft_to_string(FT_INT_MIN);
    underflow_string += "9";
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(FT_INT_MIN, ft_atoi(underflow_string.c_str(), ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, ft_errno);
    return (1);
}

FT_TEST(test_atoi_trailing_whitespace_sets_einval, "ft_atoi trailing whitespace sets FT_ERR_INVALID_ARGUMENT")
{
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(42, ft_atoi("  \t\n42  ", ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_atoi_trailing_chars_sets_einval, "ft_atoi stops at non-digit")
{
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(123, ft_atoi("123abc", ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_atoi_leading_whitespace_sets_einval, "ft_atoi ignores leading whitespace but reports it")
{
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(77, ft_atoi("   77", ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_atoi_plus_sign_success, "ft_atoi plus sign")
{
    ft_errno = FT_ERR_OUT_OF_RANGE;
    FT_ASSERT_EQ(7, ft_atoi("+7", ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_atoi_zero_success, "ft_atoi zero")
{
    ft_errno = FT_ERR_OUT_OF_RANGE;
    FT_ASSERT_EQ(0, ft_atoi("0", ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_atoi_null_input_sets_einval, "ft_atoi null input returns zero")
{
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(0, ft_atoi(ft_nullptr, ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_atoi_no_digits_sets_einval, "ft_atoi no digits sets FT_ERR_INVALID_ARGUMENT")
{
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(0, ft_atoi("abc", ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_atoi_whitespace_only_sets_einval, "ft_atoi whitespace only sets FT_ERR_INVALID_ARGUMENT")
{
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(0, ft_atoi(" \t\r\n\v\f", ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_atoi_digits_only_clears_errno, "ft_atoi digits clear ft_errno")
{
    ft_errno = FT_ERR_OUT_OF_RANGE;
    FT_ASSERT_EQ(314, ft_atoi("314", ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_atoi_errno_recovers_after_invalid, "ft_atoi clears errno after invalid input")
{
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(0, ft_atoi("invalid", ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    FT_ASSERT_EQ(81, ft_atoi("81", ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}
