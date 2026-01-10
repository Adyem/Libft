#include "libft.hpp"
#include "limits.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

static long report_atol_result(int error_code, long return_value)
{
    ft_global_error_stack_push(error_code);
    return (return_value);
}

long ft_atol(const char *string)
{
    long index = 0;
    long sign = 1;
    unsigned long long result = 0;
    const unsigned long long positive_limit = static_cast<unsigned long long>(FT_LONG_MAX);
    const unsigned long long negative_limit = static_cast<unsigned long long>(FT_LONG_MAX) + 1ULL;
    bool digit_found = false;
    int error_code = FT_ERR_SUCCESSS;

    if (string == ft_nullptr)
        return (report_atol_result(FT_ERR_INVALID_ARGUMENT, 0));
    while (string[index] == ' ' || ((string[index] >= '\t')
            && (string[index] <= '\r')))
        index++;
    if (string[index] == '+' || string[index] == '-')
    {
        if (string[index] == '-')
            sign = -1;
        index++;
    }
    while (string[index] && ((string[index] >= '0')
            && (string[index] <= '9')))
    {
        int digit = string[index] - '0';

        digit_found = true;
        if (sign == 1)
        {
            if (result > positive_limit / 10)
                return (report_atol_result(FT_ERR_OUT_OF_RANGE, FT_LONG_MAX));
            if (result == positive_limit / 10
                && static_cast<unsigned long long>(digit) > positive_limit % 10)
                return (report_atol_result(FT_ERR_OUT_OF_RANGE, FT_LONG_MAX));
        }
        else
        {
            if (result > negative_limit / 10)
                return (report_atol_result(FT_ERR_OUT_OF_RANGE, FT_LONG_MIN));
            if (result == negative_limit / 10
                && static_cast<unsigned long long>(digit) > negative_limit % 10)
                return (report_atol_result(FT_ERR_OUT_OF_RANGE, FT_LONG_MIN));
        }
        result = result * 10 + static_cast<unsigned long long>(digit);
        index++;
    }
    if (digit_found == false)
        return (report_atol_result(FT_ERR_INVALID_ARGUMENT, 0));
    if (string[index] != '\0')
        error_code = FT_ERR_INVALID_ARGUMENT;
    if (error_code != FT_ERR_SUCCESSS)
    {
        if (sign == -1)
        {
            if (result == negative_limit)
            {
                return (report_atol_result(error_code, FT_LONG_MIN));
            }
            long signed_result = -static_cast<long>(result);

            return (report_atol_result(error_code, signed_result));
        }
        return (report_atol_result(error_code, static_cast<long>(result)));
    }
    if (sign == -1)
    {
        if (result == negative_limit)
            return (report_atol_result(FT_ERR_SUCCESSS, FT_LONG_MIN));
        long signed_result = -static_cast<long>(result);

        return (report_atol_result(FT_ERR_SUCCESSS, signed_result));
    }
    return (report_atol_result(FT_ERR_SUCCESSS, static_cast<long>(result)));
}
