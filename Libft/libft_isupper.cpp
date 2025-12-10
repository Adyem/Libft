#include "libft.hpp"

int ft_isupper(int character)
{
    ft_errno = FT_ER_SUCCESSS;
    if (character >= 'A' && character <= 'Z')
        return (1);
    return (0);
}
