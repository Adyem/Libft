#include <cstdio>
#include "../../Libft/libft.hpp"
#include "../../Libft/limits.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_strtoul_decimal, "ft_strtoul decimal with end pointer")
{
    char *end;
    FT_ASSERT_EQ(static_cast<unsigned long>(123), ft_strtoul("123xyz", &end, 10));
    FT_ASSERT_EQ('x', *end);
    return (1);
}

FT_TEST(test_strtoul_hex, "ft_strtoul hex base 16")
{
    FT_ASSERT_EQ(static_cast<unsigned long>(0x2a), ft_strtoul("2a", ft_nullptr, 16));
    return (1);
}

FT_TEST(test_strtoul_negative, "ft_strtoul negative input")
{
    FT_ASSERT_EQ(static_cast<unsigned long>(-1), ft_strtoul("-1", ft_nullptr, 10));
    return (1);
}

FT_TEST(test_strtoul_sign_only_sets_error_and_input_pointer,
        "ft_strtoul sign-only inputs set errno and reset end pointer")
{
    const char *plus_input = "+";
    const char *minus_input = "-";
    char *end_pointer;

    end_pointer = reinterpret_cast<char *>(0x1);
    ft_errno = FT_ER_SUCCESSS;
    FT_ASSERT_EQ(0UL, ft_strtoul(plus_input, &end_pointer, 10));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    FT_ASSERT_EQ(const_cast<char *>(plus_input), end_pointer);

    end_pointer = reinterpret_cast<char *>(0x1);
    ft_errno = FT_ER_SUCCESSS;
    FT_ASSERT_EQ(0UL, ft_strtoul(minus_input, &end_pointer, 10));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    FT_ASSERT_EQ(const_cast<char *>(minus_input), end_pointer);
    return (1);
}

FT_TEST(test_strtoul_base0, "ft_strtoul base 0 hex prefix")
{
    char *end;
    FT_ASSERT_EQ(static_cast<unsigned long>(0x2a), ft_strtoul("0x2a", &end, 0));
    FT_ASSERT_EQ('\0', *end);
    return (1);
}

FT_TEST(test_strtoul_base36_mixed_case, "ft_strtoul parses base 36 digits in mixed case")
{
    char *end_pointer;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(static_cast<unsigned long>(1295), ft_strtoul("Zz", &end_pointer, 36));
    FT_ASSERT_EQ('\0', *end_pointer);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_strtoul_invalid_base, "ft_strtoul invalid base returns error and input pointer")
{
    const char *input_string = "456";
    char *end_pointer;

    end_pointer = reinterpret_cast<char *>(0x1);
    ft_errno = FT_ER_SUCCESSS;
    FT_ASSERT_EQ(0UL, ft_strtoul(input_string, &end_pointer, 1));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    FT_ASSERT_EQ(const_cast<char *>(input_string), end_pointer);

    end_pointer = reinterpret_cast<char *>(0x1);
    ft_errno = FT_ER_SUCCESSS;
    FT_ASSERT_EQ(0UL, ft_strtoul(input_string, &end_pointer, 37));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    FT_ASSERT_EQ(const_cast<char *>(input_string), end_pointer);
    return (1);
}

FT_TEST(test_strtoul_above_long_max, "ft_strtoul parses values above FT_LONG_MAX")
{
    char value_buffer[64];
    char *end;
    unsigned long expected_value = static_cast<unsigned long>(FT_LONG_MAX);

    expected_value = expected_value + 1UL;
    std::snprintf(value_buffer, sizeof(value_buffer), "%lu", expected_value);
    ft_errno = FT_ERR_OUT_OF_RANGE;
    FT_ASSERT_EQ(expected_value, ft_strtoul(value_buffer, &end, 10));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    FT_ASSERT_EQ('\0', *end);
    return (1);
}

