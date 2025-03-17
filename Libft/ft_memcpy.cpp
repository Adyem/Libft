#include "libft.hpp"
#include "../CPP_class/nullptr.hpp"

void* ft_memcpy(void* destination, const void* source, size_t size)
{
    if (destination == ft_nullptr || source == ft_nullptr)
        return (ft_nullptr);
    char* dest = static_cast<char*>(destination);
    const char* src = static_cast<const char*>(source);
	std::size_t index = 0;
    while (index < size)
    {
        dest[index] = src[index];
		index++;
    }
    return destination;
}
