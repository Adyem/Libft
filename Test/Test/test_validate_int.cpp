#include "../../Libft/libft.hpp"
#include "../../Math/math.hpp"
#include "../../CMA/CMA.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_validate_int_ok, "validate int ok")
{
    FT_ASSERT_EQ(0, ft_validate_int("123"));
    FT_ASSERT_EQ(0, math_validate_int("456"));
    return (1);
}

FT_TEST(test_validate_int_empty, "validate int empty")
{
    FT_ASSERT_EQ(1, ft_validate_int("+"));
    FT_ASSERT_EQ(1, math_validate_int("-"));
    return (1);
}

FT_TEST(test_validate_int_range, "validate int range")
{
    FT_ASSERT_EQ(2, ft_validate_int("2147483648"));
    FT_ASSERT_EQ(2, math_validate_int("-2147483649"));
    return (1);
}

FT_TEST(test_validate_int_invalid, "validate int invalid")
{
    FT_ASSERT_EQ(3, ft_validate_int("12a3"));
    FT_ASSERT_EQ(3, math_validate_int("123b"));
    return (1);
}

FT_TEST(test_cma_atoi_ok, "cma atoi ok")
{
    int *number;
    int test_ok;

    number = cma_atoi("789");
    if (number == ft_nullptr)
        return (0);
    test_ok = (*number == 789);
    cma_free(number);
    return (test_ok);
}

FT_TEST(test_cma_atoi_invalid, "cma atoi invalid")
{
    FT_ASSERT_EQ(ft_nullptr, cma_atoi("99x"));
    return (1);
}
