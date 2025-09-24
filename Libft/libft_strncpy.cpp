#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

char *ft_strncpy(char *destination, const char *source, size_t number_of_characters)
{
    size_t index;

    ft_errno = ER_SUCCESS;
    if (destination == ft_nullptr || source == ft_nullptr)
    {
        ft_errno = FT_EINVAL;
        return (ft_nullptr);
    }
    index = 0;
    while (index < number_of_characters && source[index] != '\0')
    {
        destination[index] = source[index];
        index++;
    }
    while (index < number_of_characters)
    {
        destination[index] = '\0';
        index++;
    }
    return (destination);
}
