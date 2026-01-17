#include "libft.hpp"
#include "../Errno/errno.hpp"
#include <cstdio>

static ft_string create_string_from_buffer(const char *buffer, int *error_code)
{
    ft_string result(buffer);

    *error_code = ft_string::last_operation_error();
    return (result);
}

static ft_string format_signed_long(long number, int *error_code)
{
    char buffer[64];
    int conversion_result;

    conversion_result = std::snprintf(buffer, sizeof(buffer), "%ld", number);
    if (conversion_result < 0)
    {
        *error_code = FT_ERR_INTERNAL;
        return (ft_string(FT_ERR_INTERNAL));
    }
    if (static_cast<size_t>(conversion_result) >= sizeof(buffer))
    {
        *error_code = FT_ERR_INTERNAL;
        return (ft_string(FT_ERR_INTERNAL));
    }
    return (create_string_from_buffer(buffer, error_code));
}

static ft_string format_unsigned_long(unsigned long number, int *error_code)
{
    char buffer[64];
    int conversion_result;

    conversion_result = std::snprintf(buffer, sizeof(buffer), "%lu", number);
    if (conversion_result < 0)
    {
        *error_code = FT_ERR_INTERNAL;
        return (ft_string(FT_ERR_INTERNAL));
    }
    if (static_cast<size_t>(conversion_result) >= sizeof(buffer))
    {
        *error_code = FT_ERR_INTERNAL;
        return (ft_string(FT_ERR_INTERNAL));
    }
    return (create_string_from_buffer(buffer, error_code));
}

static ft_string format_double_value(double number, int *error_code)
{
    char buffer[128];
    int conversion_result;

    conversion_result = std::snprintf(buffer, sizeof(buffer), "%.17g", number);
    if (conversion_result < 0)
    {
        *error_code = FT_ERR_INTERNAL;
        return (ft_string(FT_ERR_INTERNAL));
    }
    if (static_cast<size_t>(conversion_result) >= sizeof(buffer))
    {
        *error_code = FT_ERR_INTERNAL;
        return (ft_string(FT_ERR_INTERNAL));
    }
    return (create_string_from_buffer(buffer, error_code));
}

static ft_string format_unsigned_long_long(unsigned long long number, int *error_code)
{
    char buffer[64];
    int conversion_result;

    conversion_result = std::snprintf(buffer, sizeof(buffer), "%llu", number);
    if (conversion_result < 0)
    {
        *error_code = FT_ERR_INTERNAL;
        return (ft_string(FT_ERR_INTERNAL));
    }
    if (static_cast<size_t>(conversion_result) >= sizeof(buffer))
    {
        *error_code = FT_ERR_INTERNAL;
        return (ft_string(FT_ERR_INTERNAL));
    }
    return (create_string_from_buffer(buffer, error_code));
}

ft_string ft_to_string(long number)
{
    ft_string result;
    int error_code;

    result = format_signed_long(number, &error_code);
    ft_global_error_stack_push(error_code);
    return (result);
}

ft_string ft_to_string(unsigned long number)
{
    ft_string result;
    int error_code;

    result = format_unsigned_long(number, &error_code);
    ft_global_error_stack_push(error_code);
    return (result);
}

ft_string ft_to_string(unsigned long long number)
{
    ft_string result;
    int error_code;

    result = format_unsigned_long_long(number, &error_code);
    ft_global_error_stack_push(error_code);
    return (result);
}

ft_string ft_to_string(double number)
{
    ft_string result;
    int error_code;

    result = format_double_value(number, &error_code);
    ft_global_error_stack_push(error_code);
    return (result);
}

ft_string ft_to_string(int number)
{
    ft_string result;
    int error_code;

    result = format_signed_long(static_cast<long>(number), &error_code);
    ft_global_error_stack_push(error_code);
    return (result);
}

ft_string ft_to_string(unsigned int number)
{
    ft_string result;
    int error_code;

    result = format_unsigned_long(static_cast<unsigned long>(number), &error_code);
    ft_global_error_stack_push(error_code);
    return (result);
}

ft_string ft_to_string(float number)
{
    ft_string result;
    int error_code;

    result = format_double_value(static_cast<double>(number), &error_code);
    ft_global_error_stack_push(error_code);
    return (result);
}
