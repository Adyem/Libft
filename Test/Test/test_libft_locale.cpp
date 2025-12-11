#include "../../Libft/libft.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../CPP_class/class_nullptr.hpp"

#if LIBFT_HAS_LOCALE_HELPERS

FT_TEST(test_locale_compare_basic_ordering, "ft_locale_compare orders strings with locale collation")
{
    int comparison_result;

    comparison_result = ft_locale_compare("apple", "banana", ft_nullptr);
    FT_ASSERT(comparison_result < 0);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_locale_compare_invalid_arguments, "ft_locale_compare rejects null pointers")
{
    FT_ASSERT_EQ(-1, ft_locale_compare(ft_nullptr, "value", "C"));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_locale_compare_invalid_locale, "ft_locale_compare propagates locale load failures")
{
    FT_ASSERT_EQ(-1, ft_locale_compare("alpha", "beta", "nonexistent_locale"));
    FT_ASSERT_EQ(FT_ERR_CONFIGURATION, ft_errno);
    return (1);
}

FT_TEST(test_locale_casefold_basic_transformation, "ft_locale_casefold lowers mixed case strings")
{
    ft_string folded;

    folded = ft_locale_casefold("MiXeD", "C");
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    FT_ASSERT(folded == "mixed");
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, folded.get_error());
    return (1);
}

FT_TEST(test_locale_casefold_invalid_locale, "ft_locale_casefold reports locale configuration errors")
{
    ft_string folded;

    folded = ft_locale_casefold("Input", "missing_LOCALE");
    FT_ASSERT_EQ(FT_ERR_CONFIGURATION, ft_errno);
    FT_ASSERT_EQ(FT_ERR_CONFIGURATION, folded.get_error());
    return (1);
}

FT_TEST(test_locale_casefold_invalid_argument, "ft_locale_casefold rejects null input strings")
{
    ft_string folded;

    folded = ft_locale_casefold(ft_nullptr, ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, folded.get_error());
    return (1);
}

#endif
