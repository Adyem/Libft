#include "libft.hpp"

int ft_isalpha(int character)
{
    ft_errno = FT_ER_SUCCESSS;
    if ((character >= 'a' && character <= 'z') ||
        (character >= 'A' && character <= 'Z'))
        return (1);
    return (0);
}
