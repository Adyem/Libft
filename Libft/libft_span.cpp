#include "libft.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

static char *report_span_dup_result(int error_code, char *result)
{
    ft_global_error_stack_push(error_code);
    return (result);
}

static ft_string report_span_string_result(int error_code, const ft_string &result)
{
    ft_global_error_stack_push(error_code);
    return (result);
}

char *ft_span_dup(const char *buffer, size_t length)
{
    char *duplicate;
    size_t index;

    if (buffer == ft_nullptr && length != 0)
        return (report_span_dup_result(FT_ERR_INVALID_ARGUMENT, ft_nullptr));
    duplicate = static_cast<char *>(cma_malloc(length + 1));
    if (duplicate == ft_nullptr)
        return (report_span_dup_result(FT_ERR_NO_MEMORY, ft_nullptr));
    index = 0;
    while (index < length)
    {
        duplicate[index] = buffer[index];
        index++;
    }
    duplicate[length] = '\0';
    return (report_span_dup_result(FT_ERR_SUCCESSS, duplicate));
}

ft_string ft_span_to_string(const char *buffer, size_t length)
{
    ft_string result;

    if (buffer == ft_nullptr && length != 0)
        return (report_span_string_result(FT_ERR_INVALID_ARGUMENT,
            ft_string(FT_ERR_INVALID_ARGUMENT)));
    if (length == 0)
        return (report_span_string_result(FT_ERR_SUCCESSS, result));
    result.assign(buffer, length);
    if (ft_string::last_operation_error() != FT_ERR_SUCCESSS)
        return (report_span_string_result(ft_string::last_operation_error(),
            ft_string(ft_string::last_operation_error())));
    return (report_span_string_result(FT_ERR_SUCCESSS, result));
}
