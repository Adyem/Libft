#include "libft.hpp"
#include <unistd.h>

int    ft_memcmp(const void *pointer1, const void *pointer2, size_t size)
{
    const unsigned char    *string1;
    const unsigned char    *string2;
    size_t                index;

    index = 0;
    string1 = static_cast<const unsigned char *>(pointer1);
    string2 = static_cast<const unsigned char *>(pointer2);
    while (index < size)
    {
        if (string1[index] != string2[index])
            return (string1[index] - string2[index]);
        index++;
    }
    return (0);
}
