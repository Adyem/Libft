#include "../test_internal.hpp"
#include "../../CPP_class/class_big_number.hpp"
#include "../../System_utils/test_runner.hpp"
#include <cstring>

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_ft_big_number_assign_base_invalid_input_keeps_value,
    "ft_big_number assign_base invalid input keeps previous value")
{
    ft_big_number number_value;

    number_value.assign("42");
    number_value.assign_base("1G", 16);
    FT_ASSERT_EQ(0, std::strcmp(number_value.c_str(), "42"));
    return (1);
}

FT_TEST(test_ft_big_number_divide_by_zero_returns_zero_result,
    "ft_big_number divide by zero returns default result value")
{
    ft_big_number numerator;
    ft_big_number denominator;
    ft_big_number quotient;

    numerator.assign("25");
    denominator.assign("0");
    quotient = numerator / denominator;
    FT_ASSERT_EQ(0, std::strcmp(quotient.c_str(), "0"));
    return (1);
}
