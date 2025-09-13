#include "../../Libft/libft.hpp"
#include "../../System_utils/test_runner.hpp"

static void to_upper_iter(unsigned int index, char *character)
{
    (void)index;
    if (*character >= 'a' && *character <= 'z')
        *character -= 32;
    return ;
}

FT_TEST(test_striteri_basic, "ft_striteri basic")
{
    char buffer[4];

    buffer[0] = 'a';
    buffer[1] = 'b';
    buffer[2] = 'c';
    buffer[3] = '\0';
    ft_striteri(buffer, to_upper_iter);
    FT_ASSERT_EQ(0, ft_strcmp("ABC", buffer));

    ft_striteri(ft_nullptr, to_upper_iter);
    ft_striteri(buffer, ft_nullptr);
    return (1);
}
