#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

int    ft_memcmp(const void *pointer1, const void *pointer2, size_t size)
{
    const unsigned char    *string1;
    const unsigned char    *string2;
    size_t                index;

    ft_errno = FT_ER_SUCCESSS;
    if (size == 0)
        return (0);
    if (pointer1 == ft_nullptr || pointer2 == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (0);
    }

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
