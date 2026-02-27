#include "roll.hpp"
#include "math_internal.hpp"
#include "../Printf/printf.hpp"
#include "../RNG/rng.hpp"
#include <climits>

static int math_handle_dice_roll(char *string, int *index, int *next_number_index, int *error)
{
    int first_number;
    int second_number;

    if (DEBUG == 1)
        pf_printf("The value of x = %d\n", *next_number_index);
    if (string[*index] >= '0' && string[*index] <= '9')
        first_number = math_roll_convert_previous(string, index, error);
    else
        first_number = 1;
    second_number = math_roll_convert_next(string, *next_number_index, error);
    if (*error)
        return (1);
    if (first_number <= 0)
    {
        if (DEBUG == 1)
        {
            pf_printf_fd(2, "178-Error: The number of dice must be greater than 0. Current value: %d\n", first_number);
        }
        return (1);
    }
    if (second_number <= 0)
    {
        if (DEBUG == 1)
        {
            pf_printf_fd(2, "179-Error: The number of faces on a die must be greater than 0. Current value: %d, "
                          "the result can't be higher than %d\n", second_number, INT_MAX);
        }
        return (1);
    }
    return ft_dice_roll(first_number, second_number);
}

static int math_handle_result_replacement(char *string, int *index, int read_index, int result)
{
    if (math_roll_itoa(result, index, string))
        return (1);
    if (DEBUG == 1)
        pf_printf("1 The value of i = %d and x = %d\n", *index, read_index);
    if (string[read_index] == '-' || string[read_index] == '+')
        read_index++;
    while (string[read_index] >= '0' && string[read_index] <= '9')
        read_index++;
    if (DEBUG == 1)
        pf_printf("2 The value of i = %d and x = %d\n", *index, read_index);
    while (string[read_index])
    {
        string[*index] = string[read_index];
        (*index)++;
        read_index++;
    }
    string[*index] = '\0';
    *index = 0;
    return (0);
}

int math_roll_excecute_droll(char *string, int *index, int string_boundary)
{
    int result;
    int next_number_index;
    int error = 0;

    while (*index < string_boundary)
    {
        if (!string[*index] || string[*index] == ')')
            break ;
        if (string[*index] == 'd')
        {
            next_number_index = *index;
            if (*index > 0)
                (*index)--;
            next_number_index++;
            result = math_handle_dice_roll(string, index, &next_number_index, &error);
            if (result == -1 || error)
                return (1);

            if (math_handle_result_replacement(string, index, next_number_index, result))
                return (1);
            math_calculate_j(string, &string_boundary);
            *index = 0;
        }
        else
            (*index)++;
    }
    math_calculate_j(string, &string_boundary);
    if (DEBUG == 1)
        pf_printf_fd(2, "After dice rolling result is %s\n", string);
    return (0);
}
