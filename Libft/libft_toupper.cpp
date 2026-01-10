#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

static void report_case_error(int error_code)
{
    ft_global_error_stack_push(error_code);
    return ;
}

void ft_to_upper(char *string)
{
    if (string == ft_nullptr)
    {
        report_case_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    while (*string != '\0')
    {
        if (*string >= 'a' && *string <= 'z')
            *string -= 32;
        string++;
    }
    report_case_error(FT_ERR_SUCCESSS);
    return ;
}
