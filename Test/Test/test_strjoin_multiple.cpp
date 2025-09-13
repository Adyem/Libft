#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../CMA/CMA.hpp"

FT_TEST(test_strjoin_multiple_basic, "ft_strjoin_multiple basic concatenation")
{
    char *joined = ft_strjoin_multiple(3, "foo", "bar", "baz");
    if (joined == ft_nullptr)
        return (0);
    int ok = ft_strcmp(joined, "foobarbaz") == 0;
    cma_free(joined);
    return (ok);
}

FT_TEST(test_strjoin_multiple_null_argument, "ft_strjoin_multiple null argument")
{
    char *joined = ft_strjoin_multiple(2, static_cast<const char *>(ft_nullptr), "bar");
    if (joined == ft_nullptr)
        return (0);
    int ok = ft_strcmp(joined, "bar") == 0;
    cma_free(joined);
    return (ok);
}

FT_TEST(test_strjoin_multiple_zero_count, "ft_strjoin_multiple zero count")
{
    FT_ASSERT_EQ(ft_nullptr, ft_strjoin_multiple(0));
    return (1);
}

FT_TEST(test_strjoin_multiple_negative_count, "ft_strjoin_multiple negative count")
{
    FT_ASSERT_EQ(ft_nullptr, ft_strjoin_multiple(-5));
    return (1);
}

FT_TEST(test_strjoin_multiple_single, "ft_strjoin_multiple single string")
{
    char *joined = ft_strjoin_multiple(1, "solo");
    if (joined == ft_nullptr)
        return (0);
    int ok = ft_strcmp(joined, "solo") == 0;
    cma_free(joined);
    return (ok);
}
