#include <cstdio>
#include "../../Libft/libft.hpp"
#include "../../Libft/limits.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
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

FT_TEST(test_strtoul_above_long_max, "ft_strtoul parses values above FT_LONG_MAX")
{
    char value_buffer[64];
    char *end;
    unsigned long expected_value = static_cast<unsigned long>(FT_LONG_MAX);

    expected_value = expected_value + 1UL;
    std::snprintf(value_buffer, sizeof(value_buffer), "%lu", expected_value);
    ft_errno = FT_ERANGE;
    FT_ASSERT_EQ(expected_value, ft_strtoul(value_buffer, &end, 10));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ('\0', *end);
    return (1);
}

FT_TEST(test_strtoul_unsigned_long_max, "ft_strtoul parses FT_ULONG_MAX without overflow")
{
    char value_buffer[64];
    char *end;

    std::snprintf(value_buffer, sizeof(value_buffer), "%lu", FT_ULONG_MAX);
    ft_errno = FT_ERANGE;
    FT_ASSERT_EQ(FT_ULONG_MAX, ft_strtoul(value_buffer, &end, 10));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ('\0', *end);
    return (1);
}

FT_TEST(test_strtoul_overflow, "ft_strtoul clamps overflow and reports error")
{
    char value_buffer[64];
    char *end;

    std::snprintf(value_buffer, sizeof(value_buffer), "%lu", FT_ULONG_MAX);
    ft_string overflow_string = value_buffer;
    overflow_string.append('9');
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(FT_ULONG_MAX, ft_strtoul(overflow_string.c_str(), &end, 10));
    FT_ASSERT_EQ(FT_ERANGE, ft_errno);
    FT_ASSERT_EQ('9', *end);
    return (1);
}

FT_TEST(test_strtoul_null_input, "ft_strtoul null input sets errno and end pointer")
{
    char *end_pointer = reinterpret_cast<char *>(0x1);

    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(0UL, ft_strtoul(ft_nullptr, &end_pointer, 10));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    FT_ASSERT_EQ(ft_nullptr, end_pointer);
    return (1);
}
