#include "../test_internal.hpp"
#include "../../Modules/Advanced/advanced.hpp"
#include "../../Modules/CMA/CMA.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_adv_str_to_title_case)
{
    char *transformed_value;
    ft_bool success;

    transformed_value = adv_str_to_title_case("hello world");
    success = FT_TRUE;
    if (transformed_value == ft_nullptr)
        success = FT_FALSE;
    if (success == FT_TRUE && ft_strcmp("Hello World", transformed_value) != 0)
        success = FT_FALSE;
    cma_free(transformed_value);
    if (success == FT_TRUE)
        return (1);
    return (0);
}

FT_TEST(test_adv_str_to_snake_case)
{
    char *transformed_value;
    ft_bool success;

    transformed_value = adv_str_to_snake_case("Hello, World!");
    success = FT_TRUE;
    if (transformed_value == ft_nullptr)
        success = FT_FALSE;
    if (success == FT_TRUE && ft_strcmp("hello_world", transformed_value) != 0)
        success = FT_FALSE;
    cma_free(transformed_value);
    if (success == FT_TRUE)
        return (1);
    return (0);
}

FT_TEST(test_adv_str_to_camel_case)
{
    char *transformed_value;
    ft_bool success;

    transformed_value = adv_str_to_camel_case("hello world");
    success = FT_TRUE;
    if (transformed_value == ft_nullptr)
        success = FT_FALSE;
    if (success == FT_TRUE && ft_strcmp("helloWorld", transformed_value) != 0)
        success = FT_FALSE;
    cma_free(transformed_value);
    if (success == FT_TRUE)
        return (1);
    return (0);
}

FT_TEST(test_adv_str_normalize_whitespace)
{
    char *transformed_value;
    ft_bool success;

    transformed_value = adv_str_normalize_whitespace("  alpha\t beta\n gamma  ");
    success = FT_TRUE;
    if (transformed_value == ft_nullptr)
        success = FT_FALSE;
    if (success == FT_TRUE && ft_strcmp("alpha beta gamma", transformed_value) != 0)
        success = FT_FALSE;
    cma_free(transformed_value);
    if (success == FT_TRUE)
        return (1);
    return (0);
}

FT_TEST(test_adv_text_transform_null_input)
{
    FT_ASSERT_EQ(ft_nullptr, adv_str_to_title_case(ft_nullptr));
    FT_ASSERT_EQ(ft_nullptr, adv_str_to_snake_case(ft_nullptr));
    FT_ASSERT_EQ(ft_nullptr, adv_str_to_camel_case(ft_nullptr));
    FT_ASSERT_EQ(ft_nullptr, adv_str_normalize_whitespace(ft_nullptr));
    return (1);
}

FT_TEST(test_adv_text_transform_allocation_failure)
{
    char *transformed_value;

    cma_set_alloc_limit(1);
    transformed_value = adv_str_to_snake_case("allocation failure");
    cma_set_alloc_limit(0);
    if (transformed_value != ft_nullptr)
    {
        cma_free(transformed_value);
        return (0);
    }
    return (1);
}
