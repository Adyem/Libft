#include "../Libft/libft.hpp"
#include "../CPP_class/nullptr.hpp"

int test_memset_null(void)
{
    if (ft_memset(ft_nullptr, 'A', 3) == ft_nullptr)
        return (1);
    return (0);
}

int test_memset_basic(void)
{
    char buf[4];
    ft_memset(buf, 'x', 3);
    buf[3] = '\0';
    if (ft_strcmp(buf, "xxx") == 0)
        return (1);
    return (0);
}
