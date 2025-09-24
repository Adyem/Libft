#include "../../Libft/libft.hpp"
#include "../../Math/math.hpp"
#include "../../CMA/CMA.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_validate_int_ok, "validate int ok")
{
    ft_errno = FT_EINVAL;
    FT_ASSERT_EQ(FT_SUCCESS, ft_validate_int("123"));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    ft_errno = FT_EINVAL;
    FT_ASSERT_EQ(FT_SUCCESS, math_validate_int("456"));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_validate_int_empty, "validate int empty")
{
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(FT_FAILURE, ft_validate_int("+"));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(FT_FAILURE, math_validate_int("-"));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_validate_int_range, "validate int range")
{
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(FT_FAILURE, ft_validate_int("2147483648"));
    FT_ASSERT_EQ(FT_ERANGE, ft_errno);
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(FT_FAILURE, math_validate_int("-2147483649"));
    FT_ASSERT_EQ(FT_ERANGE, ft_errno);
    return (1);
}

FT_TEST(test_validate_int_invalid, "validate int invalid")
{
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(FT_FAILURE, ft_validate_int("12a3"));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(FT_FAILURE, math_validate_int("123b"));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_validate_int_nullptr, "validate int nullptr")
{
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(FT_FAILURE, ft_validate_int(ft_nullptr));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
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
