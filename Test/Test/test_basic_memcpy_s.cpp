#include "../test_internal.hpp"
#include "../../Basic/basic.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_basic_memcpy_s_basic)
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
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_memcpy_s(destination, sizeof(destination), source, 4));
    FT_ASSERT_EQ('a', destination[0]);
    FT_ASSERT_EQ('b', destination[1]);
    FT_ASSERT_EQ('c', destination[2]);
    FT_ASSERT_EQ('\0', destination[3]);
    return (1);
}

FT_TEST(test_basic_memcpy_s_destination_too_small)
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
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_memcpy_s(destination, sizeof(destination), source, 4));
    FT_ASSERT_EQ('\0', destination[0]);
    FT_ASSERT_EQ('\0', destination[1]);
    FT_ASSERT_EQ('\0', destination[2]);
    return (1);
}

FT_TEST(test_basic_memcpy_s_null_source_rejects_copy)
{
    char destination[3];

    destination[0] = 'z';
    destination[1] = 'y';
    destination[2] = '\0';
    FT_ASSERT_EQ(FT_ERR_INVALID_POINTER, ft_memcpy_s(destination, sizeof(destination), ft_nullptr, 2));
    FT_ASSERT_EQ('\0', destination[0]);
    FT_ASSERT_EQ('\0', destination[1]);
    FT_ASSERT_EQ('\0', destination[2]);
    return (1);
}

FT_TEST(test_basic_memcpy_s_overlap_detected)
{
    char buffer[6];

    buffer[0] = '1';
    buffer[1] = '2';
    buffer[2] = '3';
    buffer[3] = '4';
    buffer[4] = '5';
    buffer[5] = '\0';
    FT_ASSERT_EQ(FT_ERR_OVERLAP, ft_memcpy_s(buffer + 1, 5, buffer, 4));
    FT_ASSERT_EQ('\0', buffer[1]);
    FT_ASSERT_EQ('\0', buffer[2]);
    FT_ASSERT_EQ('\0', buffer[3]);
    FT_ASSERT_EQ('\0', buffer[4]);
    FT_ASSERT_EQ('\0', buffer[5]);
    return (1);
}

FT_TEST(test_basic_memcpy_s_recovers_after_overlap)
{
    char source[3];
    char destination[3];

    source[0] = 'x';
    source[1] = 'y';
    source[2] = '\0';
    destination[0] = 'a';
    destination[1] = 'b';
    destination[2] = '\0';
    FT_ASSERT_EQ(FT_ERR_OVERLAP, ft_memcpy_s(destination, 3, destination, 2));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_memcpy_s(destination, 3, source, 3));
    FT_ASSERT_EQ('x', destination[0]);
    FT_ASSERT_EQ('y', destination[1]);
    FT_ASSERT_EQ('\0', destination[2]);
    return (1);
}

FT_TEST(test_basic_memcpy_s_zero_length_allows_null)
{
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_memcpy_s(ft_nullptr, 0, ft_nullptr, 0));
    return (1);
}

FT_TEST(test_basic_memcpy_s_zero_length_preserves_buffer)
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
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_memcpy_s(destination, sizeof(destination), source, 0));
    FT_ASSERT_EQ('x', destination[0]);
    FT_ASSERT_EQ('y', destination[1]);
    FT_ASSERT_EQ('z', destination[2]);
    FT_ASSERT_EQ('\0', destination[3]);
    return (1);
}

FT_TEST(test_basic_memmove_s_basic)
{
    char buffer[6];

    buffer[0] = '1';
    buffer[1] = '2';
    buffer[2] = '3';
    buffer[3] = '4';
    buffer[4] = '5';
    buffer[5] = '\0';
    FT_ASSERT_EQ(FT_ERR_OVERLAP, ft_memmove_s(buffer + 1, 5, buffer, 5));
    FT_ASSERT_EQ('\0', buffer[1]);
    FT_ASSERT_EQ('\0', buffer[2]);
    FT_ASSERT_EQ('\0', buffer[3]);
    FT_ASSERT_EQ('\0', buffer[4]);
    FT_ASSERT_EQ('\0', buffer[5]);
    return (1);
}

FT_TEST(test_basic_memmove_s_zero_length_allows_null)
{
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ft_memmove_s(ft_nullptr, 0, ft_nullptr, 0));
    return (1);
}

FT_TEST(test_basic_memmove_s_destination_too_small)
{
    char buffer[4];

    buffer[0] = 'a';
    buffer[1] = 'b';
    buffer[2] = 'c';
    buffer[3] = 'd';
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_memmove_s(buffer, 3, buffer + 1, 4));
    FT_ASSERT_EQ('\0', buffer[0]);
    FT_ASSERT_EQ('\0', buffer[1]);
    FT_ASSERT_EQ('\0', buffer[2]);
    FT_ASSERT_EQ('d', buffer[3]);
    return (1);
}
