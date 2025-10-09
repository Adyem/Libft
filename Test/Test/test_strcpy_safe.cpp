#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_strcpy_s_basic, "ft_strcpy_s copies string when buffer fits")
{
    char destination[6];

    destination[0] = 'x';
    destination[1] = 'x';
    destination[2] = 'x';
    destination[3] = 'x';
    destination[4] = 'x';
    destination[5] = '\0';
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(0, ft_strcpy_s(destination, sizeof(destination), "hi"));
    FT_ASSERT_EQ('h', destination[0]);
    FT_ASSERT_EQ('i', destination[1]);
    FT_ASSERT_EQ('\0', destination[2]);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strcpy_s_too_small, "ft_strcpy_s zeros destination when too small")
{
    char destination[3];

    destination[0] = 'a';
    destination[1] = 'b';
    destination[2] = 'c';
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(-1, ft_strcpy_s(destination, sizeof(destination), "hello"));
    FT_ASSERT_EQ('\0', destination[0]);
    FT_ASSERT_EQ('\0', destination[1]);
    FT_ASSERT_EQ('\0', destination[2]);
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, ft_errno);
    return (1);
}

FT_TEST(test_strcpy_s_null_argument, "ft_strcpy_s rejects nullptr")
{
    char destination[2];

    destination[0] = 'x';
    destination[1] = '\0';
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(-1, ft_strcpy_s(ft_nullptr, sizeof(destination), "a"));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(-1, ft_strcpy_s(destination, sizeof(destination), ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_strcat_s_basic, "ft_strcat_s appends when space permits")
{
    char destination[9];

    destination[0] = 'h';
    destination[1] = 'i';
    destination[2] = '\0';
    destination[3] = 'x';
    destination[4] = 'x';
    destination[5] = 'x';
    destination[6] = 'x';
    destination[7] = 'x';
    destination[8] = '\0';
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    FT_ASSERT_EQ(0, ft_strcat_s(destination, sizeof(destination), " there"));
    FT_ASSERT_EQ('h', destination[0]);
    FT_ASSERT_EQ('i', destination[1]);
    FT_ASSERT_EQ(' ', destination[2]);
    FT_ASSERT_EQ('t', destination[3]);
    FT_ASSERT_EQ('h', destination[4]);
    FT_ASSERT_EQ('e', destination[5]);
    FT_ASSERT_EQ('r', destination[6]);
    FT_ASSERT_EQ('e', destination[7]);
    FT_ASSERT_EQ('\0', destination[8]);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_strcat_s_too_small, "ft_strcat_s zeros destination on truncation")
{
    char destination[6];

    destination[0] = 'h';
    destination[1] = 'i';
    destination[2] = '\0';
    destination[3] = 'x';
    destination[4] = 'x';
    destination[5] = '\0';
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(-1, ft_strcat_s(destination, sizeof(destination), " world"));
    FT_ASSERT_EQ('\0', destination[0]);
    FT_ASSERT_EQ('\0', destination[1]);
    FT_ASSERT_EQ('\0', destination[2]);
    FT_ASSERT_EQ('\0', destination[3]);
    FT_ASSERT_EQ('\0', destination[4]);
    FT_ASSERT_EQ('\0', destination[5]);
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, ft_errno);
    return (1);
}

FT_TEST(test_strcat_s_null_argument, "ft_strcat_s rejects nullptr")
{
    char destination[2];

    destination[0] = 'a';
    destination[1] = '\0';
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(-1, ft_strcat_s(ft_nullptr, sizeof(destination), "a"));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(-1, ft_strcat_s(destination, sizeof(destination), ft_nullptr));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}
