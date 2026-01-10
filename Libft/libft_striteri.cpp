#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

static void report_striteri_error(int error_code)
{
    ft_global_error_stack_push(error_code);
    return ;
}

void ft_striteri(char *string, void (*function)(unsigned int, char *))
{
    if (string == ft_nullptr || function == ft_nullptr)
    {
        report_striteri_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    unsigned int index = 0;
    while (string[index] != '\0')
    {
        function(index, &string[index]);
        index++;
    }
    report_striteri_error(FT_ERR_SUCCESSS);
    return ;
}
