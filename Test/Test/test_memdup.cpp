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
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
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
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    cma_free(duplicate);
    return (1);
}

FT_TEST(test_memdup_zero_size_null_source_allocates, "ft_memdup allocates even when size is zero and source is null")
{
    void *duplicate;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    duplicate = ft_memdup(ft_nullptr, 0);
    FT_ASSERT(duplicate != ft_nullptr);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    cma_free(duplicate);
    return (1);
}

FT_TEST(test_memdup_null_source, "ft_memdup null source")
{
    ft_errno = FT_ER_SUCCESSS;
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
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    cma_free(duplicate);
    return (1);
}

FT_TEST(test_memdup_resets_errno_after_null_source, "ft_memdup clears errno after failure")
{
    void *duplicate;

    ft_errno = FT_ER_SUCCESSS;
    FT_ASSERT_EQ(ft_nullptr, ft_memdup(ft_nullptr, 3));
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    duplicate = ft_memdup("ok", 2);
    FT_ASSERT(duplicate != ft_nullptr);
    FT_ASSERT_EQ(0, ft_memcmp(duplicate, "ok", 2));
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    cma_free(duplicate);
    return (1);
}

FT_TEST(test_memdup_preserves_embedded_null_bytes, "ft_memdup copies buffers containing null bytes")
{
    unsigned char source[5];
    unsigned char *duplicate;

    source[0] = 'a';
    source[1] = 'b';
    source[2] = '\0';
    source[3] = 'c';
    source[4] = 'd';
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    duplicate = static_cast<unsigned char *>(ft_memdup(source, sizeof(source)));
    FT_ASSERT(duplicate != ft_nullptr);
    FT_ASSERT_EQ('a', duplicate[0]);
    FT_ASSERT_EQ('b', duplicate[1]);
    FT_ASSERT_EQ('\0', duplicate[2]);
    FT_ASSERT_EQ('c', duplicate[3]);
    FT_ASSERT_EQ('d', duplicate[4]);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, ft_errno);
    cma_free(duplicate);
    return (1);
}
