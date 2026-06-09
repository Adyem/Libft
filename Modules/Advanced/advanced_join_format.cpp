#include "advanced.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_string.hpp"
#include "../Template/vector.hpp"
#include <cstdarg>
#include <cstdio>
#include <new>

static ft_string *adv_build_formatted_string_from_arguments(const char *format,
    va_list argument_list)
{
    va_list copied_arguments;
    int32_t required_length;
    char *formatted_buffer;
    int32_t written_length;
    ft_string *result_string;

    if (format == ft_nullptr)
        return (ft_nullptr);
    va_copy(copied_arguments, argument_list);
    required_length = std::vsnprintf(ft_nullptr, 0, format, copied_arguments);
    va_end(copied_arguments);
    if (required_length < 0)
        return (ft_nullptr);
    formatted_buffer = static_cast<char *>(
            cma_malloc(static_cast<ft_size_t>(required_length) + 1));
    if (formatted_buffer == ft_nullptr)
        return (ft_nullptr);
    va_copy(copied_arguments, argument_list);
    written_length = std::vsnprintf(formatted_buffer,
            static_cast<ft_size_t>(required_length) + 1, format, copied_arguments);
    va_end(copied_arguments);
    if (written_length < 0 || written_length != required_length)
    {
        cma_free(formatted_buffer);
        return (ft_nullptr);
    }
    result_string = new (std::nothrow) ft_string();
    if (result_string == ft_nullptr)
    {
        cma_free(formatted_buffer);
        return (ft_nullptr);
    }
    if (result_string->initialize(formatted_buffer) != FT_ERR_SUCCESS)
    {
        cma_free(formatted_buffer);
        delete result_string;
        return (ft_nullptr);
    }
    cma_free(formatted_buffer);
    return (result_string);
}

ft_string *adv_strjoin_vector(const ft_vector<ft_string> &strings,
    const char *separator)
{
    ft_string *joined_string;
    ft_size_t index;
    ft_size_t string_count;
    ft_size_t separator_length;

    if (strings.is_initialised() == FT_FALSE)
        return (ft_nullptr);
    if (separator == ft_nullptr)
        separator = "";
    joined_string = new (std::nothrow) ft_string();
    if (joined_string == ft_nullptr)
        return (ft_nullptr);
    if (joined_string->initialize() != FT_ERR_SUCCESS)
    {
        delete joined_string;
        return (ft_nullptr);
    }
    string_count = strings.size();
    separator_length = ft_strlen_size_t(separator);
    index = 0;
    while (index < string_count)
    {
        if (index > 0 && separator_length > 0)
        {
            if (joined_string->append(separator, separator_length) != FT_ERR_SUCCESS)
            {
                delete joined_string;
                return (ft_nullptr);
            }
        }
        if (joined_string->append(strings[index]) != FT_ERR_SUCCESS)
        {
            delete joined_string;
            return (ft_nullptr);
        }
        ++index;
    }
    return (joined_string);
}

ft_string *adv_format_string(const char *format, ...)
{
    va_list argument_list;
    ft_string *result_string;

    va_start(argument_list, format);
    result_string = adv_build_formatted_string_from_arguments(format, argument_list);
    va_end(argument_list);
    return (result_string);
}
