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

static int report_copy_error(void *destination, size_t destination_size, int error_code)
{
    zero_buffer(destination, destination_size);
    ft_global_error_stack_push(error_code);
    return (-1);
}

int ft_memcpy_s(void *destination, size_t destination_size, const void *source, size_t number_of_bytes)
{
    void *copy_result;
    char *destination_bytes;
    const char *source_bytes;
    int error_code;

    if (number_of_bytes == 0)
    {
        error_code = FT_ERR_SUCCESSS;
        ft_global_error_stack_push(error_code);
        return (0);
    }
    if (destination == ft_nullptr || source == ft_nullptr)
        return (report_copy_error(destination, destination_size,
            FT_ERR_INVALID_ARGUMENT));
    destination_bytes = static_cast<char *>(destination);
    source_bytes = static_cast<const char *>(source);
    if (destination_size < number_of_bytes)
        return (report_copy_error(destination, destination_size,
            FT_ERR_OUT_OF_RANGE));
    if ((destination_bytes < source_bytes + number_of_bytes)
        && (source_bytes < destination_bytes + number_of_bytes))
        return (report_copy_error(destination, destination_size,
            FT_ERR_OVERLAP));
    copy_result = ft_memcpy(destination, source, number_of_bytes);
    error_code = ft_global_error_stack_pop_newest();
    if (error_code != FT_ERR_SUCCESSS || copy_result == ft_nullptr)
    {
        if (error_code == FT_ERR_SUCCESSS)
            error_code = FT_ERR_INVALID_ARGUMENT;
        return (report_copy_error(destination, destination_size, error_code));
    }
    error_code = FT_ERR_SUCCESSS;
    ft_global_error_stack_push(error_code);
    return (0);
}
#endif
