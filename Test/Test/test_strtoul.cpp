#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_strtoul_decimal, "ft_strtoul decimal with end pointer")
{
    char *end;
    FT_ASSERT_EQ(static_cast<unsigned long>(123), ft_strtoul("123xyz", &end, 10));
    FT_ASSERT_EQ('x', *end);
    return (1);
}

FT_TEST(test_strtoul_hex, "ft_strtoul hex base 16")
{
    FT_ASSERT_EQ(static_cast<unsigned long>(0x2a), ft_strtoul("2a", ft_nullptr, 16));
    return (1);
}

FT_TEST(test_strtoul_negative, "ft_strtoul negative input")
{
    FT_ASSERT_EQ(static_cast<unsigned long>(-1), ft_strtoul("-1", ft_nullptr, 10));
    return (1);
}

FT_TEST(test_strtoul_base0, "ft_strtoul base 0 hex prefix")
{
    char *end;
    FT_ASSERT_EQ(static_cast<unsigned long>(0x2a), ft_strtoul("0x2a", &end, 0));
    FT_ASSERT_EQ('\0', *end);
    return (1);
}
