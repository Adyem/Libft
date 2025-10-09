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

int ft_strcpy_s(char *destination, size_t destination_size, const char *source)
{
    size_t source_length;

    ft_errno = ER_SUCCESS;
    if (destination == ft_nullptr || source == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    if (destination_size == 0)
    {
        ft_errno = FT_ERR_OUT_OF_RANGE;
        return (-1);
    }
    source_length = static_cast<size_t>(ft_strlen(source));
    if (ft_errno != ER_SUCCESS)
    {
        zero_buffer(destination, destination_size);
        return (-1);
    }
    if (source_length + 1 > destination_size)
    {
        zero_buffer(destination, destination_size);
        ft_errno = FT_ERR_OUT_OF_RANGE;
        return (-1);
    }
    if (ft_memcpy(destination, source, source_length + 1) == ft_nullptr)
    {
        zero_buffer(destination, destination_size);
        return (-1);
    }
    if (ft_errno != ER_SUCCESS)
    {
        zero_buffer(destination, destination_size);
        return (-1);
    }
    return (0);
}
