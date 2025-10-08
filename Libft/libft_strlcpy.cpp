#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

size_t ft_strlcpy(char *destination, const char *source, size_t buffer_size)
{
    size_t source_length;

    ft_errno = ER_SUCCESS;
    if (source == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (0);
    }
    source_length = 0;
    if (buffer_size == 0)
    {
        while (source[source_length] != '\0')
        {
            source_length++;
        }
        return (source_length);
    }
    if (destination == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (0);
    }
    while (source_length < buffer_size - 1 && source[source_length] != '\0')
    {
        destination[source_length] = source[source_length];
        source_length++;
    }
    if (source_length < buffer_size)
        destination[source_length] = '\0';
    while (source[source_length] != '\0')
    {
        source_length++;
    }
    return (source_length);
}
