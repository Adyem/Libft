#include "libft.hpp"
#include "../Errno/errno.hpp"
#include <cstdio>

static ft_string create_string_from_buffer(const char *buffer)
{
    ft_string result(buffer);
    int result_error;

    result_error = result.get_error();
    if (result_error != FT_ER_SUCCESSS)
    {
        ft_errno = result_error;
    }
    else
    {
        ft_errno = FT_ER_SUCCESSS;
    }
    return (result);
}

static ft_string format_signed_long(long number)
{
    char buffer[64];
    int conversion_result;

    conversion_result = std::snprintf(buffer, sizeof(buffer), "%ld", number);
    if (conversion_result < 0)
    {
        ft_errno = FT_ERR_INTERNAL;
        return (ft_string(FT_ERR_INTERNAL));
    }
    if (static_cast<size_t>(conversion_result) >= sizeof(buffer))
    {
        ft_errno = FT_ERR_INTERNAL;
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
        ft_errno = FT_ERR_INTERNAL;
        return (ft_string(FT_ERR_INTERNAL));
    }
    if (static_cast<size_t>(conversion_result) >= sizeof(buffer))
    {
        ft_errno = FT_ERR_INTERNAL;
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
        ft_errno = FT_ERR_INTERNAL;
        return (ft_string(FT_ERR_INTERNAL));
    }
    if (static_cast<size_t>(conversion_result) >= sizeof(buffer))
    {
        ft_errno = FT_ERR_INTERNAL;
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
        ft_errno = FT_ERR_INTERNAL;
        return (ft_string(FT_ERR_INTERNAL));
    }
    if (static_cast<size_t>(conversion_result) >= sizeof(buffer))
    {
        ft_errno = FT_ERR_INTERNAL;
        return (ft_string(FT_ERR_INTERNAL));
    }
    return (create_string_from_buffer(buffer));
}

ft_string ft_to_string(long number)
{
    ft_errno = FT_ER_SUCCESSS;
    return (format_signed_long(number));
}

ft_string ft_to_string(unsigned long number)
{
    ft_errno = FT_ER_SUCCESSS;
    return (format_unsigned_long(number));
}

ft_string ft_to_string(unsigned long long number)
{
    ft_errno = FT_ER_SUCCESSS;
    return (format_unsigned_long_long(number));
}

ft_string ft_to_string(double number)
{
    ft_errno = FT_ER_SUCCESSS;
    return (format_double_value(number));
}

ft_string ft_to_string(int number)
{
    return (ft_to_string(static_cast<long>(number)));
}

ft_string ft_to_string(unsigned int number)
{
    return (ft_to_string(static_cast<unsigned long>(number)));
}

ft_string ft_to_string(float number)
{
    return (ft_to_string(static_cast<double>(number)));
}
