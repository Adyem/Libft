#include "../test_internal.hpp"
#include "../../Advanced/advanced.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../CPP_class/class_string.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../CMA/CMA.hpp"
#include <limits>
#include <string>
#include <cstdio>

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_adv_to_string_positive_number,
        "adv_to_string converts positive numbers")
{
    ft_string *converted_value;

    converted_value = adv_to_string(12345);
    FT_ASSERT(converted_value != ft_nullptr);
    FT_ASSERT(std::string(converted_value->c_str()) == "12345");
    delete converted_value;
    return (1);
}

FT_TEST(test_adv_to_string_negative_number,
        "adv_to_string preserves sign for negative values")
{
    ft_string *converted_value;

    converted_value = adv_to_string(-9876);
    FT_ASSERT(converted_value != ft_nullptr);
    FT_ASSERT(std::string(converted_value->c_str()) == "-9876");
    delete converted_value;
    return (1);
}

FT_TEST(test_adv_to_string_extreme_values,
        "adv_to_string handles zero and long minimum")
{
    ft_string *zero_string;
    ft_string *minimum_string;
    char expected_buffer[64];
    int expected_length;

    zero_string = adv_to_string(static_cast<int64_t>(0));
    minimum_string = adv_to_string(std::numeric_limits<long>::min());
    FT_ASSERT(zero_string != ft_nullptr);
    FT_ASSERT(minimum_string != ft_nullptr);
    expected_length = std::snprintf(expected_buffer, sizeof(expected_buffer),
            "%ld", std::numeric_limits<long>::min());
    FT_ASSERT(expected_length > 0);
    FT_ASSERT(std::string(zero_string->c_str()) == "0");
    FT_ASSERT(std::string(minimum_string->c_str()) == std::string(expected_buffer));
    delete zero_string;
    delete minimum_string;
    return (1);
}

FT_TEST(test_adv_to_string_independent_instances,
        "adv_to_string returns independent ft_string objects")
{
    ft_string *first_result;
    ft_string *second_result;

    first_result = adv_to_string(42);
    second_result = adv_to_string(42);
    FT_ASSERT(first_result != ft_nullptr);
    FT_ASSERT(second_result != ft_nullptr);
    FT_ASSERT(first_result != second_result);
    first_result->append('7');
    FT_ASSERT(std::string(first_result->c_str()) == "427");
    FT_ASSERT(std::string(second_result->c_str()) == "42");
    delete first_result;
    delete second_result;
    return (1);
}

FT_TEST(test_adv_to_string_unsigned_long,
        "adv_to_string formats unsigned values")
{
    ft_string *converted_value;
    uint64_t input_value;
    char expected_buffer[64];
    int expected_length;

    input_value = std::numeric_limits<unsigned long>::max();
    expected_length = std::snprintf(expected_buffer, sizeof(expected_buffer),
            "%lu", static_cast<unsigned long>(input_value));
    FT_ASSERT(expected_length > 0);
    converted_value = adv_to_string(input_value);
    FT_ASSERT(converted_value != ft_nullptr);
    FT_ASSERT(std::string(converted_value->c_str()) == std::string(expected_buffer));
    delete converted_value;
    return (1);
}

FT_TEST(test_adv_to_string_double_precision,
        "adv_to_string preserves precision for double values")
{
    ft_string *converted_value;
    double input_value;
    char expected_buffer[128];
    int expected_length;

    input_value = 3.141592653589793;
    expected_length = std::snprintf(expected_buffer, sizeof(expected_buffer),
            "%.17g", input_value);
    FT_ASSERT(expected_length > 0);
    converted_value = adv_to_string(input_value);
    FT_ASSERT(converted_value != ft_nullptr);
    FT_ASSERT(std::string(converted_value->c_str()) == std::string(expected_buffer));
    delete converted_value;
    return (1);
}

FT_TEST(test_adv_to_string_double_special_values,
        "adv_to_string handles infinity and NaN")
{
    ft_string *positive_infinity;
    ft_string *negative_infinity;
    ft_string *not_a_number;
    std::string positive_result;
    std::string negative_result;
    std::string nan_result;

    positive_infinity = adv_to_string(std::numeric_limits<double>::infinity());
    negative_infinity = adv_to_string(-std::numeric_limits<double>::infinity());
    not_a_number = adv_to_string(std::numeric_limits<double>::quiet_NaN());
    FT_ASSERT(positive_infinity != ft_nullptr);
    FT_ASSERT(negative_infinity != ft_nullptr);
    FT_ASSERT(not_a_number != ft_nullptr);
    positive_result = positive_infinity->c_str();
    negative_result = negative_infinity->c_str();
    nan_result = not_a_number->c_str();
    FT_ASSERT(positive_result == "inf" || positive_result == "infinity");
    FT_ASSERT(negative_result == "-inf" || negative_result == "-infinity");
    FT_ASSERT(nan_result == "nan" || nan_result == "nan(ind)");
    delete positive_infinity;
    delete negative_infinity;
    delete not_a_number;
    return (1);
}

FT_TEST(test_adv_to_string_allocation_failure,
        "adv_to_string returns null when allocation fails")
{
    ft_string *failed_result;
    ft_string *recovered_result;

    cma_set_alloc_limit(1);
    failed_result = adv_to_string(12345);
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(ft_nullptr, failed_result);

    recovered_result = adv_to_string(-321);
    FT_ASSERT(recovered_result != ft_nullptr);
    FT_ASSERT(std::string(recovered_result->c_str()) == "-321");
    delete recovered_result;
    return (1);
}

FT_TEST(test_adv_to_string_uint32_and_float_overloads,
        "adv_to_string supports uint32_t and float overloads")
{
    ft_string *unsigned_result;
    ft_string *float_result;

    unsigned_result = adv_to_string(static_cast<uint32_t>(42));
    float_result = adv_to_string(2.5f);
    FT_ASSERT(unsigned_result != ft_nullptr);
    FT_ASSERT(float_result != ft_nullptr);
    FT_ASSERT(std::string(unsigned_result->c_str()) == "42");
    FT_ASSERT(std::string(float_result->c_str()) == "2.5");
    delete unsigned_result;
    delete float_result;
    return (1);
}
