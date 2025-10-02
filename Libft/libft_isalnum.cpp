#include "libft.hpp"

int ft_isalnum(int character)
{
    ft_errno = ER_SUCCESS;
    if (ft_isdigit(character) || ft_isalpha(character))
        return (1);
    return (0);
}
