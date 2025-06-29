#include "libft.hpp"
#include "../CPP_class/nullptr.hpp"

void* ft_memchr(const void* pointer, int number, size_t size)
{
    size_t index;
    const unsigned char *string = static_cast<const unsigned char*>(pointer);
    unsigned char character = static_cast<unsigned char>(number);

    index = 0;
    while (index < size)
    {
        if (*string == character)
            return (const_cast<void*>(static_cast<const void*>(string)));
        string++;
        index++;
    }
    return (ft_nullptr);
}
