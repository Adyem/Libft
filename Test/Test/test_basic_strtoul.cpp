#include "../test_internal.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/CPP_class/class_nullptr.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_basic_strtoul_decimal)
{
    char *end;
    FT_ASSERT_EQ(static_cast<unsigned long>(123), ft_strtoul("123xyz", &end, 10));
    FT_ASSERT_EQ('x', *end);
    return (1);
}

FT_TEST(test_basic_strtoul_hex)
{
    FT_ASSERT_EQ(static_cast<unsigned long>(0x2a), ft_strtoul("2a", ft_nullptr, 16));
    return (1);
}

FT_TEST(test_basic_strtoul_negative)
{
    FT_ASSERT_EQ(static_cast<unsigned long>(-1), ft_strtoul("-1", ft_nullptr, 10));
    return (1);
}

FT_TEST(test_basic_strtoul_sign_only_sets_error_and_input_pointer)
{
    const char *plus_input = "+";
    const char *minus_input = "-";
    char *end_pointer;

    end_pointer = reinterpret_cast<char *>(0x1);
    FT_ASSERT_EQ(0UL, ft_strtoul(plus_input, &end_pointer, 10));
    FT_ASSERT_EQ(const_cast<char *>(plus_input), end_pointer);

    end_pointer = reinterpret_cast<char *>(0x1);
    FT_ASSERT_EQ(0UL, ft_strtoul(minus_input, &end_pointer, 10));
    FT_ASSERT_EQ(const_cast<char *>(minus_input), end_pointer);
    return (1);
}

FT_TEST(test_basic_strtoul_base0)
{
    char *end;
    FT_ASSERT_EQ(static_cast<unsigned long>(0x2a), ft_strtoul("0x2a", &end, 0));
    FT_ASSERT_EQ('\0', *end);
    return (1);
}

FT_TEST(test_basic_strtoul_base36_mixed_case)
{
    char *end_pointer;

    FT_ASSERT_EQ(static_cast<unsigned long>(1295), ft_strtoul("Zz", &end_pointer, 36));
    FT_ASSERT_EQ('\0', *end_pointer);
    return (1);
}

FT_TEST(test_basic_strtoul_invalid_base)
{
    const char *input_string = "456";
    char *end_pointer;

    end_pointer = reinterpret_cast<char *>(0x1);
    FT_ASSERT_EQ(0UL, ft_strtoul(input_string, &end_pointer, 1));
    FT_ASSERT_EQ(const_cast<char *>(input_string), end_pointer);

    end_pointer = reinterpret_cast<char *>(0x1);
    FT_ASSERT_EQ(0UL, ft_strtoul(input_string, &end_pointer, 37));
    FT_ASSERT_EQ(const_cast<char *>(input_string), end_pointer);
    return (1);
}

FT_TEST(test_basic_strtoul_above_long_max)
{
    char *end;
    unsigned long long expected_value;

    expected_value = static_cast<unsigned long long>(FT_LLONG_MAX) + 1ULL;
    FT_ASSERT_EQ(expected_value, ft_strtoul("9223372036854775808", &end, 10));
    FT_ASSERT_EQ('\0', *end);
    return (1);
}

FT_TEST(test_basic_strtoul_unsigned_long_max)
{
    char *end;

    FT_ASSERT_EQ(FT_ULLONG_MAX, ft_strtoul("18446744073709551615", &end, 10));
    FT_ASSERT_EQ('\0', *end);
    return (1);
}

FT_TEST(test_basic_strtoul_overflow)
{
    char *end;
    const char *overflow_string;

    overflow_string = "184467440737095516159";
    FT_ASSERT_EQ(FT_ULLONG_MAX, ft_strtoul(overflow_string, &end, 10));
    FT_ASSERT_EQ('9', *end);
    return (1);
}

FT_TEST(test_basic_strtoul_skips_leading_whitespace)
{
    char *end_pointer;

    FT_ASSERT_EQ(static_cast<unsigned long>(987), ft_strtoul("\r\n\t 987", &end_pointer, 10));
    FT_ASSERT_EQ('\0', *end_pointer);
    return (1);
}

FT_TEST(test_basic_strtoul_null_input)
{
    char *end_pointer = reinterpret_cast<char *>(0x1);

    FT_ASSERT_EQ(0UL, ft_strtoul(ft_nullptr, &end_pointer, 10));
    FT_ASSERT_EQ(ft_nullptr, end_pointer);
    return (1);
}

FT_TEST(test_basic_strtoul_recovers_after_null_input)
{
    char *end_pointer;
    unsigned long parsed_value;

    end_pointer = reinterpret_cast<char *>(0x1);
    FT_ASSERT_EQ(0UL, ft_strtoul(ft_nullptr, &end_pointer, 0));
    FT_ASSERT_EQ(ft_nullptr, end_pointer);
    end_pointer = reinterpret_cast<char *>(0x1);
    parsed_value = ft_strtoul("42", &end_pointer, 10);
    FT_ASSERT_EQ(static_cast<unsigned long>(42), parsed_value);
    FT_ASSERT_EQ('\0', *end_pointer);
    return (1);
}

FT_TEST(test_basic_strtoul_base_eight_rejects_invalid_digit)
{
    char *end_pointer;
    unsigned long parsed_value;

    parsed_value = ft_strtoul("0778", &end_pointer, 8);
    FT_ASSERT_EQ(static_cast<unsigned long>(63), parsed_value);
    FT_ASSERT_EQ('8', *end_pointer);
    return (1);
}

FT_TEST(test_basic_strtoul_mixed_case_digits_custom_base)
{
    char *end_pointer;
    unsigned long parsed_value;

    parsed_value = ft_strtoul("gF0", &end_pointer, 17);
    FT_ASSERT_EQ(static_cast<unsigned long>(4879), parsed_value);
    FT_ASSERT_EQ('\0', *end_pointer);
    return (1);
}

FT_TEST(test_basic_strtoul_rejects_at_symbol_in_high_base)
{
    char *end_pointer;
    unsigned long parsed_value;

    parsed_value = ft_strtoul("@777", &end_pointer, 36);
    FT_ASSERT_EQ(0UL, parsed_value);
    FT_ASSERT_EQ('@', *end_pointer);
    return (1);
}

FT_TEST(test_basic_strtoul_stops_before_brace_character)
{
    char *end_pointer;
    unsigned long parsed_value;

    parsed_value = ft_strtoul("Zz{", &end_pointer, 36);
    FT_ASSERT_EQ(static_cast<unsigned long>(1295), parsed_value);
    FT_ASSERT_EQ('{', *end_pointer);
    return (1);
}
