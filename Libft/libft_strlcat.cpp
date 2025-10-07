#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

size_t ft_strlcat(char *destination, const char *source, size_t buffer_size)
{
    size_t destination_length;
    size_t source_length;
    size_t copy_index;

    ft_errno = ER_SUCCESS;
    if (source == ft_nullptr)
    {
        ft_errno = FT_EINVAL;
        return (0);
    }
    source_length = ft_strlen_size_t(source);
    if (ft_errno != ER_SUCCESS)
        return (0);
    if (buffer_size == 0)
    {
        return (source_length);
    }
    if (destination == ft_nullptr)
    {
        ft_errno = FT_EINVAL;
        return (0);
    }
    destination_length = 0;
    while (destination_length < buffer_size && destination[destination_length] != '\0')
        destination_length++;
    if (destination_length == buffer_size)
        return (buffer_size + source_length);
    copy_index = 0;
    while ((destination_length + copy_index + 1) < buffer_size && source[copy_index])
    {
        destination[destination_length + copy_index] = source[copy_index];
        copy_index++;
    }
    destination[destination_length + copy_index] = '\0';
    return (destination_length + source_length);
}
