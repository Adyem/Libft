#include "roll.hpp"
#include "math_internal.hpp"
#include "../CMA/CMA.hpp"
#include "../Printf/printf.hpp"
#include "../GetNextLine/get_next_line.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Basic/basic.hpp"
#include "../Errno/errno.hpp"

typedef int (*RollExecuteFunc)(char *, int *, int);

void math_calculate_j(char *string, int *j)
{
    *j = 0;
    while (string[*j] && string[*j] != ')')
        (*j)++;
    if (DEBUG == 1)
        pf_printf("The new value of J is %d\n", *j);
    return ;
}

static int  *math_roll_report_error(char *buffer)
{
    if (buffer)
        cma_free(buffer);
    return (ft_nullptr);
}

static int execute_roll_function(char *string, RollExecuteFunc func)
{
    int currentIndex = 0;
    int stringBoundary;
    int error;

    error = 0;
    math_calculate_j(string, &stringBoundary);
    while (currentIndex < stringBoundary)
    {
        error = func(string, &currentIndex, stringBoundary);
        if (error)
            return (error);
        math_calculate_j(string, &stringBoundary);
        currentIndex++;
    }
    return (0);
}

static int math_roll_parse(char *string, int nested)
{
    int error;
    int index = nested;

    while (string[index] != '(' && string[index])
        index++;
    if (string[index] == '(')
    {
        error = math_roll_parse(&string[index], 1);
        if (error)
            return (1);
    }
    if (execute_roll_function(string, math_roll_excecute_droll))
        return 3;
    if (execute_roll_function(string, math_roll_excecute_md))
        return 4;
    if (execute_roll_function(string, math_roll_excecute_pm))
        return 5;
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

int *math_roll(const char *expression)
{
    char    *result;
    int     *value;
    int     parse_error;
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
    value = reinterpret_cast<int*>(cma_malloc(sizeof(int)));
    if (!value)
    {
        return (math_roll_report_error(result));
    }
    *value = ft_atoi(result);
    cma_free(result);
    return (value);
}
