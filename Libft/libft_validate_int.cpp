#include "libft.hpp"
#include "limits.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

static int report_validate_int_result(int error_code, int return_value)
{
    ft_global_error_stack_push(error_code);
    return (return_value);
}

int ft_validate_int(const char *input)
{
    long result;
    long maximum_value;
    long minimum_value;
    int index;
    int sign;
    int digit;

    if (input == ft_nullptr)
        return (report_validate_int_result(FT_ERR_INVALID_ARGUMENT,
            FT_FAILURE));
    result = 0;
    maximum_value = static_cast<long>(FT_INT_MAX);
    minimum_value = static_cast<long>(FT_INT_MIN);
    index = 0;
    sign = 1;
    if (input[index] == '+' || input[index] == '-')
    {
        if (input[index] == '-')
            sign = -1;
        index++;
    }
    if (input[index] == '\0')
        return (report_validate_int_result(FT_ERR_INVALID_ARGUMENT,
            FT_FAILURE));
    while (input[index] != '\0')
    {
        if (input[index] >= '0' && input[index] <= '9')
        {
            digit = input[index] - '0';
            if (sign == 1)
            {
                if (result > (maximum_value - digit) / 10)
                    return (report_validate_int_result(FT_ERR_OUT_OF_RANGE,
                        FT_FAILURE));
                result = (result * 10) + digit;
            }
            else
            {
                if (result < (minimum_value + digit) / 10)
                    return (report_validate_int_result(FT_ERR_OUT_OF_RANGE,
                        FT_FAILURE));
                result = (result * 10) - digit;
            }
            index++;
        }
        else
        {
            return (report_validate_int_result(FT_ERR_INVALID_ARGUMENT,
                FT_FAILURE));
        }
    }
    return (report_validate_int_result(FT_ERR_SUCCESSS, FT_SUCCESS));
}
