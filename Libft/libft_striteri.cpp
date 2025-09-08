#include "libft.hpp"
#include "../CPP_class/cpp_class_nullptr.hpp"

void ft_striteri(char *string, void (*function)(unsigned int, char *))
{
    if (string == ft_nullptr || function == ft_nullptr)
        return ;
    unsigned int index = 0;
    while (string[index] != '\0')
    {
        function(index, &string[index]);
        index++;
    }
}
