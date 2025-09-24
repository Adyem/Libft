#include "../../Libft/libft.hpp"
#include "../../Errno/errno.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"

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

FT_TEST(test_strlen_empty, "ft_strlen empty string")
{
    FT_ASSERT_EQ(0, ft_strlen(""));
    return (1);
}

FT_TEST(test_strlen_embedded_null, "ft_strlen embedded null")
{
    char string[6];

    string[0] = 'a';
    string[1] = 'b';
    string[2] = '\0';
    string[3] = 'c';
    string[4] = 'd';
    string[5] = '\0';
    FT_ASSERT_EQ(2, ft_strlen(string));
    return (1);
}

FT_TEST(test_strlen_resets_errno_on_success, "ft_strlen clears ft_errno before measuring")
{
    ft_errno = FT_ERANGE;
    FT_ASSERT_EQ(3, ft_strlen("abc"));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}
