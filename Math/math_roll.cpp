#include "math_roll.hpp"
#include "math_internal.hpp"
#include "../CMA/CMA.hpp"
#include "../Printf/printf.hpp"
#include "../GetNextLine/get_next_line.hpp"
#include "../CPP_class/cpp_class_nullptr.hpp"
#include "../Libft/libft.hpp"
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

    if (!expression)
    {
        ft_errno = FT_EINVAL;
        return (ft_nullptr);
    }
    result = cma_strdup(expression);
    if (!result)
    {
        ft_errno = FT_EALLOC;
        pf_printf_fd(2, "168-Error: Malloc failed in cma_strdup\n");
        return (ft_nullptr);
    }
    if (math_roll_validate(result))
    {
        ft_errno = FT_EINVAL;
        pf_printf_fd(2, "169-Command Roll Error with the string: %s\n", result);
        cma_free(result);
        return (ft_nullptr);
    }
    parse_error = math_roll_parse(result, 0);
    if (parse_error)
    {
        ft_errno = FT_EINVAL;
        cma_free(result);
        return (ft_nullptr);
    }
    if (!math_check_string_number(result))
    {
        ft_errno = FT_EINVAL;
        cma_free(result);
        return (ft_nullptr);
    }
    value = reinterpret_cast<int*>(cma_malloc(sizeof(int)));
    if (!value)
    {
        ft_errno = FT_EALLOC;
        cma_free(result);
        return (ft_nullptr);
    }
    *value = ft_atoi(result);
    cma_free(result);
    return (value);
}
