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

int ft_memcpy_s(void *destination, size_t destination_size, const void *source, size_t number_of_bytes)
{
    void *copy_result;

    ft_errno = ER_SUCCESS;
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
    copy_result = ft_memcpy(destination, source, number_of_bytes);
    if (ft_errno != ER_SUCCESS)
    {
        zero_buffer(destination, destination_size);
        return (-1);
    }
    if (copy_result == ft_nullptr)
        return (-1);
    return (0);
}
