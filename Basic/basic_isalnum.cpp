#include "basic.hpp"

int ft_isalnum(int32_t character)
{
    if (ft_isdigit(character) || ft_isalpha(character))
        return (1);
    return (0);
}
