#include "../../System_utils/system_utils.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../CPP_class/class_string_class.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

FT_TEST(test_su_locale_compare_default_ordering,
    "su_locale_compare orders strings using the selected locale")
{
    int comparison_result;

    comparison_result = 0;
    ft_errno = FT_ERR_INVALID_OPERATION;
    FT_ASSERT_EQ(0, su_locale_compare("apple", "banana", ft_nullptr, &comparison_result));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT(comparison_result < 0);
    ft_errno = FT_ERR_CONFIGURATION;
    FT_ASSERT_EQ(0, su_locale_compare("banana", "banana", ft_nullptr, &comparison_result));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(0, comparison_result);
    return (1);
}

FT_TEST(test_su_locale_compare_rejects_invalid_arguments,
    "su_locale_compare validates input pointers")
{
    int comparison_result;

    comparison_result = 99;
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(-1, su_locale_compare(ft_nullptr, "banana", ft_nullptr, &comparison_result));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(-1, su_locale_compare("apple", "banana", ft_nullptr, ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_su_locale_compare_invalid_locale_sets_configuration_error,
    "su_locale_compare reports configuration errors for unknown locales")
{
    int comparison_result;

    comparison_result = 0;
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(-1, su_locale_compare("apple", "banana", "invalid_locale_name", &comparison_result));
    FT_ASSERT_EQ(FT_ERR_CONFIGURATION, ft_errno);
    return (1);
}

FT_TEST(test_su_locale_casefold_lowercases_ascii_input,
    "su_locale_casefold transforms input using locale rules")
{
    ft_string output;

    ft_errno = FT_ERR_TERMINATED;
    FT_ASSERT_EQ(0, su_locale_casefold("MiXeD CaSe", ft_nullptr, output));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT(output == "mixed case");
    return (1);
}

FT_TEST(test_su_locale_casefold_rejects_null_input,
    "su_locale_casefold validates the input pointer")
{
    ft_string output;

    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(-1, su_locale_casefold(ft_nullptr, ft_nullptr, output));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_su_locale_casefold_invalid_locale_sets_configuration_error,
    "su_locale_casefold propagates locale acquisition failures")
{
    ft_string output;

    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(-1, su_locale_casefold("Example", "invalid_locale_name", output));
    FT_ASSERT_EQ(FT_ERR_CONFIGURATION, ft_errno);
    return (1);
}
