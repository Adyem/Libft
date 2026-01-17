#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

void ft_striteri(char *string, void (*function)(unsigned int, char *))
{
    if (string == ft_nullptr || function == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    unsigned int index = 0;
    while (string[index] != '\0')
    {
        function(index, &string[index]);
        index++;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}
