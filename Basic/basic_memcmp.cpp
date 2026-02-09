#include "basic.hpp"
#include "../CPP_class/class_nullptr.hpp"

int    ft_memcmp(const void *pointer1, const void *pointer2, size_t size)
{
    if (size == 0 || pointer1 == ft_nullptr || pointer2 == ft_nullptr)
        return (0);

    const unsigned char *string1 = static_cast<const unsigned char *>(pointer1);
    const unsigned char *string2 = static_cast<const unsigned char *>(pointer2);

    size_t index = 0;
    while (index < size)
    {
        if (string1[index] != string2[index])
            return (static_cast<int>(string1[index])
                    - static_cast<int>(string2[index]));
        ++index;
    }
    return (0);
}
