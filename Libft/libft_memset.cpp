#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

static void *report_memset_result(int error_code, void *result)
{
    ft_global_error_stack_push(error_code);
    return (result);
}

void *ft_memset(void *destination, int value, size_t number_of_bytes)
{
    int error_code;

    if (destination == ft_nullptr)
    {
        if (number_of_bytes == 0)
        {
            error_code = FT_ERR_SUCCESSS;
            return (report_memset_result(error_code, ft_nullptr));
        }
        return (report_memset_result(FT_ERR_INVALID_ARGUMENT, ft_nullptr));
    }
    unsigned char *destination_pointer = static_cast<unsigned char *>(destination);
    unsigned char byte_value = static_cast<unsigned char>(value);

    while (number_of_bytes)
    {
        *destination_pointer = byte_value;
        destination_pointer++;
        number_of_bytes--;
    }

    error_code = FT_ERR_SUCCESSS;
    return (report_memset_result(error_code, destination));
}
