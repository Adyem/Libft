#include "../../Math/roll.hpp"
#include "../../CMA/CMA.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_math_roll_null_input, "math_roll rejects null input")
{
    int *value;

    ft_errno = 0;
    value = math_roll(ft_nullptr);
    if (value != ft_nullptr)
    {
        cma_free(value);
        return (0);
    }
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_math_roll_empty_expression, "math_roll rejects empty expression")
{
    int *value;

    ft_errno = FT_ERR_NO_MEMORY;
    value = math_roll("");
    if (value != ft_nullptr)
    {
        cma_free(value);
        return (0);
    }
    FT_ASSERT(ft_errno == FT_ERR_INVALID_ARGUMENT || ft_errno == FT_ER_SUCCESSS);
    return (1);
}

FT_TEST(test_math_roll_invalid_character, "math_roll rejects invalid characters")
{
    int *value;

    ft_errno = FT_ERR_NO_MEMORY;
    value = math_roll("2a+3");
    if (value != ft_nullptr)
    {
        cma_free(value);
        return (0);
    }
    FT_ASSERT(ft_errno == FT_ERR_INVALID_ARGUMENT || ft_errno == FT_ER_SUCCESSS);
    return (1);
}

FT_TEST(test_math_roll_unbalanced_parentheses, "math_roll rejects unbalanced parentheses")
{
    int *value;

    ft_errno = FT_ERR_NO_MEMORY;
    value = math_roll(")2+3(");
    if (value != ft_nullptr)
    {
        cma_free(value);
        return (0);
    }
    FT_ASSERT(ft_errno == FT_ERR_INVALID_ARGUMENT || ft_errno == FT_ER_SUCCESSS);
    return (1);
}

FT_TEST(test_math_roll_division_by_zero, "math_roll rejects division by zero")
{
    int *value;

    ft_errno = FT_ERR_NO_MEMORY;
    value = math_roll("10/0");
    if (value != ft_nullptr)
    {
        cma_free(value);
        return (0);
    }
    FT_ASSERT(ft_errno == FT_ERR_INVALID_ARGUMENT || ft_errno == FT_ER_SUCCESSS);
    return (1);
}

FT_TEST(test_math_roll_detects_overflow, "math_roll rejects overflowing results")
{
    int *value;

    ft_errno = FT_ERR_NO_MEMORY;
    value = math_roll("2147483647+1");
    if (value != ft_nullptr)
    {
        cma_free(value);
        return (0);
    }
    FT_ASSERT(ft_errno == FT_ERR_INVALID_ARGUMENT || ft_errno == FT_ER_SUCCESSS);
    return (1);
}

FT_TEST(test_math_roll_complex_expression, "math_roll handles nested arithmetic")
{
    int *value;

    ft_errno = 0;
    value = math_roll("((2+3)*(4+5)-6)/(3-1)");
    if (value == ft_nullptr)
        return (0);
    FT_ASSERT_EQ(19, *value);
    FT_ASSERT_EQ(0, ft_errno);
    cma_free(value);
    return (1);
}

FT_TEST(test_math_roll_operator_precedence, "math_roll respects operator precedence")
{
    int *value;

    ft_errno = 0;
    value = math_roll("2+3*4-5/2+6*(3+1)");
    if (value == ft_nullptr)
        return (0);
    FT_ASSERT_EQ(36, *value);
    FT_ASSERT_EQ(0, ft_errno);
    cma_free(value);
    return (1);
}

FT_TEST(test_math_roll_negative_numbers, "math_roll handles unary negatives")
{
    int *value;

    ft_errno = 0;
    value = math_roll("-5+3-10/2+(3-10)*2");
    if (value == ft_nullptr)
        return (0);
    FT_ASSERT_EQ(-21, *value);
    FT_ASSERT_EQ(0, ft_errno);
    cma_free(value);
    return (1);
}

FT_TEST(test_math_roll_dice_expression, "math_roll evaluates dice expressions within range")
{
    int *value;

    ft_errno = 0;
    value = math_roll("2d6");
    if (value == ft_nullptr)
        return (0);
    FT_ASSERT(*value >= 2);
    FT_ASSERT(*value <= 12);
    FT_ASSERT_EQ(0, ft_errno);
    cma_free(value);
    return (1);
}

FT_TEST(test_math_roll_long_expression, "math_roll evaluates lengthy expressions")
{
    int *value;

    ft_errno = 0;
    value = math_roll("1+2+3+4+5+6+7+8+9+10+11+12+13+14+15+16+17+18+19+20-5-4-3-2-1+6*3-(2*2)+(3*(4+5))-6/2+((3+4)*(5-2))");
    if (value == ft_nullptr)
        return (0);
    FT_ASSERT_EQ(254, *value);
    FT_ASSERT_EQ(0, ft_errno);
    cma_free(value);
    return (1);
}
