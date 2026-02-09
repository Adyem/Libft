#include "../../Basic/basic.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../CMA/CMA.hpp"
#include <limits>
#include <string>
#include <cstdio>

FT_TEST(test_ft_to_string_positive_number,
        "ft_to_string converts positive numbers without touching errno")
{
    ft_string converted_value;
    std::string actual_value;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    converted_value = ft_to_string(12345);
    actual_value = converted_value.c_str();
    FT_ASSERT(actual_value == "12345");
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_ft_to_string_negative_number,
        "ft_to_string preserves the sign for negative values")
{
    ft_string converted_value;
    std::string actual_value;

    ft_errno = FT_ERR_SUCCESSS;
    converted_value = ft_to_string(-9876);
    actual_value = converted_value.c_str();
    FT_ASSERT(actual_value == "-9876");
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_ft_to_string_extreme_values,
        "ft_to_string handles zero and FT_LONG_MIN edge cases")
{
    ft_string zero_string;
    ft_string minimum_string;
    char expected_buffer[64];
    int expected_length;
    std::string zero_result;
    std::string minimum_result;
    std::string minimum_expected;

    ft_errno = FT_ERR_INVALID_HANDLE;
    zero_string = ft_to_string(0);
    minimum_string = ft_to_string(FT_LONG_MIN);
    expected_length = std::snprintf(expected_buffer,
            sizeof(expected_buffer), "%ld", std::numeric_limits<long>::min());
    FT_ASSERT(expected_length > 0);
    minimum_expected = expected_buffer;
    zero_result = zero_string.c_str();
    minimum_result = minimum_string.c_str();
    FT_ASSERT(zero_result == "0");
    FT_ASSERT(minimum_result == minimum_expected);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_ft_to_string_independent_instances,
        "ft_to_string returns independent ft_string buffers")
{
    ft_string first_result;
    ft_string second_result;
    const char *first_pointer;
    const char *second_pointer;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    first_result = ft_to_string(42);
    second_result = ft_to_string(42);
    first_pointer = first_result.c_str();
    second_pointer = second_result.c_str();
    FT_ASSERT(first_pointer != second_pointer);
    first_result.append('7');
    FT_ASSERT(std::string(first_result.c_str()) == "427");
    FT_ASSERT(std::string(second_result.c_str()) == "42");
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_ft_to_string_unsigned_long,
        "ft_to_string formats unsigned values without altering errno")
{
    ft_string converted_value;
    unsigned long input_value;
    std::string actual_value;
    char expected_buffer[64];
    int expected_length;

    input_value = std::numeric_limits<unsigned long>::max();
    expected_length = std::snprintf(expected_buffer, sizeof(expected_buffer),
            "%lu", input_value);
    FT_ASSERT(expected_length > 0);
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    converted_value = ft_to_string(input_value);
    actual_value = converted_value.c_str();
    FT_ASSERT(actual_value == std::string(expected_buffer));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_ft_to_string_double_precision,
        "ft_to_string preserves precision for double values")
{
    ft_string converted_value;
    double input_value;
    std::string actual_value;
    char expected_buffer[128];
    int expected_length;

    input_value = 3.141592653589793;
    expected_length = std::snprintf(expected_buffer, sizeof(expected_buffer),
            "%.17g", input_value);
    FT_ASSERT(expected_length > 0);
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    converted_value = ft_to_string(input_value);
    actual_value = converted_value.c_str();
    FT_ASSERT(actual_value == std::string(expected_buffer));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_ft_to_string_double_special_values,
        "ft_to_string handles infinity and NaN inputs")
{
    ft_string positive_infinity;
    ft_string negative_infinity;
    ft_string not_a_number;
    std::string positive_result;
    std::string negative_result;
    std::string nan_result;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    positive_infinity = ft_to_string(std::numeric_limits<double>::infinity());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    negative_infinity = ft_to_string(-std::numeric_limits<double>::infinity());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    not_a_number = ft_to_string(std::numeric_limits<double>::quiet_NaN());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    positive_result = positive_infinity.c_str();
    negative_result = negative_infinity.c_str();
    nan_result = not_a_number.c_str();
    FT_ASSERT(positive_result == "inf" || positive_result == "infinity");
    FT_ASSERT(negative_result == "-inf" || negative_result == "-infinity");
    FT_ASSERT(nan_result == "nan" || nan_result == "nan(ind)");
    return (1);
}

FT_TEST(test_ft_to_string_recovers_after_allocation_failure,
        "ft_to_string clears errno after retrying following allocation failure")
{
    ft_string failed_result;
    ft_string recovered_result;
    std::string recovered_text;

    cma_set_alloc_limit(1);
    ft_errno = FT_ERR_SUCCESSS;
    failed_result = ft_to_string(321L);
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, ft_errno);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, ft_string::last_operation_error());

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    recovered_result = ft_to_string(-321L);
    recovered_text = recovered_result.c_str();
    FT_ASSERT(recovered_text == "-321");
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_ft_to_string_allocation_failure,
        "ft_to_string propagates allocation failures from ft_string")
{
    ft_string failed_result;

    cma_set_alloc_limit(1);
    ft_errno = FT_ERR_SUCCESSS;
    failed_result = ft_to_string(12345L);
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, ft_errno);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, ft_string::last_operation_error());
    return (1);
}

FT_TEST(test_ft_to_string_unsigned_zero_clears_errno,
        "ft_to_string formats unsigned zero while resetting errno")
{
    ft_string converted_value;
    std::string actual_value;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    converted_value = ft_to_string(static_cast<unsigned int>(0));
    actual_value = converted_value.c_str();
    FT_ASSERT(actual_value == "0");
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}
