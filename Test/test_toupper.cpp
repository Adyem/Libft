#include "../Libft/libft.hpp"

int test_toupper_basic(void)
{
    char str[4] = "abc";
    ft_to_upper(str);
    if (ft_strcmp(str, "ABC") == 0)
        return (1);
    return (0);
}
