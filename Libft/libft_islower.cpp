#include "libft.hpp"

int ft_islower(int character)
{
    ft_errno = FT_ER_SUCCESSS;
    if (character >= 'a' && character <= 'z')
        return (1);
    return (0);
}
