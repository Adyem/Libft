#include "../../Libft/libft.hpp"
#include "../../Libft/limits.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
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

FT_TEST(test_strtol_maximum_in_range, "ft_strtol handles FT_LONG_MAX without overflow")
{
    char *end;
    ft_string max_string = ft_to_string(FT_LONG_MAX);

    ft_errno = FT_ERANGE;
    FT_ASSERT_EQ(FT_LONG_MAX, ft_strtol(max_string.c_str(), &end, 10));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ('\0', *end);
    return (1);
}

FT_TEST(test_strtol_positive_overflow, "ft_strtol clamps positive overflow and reports error")
{
    char *end;
    ft_string max_string = ft_to_string(FT_LONG_MAX);
    ft_string overflow_string = max_string;

    overflow_string.append('9');
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(FT_LONG_MAX, ft_strtol(overflow_string.c_str(), &end, 10));
    FT_ASSERT_EQ(FT_ERANGE, ft_errno);
    FT_ASSERT_EQ('9', *end);
    return (1);
}

FT_TEST(test_strtol_negative_overflow, "ft_strtol clamps negative overflow and reports error")
{
    char *end;
    ft_string min_string = ft_to_string(FT_LONG_MIN);
    ft_string underflow_string = min_string;

    underflow_string.append('9');
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(FT_LONG_MIN, ft_strtol(underflow_string.c_str(), &end, 10));
    FT_ASSERT_EQ(FT_ERANGE, ft_errno);
    FT_ASSERT_EQ('9', *end);
    return (1);
}

FT_TEST(test_strtol_null_input, "ft_strtol null input sets errno and end pointer")
{
    char *end_pointer = reinterpret_cast<char *>(0x1);

    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(0, ft_strtol(ft_nullptr, &end_pointer, 10));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    FT_ASSERT_EQ(ft_nullptr, end_pointer);
    return (1);
}
