#include "../Libft/libft.hpp"
#include "../CPP_class/nullptr.hpp"

int test_strlen_nullptr(void)
{
    if (ft_strlen(ft_nullptr) == 0)
        return (1);
    return (0);
}

int test_strlen_simple(void)
{
    if (ft_strlen("test") == 4)
        return (1);
    return (0);
}

int test_strlen_long(void)
{
    static char buf[1025];
    for (int i = 0; i < 1024; ++i)
        buf[i] = 'a';
    buf[1024] = '\0';
    return (ft_strlen(buf) == 1024);
}
