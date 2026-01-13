#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

static int report_strcmp_result(int error_code, int return_value)
{
    ft_global_error_stack_push(error_code);
    return (return_value);
}

int    ft_strcmp(const char *string1, const char *string2)
{
    int error_code;

    if (string1 == ft_nullptr || string2 == ft_nullptr)
        return (report_strcmp_result(FT_ERR_INVALID_ARGUMENT, -1));
    while (*string1 != '\0' && static_cast<unsigned char>(*string1) == static_cast<unsigned char>(*string2))
    {
        string1++;
        string2++;
    }
    unsigned char left_character = static_cast<unsigned char>(*string1);
    unsigned char right_character = static_cast<unsigned char>(*string2);
    int left_value = static_cast<int>(left_character);
    int right_value = static_cast<int>(right_character);
    int result = left_value - right_value;
    error_code = FT_ERR_SUCCESSS;
    return (report_strcmp_result(error_code, result));
}
