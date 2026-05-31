#include "../test_internal.hpp"
#include "../../Modules/CMA/CMA.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

static void *test_cma_memdup(const void *source_pointer, size_t size_value)
{
    void *duplicate_pointer;

    if (size_value > 0 && source_pointer == ft_nullptr)
        return (ft_nullptr);
    duplicate_pointer = cma_malloc(size_value);
    if (duplicate_pointer == ft_nullptr)
        return (ft_nullptr);
    if (size_value > 0)
        ft_memcpy(duplicate_pointer, source_pointer, size_value);
    return (duplicate_pointer);
}

FT_TEST(test_memdup_basic)
{
    const char source[] = "hello";
    unsigned char *duplicate = static_cast<unsigned char *>(test_cma_memdup(source, sizeof(source)));

    FT_ASSERT(duplicate != ft_nullptr);
    FT_ASSERT(ft_strcmp(reinterpret_cast<char *>(duplicate), source) == 0);
    FT_ASSERT(duplicate != reinterpret_cast<const unsigned char *>(source));
    cma_free(duplicate);
    return (1);
}

FT_TEST(test_memdup_zero_size)
{
    unsigned char *duplicate = static_cast<unsigned char *>(test_cma_memdup(ft_nullptr, 0));

    FT_ASSERT(duplicate != ft_nullptr);
    cma_free(duplicate);
    return (1);
}

FT_TEST(test_memdup_null_source)
{
    unsigned char *duplicate = static_cast<unsigned char *>(test_cma_memdup(ft_nullptr, 5));

    FT_ASSERT_EQ(ft_nullptr, duplicate);
    return (1);
}

FT_TEST(test_memdup_independent_copy)
{
    const unsigned char source[] = {0x01, 0x02, 0xFF};
    unsigned char *duplicate = static_cast<unsigned char *>(test_cma_memdup(source, sizeof(source)));

    FT_ASSERT(duplicate != ft_nullptr);
    duplicate[0] = 0xAA;
    FT_ASSERT_EQ(source[0], static_cast<unsigned char>(0x01));
    cma_free(duplicate);
    return (1);
}

FT_TEST(test_memdup_preserves_embedded_null_bytes)
{
    const unsigned char source[] = {0x30, 0x00, 0x31};
    unsigned char *duplicate = static_cast<unsigned char *>(test_cma_memdup(source, sizeof(source)));

    FT_ASSERT(duplicate != ft_nullptr);
    FT_ASSERT_EQ(duplicate[1], static_cast<unsigned char>(0x00));
    cma_free(duplicate);
    return (1);
}
