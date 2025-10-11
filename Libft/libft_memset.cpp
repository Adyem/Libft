#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

void *ft_memset(void *destination, int value, size_t number_of_bytes)
{
    if (destination == ft_nullptr)
    {
        if (number_of_bytes == 0)
        {
            ft_errno = ER_SUCCESS;
            return (ft_nullptr);
        }
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    ft_errno = ER_SUCCESS;
    unsigned char *destination_pointer = static_cast<unsigned char *>(destination);
    unsigned char byte_value = static_cast<unsigned char>(value);

    while (number_of_bytes)
    {
        *destination_pointer = byte_value;
        destination_pointer++;
        number_of_bytes--;
    }

    return (destination);
}
