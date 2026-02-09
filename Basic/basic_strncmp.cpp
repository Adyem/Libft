#include "basic.hpp"
#include "../CPP_class/class_nullptr.hpp"

int ft_strncmp(const char *string_1, const char *string_2, size_t maximum_length)
{
    if (maximum_length == 0 || string_1 == ft_nullptr || string_2 == ft_nullptr)
        return (0);

    ft_size_t index = 0;
    while (index < maximum_length)
    {
        unsigned char string_1_char = static_cast<unsigned char>(string_1[index]);
        unsigned char string_2_char = static_cast<unsigned char>(string_2[index]);
        if (string_1_char != string_2_char)
            return (static_cast<int>(string_1_char) - static_cast<int>(string_2_char));
        if (string_1_char == '\0')
            return (0);
        ++index;
    }
    return (0);
}
