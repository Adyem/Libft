#include "libft_config.hpp"

#if LIBFT_ENABLE_BOUNDS_CHECKED_HELPERS
#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

static void zero_buffer(void *buffer, size_t buffer_size)
{
    unsigned char *buffer_bytes;
    size_t offset;

    if (buffer == ft_nullptr)
        return ;
    if (buffer_size == 0)
        return ;
    buffer_bytes = static_cast<unsigned char *>(buffer);
    offset = 0;
    while (offset < buffer_size)
    {
        buffer_bytes[offset] = 0;
        offset++;
    }
    return ;
}

static int report_move_error(void *destination, size_t destination_size, int error_code)
{
    zero_buffer(destination, destination_size);
    ft_global_error_stack_push(error_code);
    return (-1);
}

int ft_memmove_s(void *destination, size_t destination_size, const void *source, size_t number_of_bytes)
{
    void *move_result;
    int error_code;

    if (number_of_bytes == 0)
    {
        error_code = FT_ERR_SUCCESSS;
        ft_global_error_stack_push(error_code);
        return (0);
    }
    if (destination == ft_nullptr || source == ft_nullptr)
        return (report_move_error(destination, destination_size,
            FT_ERR_INVALID_ARGUMENT));
    if (destination_size < number_of_bytes)
        return (report_move_error(destination, destination_size,
            FT_ERR_OUT_OF_RANGE));
    move_result = ft_memmove(destination, source, number_of_bytes);
    error_code = ft_global_error_stack_pop_newest();
    if (error_code != FT_ERR_SUCCESSS)
        return (report_move_error(destination, destination_size, error_code));
    if (move_result == ft_nullptr)
        return (report_move_error(destination, destination_size,
            FT_ERR_INVALID_ARGUMENT));
    error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return (0);
}
#endif
