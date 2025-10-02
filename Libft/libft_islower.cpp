#include "libft.hpp"

int ft_islower(int character)
{
    ft_errno = ER_SUCCESS;
    if (character >= 'a' && character <= 'z')
        return (1);
    return (0);
}
