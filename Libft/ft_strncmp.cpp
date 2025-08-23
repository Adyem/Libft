#include "libft.hpp"

int ft_strncmp(const char *string_1, const char *string_2, size_t max_len)
{
    if (!string_1 || !string_2)
        return (string_1 ? 1 : (string_2 ? -1 : 0));
    size_t i = 0;
    while (i < max_len)
    {
        unsigned char c1 = static_cast<unsigned char>(string_1[i]);
        unsigned char c2 = static_cast<unsigned char>(string_2[i]);
        if (c1 != c2 || c1 == '\0' || c2 == '\0')
            return (c1 - c2);
        ++i;
    }
    return (0);
}
