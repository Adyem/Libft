#include "advanced.hpp"
#include "../CPP_class/class_string.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include <cstdio>

static ft_string *create_string_from_buffer(const char *buffer)
{
    ft_string *result = new (std::nothrow) ft_string();
    if (result == ft_nullptr)
        return (ft_nullptr);
    int32_t initialization_error = result->initialize(buffer);
    if (initialization_error != FT_ERR_SUCCESS)
    {
        delete result;
        return (ft_nullptr);
    }
    return (result);
}

static ft_string *format_signed_long(int64_t number)
{
    char buffer[64];
    int32_t conversion_result;

    conversion_result = std::snprintf(buffer, sizeof(buffer), "%ld", number);
    if (conversion_result < 0)
        return (ft_nullptr);
    if (static_cast<ft_size_t>(conversion_result) >= sizeof(buffer))
        return (ft_nullptr);
    return (create_string_from_buffer(buffer));
}

static ft_string *format_unsigned_long(uint64_t number)
{
    char buffer[64];
    int32_t conversion_result;

    conversion_result = std::snprintf(buffer, sizeof(buffer), "%lu", number);
    if (conversion_result < 0)
        return (ft_nullptr);
    if (static_cast<ft_size_t>(conversion_result) >= sizeof(buffer))
        return (ft_nullptr);
    return (create_string_from_buffer(buffer));
}

static ft_string *format_double_value(double number)
{
    char buffer[128];
    int32_t conversion_result;

    conversion_result = std::snprintf(buffer, sizeof(buffer), "%.17g", number);
    if (conversion_result < 0)
        return (ft_nullptr);
    if (static_cast<ft_size_t>(conversion_result) >= sizeof(buffer))
        return (ft_nullptr);
    return (create_string_from_buffer(buffer));
}

ft_string *adv_to_string(int64_t number)
{
    return (format_signed_long(number));
}

ft_string *adv_to_string(uint64_t number)
{
    return (format_unsigned_long(number));
}

ft_string *adv_to_string(double number)
{
    return (format_double_value(number));
}

ft_string *adv_to_string(int32_t number)
{
    return (format_signed_long(static_cast<int64_t>(number)));
}

ft_string *adv_to_string(uint32_t number)
{
    return (format_unsigned_long(static_cast<uint64_t>(number)));
}

ft_string *adv_to_string(float number)
{
    return (format_double_value(static_cast<double>(number)));
}
