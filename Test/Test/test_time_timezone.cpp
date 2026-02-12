#include "../test_internal.hpp"
#include "../../Time/time.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"
#include <cstddef>
#include <ctime>

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_time_format_iso8601_with_offset_zero, "time_format_iso8601_with_offset formats UTC time with zero offset")
{
    ft_string expected_string("1970-01-01T00:00:00+00:00");
    ft_string result_string;

    ft_errno = FT_ERR_OUT_OF_RANGE;
    result_string = time_format_iso8601_with_offset(0, 0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_errno);
    FT_ASSERT_EQ(expected_string, result_string);
    return (1);
}

FT_TEST(test_time_format_iso8601_with_offset_positive, "time_format_iso8601_with_offset formats positive offset")
{
    ft_string expected_string("1970-01-01T05:30:00+05:30");
    ft_string result_string;

    ft_errno = FT_ERR_OUT_OF_RANGE;
    result_string = time_format_iso8601_with_offset(0, 330);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_errno);
    FT_ASSERT_EQ(expected_string, result_string);
    return (1);
}

FT_TEST(test_time_format_iso8601_with_offset_negative, "time_format_iso8601_with_offset formats negative offset")
{
    ft_string expected_string("1969-12-31T16:00:00-08:00");
    ft_string result_string;

    ft_errno = FT_ERR_SUCCESS;
    result_string = time_format_iso8601_with_offset(0, -480);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_errno);
    FT_ASSERT_EQ(expected_string, result_string);
    return (1);
}

FT_TEST(test_time_convert_timezone_success, "time_convert_timezone converts between offsets")
{
    t_time converted_time;
    bool conversion_result;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    conversion_result = time_convert_timezone(3600, 120, -300, &converted_time);
    FT_ASSERT(conversion_result);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_errno);
    FT_ASSERT_EQ(static_cast<t_time>(-21600), converted_time);
    return (1);
}

FT_TEST(test_time_convert_timezone_invalid_argument, "time_convert_timezone validates output pointer")
{
    bool conversion_result;

    ft_errno = FT_ERR_SUCCESS;
    conversion_result = time_convert_timezone(0, 0, 0, NULL);
    FT_ASSERT_EQ(false, conversion_result);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_time_get_local_offset_requires_output, "time_get_local_offset requires at least one output pointer")
{
    bool offset_result;

    ft_errno = FT_ERR_SUCCESS;
    offset_result = time_get_local_offset(0, NULL, NULL);
    FT_ASSERT_EQ(false, offset_result);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}
