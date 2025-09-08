#include "../Math/math_roll.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/nullptr.hpp"
#include "../Errno/errno.hpp"

int test_math_eval_basic(void)
{
    int *value;
    int ok;

    value = math_eval("1+2");
    if (!value)
        return (0);
    ok = (*value == 3);
    cma_free(value);
    return (ok);
}

int test_math_eval_parentheses(void)
{
    int *value;
    int ok;

    value = math_eval("2*(3+4)");
    if (!value)
        return (0);
    ok = (*value == 14);
    cma_free(value);
    return (ok);
}

int test_math_eval_dice_rejected(void)
{
    int *value;

    ft_errno = 0;
    value = math_eval("1d6");
    if (value != ft_nullptr || ft_errno != FT_EINVAL)
    {
        if (value)
            cma_free(value);
        return (0);
    }
    return (1);
}

