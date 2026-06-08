#include "../test_internal.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/Advanced/advanced.hpp"
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
    return (1);
}

FT_TEST(test_locale_compare_invalid_arguments)
{
    FT_ASSERT_EQ(-1, ft_locale_compare(ft_nullptr, "value", "C"));
    return (1);
}

FT_TEST(test_locale_compare_invalid_locale)
{
    FT_ASSERT_EQ(-1, ft_locale_compare("alpha", "beta", "nonexistent_locale"));
    return (1);
}

FT_TEST(test_locale_casefold_basic_transformation)
{
    ft_string *folded;

    folded = adv_locale_casefold("MiXeD", "C");
    FT_ASSERT(folded != ft_nullptr);
    FT_ASSERT(*folded == "mixed");
    FT_ASSERT_EQ(FT_ERR_SUCCESS, folded->get_error());
    (void)folded->destroy();
    delete folded;
    return (1);
}

FT_TEST(test_locale_casefold_invalid_locale)
{
    ft_string *folded;

    folded = adv_locale_casefold("Input", "missing_LOCALE");
    FT_ASSERT_EQ(ft_nullptr, folded);
    return (1);
}

FT_TEST(test_locale_casefold_invalid_argument)
{
    ft_string *folded;

    folded = adv_locale_casefold(ft_nullptr, ft_nullptr);
    FT_ASSERT_EQ(ft_nullptr, folded);
    return (1);
}

#endif
