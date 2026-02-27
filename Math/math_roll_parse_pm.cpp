#include "roll.hpp"
#include "math_internal.hpp"
#include "../Printf/printf.hpp"

int math_roll_excecute_pm(char *string, int *index, int string_boundary)
{
    int error = 0;

    if (DEBUG == 1)
        pf_printf_fd(1, "CHECKING PM\n");
    while (*index < string_boundary)
    {
        if (DEBUG == 1)
            pf_printf("PM i=%d and j=%d\n", *index, string_boundary);
        if (!string[*index] || string[*index] == ')')
            break ;
        if (string[*index] == '+' || string[*index] == '-')
        {
            if (math_is_unary_sign(string, *index))
            {
                (*index)++;
                continue ;
            }
            if (math_process_sign(string, index, string_boundary, &error))
                return (1);
            math_calculate_j(string, &string_boundary);
            *index = 0;
        }
        else
            (*index)++;
    }
    math_calculate_j(string, &string_boundary);
    if (DEBUG == 1)
        pf_printf("The result after pm is %s\n", string);
    return (0);
}
