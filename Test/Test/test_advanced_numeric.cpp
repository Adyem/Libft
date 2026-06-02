#include "../test_internal.hpp"
#include "../../Modules/Advanced/advanced.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/CPP_class/class_string.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/CMA/CMA.hpp"
#include <limits>

#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_adv_atoi_parses_number)
{
    int32_t *parsed_value;

    parsed_value = adv_atoi("12345");
    FT_ASSERT(parsed_value != ft_nullptr);
    FT_ASSERT_EQ(12345, *parsed_value);
    cma_free(parsed_value);
    return (1);
}

FT_TEST(test_adv_atoi_rejects_invalid_input)
{
    FT_ASSERT_EQ(ft_nullptr, adv_atoi("12x"));
    FT_ASSERT_EQ(ft_nullptr, adv_atoi(" 42"));
    FT_ASSERT_EQ(ft_nullptr, adv_atoi(ft_nullptr));
    return (1);
}

FT_TEST(test_adv_atoi_accepts_explicit_sign)
{
    int32_t *parsed_value;

    parsed_value = adv_atoi("+42");
    FT_ASSERT(parsed_value != ft_nullptr);
    FT_ASSERT_EQ(42, *parsed_value);
    cma_free(parsed_value);
    return (1);
}

FT_TEST(test_adv_atoi_allocation_failure)
{
    int32_t *parsed_value;

    cma_set_alloc_limit(1);
    parsed_value = adv_atoi("99");
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(ft_nullptr, parsed_value);
    return (1);
}

FT_TEST(test_adv_itoa_handles_zero_and_negative)
{
    char *zero_value;
    char *negative_value;

    zero_value = adv_itoa(0);
    negative_value = adv_itoa(-42);
    FT_ASSERT(zero_value != ft_nullptr);
    FT_ASSERT(negative_value != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strcmp("0", zero_value));
    FT_ASSERT_EQ(0, ft_strcmp("-42", negative_value));
    cma_free(zero_value);
    cma_free(negative_value);
    return (1);
}

FT_TEST(test_adv_itoa_handles_int_minimum)
{
    char *minimum_value;

    minimum_value = adv_itoa(std::numeric_limits<int32_t>::min());
    FT_ASSERT(minimum_value != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strcmp("-2147483648", minimum_value));
    cma_free(minimum_value);
    return (1);
}

FT_TEST(test_adv_itoa_allocation_failure)
{
    char *value_string;

    cma_set_alloc_limit(1);
    value_string = adv_itoa(123);
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(ft_nullptr, value_string);
    return (1);
}

FT_TEST(test_adv_itoa_base_formats_multiple_bases)
{
    char *binary_value;
    char *decimal_value;
    char *hexadecimal_value;

    binary_value = adv_itoa_base(10, 2);
    decimal_value = adv_itoa_base(-10, 10);
    hexadecimal_value = adv_itoa_base(255, 16);
    FT_ASSERT(binary_value != ft_nullptr);
    FT_ASSERT(decimal_value != ft_nullptr);
    FT_ASSERT(hexadecimal_value != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strcmp("1010", binary_value));
    FT_ASSERT_EQ(0, ft_strcmp("-10", decimal_value));
    FT_ASSERT_EQ(0, ft_strcmp("FF", hexadecimal_value));
    cma_free(binary_value);
    cma_free(decimal_value);
    cma_free(hexadecimal_value);
    return (1);
}

FT_TEST(test_adv_itoa_base_rejects_invalid_base)
{
    FT_ASSERT_EQ(ft_nullptr, adv_itoa_base(5, 1));
    FT_ASSERT_EQ(ft_nullptr, adv_itoa_base(5, 17));
    return (1);
}

FT_TEST(test_adv_itoa_base_negative_non_decimal)
{
    char *binary_value;

    binary_value = adv_itoa_base(-10, 2);
    FT_ASSERT(binary_value != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strcmp("1010", binary_value));
    cma_free(binary_value);
    return (1);
}

FT_TEST(test_adv_itoa_base_allocation_failure)
{
    char *value_string;

    cma_set_alloc_limit(1);
    value_string = adv_itoa_base(255, 16);
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(ft_nullptr, value_string);
    return (1);
}

FT_TEST(test_adv_to_string_int32_boundaries)
{
    ft_string *minimum_string;
    ft_string *maximum_string;

    minimum_string = adv_to_string(std::numeric_limits<int32_t>::min());
    maximum_string = adv_to_string(std::numeric_limits<int32_t>::max());
    FT_ASSERT(minimum_string != ft_nullptr);
    FT_ASSERT(maximum_string != ft_nullptr);
    FT_ASSERT_EQ(0, ft_strcmp("-2147483648", minimum_string->c_str()));
    FT_ASSERT_EQ(0, ft_strcmp("2147483647", maximum_string->c_str()));
    delete minimum_string;
    delete maximum_string;
    return (1);
}
