#include "libft.hpp"
#include "../CPP_class/nullptr.hpp"

void* ft_memcpy(void* destination, const void* source, size_t num)
{
    if (destination == ft_nullptr && source == ft_nullptr)
        return ft_nullptr;

    char* destPtr = static_cast<char*>(destination);
    const char* srcPtr = static_cast<const char*>(source);
    size_t index = 0;
    while (index < num)
    {
        destPtr[index] = srcPtr[index];
        ++index;
    }
    return destination;
}

