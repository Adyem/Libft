#include "roll.hpp"
#include "math_internal.hpp"
#include "../Printf/printf.hpp"

int math_roll_excecute_md(char *string, int *index, int string_boundary)
{
    int error = 0;

    while (*index < string_boundary)
    {
        if (DEBUG == 1)
            pf_printf("MD i=%d and j=%d\n", *index, string_boundary);
        if (!string[*index] || string[*index] == ')')
            break ;
        if (string[*index] == '*' || string[*index] == '/')
        {
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
        pf_printf("The result after md is %s\n", string);
    return (0);
}
