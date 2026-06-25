#include "../test_internal.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_basic_memset_null)
{
    FT_ASSERT_EQ(ft_nullptr, ft_memset(ft_nullptr, 'A', 3));
    return (1);
}

FT_TEST(test_basic_memset_basic)
{
    char buffer[4];

    ft_memset(buffer, 'x', 3);
    buffer[3] = '\0';
    FT_ASSERT_EQ(0, ft_strcmp(buffer, "xxx"));
    return (1);
}

FT_TEST(test_basic_memset_large)
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

FT_TEST(test_basic_memset_zero_length)
{
    char buffer[4];

    buffer[0] = 'a';
    buffer[1] = 'b';
    buffer[2] = 'c';
    buffer[3] = '\0';
    FT_ASSERT_EQ(buffer, ft_memset(buffer, 'x', 0));
    FT_ASSERT_EQ(0, ft_strcmp(buffer, "abc"));
    return (1);
}

FT_TEST(test_basic_memset_null_zero_length_recovers_errno)
{
    FT_ASSERT_EQ(ft_nullptr, ft_memset(ft_nullptr, 'a', 0));
    return (1);
}

FT_TEST(test_basic_memset_negative)
{
    char buffer[4];
    size_t index;

    ft_memset(buffer, -1, 3);
    index = 0;
    while (index < 3)
    {
        if (static_cast<unsigned char>(buffer[index]) != 0xFF)
            FT_ASSERT(0);
        index++;
    }
    return (1);
}

FT_TEST(test_basic_memset_partial_fill_preserves_tail)
{
    char buffer[5];

    buffer[0] = 'u';
    buffer[1] = 'v';
    buffer[2] = 'w';
    buffer[3] = 'x';
    buffer[4] = '\0';
    ft_memset(buffer, 'a', 2);
    FT_ASSERT_EQ('a', buffer[0]);
    FT_ASSERT_EQ('a', buffer[1]);
    FT_ASSERT_EQ('w', buffer[2]);
    FT_ASSERT_EQ('x', buffer[3]);
    FT_ASSERT_EQ('\0', buffer[4]);
    return (1);
}

FT_TEST(test_basic_memset_overflow)
{
    char buffer[4];
    size_t index;

    ft_memset(buffer, 256, 3);
    index = 0;
    while (index < 3)
    {
        if (buffer[index] != '\0')
            FT_ASSERT(0);
        index++;
    }
    return (1);
}

FT_TEST(test_basic_memset_partial_preserves_unwritten_bytes)
{
    char buffer[5];

    buffer[0] = 'a';
    buffer[1] = 'b';
    buffer[2] = 'c';
    buffer[3] = 'd';
    buffer[4] = '\0';
    ft_memset(buffer + 1, 'z', 2);
    FT_ASSERT_EQ('a', buffer[0]);
    FT_ASSERT_EQ('z', buffer[1]);
    FT_ASSERT_EQ('z', buffer[2]);
    FT_ASSERT_EQ('d', buffer[3]);
    FT_ASSERT_EQ('\0', buffer[4]);
    return (1);
}

FT_TEST(test_basic_memset_recovers_after_null_failure)
{
    char buffer[4];

    buffer[0] = 'r';
    buffer[1] = 'e';
    buffer[2] = 's';
    buffer[3] = 't';
    FT_ASSERT_EQ(ft_nullptr, ft_memset(ft_nullptr, 'x', 2));
    FT_ASSERT_EQ(buffer, ft_memset(buffer, 'q', 3));
    FT_ASSERT_EQ('q', buffer[0]);
    FT_ASSERT_EQ('q', buffer[1]);
    FT_ASSERT_EQ('q', buffer[2]);
    FT_ASSERT_EQ('t', buffer[3]);
    return (1);
}

FT_TEST(test_basic_memset_returns_destination_pointer)
{
    char buffer[6];

    buffer[0] = 'j';
    buffer[1] = 'k';
    buffer[2] = 'l';
    buffer[3] = 'm';
    buffer[4] = 'n';
    buffer[5] = '\0';
    FT_ASSERT_EQ(buffer + 1, ft_memset(buffer + 1, 'x', 3));
    FT_ASSERT_EQ('j', buffer[0]);
    FT_ASSERT_EQ('x', buffer[1]);
    FT_ASSERT_EQ('x', buffer[2]);
    FT_ASSERT_EQ('x', buffer[3]);
    FT_ASSERT_EQ('n', buffer[4]);
    FT_ASSERT_EQ('\0', buffer[5]);
    return (1);
}
