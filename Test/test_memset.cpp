#include "../Libft/libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../System_utils/test_runner.hpp"

FT_TEST(test_memset_null, "ft_memset nullptr")
{
    FT_ASSERT_EQ(ft_nullptr, ft_memset(ft_nullptr, 'A', 3));
    return (1);
}

FT_TEST(test_memset_basic, "ft_memset basic")
{
    char buffer[4];

    ft_memset(buffer, 'x', 3);
    buffer[3] = '\0';
    FT_ASSERT_EQ(0, ft_strcmp(buffer, "xxx"));
    return (1);
}

FT_TEST(test_memset_large, "ft_memset large buffer")
{
    char buffer[1024];
    size_t index;

    ft_memset(buffer, 0xAB, sizeof(buffer));
    index = 0;
    while (index < sizeof(buffer))
    {
        if (static_cast<unsigned char>(buffer[index]) != 0xAB)
            FT_ASSERT(0);
        index++;
    }
    return (1);
}
