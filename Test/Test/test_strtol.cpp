#include "../../Libft/libft.hpp"
#include "../../Libft/limits.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_strtol_decimal, "ft_strtol decimal with end pointer")
{
    char *end;
    FT_ASSERT_EQ(123, ft_strtol("123abc", &end, 10));
    FT_ASSERT_EQ('a', *end);
    return (1);
}

FT_TEST(test_strtol_hex_prefix, "ft_strtol hex prefix base 0")
{
    char *end;
    FT_ASSERT_EQ(26, ft_strtol("0x1a", &end, 0));
    FT_ASSERT_EQ('\0', *end);
    return (1);
}

FT_TEST(test_strtol_octal_auto, "ft_strtol octal auto base")
{
    char *end;
    FT_ASSERT_EQ(9, ft_strtol("011", &end, 0));
    FT_ASSERT_EQ('\0', *end);
    return (1);
}

FT_TEST(test_strtol_negative, "ft_strtol negative number")
{
    FT_ASSERT_EQ(-42, ft_strtol("-42", ft_nullptr, 10));
    return (1);
}

FT_TEST(test_strtol_base16_skip, "ft_strtol base16 skip prefix")
{
    char *end;
    FT_ASSERT_EQ(26, ft_strtol("0x1a", &end, 16));
    FT_ASSERT_EQ('\0', *end);
    return (1);
}

FT_TEST(test_strtol_invalid, "ft_strtol invalid string")
{
    char *end;
    FT_ASSERT_EQ(0, ft_strtol("xyz", &end, 10));
    FT_ASSERT_EQ('x', *end);
    return (1);
}

FT_TEST(test_strtol_sign_only_sets_error_and_input_pointer,
        "ft_strtol sign-only inputs set errno and reset end pointer")
{
    const char *plus_input = "+";
    const char *minus_input = "-";
    char *end_pointer;

    end_pointer = reinterpret_cast<char *>(0x1);
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(0L, ft_strtol(plus_input, &end_pointer, 10));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    FT_ASSERT_EQ(const_cast<char *>(plus_input), end_pointer);

    end_pointer = reinterpret_cast<char *>(0x1);
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(0L, ft_strtol(minus_input, &end_pointer, 10));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    FT_ASSERT_EQ(const_cast<char *>(minus_input), end_pointer);
    return (1);
}

FT_TEST(test_strtol_invalid_base, "ft_strtol invalid base returns error and input pointer")
{
    const char *input_string = "123";
    char *end_pointer;

    end_pointer = reinterpret_cast<char *>(0x1);
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(0, ft_strtol(input_string, &end_pointer, 1));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    FT_ASSERT_EQ(const_cast<char *>(input_string), end_pointer);

    end_pointer = reinterpret_cast<char *>(0x1);
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(0, ft_strtol(input_string, &end_pointer, 37));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    FT_ASSERT_EQ(const_cast<char *>(input_string), end_pointer);
    return (1);
}

FT_TEST(test_strtol_maximum_in_range, "ft_strtol handles FT_LONG_MAX without overflow")
{
    char *end;
    ft_string max_string = ft_to_string(FT_LONG_MAX);

    ft_errno = FT_ERR_OUT_OF_RANGE;
    FT_ASSERT_EQ(FT_LONG_MAX, ft_strtol(max_string.c_str(), &end, 10));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ('\0', *end);
    return (1);
}

FT_TEST(test_strtol_positive_overflow, "ft_strtol clamps positive overflow and reports error")
{
    char *end;
    ft_string max_string = ft_to_string(FT_LONG_MAX);
    ft_string overflow_string = max_string;

    overflow_string.append('9');
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(FT_LONG_MAX, ft_strtol(overflow_string.c_str(), &end, 10));
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, ft_errno);
    FT_ASSERT_EQ('\0', *end);
    FT_ASSERT_EQ(static_cast<size_t>(overflow_string.size()),
        static_cast<size_t>(end - overflow_string.c_str()));
    return (1);
}

FT_TEST(test_strtol_negative_overflow, "ft_strtol clamps negative overflow and reports error")
{
    char *end;
    ft_string min_string = ft_to_string(FT_LONG_MIN);
    ft_string underflow_string = min_string;

    underflow_string.append('9');
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(FT_LONG_MIN, ft_strtol(underflow_string.c_str(), &end, 10));
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, ft_errno);
    FT_ASSERT_EQ('\0', *end);
    FT_ASSERT_EQ(static_cast<size_t>(underflow_string.size()),
        static_cast<size_t>(end - underflow_string.c_str()));
    return (1);
}

FT_TEST(test_strtol_uppercase_hex_prefix, "ft_strtol accepts uppercase hex prefix")
{
    char *end_pointer;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(255, ft_strtol("0Xff", &end_pointer, 0));
    FT_ASSERT_EQ('\0', *end_pointer);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strtol_base36_mixed_case, "ft_strtol parses base 36 digits in any case")
{
    char *end_pointer;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(1294L, ft_strtol("Zy", &end_pointer, 36));
    FT_ASSERT_EQ('\0', *end_pointer);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strtol_skips_leading_whitespace, "ft_strtol ignores leading whitespace before digits")
{
    char *end_pointer;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(678L, ft_strtol("\t  678xyz", &end_pointer, 10));
    FT_ASSERT_EQ('x', *end_pointer);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strtol_null_input, "ft_strtol null input sets errno and end pointer")
{
    char *end_pointer = reinterpret_cast<char *>(0x1);

    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(0, ft_strtol(ft_nullptr, &end_pointer, 10));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    FT_ASSERT_EQ(ft_nullptr, end_pointer);
    return (1);
}

FT_TEST(test_strtol_base_two_rejects_invalid_digit,
        "ft_strtol stops parsing when encountering digits beyond the base")
{
    char *end_pointer;
    long parsed_value;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    parsed_value = ft_strtol("10102", &end_pointer, 2);
    FT_ASSERT_EQ(10L, parsed_value);
    FT_ASSERT_EQ('2', *end_pointer);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strtol_mixed_case_digits_custom_base,
        "ft_strtol accepts mixed-case digits for bases above ten")
{
    char *end_pointer;
    long parsed_value;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    parsed_value = ft_strtol("aB19", &end_pointer, 20);
    FT_ASSERT_EQ(84429L, parsed_value);
    FT_ASSERT_EQ('\0', *end_pointer);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strtol_rejects_at_symbol_in_high_base,
        "ft_strtol rejects characters preceding '0' even in high bases")
{
    char *end_pointer;
    long parsed_value;

    ft_errno = ER_SUCCESS;
    parsed_value = ft_strtol("@123", &end_pointer, 36);
    FT_ASSERT_EQ(0L, parsed_value);
    FT_ASSERT_EQ('@', *end_pointer);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_strtol_stops_before_brace_character,
        "ft_strtol stops parsing when encountering characters beyond 'z'")
{
    char *end_pointer;
    long parsed_value;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    parsed_value = ft_strtol("Zz{", &end_pointer, 36);
    FT_ASSERT_EQ(1295L, parsed_value);
    FT_ASSERT_EQ('{', *end_pointer);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}
