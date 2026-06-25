#include "../test_internal.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/Math/math.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/Math/math_interval.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_basic_validate_int_ok)
{
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_validate_int("123"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, math_validate_int("456"));
    return (1);
}

FT_TEST(test_basic_validate_int_empty)
{
    FT_ASSERT(FT_ERR_SUCCESS != ft_validate_int("+"));
    FT_ASSERT(FT_ERR_SUCCESS != math_validate_int("-"));
    return (1);
}

FT_TEST(test_basic_validate_int_range)
{
    FT_ASSERT(FT_ERR_SUCCESS != ft_validate_int("2147483648"));
    FT_ASSERT(FT_ERR_SUCCESS != math_validate_int("-2147483649"));
    return (1);
}

FT_TEST(test_basic_validate_int_invalid)
{
    FT_ASSERT(FT_ERR_SUCCESS != ft_validate_int("12a3"));
    FT_ASSERT(FT_ERR_SUCCESS != math_validate_int("123b"));
    return (1);
}

FT_TEST(test_basic_validate_int_nullptr)
{
    FT_ASSERT(FT_ERR_SUCCESS != ft_validate_int(ft_nullptr));
    return (1);
}

FT_TEST(test_basic_validate_int_leading_whitespace_fails)
{
    FT_ASSERT(FT_ERR_SUCCESS != ft_validate_int(" 123"));
    FT_ASSERT(FT_ERR_SUCCESS != ft_validate_int("\t456"));
    return (1);
}

FT_TEST(test_basic_validate_int_trailing_whitespace_fails)
{
    FT_ASSERT(FT_ERR_SUCCESS != ft_validate_int("789 "));
    FT_ASSERT(FT_ERR_SUCCESS != ft_validate_int("321\n"));
    return (1);
}

FT_TEST(test_basic_validate_int_accepts_signed_limits)
{
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_validate_int("2147483647"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_validate_int("-2147483648"));
    return (1);
}

FT_TEST(test_basic_validate_int_allows_leading_plus_and_zeroes)
{
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_validate_int("+42"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, math_validate_int("00099"));
    return (1);
}

FT_TEST(test_basic_validate_int_allows_plus_signed_limits)
{
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_validate_int("+2147483647"));
    FT_ASSERT(FT_ERR_SUCCESS != ft_validate_int("+2147483648"));
    return (1);
}

FT_TEST(test_basic_validate_int_accepts_zero_variants)
{
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_validate_int("0"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, math_validate_int("-0"));
    return (1);
}

FT_TEST(test_basic_validate_int_rejects_embedded_spacing_and_signs)
{
    FT_ASSERT(FT_ERR_SUCCESS != ft_validate_int("1 23"));
    FT_ASSERT(FT_ERR_SUCCESS != math_validate_int("12-3"));
    return (1);
}

FT_TEST(test_basic_validate_int_rejects_empty_string)
{
    FT_ASSERT(FT_ERR_SUCCESS != ft_validate_int(""));
    FT_ASSERT(FT_ERR_SUCCESS != math_validate_int(""));
    return (1);
}

FT_TEST(test_basic_validate_int_rejects_double_signs)
{
    FT_ASSERT(FT_ERR_SUCCESS != ft_validate_int("++1"));
    FT_ASSERT(FT_ERR_SUCCESS != math_validate_int("--2"));
    return (1);
}

FT_TEST(test_basic_validate_int_rejects_mixed_signs_and_trailing_sign)
{
    FT_ASSERT(FT_ERR_SUCCESS != ft_validate_int("+-3"));
    FT_ASSERT(FT_ERR_SUCCESS != math_validate_int("-+4"));
    FT_ASSERT(FT_ERR_SUCCESS != ft_validate_int("123+"));
    FT_ASSERT(FT_ERR_SUCCESS != math_validate_int("-456-"));
    return (1);
}

FT_TEST(test_basic_math_validate_int_accepts_signed_limits)
{
    FT_ASSERT_EQ(FT_ERR_SUCCESS, math_validate_int("2147483647"));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, math_validate_int("-2147483648"));
    return (1);
}

FT_TEST(test_basic_math_validate_int_rejects_overflow)
{
    FT_ASSERT(FT_ERR_SUCCESS != math_validate_int("2147483648"));
    FT_ASSERT(FT_ERR_SUCCESS != math_validate_int("-2147483649"));
    FT_ASSERT(FT_ERR_SUCCESS != math_validate_int("+2147483648"));
    return (1);
}
