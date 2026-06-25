#include "../test_internal.hpp"
#include "../../Modules/Advanced/advanced.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/CMA/CMA.hpp"
#include "../../Modules/CPP_class/class_string.hpp"
#include "../../Modules/Template/vector.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_adv_strjoin_vector_joins_with_separator)
{
    ft_vector<ft_string> strings;
    ft_string first_string;
    ft_string second_string;
    ft_string *joined_string;
    ft_bool success;

    success = FT_TRUE;
    if (strings.initialize() != FT_ERR_SUCCESS)
        success = FT_FALSE;
    if (success == FT_TRUE && first_string.initialize("alpha") != FT_ERR_SUCCESS)
        success = FT_FALSE;
    if (success == FT_TRUE && second_string.initialize("beta") != FT_ERR_SUCCESS)
        success = FT_FALSE;
    if (success == FT_TRUE && strings.push_back(first_string) != FT_ERR_SUCCESS)
        success = FT_FALSE;
    if (success == FT_TRUE && strings.push_back(second_string) != FT_ERR_SUCCESS)
        success = FT_FALSE;
    joined_string = adv_strjoin_vector(strings, " + ");
    if (success == FT_TRUE && joined_string == ft_nullptr)
        success = FT_FALSE;
    if (success == FT_TRUE && ft_strcmp("alpha + beta", joined_string->c_str()) != 0)
        success = FT_FALSE;
    if (joined_string != ft_nullptr)
        delete joined_string;
    if (success == FT_TRUE)
        return (1);
    return (0);
}

FT_TEST(test_adv_strjoin_vector_handles_empty_vector)
{
    ft_vector<ft_string> strings;
    ft_string *joined_string;
    ft_bool success;

    success = FT_TRUE;
    if (strings.initialize() != FT_ERR_SUCCESS)
        success = FT_FALSE;
    joined_string = adv_strjoin_vector(strings, ",");
    if (success == FT_TRUE && joined_string == ft_nullptr)
        success = FT_FALSE;
    if (success == FT_TRUE && ft_strcmp("", joined_string->c_str()) != 0)
        success = FT_FALSE;
    if (joined_string != ft_nullptr)
        delete joined_string;
    if (success == FT_TRUE)
        return (1);
    return (0);
}

FT_TEST(test_adv_format_string_formats_allocated_string)
{
    ft_string *formatted_string;
    ft_bool success;

    success = FT_TRUE;
    formatted_string = adv_format_string("%s %d", "value", 42);
    if (formatted_string == ft_nullptr)
        success = FT_FALSE;
    if (success == FT_TRUE && ft_strcmp("value 42", formatted_string->c_str()) != 0)
        success = FT_FALSE;
    if (formatted_string != ft_nullptr)
        delete formatted_string;
    if (success == FT_TRUE)
        return (1);
    return (0);
}

FT_TEST(test_adv_format_string_null_format_returns_null)
{
    ft_string *formatted_string;

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-security"
#endif
    formatted_string = adv_format_string(ft_nullptr);
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
    FT_ASSERT_EQ(ft_nullptr, formatted_string);
    return (1);
}

FT_TEST(test_adv_format_string_allocation_failure)
{
    ft_string *formatted_string;

    cma_set_alloc_limit(1);
    formatted_string = adv_format_string("%s", "failure");
    cma_set_alloc_limit(0);
    if (formatted_string != ft_nullptr)
    {
        delete formatted_string;
        return (0);
    }
    return (1);
}
