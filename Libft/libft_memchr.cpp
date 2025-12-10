#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

void* ft_memchr(const void* pointer, int number, size_t size)
{
    size_t index;
    const unsigned char *string;
    unsigned char character;

    ft_errno = FT_ER_SUCCESSS;
    if (size == 0)
        return (ft_nullptr);
    if (pointer == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    string = static_cast<const unsigned char*>(pointer);
    character = static_cast<unsigned char>(number);
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
