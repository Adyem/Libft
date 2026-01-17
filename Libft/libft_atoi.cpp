#include "libft.hpp"
#include "limits.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

int    ft_atoi(const char *string)
{
    int    index = 0;
    int    sign = 1;
    unsigned long long result = 0;
    const unsigned long long positive_limit = static_cast<unsigned long long>(FT_INT_MAX);
    const unsigned long long negative_limit = static_cast<unsigned long long>(FT_INT_MAX) + 1ULL;
    bool    digit_found = false;
    bool    whitespace_found = false;
    int error_code = FT_ERR_SUCCESSS;

    if (string == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (0);
    }
    while (string[index] == ' ' || ((string[index] >= '\t')
                && (string[index] <= '\r')))
    {
        whitespace_found = true;
        index++;
    }
    if (string[index] == '+' || string[index] == '-')
    {
        if (string[index] == '-')
            sign = -1;
        index++;
    }
    while (string[index] && ((string[index] >= '0')
                && (string[index] <= '9')))
    {
        int    digit = string[index] - '0';

        digit_found = true;
        if (sign == 1)
        {
            if (result > positive_limit / 10)
            {
                ft_global_error_stack_push(FT_ERR_OUT_OF_RANGE);
                return (FT_INT_MAX);
            }
            if (result == positive_limit / 10
                && static_cast<unsigned long long>(digit) > positive_limit % 10)
            {
                ft_global_error_stack_push(FT_ERR_OUT_OF_RANGE);
                return (FT_INT_MAX);
            }
        }
        else
        {
            if (result > negative_limit / 10)
            {
                ft_global_error_stack_push(FT_ERR_OUT_OF_RANGE);
                return (FT_INT_MIN);
            }
            if (result == negative_limit / 10
                && static_cast<unsigned long long>(digit) > negative_limit % 10)
            {
                ft_global_error_stack_push(FT_ERR_OUT_OF_RANGE);
                return (FT_INT_MIN);
            }
        }
        result = result * 10 + static_cast<unsigned long long>(digit);
        index++;
    }
    if (digit_found == false)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (0);
    }
    if (string[index] != '\0')
        error_code = FT_ERR_INVALID_ARGUMENT;
    if (whitespace_found == true && error_code == FT_ERR_SUCCESSS)
        error_code = FT_ERR_INVALID_ARGUMENT;
    if (error_code != FT_ERR_SUCCESSS)
    {
        if (sign == -1)
        {
            long long signed_result = -static_cast<long long>(result);

            ft_global_error_stack_push(error_code);
            return (static_cast<int>(signed_result));
        }
        ft_global_error_stack_push(error_code);
        return (static_cast<int>(result));
    }
    if (sign == -1)
    {
        long long signed_result = -static_cast<long long>(result);

        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (static_cast<int>(signed_result));
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (static_cast<int>(result));
}
