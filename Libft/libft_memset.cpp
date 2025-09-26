#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include <cstdint>

void *ft_memset(void *destination, int value, size_t number_of_bytes)
{
    if (destination == ft_nullptr)
    {
        if (number_of_bytes == 0)
        {
            ft_errno = ER_SUCCESS;
            return (ft_nullptr);
        }
        ft_errno = FT_EINVAL;
        return (ft_nullptr);
    }
    ft_errno = ER_SUCCESS;
    unsigned char *dest = static_cast<unsigned char *>(destination);
    unsigned char byte = static_cast<unsigned char>(value);

    while (number_of_bytes && (reinterpret_cast<uintptr_t>(dest) & (sizeof(size_t) - 1)))
    {
        *dest = byte;
        dest++;
        number_of_bytes--;
    }

    size_t pattern = byte;
    pattern |= pattern << 8;
    pattern |= pattern << 16;
    if (sizeof(size_t) == 8)
        pattern |= pattern << 32;

    size_t *dest_word = reinterpret_cast<size_t *>(dest);
    while (number_of_bytes >= sizeof(size_t))
    {
        *dest_word = pattern;
        dest_word++;
        number_of_bytes -= sizeof(size_t);
    }

    dest = reinterpret_cast<unsigned char *>(dest_word);
    while (number_of_bytes)
    {
        *dest = byte;
        dest++;
        number_of_bytes--;
    }

    return (destination);
}
