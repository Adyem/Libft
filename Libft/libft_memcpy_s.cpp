#include "libft_config.hpp"

#if LIBFT_ENABLE_BOUNDS_CHECKED_HELPERS
#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

static void zero_buffer(void *buffer, size_t buffer_size)
{
    int saved_errno;

    if (buffer == ft_nullptr)
        return ;
    if (buffer_size == 0)
        return ;
    saved_errno = ft_errno;
    ft_memset(buffer, 0, buffer_size);
    ft_errno = saved_errno;
    return ;
}

int ft_memcpy_s(void *destination, size_t destination_size, const void *source, size_t number_of_bytes)
{
    void *copy_result;
    char *destination_bytes;
    const char *source_bytes;

    ft_errno = FT_ERR_SUCCESSS;
    if (number_of_bytes == 0)
        return (0);
    if (destination == ft_nullptr || source == ft_nullptr)
    {
        zero_buffer(destination, destination_size);
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    destination_bytes = static_cast<char *>(destination);
    source_bytes = static_cast<const char *>(source);
    if (destination_size < number_of_bytes)
    {
        zero_buffer(destination, destination_size);
        ft_errno = FT_ERR_OUT_OF_RANGE;
        return (-1);
    }
    if ((destination_bytes < source_bytes + number_of_bytes)
        && (source_bytes < destination_bytes + number_of_bytes))
    {
        zero_buffer(destination, destination_size);
        ft_errno = FT_ERR_OVERLAP;
        return (-1);
    }
    copy_result = ft_memcpy(destination, source, number_of_bytes);
    if (ft_errno != FT_ERR_SUCCESSS)
    {
        zero_buffer(destination, destination_size);
        return (-1);
    }
    if (copy_result == ft_nullptr)
        return (-1);
    return (0);
}
#endif
