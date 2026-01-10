#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

static void *report_memchr_result(int error_code, void *result)
{
    ft_global_error_stack_push(error_code);
    return (result);
}

void* ft_memchr(const void* pointer, int number, size_t size)
{
    size_t index;
    const unsigned char *string;
    unsigned char character;

    if (size == 0)
        return (report_memchr_result(FT_ERR_SUCCESSS, ft_nullptr));
    if (pointer == ft_nullptr)
        return (report_memchr_result(FT_ERR_INVALID_ARGUMENT, ft_nullptr));
    string = static_cast<const unsigned char*>(pointer);
    character = static_cast<unsigned char>(number);
    index = 0;
    while (index < size)
    {
        if (*string == character)
        {
            return (report_memchr_result(FT_ERR_SUCCESSS,
                const_cast<void*>(static_cast<const void*>(string))));
        }
        string++;
        index++;
    }
    return (report_memchr_result(FT_ERR_SUCCESSS, ft_nullptr));
}
