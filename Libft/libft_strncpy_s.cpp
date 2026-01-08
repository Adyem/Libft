#include "libft_config.hpp"

#if LIBFT_ENABLE_BOUNDS_CHECKED_HELPERS
#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

static void zero_buffer(char *buffer, size_t buffer_size)
{
    if (buffer == ft_nullptr)
        return ;
    if (buffer_size == 0)
        return ;
    ft_memset(buffer, 0, buffer_size);
    return ;
}

int ft_strncpy_s(char *destination, size_t destination_size, const char *source, size_t max_copy_length)
{
    size_t source_length;
    int error_code;

    if (destination == ft_nullptr || source == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (destination_size == 0)
    {
        ft_global_error_stack_push(FT_ERR_OUT_OF_RANGE);
        return (-1);
    }
    source_length = static_cast<size_t>(ft_strlen(source));
    error_code = ft_global_error_stack_pop_newest();
    if (error_code != FT_ERR_SUCCESSS)
    {
        zero_buffer(destination, destination_size);
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    if (source_length > max_copy_length)
    {
        zero_buffer(destination, destination_size);
        ft_global_error_stack_push(FT_ERR_OUT_OF_RANGE);
        return (-1);
    }
    if (source_length + 1 > destination_size)
    {
        zero_buffer(destination, destination_size);
        ft_global_error_stack_push(FT_ERR_OUT_OF_RANGE);
        return (-1);
    }
    if (ft_memcpy(destination, source, source_length + 1) == ft_nullptr)
    {
        error_code = ft_global_error_stack_pop_newest();
        zero_buffer(destination, destination_size);
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    error_code = ft_global_error_stack_pop_newest();
    if (error_code != FT_ERR_SUCCESSS)
    {
        zero_buffer(destination, destination_size);
        ft_global_error_stack_push(error_code);
        return (-1);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}
#endif
