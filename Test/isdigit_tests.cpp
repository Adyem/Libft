#include "../Libft/libft.hpp"

int test_isdigit_true(void)
{
    if (ft_isdigit('5') == 1)
        return (1);
    return (0);
}

int test_isdigit_false(void)
{
    if (ft_isdigit('a') == 0)
        return (1);
    return (0);
}
