#include "roll.hpp"
#include "../Printf/printf.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

static int  *math_eval_report_error(int error_code)
{
    ft_global_error_stack_push(error_code);
    return (ft_nullptr);
}

int    *math_eval(const char *expression)
{
    int     index;
    int     *result;
    int     error_code;

    if (!expression)
    {
        return (math_eval_report_error(FT_ERR_INVALID_ARGUMENT));
    }
    index = 0;
    while (expression[index])
    {
        if (expression[index] == 'd')
        {
            if (DEBUG == 1)
            {
                pf_printf_fd(2, "dice rolls are not allowed\n");
            }
            return (math_eval_report_error(FT_ERR_INVALID_ARGUMENT));
        }
        index++;
    }
    result = math_roll(expression);
    if (!result)
    {
        error_code = ft_global_error_stack_pop_newest();
        if (error_code == FT_ERR_SUCCESSS)
            error_code = FT_ERR_INTERNAL;
        return (math_eval_report_error(error_code));
    }
    error_code = ft_global_error_stack_pop_newest();
    if (error_code != FT_ERR_SUCCESSS)
        return (math_eval_report_error(error_code));
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}
