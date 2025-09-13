#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_strtol_decimal, "ft_strtol decimal with end pointer")
{
    char *end;
    FT_ASSERT_EQ(123, ft_strtol("123abc", &end, 10));
    FT_ASSERT_EQ('a', *end);
    return (1);
}

FT_TEST(test_strtol_hex_prefix, "ft_strtol hex prefix base 0")
{
    char *end;
    FT_ASSERT_EQ(26, ft_strtol("0x1a", &end, 0));
    FT_ASSERT_EQ('\0', *end);
    return (1);
}

FT_TEST(test_strtol_octal_auto, "ft_strtol octal auto base")
{
    char *end;
    FT_ASSERT_EQ(9, ft_strtol("011", &end, 0));
    FT_ASSERT_EQ('\0', *end);
    return (1);
}

FT_TEST(test_strtol_negative, "ft_strtol negative number")
{
    FT_ASSERT_EQ(-42, ft_strtol("-42", ft_nullptr, 10));
    return (1);
}

FT_TEST(test_strtol_base16_skip, "ft_strtol base16 skip prefix")
{
    char *end;
    FT_ASSERT_EQ(26, ft_strtol("0x1a", &end, 16));
    FT_ASSERT_EQ('\0', *end);
    return (1);
}

FT_TEST(test_strtol_invalid, "ft_strtol invalid string")
{
    char *end;
    FT_ASSERT_EQ(0, ft_strtol("xyz", &end, 10));
    FT_ASSERT_EQ('x', *end);
    return (1);
}
