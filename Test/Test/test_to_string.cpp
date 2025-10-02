#include "../../Libft/libft.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"
#include <limits>
#include <sstream>
#include <string>

FT_TEST(test_ft_to_string_positive_number,
        "ft_to_string converts positive numbers without touching errno")
{
    ft_string converted_value;
    std::string actual_value;

    ft_errno = FT_EINVAL;
    converted_value = ft_to_string(12345);
    actual_value = converted_value.c_str();
    FT_ASSERT(actual_value == "12345");
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_ft_to_string_negative_number,
        "ft_to_string preserves the sign for negative values")
{
    ft_string converted_value;
    std::string actual_value;

    ft_errno = ER_SUCCESS;
    converted_value = ft_to_string(-9876);
    actual_value = converted_value.c_str();
    FT_ASSERT(actual_value == "-9876");
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_ft_to_string_extreme_values,
        "ft_to_string handles zero and FT_LONG_MIN edge cases")
{
    ft_string zero_string;
    ft_string minimum_string;
    std::ostringstream expected_stream;
    std::string expected_minimum;
    std::string zero_result;
    std::string minimum_result;

    ft_errno = FILE_INVALID_FD;
    zero_string = ft_to_string(0);
    minimum_string = ft_to_string(FT_LONG_MIN);
    expected_stream << std::numeric_limits<long>::min();
    expected_minimum = expected_stream.str();
    zero_result = zero_string.c_str();
    minimum_result = minimum_string.c_str();
    FT_ASSERT(zero_result == "0");
    FT_ASSERT(minimum_result == expected_minimum);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}
