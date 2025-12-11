#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

size_t ft_strnlen(const char *string, size_t maximum_length)
{
    size_t index;

    ft_errno = FT_ERR_SUCCESSS;
    if (string == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (0);
    }
    index = 0;
    while (index < maximum_length && string[index] != '\0')
    {
        index++;
    }
    return (index);
}
