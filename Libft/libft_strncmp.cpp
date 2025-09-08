#include "libft.hpp"
#include <unistd.h>

int ft_strncmp(const char *string_1, const char *string_2, size_t max_len)
{
    unsigned int current_index = 0;

    while (string_1[current_index] != '\0' &&
           string_2[current_index] != '\0' &&
           current_index < max_len)
    {
        unsigned char char1 = static_cast<unsigned char>(string_1[current_index]);
        unsigned char char2 = static_cast<unsigned char>(string_2[current_index]);
        if (char1 != char2)
            return (char1 - char2);
        current_index++;
    }
    if (current_index == max_len)
        return (0);

    return (static_cast<unsigned char>(string_1[current_index]) -
           static_cast<unsigned char>(string_2[current_index]));
}
