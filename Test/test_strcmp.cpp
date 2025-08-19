#include "../Libft/libft.hpp"
#include "../CPP_class/nullptr.hpp"

int test_strcmp_equal(void)
{
    if (ft_strcmp("abc", "abc") == 0)
        return (1);
    return (0);
}

int test_strcmp_null(void)
{
    if (ft_strcmp(ft_nullptr, "abc") == -1)
        return (1);
    return (0);
}
