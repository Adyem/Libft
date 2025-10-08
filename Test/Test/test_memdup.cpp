#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../CMA/CMA.hpp"

FT_TEST(test_memdup_basic, "ft_memdup basic")
{
    char source[5];
    void *duplicate;

    source[0] = 'h';
    source[1] = 'e';
    source[2] = 'l';
    source[3] = 'l';
    source[4] = 'o';
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    duplicate = ft_memdup(source, 5);
    FT_ASSERT(duplicate != ft_nullptr);
    FT_ASSERT(duplicate != source);
    FT_ASSERT_EQ(0, ft_memcmp(source, duplicate, 5));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    cma_free(duplicate);
    return (1);
}

FT_TEST(test_memdup_zero_size, "ft_memdup zero size")
{
    char buffer[3];

    buffer[0] = 'a';
    buffer[1] = 'b';
    buffer[2] = 'c';
    void *duplicate;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    duplicate = ft_memdup(buffer, 0);
    FT_ASSERT(duplicate != ft_nullptr);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    cma_free(duplicate);
    return (1);
}

FT_TEST(test_memdup_null_source, "ft_memdup null source")
{
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(ft_nullptr, ft_memdup(ft_nullptr, 5));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    return (1);
}

FT_TEST(test_memdup_independent_copy, "ft_memdup duplicates without sharing storage")
{
    char source[4];
    unsigned char *duplicate;

    source[0] = 'f';
    source[1] = 'o';
    source[2] = 'o';
    source[3] = '!';
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    duplicate = static_cast<unsigned char *>(ft_memdup(source, sizeof(source)));
    FT_ASSERT(duplicate != ft_nullptr);
    duplicate[0] = 'b';
    duplicate[1] = 'a';
    duplicate[2] = 'r';
    duplicate[3] = '?';
    FT_ASSERT_EQ('f', source[0]);
    FT_ASSERT_EQ('o', source[1]);
    FT_ASSERT_EQ('o', source[2]);
    FT_ASSERT_EQ('!', source[3]);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    cma_free(duplicate);
    return (1);
}
