#include "libft.hpp"

int    ft_isdigit(int character)
{
    ft_errno = FT_ERR_SUCCESSS;
    if (character >= '0' && character <= '9')
        return (1);
    return (0);
}
