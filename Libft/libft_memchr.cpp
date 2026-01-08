#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

void* ft_memchr(const void* pointer, int number, size_t size)
{
    size_t index;
    const unsigned char *string;
    unsigned char character;

    if (size == 0)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (ft_nullptr);
    }
    if (pointer == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    string = static_cast<const unsigned char*>(pointer);
    character = static_cast<unsigned char>(number);
    index = 0;
    while (index < size)
    {
        if (*string == character)
        {
            ft_global_error_stack_push(FT_ERR_SUCCESSS);
            return (const_cast<void*>(static_cast<const void*>(string)));
        }
        string++;
        index++;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (ft_nullptr);
}
