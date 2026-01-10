#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

static int report_memcmp_result(int error_code, int return_value)
{
    ft_global_error_stack_push(error_code);
    return (return_value);
}

int    ft_memcmp(const void *pointer1, const void *pointer2, size_t size)
{
    const unsigned char    *string1;
    const unsigned char    *string2;
    size_t                index;

    if (size == 0)
        return (report_memcmp_result(FT_ERR_SUCCESSS, 0));
    if (pointer1 == ft_nullptr || pointer2 == ft_nullptr)
        return (report_memcmp_result(FT_ERR_INVALID_ARGUMENT, 0));

    index = 0;
    string1 = static_cast<const unsigned char *>(pointer1);
    string2 = static_cast<const unsigned char *>(pointer2);
    while (index < size)
    {
        if (string1[index] != string2[index])
        {
            return (report_memcmp_result(FT_ERR_SUCCESSS,
                string1[index] - string2[index]));
        }
        index++;
    }
    return (report_memcmp_result(FT_ERR_SUCCESSS, 0));
}
