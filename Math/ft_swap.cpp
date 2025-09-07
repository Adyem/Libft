#include "math.hpp"

void ft_swap(int *first_number, int *second_number)
{
    int temporary;

    if (!first_number || !second_number)
        return ;
    temporary = *first_number;
    *first_number = *second_number;
    *second_number = temporary;
}

