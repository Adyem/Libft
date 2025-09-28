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
    while (*string1 != '\0' && static_cast<unsigned char>(*string1) == static_cast<unsigned char>(*string2))
    {
        string1++;
        string2++;
    }
    unsigned char left_character = static_cast<unsigned char>(*string1);
    unsigned char right_character = static_cast<unsigned char>(*string2);
    int left_value = static_cast<int>(left_character);
    int right_value = static_cast<int>(right_character);
    int result = left_value - right_value;
    return (result);
}
