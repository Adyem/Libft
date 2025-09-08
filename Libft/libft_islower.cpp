#include "libft.hpp"

int ft_islower(int character)
{
    if (character >= 'a' && character <= 'z')
        return (1);
    return (0);
}
