#include "../test_internal.hpp"
#include "../../Modules/Math/roll.hpp"
#include "../../Modules/CMA/CMA.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_math_roll_null_input)
{
    int *value;

    value = math_roll(ft_nullptr);
    if (value != ft_nullptr)
    {
        cma_free(value);
        return (0);
    }
    return (1);
}

FT_TEST(test_math_roll_empty_expression)
{
    int *value;

    value = math_roll("");
    if (value != ft_nullptr)
    {
        cma_free(value);
        return (0);
    }
    return (1);
}

FT_TEST(test_math_roll_invalid_character)
{
    int *value;

    value = math_roll("2a+3");
    if (value != ft_nullptr)
    {
        cma_free(value);
        return (0);
    }
    return (1);
}

FT_TEST(test_math_roll_unbalanced_parentheses)
{
    int *value;

    value = math_roll(")2+3(");
    if (value != ft_nullptr)
    {
        cma_free(value);
        return (0);
    }
    return (1);
}

FT_TEST(test_math_roll_division_by_zero)
{
    int *value;

    value = math_roll("10/0");
    if (value != ft_nullptr)
    {
        cma_free(value);
        return (0);
    }
    return (1);
}

FT_TEST(test_math_roll_detects_overflow)
{
    int *value;

    value = math_roll("2147483647+1");
    if (value != ft_nullptr)
    {
        cma_free(value);
        return (0);
    }
    return (1);
}

FT_TEST(test_math_roll_complex_expression)
{
    int *value;

    value = math_roll("((2+3)*(4+5)-6)/(3-1)");
    if (value == ft_nullptr)
        return (0);
    FT_ASSERT_EQ(19, *value);
    cma_free(value);
    return (1);
}

FT_TEST(test_math_roll_operator_precedence)
{
    int *value;

    value = math_roll("2+3*4-5/2+6*(3+1)");
    if (value == ft_nullptr)
        return (0);
    FT_ASSERT_EQ(36, *value);
    cma_free(value);
    return (1);
}

FT_TEST(test_math_roll_negative_numbers)
{
    int *value;

    value = math_roll("-5+3-10/2+(3-10)*2");
    if (value == ft_nullptr)
        return (0);
    FT_ASSERT_EQ(-21, *value);
    cma_free(value);
    return (1);
}

FT_TEST(test_math_roll_dice_expression)
{
    int *value;

    value = math_roll("2d6");
    if (value == ft_nullptr)
        return (0);
    FT_ASSERT(*value >= 2);
    FT_ASSERT(*value <= 12);
    cma_free(value);
    return (1);
}

FT_TEST(test_math_roll_long_expression)
{
    int *value;

    value = math_roll("1+2+3+4+5+6+7+8+9+10+11+12+13+14+15+16+17+18+19+20-5-4-3-2-1+6*3-(2*2)+(3*(4+5))-6/2+((3+4)*(5-2))");
    if (value == ft_nullptr)
        return (0);
    FT_ASSERT_EQ(254, *value);
    cma_free(value);
    return (1);
}
