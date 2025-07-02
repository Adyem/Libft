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
