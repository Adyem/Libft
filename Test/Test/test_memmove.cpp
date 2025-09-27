#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_memmove_overlap_forward, "ft_memmove overlap forward")
{
    char buffer[8];

    buffer[0] = 'a';
    buffer[1] = 'b';
    buffer[2] = 'c';
    buffer[3] = 'd';
    buffer[4] = 'e';
    buffer[5] = 'f';
    buffer[6] = 'g';
    buffer[7] = '\0';
    FT_ASSERT_EQ(buffer + 2, ft_memmove(buffer + 2, buffer, 5));
    FT_ASSERT_EQ('a', buffer[0]);
    FT_ASSERT_EQ('a', buffer[2]);
    FT_ASSERT_EQ('e', buffer[6]);
    FT_ASSERT_EQ('\0', buffer[7]);
    return (1);
}

FT_TEST(test_memmove_overlap_backward, "ft_memmove overlap backward")
{
    char buffer[8];

    buffer[0] = 'a';
    buffer[1] = 'b';
    buffer[2] = 'c';
    buffer[3] = 'd';
    buffer[4] = 'e';
    buffer[5] = 'f';
    buffer[6] = 'g';
    buffer[7] = '\0';
    FT_ASSERT_EQ(buffer, ft_memmove(buffer, buffer + 2, 5));
    FT_ASSERT_EQ('c', buffer[0]);
    FT_ASSERT_EQ('g', buffer[4]);
    FT_ASSERT_EQ('\0', buffer[7]);
    return (1);
}

FT_TEST(test_memmove_zero_length, "ft_memmove zero length")
{
    char source[4];
    char destination[4];

    source[0] = 'x';
    source[1] = 'y';
    source[2] = 'z';
    source[3] = '\0';
    destination[0] = 'a';
    destination[1] = 'b';
    destination[2] = 'c';
    destination[3] = '\0';
    FT_ASSERT_EQ(destination, ft_memmove(destination, source, 0));
    FT_ASSERT_EQ('a', destination[0]);
    FT_ASSERT_EQ('c', destination[2]);
    return (1);
}

FT_TEST(test_memmove_null_zero_length_clears_errno, "ft_memmove zero length nullptr clears errno")
{
    ft_errno = FT_EINVAL;
    FT_ASSERT_EQ(ft_nullptr, ft_memmove(ft_nullptr, ft_nullptr, 0));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_memmove_null, "ft_memmove with nullptr")
{
    char source[1];
    source[0] = 'a';
    FT_ASSERT_EQ(ft_nullptr, ft_memmove(ft_nullptr, source, 1));
    FT_ASSERT_EQ(ft_nullptr, ft_memmove(source, ft_nullptr, 1));
    return (1);
}

FT_TEST(test_memmove_same_pointer, "ft_memmove same pointer")
{
    char buffer[4];

    buffer[0] = 'h';
    buffer[1] = 'i';
    buffer[2] = 'j';
    buffer[3] = 'k';
    FT_ASSERT_EQ(buffer, ft_memmove(buffer, buffer, 4));
    FT_ASSERT_EQ('h', buffer[0]);
    FT_ASSERT_EQ('k', buffer[3]);
    return (1);
}

FT_TEST(test_memmove_errno_recovers_after_failure, "ft_memmove resets errno after null failure")
{
    char source[5];
    char destination[5];

    source[0] = 'a';
    source[1] = 'b';
    source[2] = 'c';
    source[3] = 'd';
    source[4] = '\0';
    destination[0] = 'x';
    destination[1] = 'x';
    destination[2] = 'x';
    destination[3] = 'x';
    destination[4] = '\0';
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(ft_nullptr, ft_memmove(ft_nullptr, source, 3));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    FT_ASSERT_EQ(destination, ft_memmove(destination, source, 4));
    FT_ASSERT_EQ('a', destination[0]);
    FT_ASSERT_EQ('b', destination[1]);
    FT_ASSERT_EQ('c', destination[2]);
    FT_ASSERT_EQ('d', destination[3]);
    FT_ASSERT_EQ('\0', destination[4]);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}
