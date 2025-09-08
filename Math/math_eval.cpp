#include "math_roll.hpp"
#include "../Printf/printf.hpp"
#include "../CPP_class/nullptr.hpp"
#include "../Errno/errno.hpp"

int    *math_eval(const char *expression)
{
    int index;

    if (!expression)
    {
        ft_errno = FT_EINVAL;
        return (ft_nullptr);
    }
    index = 0;
    while (expression[index])
    {
        if (expression[index] == 'd')
        {
            ft_errno = FT_EINVAL;
            pf_printf_fd(2, "dice rolls are not allowed\n");
            return (ft_nullptr);
        }
        index++;
    }
    return (math_roll(expression));
}
