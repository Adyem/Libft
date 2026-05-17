#include "../test_internal.hpp"
#include "../../Modules/Math/math.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include <cstring>

#ifndef LIBFT_TEST_BUILD
#endif

#define INIT_BIG_NUMBER(number) FT_ASSERT_EQ(FT_ERR_SUCCESS, number.initialize())

static void destroy_big_number_result(ft_big_number *number)
{
    if (number == ft_nullptr)
        return ;
    (void)number->destroy();
    delete number;
    return ;
}

FT_TEST(test_math_big_gcd_large_operands)
{
    ft_big_number first_number;
    ft_big_number second_number;

    INIT_BIG_NUMBER(first_number);
    INIT_BIG_NUMBER(second_number);

    first_number.assign("123456789012345678901234567890");
    second_number.assign("987654321098765432109876543210");

    ft_big_number *gcd_value;

    gcd_value = math_big_gcd(first_number, second_number);
    FT_ASSERT(gcd_value != ft_nullptr);
    FT_ASSERT_EQ(0, std::strcmp(gcd_value->c_str(), "9000000000900000000090"));

    ft_big_number negative_first_number;
    ft_big_number negative_second_number;

    INIT_BIG_NUMBER(negative_first_number);
    INIT_BIG_NUMBER(negative_second_number);

    negative_first_number.assign("-123456789012345678901234567890");
    negative_second_number.assign("-987654321098765432109876543210");

    ft_big_number *negative_gcd_value;

    negative_gcd_value = math_big_gcd(negative_first_number,
        negative_second_number);
    FT_ASSERT(negative_gcd_value != ft_nullptr);
    FT_ASSERT_EQ(0, std::strcmp(negative_gcd_value->c_str(),
        "9000000000900000000090"));
    destroy_big_number_result(gcd_value);
    destroy_big_number_result(negative_gcd_value);
    return (1);
}

FT_TEST(test_math_big_lcm_large_operands)
{
    ft_big_number first_number;
    ft_big_number second_number;

    INIT_BIG_NUMBER(first_number);
    INIT_BIG_NUMBER(second_number);

    first_number.assign("123456789012345678901234567890");
    second_number.assign("987654321098765432109876543210");

    ft_big_number *lcm_value;

    lcm_value = math_big_lcm(first_number, second_number);
    FT_ASSERT(lcm_value != ft_nullptr);
    FT_ASSERT_EQ(0, std::strcmp(lcm_value->c_str(),
        "13548070124980948012498094801236261410"));

    ft_big_number negative_second_number;
    INIT_BIG_NUMBER(negative_second_number);

    negative_second_number.assign("-987654321098765432109876543210");

    ft_big_number *mixed_lcm_value;

    mixed_lcm_value = math_big_lcm(first_number, negative_second_number);
    FT_ASSERT(mixed_lcm_value != ft_nullptr);
    FT_ASSERT_EQ(0, std::strcmp(mixed_lcm_value->c_str(),
        "13548070124980948012498094801236261410"));

    ft_big_number zero_number;
    INIT_BIG_NUMBER(zero_number);

    ft_big_number *zero_lcm_value;

    zero_lcm_value = math_big_lcm(first_number, zero_number);
    FT_ASSERT(zero_lcm_value != ft_nullptr);
    FT_ASSERT_EQ(0, std::strcmp(zero_lcm_value->c_str(), "0"));
    destroy_big_number_result(lcm_value);
    destroy_big_number_result(mixed_lcm_value);
    destroy_big_number_result(zero_lcm_value);
    return (1);
}

#undef INIT_BIG_NUMBER
