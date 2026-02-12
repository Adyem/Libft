#include "../test_internal.hpp"
#include "../../Math/math.hpp"
#include "../../System_utils/test_runner.hpp"
#include <cstring>

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_math_big_gcd_large_operands, "math_big_gcd computes gcd for large operands")
{
    ft_big_number first_number;
    ft_big_number second_number;

    first_number.assign("123456789012345678901234567890");
    FT_ASSERT_EQ(0, ft_big_number::last_operation_error());
    second_number.assign("987654321098765432109876543210");
    FT_ASSERT_EQ(0, ft_big_number::last_operation_error());

    ft_big_number gcd_value = math_big_gcd(first_number, second_number);

    FT_ASSERT_EQ(0, ft_big_number::last_operation_error());
    FT_ASSERT_EQ(0, std::strcmp(gcd_value.c_str(), "9000000000900000000090"));

    ft_big_number negative_first_number;
    ft_big_number negative_second_number;

    negative_first_number.assign("-123456789012345678901234567890");
    FT_ASSERT_EQ(0, ft_big_number::last_operation_error());
    negative_second_number.assign("-987654321098765432109876543210");
    FT_ASSERT_EQ(0, ft_big_number::last_operation_error());

    ft_big_number negative_gcd_value = math_big_gcd(negative_first_number, negative_second_number);

    FT_ASSERT_EQ(0, ft_big_number::last_operation_error());
    FT_ASSERT_EQ(0, std::strcmp(negative_gcd_value.c_str(), "9000000000900000000090"));
    return (1);
}

FT_TEST(test_math_big_lcm_large_operands, "math_big_lcm computes lcm for large operands")
{
    ft_big_number first_number;
    ft_big_number second_number;

    first_number.assign("123456789012345678901234567890");
    FT_ASSERT_EQ(0, ft_big_number::last_operation_error());
    second_number.assign("987654321098765432109876543210");
    FT_ASSERT_EQ(0, ft_big_number::last_operation_error());

    ft_big_number lcm_value = math_big_lcm(first_number, second_number);

    FT_ASSERT_EQ(0, ft_big_number::last_operation_error());
    FT_ASSERT_EQ(0, std::strcmp(lcm_value.c_str(), "13548070124980948012498094801236261410"));

    ft_big_number negative_second_number;

    negative_second_number.assign("-987654321098765432109876543210");
    FT_ASSERT_EQ(0, ft_big_number::last_operation_error());

    ft_big_number mixed_lcm_value = math_big_lcm(first_number, negative_second_number);

    FT_ASSERT_EQ(0, ft_big_number::last_operation_error());
    FT_ASSERT_EQ(0, std::strcmp(mixed_lcm_value.c_str(), "13548070124980948012498094801236261410"));

    ft_big_number zero_number;

    ft_big_number zero_lcm_value = math_big_lcm(first_number, zero_number);

    FT_ASSERT_EQ(0, ft_big_number::last_operation_error());
    FT_ASSERT_EQ(0, std::strcmp(zero_lcm_value.c_str(), "0"));
    return (1);
}
