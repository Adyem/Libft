#include "CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include <stdarg.h>

char *cma_strjoin_multiple(int count, ...)
{
    int error_code;

    if (count <= 0)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        ft_global_error_stack_push(error_code);
        return (ft_nullptr);
    }
    if (static_cast<size_t>(count) > SIZE_MAX / sizeof(size_t))
    {
        error_code = FT_ERR_OUT_OF_RANGE;
        ft_global_error_stack_push(error_code);
        return (ft_nullptr);
    }
    size_t *cached_lengths = static_cast<size_t*>(cma_malloc(static_cast<size_t>(count) * sizeof(size_t)));
    error_code = ft_global_error_stack_drop_last_error();
    if (!cached_lengths)
    {
        ft_global_error_stack_push(error_code);
        return (ft_nullptr);
    }
    va_list args;
    va_start(args, count);
    size_t total_length = 0;
    int argument_index = 0;
    while (argument_index < count)
    {
        const char *current_string = va_arg(args, const char *);
        size_t current_length = 0;
        if (current_string)
        {
            size_t measured_length = ft_strlen_size_t(current_string);
            error_code = ft_global_error_stack_drop_last_error();
            current_length = measured_length;
            if (total_length > SIZE_MAX - current_length)
            {
                error_code = FT_ERR_OUT_OF_RANGE;
                va_end(args);
                cma_free(cached_lengths);
                error_code = ft_global_error_stack_drop_last_error();
                if (error_code == FT_ERR_SUCCESSS)
                    error_code = FT_ERR_OUT_OF_RANGE;
                ft_global_error_stack_push(error_code);
                return (ft_nullptr);
            }
            total_length += current_length;
        }
        cached_lengths[argument_index] = current_length;
        ++argument_index;
    }
    va_end(args);
    if (total_length == SIZE_MAX)
    {
        error_code = FT_ERR_OUT_OF_RANGE;
        cma_free(cached_lengths);
        error_code = ft_global_error_stack_drop_last_error();
        if (error_code == FT_ERR_SUCCESSS)
            error_code = FT_ERR_OUT_OF_RANGE;
        ft_global_error_stack_push(error_code);
        return (ft_nullptr);
    }
    char *result = static_cast<char*>(cma_malloc(total_length + 1));
    error_code = ft_global_error_stack_drop_last_error();
    if (!result)
    {
        cma_free(cached_lengths);
        error_code = ft_global_error_stack_drop_last_error();
        ft_global_error_stack_push(error_code);
        return (ft_nullptr);
    }
    va_start(args, count);
    size_t result_index = 0;
    argument_index = 0;
    while (argument_index < count)
    {
        const char *current_string = va_arg(args, const char *);
        if (current_string)
        {
            size_t current_length = cached_lengths[argument_index];
            if (current_length > 0)
            {
                ft_memcpy(result + result_index, current_string, current_length);
                result_index += current_length;
            }
        }
        ++argument_index;
    }
    va_end(args);
    cma_free(cached_lengths);
    error_code = ft_global_error_stack_drop_last_error();
    if (error_code != FT_ERR_SUCCESSS)
    {
        cma_free(result);
        ft_global_error_stack_drop_last_error();
        ft_global_error_stack_push(error_code);
        return (ft_nullptr);
    }
    result[result_index] = '\0';
    error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return (result);
}
