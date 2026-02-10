#include "basic.hpp"

ft_size_t ft_strlcat(char *destination, const char *source, ft_size_t buffer_size)
{
    if (!destination || !source)
        return (0);

    ft_size_t destination_length = 0;
    while (destination_length < buffer_size && destination[destination_length] != '\0')
        destination_length++;

    ft_size_t source_length = ft_strlen_size_t(source);
    if (destination_length == buffer_size)
        return (buffer_size + source_length);

    ft_size_t available_space = buffer_size - destination_length - 1;
    ft_size_t copy_index = 0;
    while (copy_index < available_space && source[copy_index] != '\0')
    {
        destination[destination_length + copy_index] = source[copy_index];
        copy_index++;
    }
    if (destination_length + copy_index < buffer_size)
        destination[destination_length + copy_index] = '\0';

    return (destination_length + source_length);
}
