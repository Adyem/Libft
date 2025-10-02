#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_memset_null, "ft_memset nullptr")
{
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(ft_nullptr, ft_memset(ft_nullptr, 'A', 3));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_memset_basic, "ft_memset basic")
{
    char buffer[4];

    ft_errno = FT_EINVAL;
    ft_memset(buffer, 'x', 3);
    buffer[3] = '\0';
    FT_ASSERT_EQ(0, ft_strcmp(buffer, "xxx"));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_memset_large, "ft_memset large buffer")
{
    char buffer[1024];
    size_t index;

    ft_errno = FT_EINVAL;
    ft_memset(buffer, 0xAB, sizeof(buffer));
    index = 0;
    while (index < sizeof(buffer))
    {
        if (static_cast<unsigned char>(buffer[index]) != 0xAB)
            FT_ASSERT(0);
        index++;
    }
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_memset_zero_length, "ft_memset zero length")
{
    char buffer[4];

    buffer[0] = 'a';
    buffer[1] = 'b';
    buffer[2] = 'c';
    buffer[3] = '\0';
    ft_errno = FT_EINVAL;
    FT_ASSERT_EQ(buffer, ft_memset(buffer, 'x', 0));
    FT_ASSERT_EQ(0, ft_strcmp(buffer, "abc"));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_memset_null_zero_length_recovers_errno, "ft_memset nullptr zero length clears errno")
{
    ft_errno = FT_EINVAL;
    FT_ASSERT_EQ(ft_nullptr, ft_memset(ft_nullptr, 'a', 0));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_memset_negative, "ft_memset negative value")
{
    char buffer[4];
    size_t index;

    ft_errno = FT_EINVAL;
    ft_memset(buffer, -1, 3);
    index = 0;
    while (index < 3)
    {
        if (static_cast<unsigned char>(buffer[index]) != 0xFF)
            FT_ASSERT(0);
        index++;
    }
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_memset_overflow, "ft_memset overflow value")
{
    char buffer[4];
    size_t index;

    ft_errno = FT_EINVAL;
    ft_memset(buffer, 256, 3);
    index = 0;
    while (index < 3)
    {
        if (buffer[index] != '\0')
            FT_ASSERT(0);
        index++;
    }
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}
