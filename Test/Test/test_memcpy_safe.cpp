#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_memcpy_s_basic, "ft_memcpy_s copies within bounds")
{
    char source[4];
    char destination[4];

    source[0] = 'a';
    source[1] = 'b';
    source[2] = 'c';
    source[3] = '\0';
    destination[0] = 'x';
    destination[1] = 'x';
    destination[2] = 'x';
    destination[3] = '\0';
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(0, ft_memcpy_s(destination, sizeof(destination), source, 4));
    FT_ASSERT_EQ('a', destination[0]);
    FT_ASSERT_EQ('b', destination[1]);
    FT_ASSERT_EQ('c', destination[2]);
    FT_ASSERT_EQ('\0', destination[3]);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_memcpy_s_destination_too_small, "ft_memcpy_s reports truncation")
{
    char source[4];
    char destination[3];

    source[0] = 'a';
    source[1] = 'b';
    source[2] = 'c';
    source[3] = '\0';
    destination[0] = 'q';
    destination[1] = 'q';
    destination[2] = 'q';
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(-1, ft_memcpy_s(destination, sizeof(destination), source, 4));
    FT_ASSERT_EQ('\0', destination[0]);
    FT_ASSERT_EQ('\0', destination[1]);
    FT_ASSERT_EQ('\0', destination[2]);
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, ft_errno);
    return (1);
}

FT_TEST(test_memcpy_s_null_source_rejects_copy, "ft_memcpy_s rejects null source and clears destination")
{
    char destination[3];

    destination[0] = 'z';
    destination[1] = 'y';
    destination[2] = '\0';
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(-1, ft_memcpy_s(destination, sizeof(destination), ft_nullptr, 2));
    FT_ASSERT_EQ('\0', destination[0]);
    FT_ASSERT_EQ('\0', destination[1]);
    FT_ASSERT_EQ('\0', destination[2]);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_memcpy_s_overlap_detected, "ft_memcpy_s detects overlap and clears destination")
{
    char buffer[6];

    buffer[0] = '1';
    buffer[1] = '2';
    buffer[2] = '3';
    buffer[3] = '4';
    buffer[4] = '5';
    buffer[5] = '\0';
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(-1, ft_memcpy_s(buffer + 1, 5, buffer, 4));
    FT_ASSERT_EQ('\0', buffer[1]);
    FT_ASSERT_EQ('\0', buffer[2]);
    FT_ASSERT_EQ('\0', buffer[3]);
    FT_ASSERT_EQ('\0', buffer[4]);
    FT_ASSERT_EQ('\0', buffer[5]);
    FT_ASSERT_EQ(FT_ERR_OVERLAP, ft_errno);
    return (1);
}

FT_TEST(test_memcpy_s_recovers_after_overlap, "ft_memcpy_s clears errno after a successful copy")
{
    char source[3];
    char destination[3];

    source[0] = 'x';
    source[1] = 'y';
    source[2] = '\0';
    destination[0] = 'a';
    destination[1] = 'b';
    destination[2] = '\0';
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(-1, ft_memcpy_s(destination, 3, destination, 2));
    FT_ASSERT_EQ(FT_ERR_OVERLAP, ft_errno);
    FT_ASSERT_EQ(0, ft_memcpy_s(destination, 3, source, 3));
    FT_ASSERT_EQ('x', destination[0]);
    FT_ASSERT_EQ('y', destination[1]);
    FT_ASSERT_EQ('\0', destination[2]);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_memcpy_s_zero_length_allows_null, "ft_memcpy_s allows nullptr when size is zero")
{
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(0, ft_memcpy_s(ft_nullptr, 0, ft_nullptr, 0));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_memcpy_s_zero_length_preserves_buffer, "ft_memcpy_s leaves buffers unchanged when copying zero bytes")
{
    char source[4];
    char destination[4];

    source[0] = 'a';
    source[1] = 'b';
    source[2] = 'c';
    source[3] = '\0';
    destination[0] = 'x';
    destination[1] = 'y';
    destination[2] = 'z';
    destination[3] = '\0';
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(0, ft_memcpy_s(destination, sizeof(destination), source, 0));
    FT_ASSERT_EQ('x', destination[0]);
    FT_ASSERT_EQ('y', destination[1]);
    FT_ASSERT_EQ('z', destination[2]);
    FT_ASSERT_EQ('\0', destination[3]);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_memmove_s_basic, "ft_memmove_s handles overlap within bounds")
{
    char buffer[6];

    buffer[0] = '1';
    buffer[1] = '2';
    buffer[2] = '3';
    buffer[3] = '4';
    buffer[4] = '5';
    buffer[5] = '\0';
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(0, ft_memmove_s(buffer + 1, 5, buffer, 5));
    FT_ASSERT_EQ('1', buffer[1]);
    FT_ASSERT_EQ('2', buffer[2]);
    FT_ASSERT_EQ('3', buffer[3]);
    FT_ASSERT_EQ('4', buffer[4]);
    FT_ASSERT_EQ('5', buffer[5]);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_memmove_s_zero_length_allows_null, "ft_memmove_s permits nullptr when length is zero")
{
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(0, ft_memmove_s(ft_nullptr, 0, ft_nullptr, 0));
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_memmove_s_destination_too_small, "ft_memmove_s clears destination on truncation")
{
    char buffer[4];

    buffer[0] = 'a';
    buffer[1] = 'b';
    buffer[2] = 'c';
    buffer[3] = 'd';
    ft_errno = FT_ERR_SUCCESSS;
    FT_ASSERT_EQ(-1, ft_memmove_s(buffer, 3, buffer + 1, 4));
    FT_ASSERT_EQ('\0', buffer[0]);
    FT_ASSERT_EQ('\0', buffer[1]);
    FT_ASSERT_EQ('\0', buffer[2]);
    FT_ASSERT_EQ('d', buffer[3]);
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, ft_errno);
    return (1);
}
