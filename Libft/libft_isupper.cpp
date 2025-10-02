#include "libft.hpp"

int ft_isupper(int character)
{
    ft_errno = ER_SUCCESS;
    if (character >= 'A' && character <= 'Z')
        return (1);
    return (0);
}
