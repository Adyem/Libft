#include "libft.hpp"

int ft_islower(int character)
{
    ft_errno = FT_ERR_SUCCESSS;
    if (character >= 'a' && character <= 'z')
        return (1);
    return (0);
}
