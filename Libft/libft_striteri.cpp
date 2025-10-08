#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

void ft_striteri(char *string, void (*function)(unsigned int, char *))
{
    ft_errno = ER_SUCCESS;
    if (string == ft_nullptr || function == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return ;
    }
    unsigned int index = 0;
    while (string[index] != '\0')
    {
        function(index, &string[index]);
        index++;
    }
}
