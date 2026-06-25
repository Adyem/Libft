#include "roll.hpp"
#include "../Printf/printf.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"

int32_t    *math_eval(const char *expression)
{
    int32_t     index;
    int32_t     *result;

    if (!expression)
        return (ft_nullptr);
    index = 0;
    while (expression[index])
    {
        if (expression[index] == 'd')
        {
            if (DEBUG == 1)
                pf_printf_fd(2, "dice rolls are not allowed\n");
            return (ft_nullptr);
        }
        index++;
    }
    result = math_roll(expression);
    if (!result)
        return (ft_nullptr);
    return (result);
}
