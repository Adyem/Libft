#include "libft.hpp"
#include "../Errno/errno.hpp"
#include <cstdio>

static ft_string create_string_from_buffer(const char *buffer)
{
    ft_string result(buffer);
    int result_error;

    result_error = ft_string::last_operation_error();
    if (result_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(result_error);
    }
    else
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
    }
    return (result);
}

static void report_latest_error(void)
{
    int error_code;

    error_code = ft_global_error_stack_pop_newest();
    if (error_code == FT_ERR_SUCCESSS)
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
    else
        ft_global_error_stack_push(error_code);
    return ;
}

static ft_string format_signed_long(long number)
{
    char buffer[64];
    int conversion_result;

    conversion_result = std::snprintf(buffer, sizeof(buffer), "%ld", number);
    if (conversion_result < 0)
    {
        ft_global_error_stack_push(FT_ERR_INTERNAL);
        return (ft_string(FT_ERR_INTERNAL));
    }
    if (static_cast<size_t>(conversion_result) >= sizeof(buffer))
    {
        ft_global_error_stack_push(FT_ERR_INTERNAL);
        return (ft_string(FT_ERR_INTERNAL));
    }
    return (create_string_from_buffer(buffer));
}

static ft_string format_unsigned_long(unsigned long number)
{
    char buffer[64];
    int conversion_result;

    conversion_result = std::snprintf(buffer, sizeof(buffer), "%lu", number);
    if (conversion_result < 0)
    {
        ft_global_error_stack_push(FT_ERR_INTERNAL);
        return (ft_string(FT_ERR_INTERNAL));
    }
    if (static_cast<size_t>(conversion_result) >= sizeof(buffer))
    {
        ft_global_error_stack_push(FT_ERR_INTERNAL);
        return (ft_string(FT_ERR_INTERNAL));
    }
    return (create_string_from_buffer(buffer));
}

static ft_string format_double_value(double number)
{
    char buffer[128];
    int conversion_result;

    conversion_result = std::snprintf(buffer, sizeof(buffer), "%.17g", number);
    if (conversion_result < 0)
    {
        ft_global_error_stack_push(FT_ERR_INTERNAL);
        return (ft_string(FT_ERR_INTERNAL));
    }
    if (static_cast<size_t>(conversion_result) >= sizeof(buffer))
    {
        ft_global_error_stack_push(FT_ERR_INTERNAL);
        return (ft_string(FT_ERR_INTERNAL));
    }
    return (create_string_from_buffer(buffer));
}

static ft_string format_unsigned_long_long(unsigned long long number)
{
    char buffer[64];
    int conversion_result;

    conversion_result = std::snprintf(buffer, sizeof(buffer), "%llu", number);
    if (conversion_result < 0)
    {
        ft_global_error_stack_push(FT_ERR_INTERNAL);
        return (ft_string(FT_ERR_INTERNAL));
    }
    if (static_cast<size_t>(conversion_result) >= sizeof(buffer))
    {
        ft_global_error_stack_push(FT_ERR_INTERNAL);
        return (ft_string(FT_ERR_INTERNAL));
    }
    return (create_string_from_buffer(buffer));
}

ft_string ft_to_string(long number)
{
    ft_string result;

    result = format_signed_long(number);
    report_latest_error();
    return (result);
}

ft_string ft_to_string(unsigned long number)
{
    ft_string result;

    result = format_unsigned_long(number);
    report_latest_error();
    return (result);
}

ft_string ft_to_string(unsigned long long number)
{
    ft_string result;

    result = format_unsigned_long_long(number);
    report_latest_error();
    return (result);
}

ft_string ft_to_string(double number)
{
    ft_string result;

    result = format_double_value(number);
    report_latest_error();
    return (result);
}

ft_string ft_to_string(int number)
{
    ft_string result;

    result = ft_to_string(static_cast<long>(number));
    report_latest_error();
    return (result);
}

ft_string ft_to_string(unsigned int number)
{
    ft_string result;

    result = ft_to_string(static_cast<unsigned long>(number));
    report_latest_error();
    return (result);
}

ft_string ft_to_string(float number)
{
    ft_string result;

    result = ft_to_string(static_cast<double>(number));
    report_latest_error();
    return (result);
}
