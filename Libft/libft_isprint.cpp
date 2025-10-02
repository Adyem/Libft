#include "libft.hpp"

int ft_isprint(int character)
{
    ft_errno = ER_SUCCESS;
    if (character >= 32 && character <= 126)
        return (1);
    return (0);
}
