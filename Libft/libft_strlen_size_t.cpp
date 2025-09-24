#include "libft.hpp"
#include <cstddef>
#include <cstdint>
#include "../Errno/errno.hpp"

#define FT_REPEAT_BYTE(x) (~static_cast<size_t>(0) / 0xFF * (x))
#define FT_HAS_ZERO(x) (((x) - FT_REPEAT_BYTE(0x01)) & ~(x) & FT_REPEAT_BYTE(0x80))

size_t ft_strlen_size_t(const char *string)
{
    ft_errno = ER_SUCCESS;
    if (!string)
    {
        ft_errno = FT_EINVAL;
        return (0);
    }
    const char *ptr = string;
    while (reinterpret_cast<uintptr_t>(ptr) & (sizeof(size_t) - 1))
    {
        if (*ptr == '\0')
            return (static_cast<size_t>(ptr - string));
        ++ptr;
    }
    const size_t *word_ptr = reinterpret_cast<const size_t*>(ptr);
    while (!FT_HAS_ZERO(*word_ptr))
        ++word_ptr;
    ptr = reinterpret_cast<const char*>(word_ptr);
    while (*ptr)
        ++ptr;
    return (static_cast<size_t>(ptr - string));
}

#undef FT_REPEAT_BYTE
#undef FT_HAS_ZERO
