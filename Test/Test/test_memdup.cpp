#include "../test_internal.hpp"
#include "../../CMA/CMA.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"

extern "C" void *cma_memdup(const void *source, size_t size);

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_memdup_basic, "cma_memdup duplicates memory")
{
    const char source[] = "hello";
    unsigned char *duplicate = static_cast<unsigned char *>(cma_memdup(source, sizeof(source)));

    FT_ASSERT(duplicate != ft_nullptr);
    FT_ASSERT(ft_strcmp(reinterpret_cast<char *>(duplicate), source) == 0);
    FT_ASSERT(duplicate != reinterpret_cast<const unsigned char *>(source));
    cma_free(duplicate);
    return (1);
}

FT_TEST(test_memdup_zero_size, "cma_memdup returns non-null block for zero-size target")
{
    unsigned char *duplicate = static_cast<unsigned char *>(cma_memdup(ft_nullptr, 0));

    FT_ASSERT(duplicate != ft_nullptr);
    cma_free(duplicate);
    return (1);
}

FT_TEST(test_memdup_null_source, "cma_memdup rejects null source when size > 0")
{
    unsigned char *duplicate = static_cast<unsigned char *>(cma_memdup(ft_nullptr, 5));

    FT_ASSERT_EQ(ft_nullptr, duplicate);
    return (1);
}

FT_TEST(test_memdup_independent_copy, "cma_memdup result is independent of source")
{
    const unsigned char source[] = {0x01, 0x02, 0xFF};
    unsigned char *duplicate = static_cast<unsigned char *>(cma_memdup(source, sizeof(source)));

    FT_ASSERT(duplicate != ft_nullptr);
    duplicate[0] = 0xAA;
    FT_ASSERT_EQ(source[0], static_cast<unsigned char>(0x01));
    cma_free(duplicate);
    return (1);
}

FT_TEST(test_memdup_preserves_embedded_null_bytes, "cma_memdup copies embedded nulls")
{
    const unsigned char source[] = {0x30, 0x00, 0x31};
    unsigned char *duplicate = static_cast<unsigned char *>(cma_memdup(source, sizeof(source)));

    FT_ASSERT(duplicate != ft_nullptr);
    FT_ASSERT_EQ(duplicate[1], static_cast<unsigned char>(0x00));
    cma_free(duplicate);
    return (1);
}
