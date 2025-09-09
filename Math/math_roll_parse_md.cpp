#include "roll.hpp"
#include "math_internal.hpp"
#include "../Printf/printf.hpp"

int math_roll_excecute_md(char *string, int *i, int j)
{
    int error = 0;

    while (*i < j)
    {
        if (DEBUG == 1)
            pf_printf("MD i=%d and j=%d\n", *i, j);
        if (!string[*i] || string[*i] == ')')
            break ;
        if (string[*i] == '*' || string[*i] == '/')
        {
            if (math_process_sign(string, i, j, &error))
                return 1;
            math_calculate_j(string, &j);
            *i = 0;
        }
        else
            (*i)++;
    }
    math_calculate_j(string, &j);
    if (DEBUG == 1)
        pf_printf("The result after md is %s\n", string);
    return (0);
}
