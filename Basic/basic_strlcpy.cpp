#include "basic.hpp"

size_t ft_strlcpy(char *destination, const char *source, size_t buffer_size)
{
    if (!source)
        return (0);

    size_t source_length = ft_strlen_size_t(source);

    if (destination && buffer_size > 0)
    {
        size_t copy_length;
        if (source_length >= buffer_size)
            copy_length = buffer_size - 1;
        else
            copy_length = source_length;
        size_t index = 0;
        while (index < copy_length)
        {
            destination[index] = source[index];
            ++index;
        }
        destination[copy_length] = '\0';
    }

    return (source_length);
}
