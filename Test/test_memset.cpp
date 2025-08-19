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

int test_memset_large(void)
{
    char buf[1024];
    ft_memset(buf, 0xAB, sizeof(buf));
    for (size_t i = 0; i < sizeof(buf); ++i)
    {
        if (static_cast<unsigned char>(buf[i]) != 0xAB)
            return 0;
    }
    return 1;
}
