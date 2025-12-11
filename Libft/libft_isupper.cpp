#include "libft.hpp"

int ft_isupper(int character)
{
    ft_errno = FT_ERR_SUCCESSS;
    if (character >= 'A' && character <= 'Z')
        return (1);
    return (0);
}
