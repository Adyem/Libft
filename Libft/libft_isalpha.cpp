#include "libft.hpp"

int ft_isalpha(int character)
{
    ft_errno = ER_SUCCESS;
    if ((character >= 'a' && character <= 'z') ||
        (character >= 'A' && character <= 'Z'))
        return (1);
    return (0);
}
