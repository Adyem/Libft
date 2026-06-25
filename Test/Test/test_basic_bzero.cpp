#include "../test_internal.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_basic_bzero_basic)
{
    char buffer[4];

    buffer[0] = 'a';
    buffer[1] = 'b';
    buffer[2] = 'c';
    buffer[3] = 'd';
    ft_bzero(buffer, 4);
    FT_ASSERT_EQ(0, buffer[0]);
    FT_ASSERT_EQ(0, buffer[1]);
    FT_ASSERT_EQ(0, buffer[2]);
    FT_ASSERT_EQ(0, buffer[3]);
    return (1);
}

FT_TEST(test_basic_bzero_partial)
{
    char buffer[5];

    buffer[0] = 'a';
    buffer[1] = 'b';
    buffer[2] = 'c';
    buffer[3] = 'd';
    buffer[4] = 'e';
    ft_bzero(buffer + 1, 3);
    FT_ASSERT_EQ('a', buffer[0]);
    FT_ASSERT_EQ(0, buffer[1]);
    FT_ASSERT_EQ(0, buffer[2]);
    FT_ASSERT_EQ(0, buffer[3]);
    FT_ASSERT_EQ('e', buffer[4]);
    return (1);
}

FT_TEST(test_basic_bzero_zero_length)
{
    char buffer[3];

    buffer[0] = 'x';
    buffer[1] = 'y';
    buffer[2] = '\0';
    ft_bzero(buffer, 0);
    FT_ASSERT_EQ('x', buffer[0]);
    FT_ASSERT_EQ('y', buffer[1]);
    FT_ASSERT_EQ('\0', buffer[2]);
    return (1);
}

FT_TEST(test_basic_bzero_null_zero)
{
    ft_bzero(ft_nullptr, 0);
    return (1);
}

FT_TEST(test_basic_bzero_null_sets_errno)
{
    ft_bzero(ft_nullptr, 4);
    return (1);
}

FT_TEST(test_basic_bzero_recovers_errno_after_error)
{
    char buffer[3];

    buffer[0] = 'x';
    buffer[1] = 'y';
    buffer[2] = 'z';
    ft_bzero(ft_nullptr, 2);
    ft_bzero(buffer, 2);
    FT_ASSERT_EQ(0, buffer[0]);
    FT_ASSERT_EQ(0, buffer[1]);
    FT_ASSERT_EQ('z', buffer[2]);
    return (1);
}

FT_TEST(test_basic_bzero_repeated_zero_length_keeps_errno_cleared)
{
    char buffer[2];

    buffer[0] = 'a';
    buffer[1] = '\0';
    ft_bzero(buffer, 0);
    FT_ASSERT_EQ('a', buffer[0]);
    ft_bzero(buffer, 0);
    FT_ASSERT_EQ('a', buffer[0]);
    return (1);
}

FT_TEST(test_basic_bzero_null_zero_length_recovers_after_error)
{
    ft_bzero(ft_nullptr, 3);
    ft_bzero(ft_nullptr, 0);
    return (1);
}

