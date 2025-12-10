#include "libft_config.hpp"

#if LIBFT_ENABLE_BOUNDS_CHECKED_HELPERS
#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

static void zero_buffer(void *buffer, size_t buffer_size)
{
    if (buffer == ft_nullptr)
        return ;
    if (buffer_size == 0)
        return ;
    ft_memset(buffer, 0, buffer_size);
    return ;
}

int ft_memmove_s(void *destination, size_t destination_size, const void *source, size_t number_of_bytes)
{
    void *move_result;

    ft_errno = FT_ER_SUCCESSS;
    if (number_of_bytes == 0)
        return (0);
    if (destination == ft_nullptr || source == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (destination_size < number_of_bytes)
    {
        zero_buffer(destination, destination_size);
        ft_errno = FT_ERR_OUT_OF_RANGE;
        return (-1);
    }
    move_result = ft_memmove(destination, source, number_of_bytes);
    if (ft_errno != FT_ER_SUCCESSS)
    {
        zero_buffer(destination, destination_size);
        return (-1);
    }
    if (move_result == ft_nullptr)
        return (-1);
    return (0);
}
#endif
