#include "../Libft/libft.hpp"
#include <climits>
#include <string>

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

int test_atoi_intmax(void)
{
    std::string str = std::to_string(INT_MAX);
    return (ft_atoi(str.c_str()) == INT_MAX);
}

int test_atoi_intmin(void)
{
    std::string str = std::to_string(INT_MIN);
    return (ft_atoi(str.c_str()) == INT_MIN);
}
