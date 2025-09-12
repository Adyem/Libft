#include "../Libft/libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../System_utils/test_runner.hpp"

FT_TEST(test_strlen_nullptr, "ft_strlen nullptr")
{
    FT_ASSERT_EQ(0, ft_strlen(ft_nullptr));
    return (1);
}

FT_TEST(test_strlen_simple, "ft_strlen basic")
{
    FT_ASSERT_EQ(4, ft_strlen("test"));
    return (1);
}

FT_TEST(test_strlen_long, "ft_strlen long string")
{
    static char buffer[1025];
    int index;

    index = 0;
    while (index < 1024)
    {
        buffer[index] = 'a';
        index++;
    }
    buffer[1024] = '\0';
    FT_ASSERT_EQ(1024, ft_strlen(buffer));
    return (1);
}
