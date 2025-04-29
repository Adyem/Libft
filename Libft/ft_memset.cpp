#include "libft.hpp"
#include "../CPP_class/nullptr.hpp"

void *ft_memset(void *destination, int value, size_t number_of_bytes)
{
    if (destination == ft_nullptr)
        return (ft_nullptr);
    unsigned char *byte_pointer = static_cast<unsigned char *>(destination);
    size_t index = 0;
    while (index < number_of_bytes)
    {
        byte_pointer[index] = static_cast<unsigned char>(value);
        index++;
    }
    return (destination);
}
