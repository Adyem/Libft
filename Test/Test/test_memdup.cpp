#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"
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
    duplicate = ft_memdup(source, 5);
    FT_ASSERT(duplicate != ft_nullptr);
    FT_ASSERT(duplicate != source);
    FT_ASSERT_EQ(0, ft_memcmp(source, duplicate, 5));
    cma_free(duplicate);
    return (1);
}

FT_TEST(test_memdup_zero_size, "ft_memdup zero size")
{
    char buffer[3];

    buffer[0] = 'a';
    buffer[1] = 'b';
    buffer[2] = 'c';
    FT_ASSERT_EQ(ft_nullptr, ft_memdup(buffer, 0));
    return (1);
}

FT_TEST(test_memdup_null_source, "ft_memdup null source")
{
    FT_ASSERT_EQ(ft_nullptr, ft_memdup(ft_nullptr, 5));
    return (1);
}
