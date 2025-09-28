#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

int    ft_strcmp(const char *string1, const char *string2)
{
    ft_errno = ER_SUCCESS;
    if (string1 == ft_nullptr || string2 == ft_nullptr)
    {
        ft_errno = FT_EINVAL;
        return (-1);
    }
    while (*string1 != '\0' && (unsigned char)(*string1) == (unsigned char)(*string2))
    {
        string1++;
        string2++;
    }
    unsigned char left_character = (unsigned char)(*string1);
    unsigned char right_character = (unsigned char)(*string2);
    return ((int)(left_character - right_character));
}
