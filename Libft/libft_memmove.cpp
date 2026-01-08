#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

void *ft_memmove(void *destination, const void *source, size_t size)
{
    unsigned char *destination_pointer = static_cast<unsigned char *>(destination);
    const unsigned char *source_pointer = static_cast<const unsigned char *>(source);
    size_t index;

    if (destination == ft_nullptr || source == ft_nullptr)
    {
        if (size > 0)
        {
            ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
            return (ft_nullptr);
        }
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (destination);
    }
    if (size == 0 || destination == source)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (destination);
    }
    if (destination_pointer < source_pointer)
    {
        index = 0;
        while (index < size)
        {
            destination_pointer[index] = source_pointer[index];
            index++;
        }
    }
    else
    {
        index = size;
        while (index > 0)
        {
            destination_pointer[index - 1] = source_pointer[index - 1];
            index--;
        }
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (destination);
}
