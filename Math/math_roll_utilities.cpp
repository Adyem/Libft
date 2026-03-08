#include "roll.hpp"
#include "math_internal.hpp"
#include <cstdint>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include "../CMA/CMA.hpp"
#include "../Basic/basic.hpp"
#include "../Printf/printf.hpp"

static int32_t math_check_value_roll(const char *string_pointer)
{
    int32_t                    sign;
    uint64_t            result;
    uint64_t            limit;

    sign = 1;
    result = 0;
    limit = static_cast<uint64_t>(INT_MAX) + 1;
    while (*string_pointer == ' ' || (*string_pointer >= 9 && *string_pointer <= 13))
        string_pointer++;
    if (*string_pointer == '-' || *string_pointer == '+')
    {
        if (*string_pointer == '-')
            sign = -1;
        string_pointer++;
    }
    while (*string_pointer && ft_isdigit(*string_pointer))
    {
        result = result * 10 + (static_cast<uint64_t>(*string_pointer) - '0');
        string_pointer++;
        if ((sign == 1 && result > INT_MAX) || (sign == -1 && result > limit))
            return (1);
        if (DEBUG == 1)
            pf_printf("FT_CHECK_VALUE_ROLL the string is %s\n", string_pointer);
    }
    if (sign == -1 && result > limit)
        return (1);
    return (0);
}

int32_t    math_check_string_number(char *string)
{
    int32_t    index;

    index = 0;
    if (string[index] == '+' || string[index] == '-')
        index++;
    if (!string[index])
        return (0);
    while (string[index])
    {
        if (string[index] >= '0' && string[index] <= '9')
            index++;
        else
            return (0);
    }
    return (1);
}

void    math_free_parse(char **to_parse)
{
    int32_t    index = 0;

    if (to_parse)
    {
        while (to_parse[index])
        {
            cma_free(to_parse[index]);
            index++;
        }
        cma_free(to_parse);
    }
    return ;
}

int32_t math_roll_convert_previous(char *string, int32_t *index, int32_t *error)
{
    int32_t result;
    int32_t    check;

    while (*index > 0 && (string[*index] >= '0' && string[*index] <= '9'))
        (*index)--;
    if (string[*index] == '-' || string[*index] == '+')
    {
        if (*index > 0)
        {
            if ((string[*index - 1] >= '0' && string[*index - 1] <= '9') || string[*index - 1] == ')')
                (*index)++;
        }
    }
    else if (string[*index] < '0' || string[*index] > '9')
        (*index)++;
    check = math_check_value_roll(&string[*index]);
    if (check != 0)
    {
        *error = 1;
        if (DEBUG == 1)
        {
            pf_printf("171-Error: numbers cant be higher then %d or lower than %d\n", INT_MAX, INT_MIN);
        }
        return (0);
    }
    result = ft_atoi(&string[*index]);
    if (DEBUG == 1)
        pf_printf("the first number is %d and i=%d\n", result, *index);
    return (result);
}

int32_t    math_roll_convert_next(char *string, int32_t index, int32_t *error)
{
    int32_t    result;
    int32_t    check;

    check = math_check_value_roll(&string[index]);
    if (check != 0)
    {
        *error = 1;
        if (DEBUG == 1)
        {
            pf_printf("170-Error: numbers cant be higher then %d or lower than %d\n", INT_MAX, INT_MIN);
        }
        return (0);
    }
    result = ft_atoi(&string[index]);
    if (DEBUG == 1)
        pf_printf("the second number is %d\n", result);
    return (result);
}

int32_t    math_roll_itoa(int32_t result, int32_t *index, char *string)
{
    char    temp[32];
    int32_t     temp_index;
    int32_t     written_count;

    if (DEBUG == 1)
        pf_printf("roll itoa: the value of i=%d\n", *index);
    written_count = std::snprintf(temp, sizeof(temp), "%d", result);
    if (written_count < 0)
        return (1);
    temp_index = 0;
    while (temp[temp_index])
    {
        string[*index] = temp[temp_index];
        (*index)++;
        temp_index++;
    }
    return (0);
}
