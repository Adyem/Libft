#include "../CMA/CMA.hpp"
#include "../Basic/basic.hpp"
#include "../CPP_class/class_nullptr.hpp"

char    *adv_substr(const char *source, unsigned int start, size_t length)
{
    size_t  source_length;
    size_t  start_index;
    size_t  available_length;
    size_t  index;
    char    *substring;
    int     error_code;

    if (!source)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        ft_global_error_stack_push(error_code);
        return (ft_nullptr);
    }
    source_length = ft_strlen_size_t(source);
    error_code = ft_global_error_stack_drop_last_error();
    start_index = static_cast<size_t>(start);
    if (start_index >= source_length)
    {
        substring = cma_strdup("");
        error_code = ft_global_error_stack_drop_last_error();
        if (!substring)
        {
            ft_global_error_stack_push(error_code);
            return (ft_nullptr);
        }
        error_code = FT_ERR_SUCCESSS;
        ft_global_error_stack_push(error_code);
        return (substring);
    }
    available_length = source_length - start_index;
    if (length > available_length)
    {
        if (available_length >= static_cast<size_t>(FT_SYSTEM_SIZE_MAX))
        {
            error_code = FT_ERR_OUT_OF_RANGE;
            ft_global_error_stack_push(error_code);
            return (ft_nullptr);
        }
        length = available_length;
    }
    if (length >= static_cast<size_t>(FT_SYSTEM_SIZE_MAX))
    {
        error_code = FT_ERR_OUT_OF_RANGE;
        ft_global_error_stack_push(error_code);
        return (ft_nullptr);
    }
    substring = static_cast<char *>(cma_malloc(length + 1));
    error_code = ft_global_error_stack_drop_last_error();
    if (!substring)
    {
        ft_global_error_stack_push(error_code);
        return (ft_nullptr);
    }
    index = 0;
    while (index < length && source[start_index + index])
    {
        substring[index] = source[start_index + index];
        index++;
    }
    substring[index] = '\0';
    error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return (substring);
}
