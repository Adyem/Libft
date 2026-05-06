#include "roll.hpp"
#include "math_internal.hpp"
#include "../CMA/CMA.hpp"
#include "../Printf/printf.hpp"
#include "../GetNextLine/get_next_line.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Basic/basic.hpp"
#include "../Errno/errno.hpp"

typedef int32_t (*roll_execute_func)(char *, int32_t *, int32_t);

void math_calculate_j(char *string, int32_t *j)
{
    *j = 0;
    while (string[*j] && string[*j] != ')')
        (*j)++;
    if (DEBUG == 1)
        pf_printf("The new value of J is %d\n", *j);
    return ;
}

static int32_t  *math_roll_report_error(char *buffer)
{
    if (buffer)
        cma_free(buffer);
    return (ft_nullptr);
}

static int32_t execute_roll_function(char *string, roll_execute_func func)
{
    int32_t current_index = 0;
    int32_t string_boundary;
    int32_t error;

    error = 0;
    math_calculate_j(string, &string_boundary);
    while (current_index < string_boundary)
    {
        error = func(string, &current_index, string_boundary);
        if (error)
            return (error);
        math_calculate_j(string, &string_boundary);
        current_index++;
    }
    return (0);
}

static int32_t math_roll_parse(char *string, int32_t nested)
{
    int32_t error;
    int32_t index = nested;

    while (string[index] != '(' && string[index])
        index++;
    if (string[index] == '(')
    {
        error = math_roll_parse(&string[index], 1);
        if (error)
            return (1);
    }
    if (execute_roll_function(string, math_roll_excecute_droll))
        return (3);
    if (execute_roll_function(string, math_roll_excecute_md))
        return (4);
    if (execute_roll_function(string, math_roll_excecute_pm))
        return (5);
    if (DEBUG == 1)
        pf_printf("Nested is %d\n", nested);
    if (nested)
    {
        error = math_roll_parse_brackets(string);
        if (error)
            return (6);
    }
    if (DEBUG == 1 && nested)
        pf_printf("Leaving nested braces\n");
    return (0);
}

int32_t *math_roll(const char *expression)
{
    char    *result;
    int32_t     *value;
    int32_t     parse_error;
    ft_size_t index;
    ft_size_t length;

    if (!expression)
    {
        return (math_roll_report_error(ft_nullptr));
    }
    length = ft_strlen_size_t(expression);
    result = static_cast<char *>(cma_malloc(length + 1));
    if (!result)
    {
        if (DEBUG == 1)
        {
            pf_printf_fd(2, "168-Error: Malloc failed in cma_strdup\n");
        }
        return (math_roll_report_error(ft_nullptr));
    }
    index = 0;
    while (index < length)
    {
        result[index] = expression[index];
        index += 1;
    }
    result[index] = '\0';
    if (math_roll_validate(result))
    {
        if (DEBUG == 1)
        {
            pf_printf_fd(2, "169-Command Roll Error with the string: %s\n", result);
        }
        return (math_roll_report_error(result));
    }
    parse_error = math_roll_parse(result, 0);
    if (parse_error)
    {
        return (math_roll_report_error(result));
    }
    if (!math_check_string_number(result))
    {
        return (math_roll_report_error(result));
    }
    value = reinterpret_cast<int32_t*>(cma_malloc(sizeof(int32_t)));
    if (!value)
    {
        return (math_roll_report_error(result));
    }
    *value = ft_atoi(result);
    cma_free(result);
    return (value);
}
