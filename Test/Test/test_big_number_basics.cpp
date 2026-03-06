#include "../test_internal.hpp"
#include "../../CPP_class/class_big_number.hpp"
#include "../../CPP_class/class_string.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include "../../Template/move.hpp"
#include <cstring>

#ifndef LIBFT_TEST_BUILD
#endif

#define INIT_BIG_NUMBER(number) FT_ASSERT_EQ(FT_ERR_SUCCESS, number.initialize())

FT_TEST(test_big_number_default_state, "ft_big_number default state is zero")
{
    ft_big_number number;
    INIT_BIG_NUMBER(number);

    FT_ASSERT(number.empty());
    FT_ASSERT_EQ(0, number.size());
    FT_ASSERT_EQ(0, std::strcmp(number.c_str(), "0"));
    FT_ASSERT(!number.is_negative());
    FT_ASSERT(!number.is_positive());
    return (1);
}

FT_TEST(test_big_number_assign_trim, "ft_big_number assign trims leading zeros")
{
    ft_big_number number;
    INIT_BIG_NUMBER(number);

    number.assign("0001234500");
    FT_ASSERT_EQ(7, number.size());
    FT_ASSERT_EQ(0, std::strcmp(number.c_str(), "1234500"));
    FT_ASSERT(!number.is_negative());
    FT_ASSERT(number.is_positive());
    return (1);
}

FT_TEST(test_big_number_assign_negative, "ft_big_number assign parses negative numbers")
{
    ft_big_number number;
    INIT_BIG_NUMBER(number);

    number.assign("-001234");
    FT_ASSERT_EQ(4, number.size());
    FT_ASSERT_EQ(0, std::strcmp(number.c_str(), "1234"));
    FT_ASSERT(number.is_negative());
    FT_ASSERT(!number.is_positive());

    number.assign("-0");
    FT_ASSERT_EQ(1, number.size());
    FT_ASSERT_EQ(0, std::strcmp(number.c_str(), "0"));
    FT_ASSERT(!number.is_negative());
    FT_ASSERT(!number.is_positive());
    return (1);
}

FT_TEST(test_big_number_assign_base_binary, "ft_big_number assign_base decodes binary strings")
{
    ft_big_number number;
    INIT_BIG_NUMBER(number);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, number.assign_base("101010", 2));
    FT_ASSERT_EQ(0, std::strcmp(number.c_str(), "42"));
    FT_ASSERT(!number.is_negative());
    FT_ASSERT(number.is_positive());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, number.assign_base("-1111111", 2));
    FT_ASSERT_EQ(0, std::strcmp(number.c_str(), "127"));
    FT_ASSERT(number.is_negative());
    FT_ASSERT(!number.is_positive());
    return (1);
}

FT_TEST(test_big_number_assign_base_octal_hex, "ft_big_number assign_base handles octal and hexadecimal")
{
    ft_big_number octal_number;
    ft_big_number hex_number;
    ft_big_number invalid_number;

    INIT_BIG_NUMBER(octal_number);
    INIT_BIG_NUMBER(hex_number);
    INIT_BIG_NUMBER(invalid_number);

    FT_ASSERT_EQ(FT_ERR_SUCCESS, octal_number.assign_base("755", 8));
    FT_ASSERT_EQ(0, std::strcmp(octal_number.c_str(), "493"));
    FT_ASSERT(!octal_number.is_negative());
    FT_ASSERT(octal_number.is_positive());

    FT_ASSERT_EQ(FT_ERR_SUCCESS, hex_number.assign_base("1a3f", 16));
    FT_ASSERT_EQ(0, std::strcmp(hex_number.c_str(), "6719"));
    FT_ASSERT(!hex_number.is_negative());
    FT_ASSERT(hex_number.is_positive());

    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, invalid_number.assign_base("19", 8));
    return (1);
}

FT_TEST(test_big_number_append_helpers, "ft_big_number append_digit and append_unsigned extend number")
{
    ft_big_number number;
    INIT_BIG_NUMBER(number);

    number.append_digit('4');
    number.append_digit('2');
    number.append_unsigned(12345);
    FT_ASSERT_EQ(7, number.size());
    FT_ASSERT_EQ(0, std::strcmp(number.c_str(), "4212345"));
    FT_ASSERT(!number.is_negative());
    FT_ASSERT(number.is_positive());
    return (1);
}

FT_TEST(test_big_number_assign_invalid_digit, "ft_big_number assign rejects invalid characters")
{
    ft_big_number number;
    INIT_BIG_NUMBER(number);

    number.assign("12a3");
    FT_ASSERT_EQ(0, number.size());
    FT_ASSERT_EQ(0, std::strcmp(number.c_str(), "0"));
    return (1);
}

FT_TEST(test_big_number_append_invalid_digit, "ft_big_number append_digit rejects invalid characters")
{
    ft_big_number number;
    INIT_BIG_NUMBER(number);

    number.append_digit('x');
    return (1);
}

FT_TEST(test_big_number_to_string_base_conversions, "ft_big_number to_string_base converts to binary, octal, and hexadecimal")
{
    ft_big_number decimal_value;
    ft_big_number negative_value;

    INIT_BIG_NUMBER(decimal_value);
    INIT_BIG_NUMBER(negative_value);

    decimal_value.assign("255");
    negative_value.assign("-240");

    ft_string hex_output = decimal_value.to_string_base(16);
    FT_ASSERT_EQ(0, ft_string::get_error());
    FT_ASSERT(hex_output == "FF");

    ft_string octal_output = decimal_value.to_string_base(8);
    FT_ASSERT_EQ(0, ft_string::get_error());
    FT_ASSERT(octal_output == "377");

    ft_string binary_output = negative_value.to_string_base(2);
    FT_ASSERT_EQ(0, ft_string::get_error());
    FT_ASSERT(binary_output == "-11110000");

    ft_string invalid_output = decimal_value.to_string_base(1);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_string::get_error());
    return (1);
}

#undef INIT_BIG_NUMBER
