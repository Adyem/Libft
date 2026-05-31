#include "../test_internal.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

#if LIBFT_HAS_LOCALE_HELPERS

FT_TEST(test_locale_compare_basic_ordering)
{
    int comparison_result;

    comparison_result = ft_locale_compare("apple", "banana", ft_nullptr);
    FT_ASSERT(comparison_result < 0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_locale_compare_invalid_arguments)
{
    FT_ASSERT_EQ(-1, ft_locale_compare(ft_nullptr, "value", "C"));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_locale_compare_invalid_locale)
{
    FT_ASSERT_EQ(-1, ft_locale_compare("alpha", "beta", "nonexistent_locale"));
    FT_ASSERT_EQ(FT_ERR_CONFIGURATION, ft_errno);
    return (1);
}

FT_TEST(test_locale_casefold_basic_transformation)
{
    ft_string folded;

    folded = ft_locale_casefold("MiXeD", "C");
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_errno);
    FT_ASSERT(folded == "mixed");
    FT_ASSERT_EQ(FT_ERR_SUCCESS, folded.get_error());
    return (1);
}

FT_TEST(test_locale_casefold_invalid_locale)
{
    ft_string folded;

    folded = ft_locale_casefold("Input", "missing_LOCALE");
    FT_ASSERT_EQ(FT_ERR_CONFIGURATION, ft_errno);
    FT_ASSERT_EQ(FT_ERR_CONFIGURATION, folded.get_error());
    return (1);
}

FT_TEST(test_locale_casefold_invalid_argument)
{
    ft_string folded;

    folded = ft_locale_casefold(ft_nullptr, ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, folded.get_error());
    return (1);
}

#endif
