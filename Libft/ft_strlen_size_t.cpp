#include "libft.hpp"
#include <cstddef>
#include <cstdint>

static inline bool has_zero_size_t(size_t value)
{
    const size_t mask01 = ~static_cast<size_t>(0) / 0xFF;
    const size_t mask80 = mask01 << 7;
    return ((value - mask01) & (~value) & mask80) != 0;
}

size_t ft_strlen_size_t(const char *string)
{
	if (!string)
        return (0);
    const char *ptr = string;
    while (reinterpret_cast<uintptr_t>(ptr) & (sizeof(size_t) - 1))
    {
        if (*ptr == '\0')
            return (static_cast<size_t>(ptr - string));
        ++ptr;
    }
    const size_t *word_ptr = reinterpret_cast<const size_t*>(ptr);
    while (!has_zero_size_t(*word_ptr))
        ++word_ptr;
    ptr = reinterpret_cast<const char*>(word_ptr);
    while (*ptr)
        ++ptr;
    return (static_cast<size_t>(ptr - string));
}
