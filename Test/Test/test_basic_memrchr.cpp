#include "../test_internal.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/CPP_class/class_nullptr.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_basic_memrchr_found)
{
    char buffer[5];

    buffer[0] = 'a';
    buffer[1] = 'b';
    buffer[2] = 'c';
    buffer[3] = 'b';
    buffer[4] = 'd';
    FT_ASSERT_EQ(buffer + 3, ft_memrchr(buffer, 'b', 5));
    return (1);
}

FT_TEST(test_basic_memrchr_not_found)
{
    char buffer[4];

    buffer[0] = 'a';
    buffer[1] = 'b';
    buffer[2] = 'c';
    buffer[3] = 'd';
    FT_ASSERT_EQ(ft_nullptr, ft_memrchr(buffer, 'x', 4));
    return (1);
}

FT_TEST(test_basic_memrchr_null_char)
{
    char buffer[4];

    buffer[0] = 'a';
    buffer[1] = '\0';
    buffer[2] = 'b';
    buffer[3] = '\0';
    FT_ASSERT_EQ(buffer + 3, ft_memrchr(buffer, '\0', 4));
    return (1);
}

FT_TEST(test_basic_memrchr_zero_length)
{
    char buffer[1];

    buffer[0] = 'a';
    FT_ASSERT_EQ(ft_nullptr, ft_memrchr(buffer, 'a', 0));
    return (1);
}

FT_TEST(test_basic_memrchr_nullptr_zero)
{
    FT_ASSERT_EQ(ft_nullptr, ft_memrchr(ft_nullptr, 'a', 0));
    return (1);
}

FT_TEST(test_basic_memrchr_null_sets_errno)
{
    FT_ASSERT_EQ(ft_nullptr, ft_memrchr(ft_nullptr, 'a', 1));
    return (1);
}

FT_TEST(test_basic_memrchr_matches_first_byte)
{
    char buffer[4];

    buffer[0] = 'x';
    buffer[1] = 'y';
    buffer[2] = 'z';
    buffer[3] = 'w';
    FT_ASSERT_EQ(buffer, ft_memrchr(buffer, 'x', 4));
    return (1);
}

FT_TEST(test_basic_memrchr_signed_byte)
{
    char buffer[3];

    buffer[0] = 'a';
    buffer[1] = static_cast<char>(0xF2);
    buffer[2] = '\0';
    FT_ASSERT_EQ(buffer + 1, ft_memrchr(buffer, 0xF2, 3));
    return (1);
}
