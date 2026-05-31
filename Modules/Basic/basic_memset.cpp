#include "basic.hpp"
#include "../Basic/class_nullptr.hpp"

void *ft_memset(void *destination, int32_t value, ft_size_t number_of_bytes)
{
    if (destination == ft_nullptr)
        return (ft_nullptr);

    unsigned char *bytes = static_cast<unsigned char *>(destination);
    unsigned char fill = static_cast<unsigned char>(value);

    ft_size_t index = 0;
    while (index < number_of_bytes)
    {
        bytes[index] = fill;
        ++index;
    }

    return (destination);
}