FT_TEST(test_strtoul_unsigned_long_max, "ft_strtoul parses FT_ULONG_MAX without overflow")
{
    char value_buffer[64];
    char *end;

    std::snprintf(value_buffer, sizeof(value_buffer), "%lu", FT_ULONG_MAX);
    ft_errno = FT_ERR_OUT_OF_RANGE;
    FT_ASSERT_EQ(FT_ULONG_MAX, ft_strtoul(value_buffer, &end, 10));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    FT_ASSERT_EQ('\0', *end);
    return (1);
}

FT_TEST(test_strtoul_overflow, "ft_strtoul clamps overflow and reports error")
{
    char value_buffer[64];
    char *end;

    std::snprintf(value_buffer, sizeof(value_buffer), "%lu", FT_ULONG_MAX);
    ft_string overflow_string = value_buffer;
    overflow_string.append('9');
    ft_errno = FT_ER_SUCCESSS;
    FT_ASSERT_EQ(FT_ULONG_MAX, ft_strtoul(overflow_string.c_str(), &end, 10));
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, ft_errno);
    FT_ASSERT_EQ('9', *end);
    return (1);
}

FT_TEST(test_strtoul_skips_leading_whitespace, "ft_strtoul ignores leading whitespace")
{
    char *end_pointer;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(static_cast<unsigned long>(987), ft_strtoul("\r\n\t 987", &end_pointer, 10));
    FT_ASSERT_EQ('\0', *end_pointer);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_strtoul_null_input, "ft_strtoul null input sets errno and end pointer")
{
    char *end_pointer = reinterpret_cast<char *>(0x1);

    ft_errno = FT_ER_SUCCESSS;
    FT_ASSERT_EQ(0UL, ft_strtoul(ft_nullptr, &end_pointer, 10));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    FT_ASSERT_EQ(ft_nullptr, end_pointer);
    return (1);
}

FT_TEST(test_strtoul_recovers_after_null_input, "ft_strtoul clears errno after a subsequent valid parse")
{
    char *end_pointer;
    unsigned long parsed_value;

    end_pointer = reinterpret_cast<char *>(0x1);
    ft_errno = FT_ER_SUCCESSS;
    FT_ASSERT_EQ(0UL, ft_strtoul(ft_nullptr, &end_pointer, 0));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    FT_ASSERT_EQ(ft_nullptr, end_pointer);
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    end_pointer = reinterpret_cast<char *>(0x1);
    parsed_value = ft_strtoul("42", &end_pointer, 10);
    FT_ASSERT_EQ(static_cast<unsigned long>(42), parsed_value);
    FT_ASSERT_EQ('\0', *end_pointer);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_strtoul_base_eight_rejects_invalid_digit,
        "ft_strtoul stops consuming digits outside the specified base")
{
    char *end_pointer;
    unsigned long parsed_value;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    parsed_value = ft_strtoul("0778", &end_pointer, 8);
    FT_ASSERT_EQ(static_cast<unsigned long>(63), parsed_value);
    FT_ASSERT_EQ('8', *end_pointer);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_strtoul_mixed_case_digits_custom_base,
        "ft_strtoul accepts mixed-case alphabetic digits within range")
{
    char *end_pointer;
    unsigned long parsed_value;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    parsed_value = ft_strtoul("gF0", &end_pointer, 17);
    FT_ASSERT_EQ(static_cast<unsigned long>(4879), parsed_value);
    FT_ASSERT_EQ('\0', *end_pointer);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_strtoul_rejects_at_symbol_in_high_base,
        "ft_strtoul rejects characters that precede '0' in the digit table")
{
    char *end_pointer;
    unsigned long parsed_value;

    ft_errno = FT_ER_SUCCESSS;
    parsed_value = ft_strtoul("@777", &end_pointer, 36);
    FT_ASSERT_EQ(0UL, parsed_value);
    FT_ASSERT_EQ('@', *end_pointer);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_strtoul_stops_before_brace_character,
        "ft_strtoul stops parsing when encountering characters beyond 'z'")
{
    char *end_pointer;
    unsigned long parsed_value;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    parsed_value = ft_strtoul("Zz{", &end_pointer, 36);
    FT_ASSERT_EQ(static_cast<unsigned long>(1295), parsed_value);
    FT_ASSERT_EQ('{', *end_pointer);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}
