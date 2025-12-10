#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_memset_null, "ft_memset nullptr")
{
    ft_errno = FT_ER_SUCCESSS;
    FT_ASSERT_EQ(ft_nullptr, ft_memset(ft_nullptr, 'A', 3));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_memset_basic, "ft_memset basic")
{
    char buffer[4];

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    ft_memset(buffer, 'x', 3);
    buffer[3] = '\0';
    FT_ASSERT_EQ(0, ft_strcmp(buffer, "xxx"));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_memset_large, "ft_memset large buffer")
{
    char buffer[1024];
    size_t index;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    ft_memset(buffer, 0xAB, sizeof(buffer));
    index = 0;
    while (index < sizeof(buffer))
    {
        if (static_cast<unsigned char>(buffer[index]) != 0xAB)
            FT_ASSERT(0);
        index++;
    }
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_memset_zero_length, "ft_memset zero length")
{
    char buffer[4];

    buffer[0] = 'a';
    buffer[1] = 'b';
    buffer[2] = 'c';
    buffer[3] = '\0';
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(buffer, ft_memset(buffer, 'x', 0));
    FT_ASSERT_EQ(0, ft_strcmp(buffer, "abc"));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_memset_null_zero_length_recovers_errno, "ft_memset nullptr zero length clears errno")
{
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(ft_nullptr, ft_memset(ft_nullptr, 'a', 0));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_memset_negative, "ft_memset negative value")
{
    char buffer[4];
    size_t index;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    ft_memset(buffer, -1, 3);
    index = 0;
    while (index < 3)
    {
        if (static_cast<unsigned char>(buffer[index]) != 0xFF)
            FT_ASSERT(0);
        index++;
    }
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_memset_partial_fill_preserves_tail, "ft_memset writes only the requested byte count")
{
    char buffer[5];

    buffer[0] = 'u';
    buffer[1] = 'v';
    buffer[2] = 'w';
    buffer[3] = 'x';
    buffer[4] = '\0';
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    ft_memset(buffer, 'a', 2);
    FT_ASSERT_EQ('a', buffer[0]);
    FT_ASSERT_EQ('a', buffer[1]);
    FT_ASSERT_EQ('w', buffer[2]);
    FT_ASSERT_EQ('x', buffer[3]);
    FT_ASSERT_EQ('\0', buffer[4]);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_memset_overflow, "ft_memset overflow value")
{
    char buffer[4];
    size_t index;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    ft_memset(buffer, 256, 3);
    index = 0;
    while (index < 3)
    {
        if (buffer[index] != '\0')
            FT_ASSERT(0);
        index++;
    }
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_memset_partial_preserves_unwritten_bytes, "ft_memset leaves trailing bytes unchanged")
{
    char buffer[5];

    buffer[0] = 'a';
    buffer[1] = 'b';
    buffer[2] = 'c';
    buffer[3] = 'd';
    buffer[4] = '\0';
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    ft_memset(buffer + 1, 'z', 2);
    FT_ASSERT_EQ('a', buffer[0]);
    FT_ASSERT_EQ('z', buffer[1]);
    FT_ASSERT_EQ('z', buffer[2]);
    FT_ASSERT_EQ('d', buffer[3]);
    FT_ASSERT_EQ('\0', buffer[4]);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_memset_recovers_after_null_failure, "ft_memset clears errno after null pointer error")
{
    char buffer[4];

    buffer[0] = 'r';
    buffer[1] = 'e';
    buffer[2] = 's';
    buffer[3] = 't';
    ft_errno = FT_ER_SUCCESSS;
    FT_ASSERT_EQ(ft_nullptr, ft_memset(ft_nullptr, 'x', 2));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(buffer, ft_memset(buffer, 'q', 3));
    FT_ASSERT_EQ('q', buffer[0]);
    FT_ASSERT_EQ('q', buffer[1]);
    FT_ASSERT_EQ('q', buffer[2]);
    FT_ASSERT_EQ('t', buffer[3]);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}

FT_TEST(test_memset_returns_destination_pointer, "ft_memset returns destination address on success")
{
    char buffer[6];

    buffer[0] = 'j';
    buffer[1] = 'k';
    buffer[2] = 'l';
    buffer[3] = 'm';
    buffer[4] = 'n';
    buffer[5] = '\0';
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(buffer + 1, ft_memset(buffer + 1, 'x', 3));
    FT_ASSERT_EQ('j', buffer[0]);
    FT_ASSERT_EQ('x', buffer[1]);
    FT_ASSERT_EQ('x', buffer[2]);
    FT_ASSERT_EQ('x', buffer[3]);
    FT_ASSERT_EQ('n', buffer[4]);
    FT_ASSERT_EQ('\0', buffer[5]);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    return (1);
}
