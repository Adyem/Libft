#include "libft.hpp"

void ft_swap(int *a, int *b)
{
    int tmp;

    if (!a || !b)
        return;
    tmp = *a;
    *a = *b;
    *b = tmp;
}

