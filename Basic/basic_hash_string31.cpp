#include "basic.hpp"
#include "../CPP_class/class_nullptr.hpp"

int64_t ft_hash_string31(const char *string)
{
    uint32_t hash_value;
    ft_size_t index;
    uint8_t character_value;

    if (string == ft_nullptr)
        return (0);
    hash_value = 0;
    index = 0;
    while (string[index] != '\0')
    {
        character_value = static_cast<uint8_t>(string[index]);
        hash_value = hash_value * 31U + static_cast<uint32_t>(character_value);
        index++;
    }
    return (static_cast<int64_t>(static_cast<int32_t>(hash_value)));
}
