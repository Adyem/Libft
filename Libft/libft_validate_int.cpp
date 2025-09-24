#include "libft.hpp"
#include "limits.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

int ft_validate_int(const char *input)
{
    long result;
    int index;
    int sign;
    int digit;

    if (input == ft_nullptr)
    {
        ft_errno = FT_EINVAL;
        return (FT_FAILURE);
    }
    ft_errno = ER_SUCCESS;
    result = 0;
    index = 0;
    sign = 1;
    if (input[index] == '+' || input[index] == '-')
    {
        if (input[index] == '-')
            sign = -1;
        index++;
    }
    if (input[index] == '\0')
    {
        ft_errno = FT_EINVAL;
        return (FT_FAILURE);
    }
    while (input[index] != '\0')
    {
        if (input[index] >= '0' && input[index] <= '9')
        {
            digit = input[index] - '0';
            if (sign == 1)
            {
                if (result > ((long)FT_INT_MAX - digit) / 10)
                {
                    ft_errno = FT_ERANGE;
                    return (FT_FAILURE);
                }
                result = (result * 10) + digit;
            }
            else
            {
                if (result < ((long)FT_INT_MIN + digit) / 10)
                {
                    ft_errno = FT_ERANGE;
                    return (FT_FAILURE);
                }
                result = (result * 10) - digit;
            }
            index++;
        }
        else
        {
            ft_errno = FT_EINVAL;
            return (FT_FAILURE);
        }
    }
    return (FT_SUCCESS);
}
