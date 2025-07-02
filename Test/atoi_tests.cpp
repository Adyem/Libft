#include "../Libft/libft.hpp"

int test_atoi_simple(void)
{
    if (ft_atoi("42") == 42)
        return (1);
    return (0);
}

int test_atoi_negative(void)
{
    if (ft_atoi("-13") == -13)
        return (1);
    return (0);
}
