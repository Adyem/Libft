#include "../../Basic/basic.hpp"
#include "../../Errno/errno.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../CMA/CMA.hpp"

FT_TEST(test_strjoin_multiple_basic, "cma_strjoin_multiple basic concatenation")
{
    char *joined = cma_strjoin_multiple(3, "foo", "bar", "baz");
    if (joined == ft_nullptr)
        return (0);
    int ok = ft_strcmp(joined, "foobarbaz") == 0;
    cma_free(joined);
    return (ok);
}

FT_TEST(test_strjoin_multiple_null_argument, "cma_strjoin_multiple null argument")
{
    char *joined = cma_strjoin_multiple(2, static_cast<const char *>(ft_nullptr), "bar");
    if (joined == ft_nullptr)
        return (0);
    int ok = ft_strcmp(joined, "bar") == 0;
    cma_free(joined);
    return (ok);
}

FT_TEST(test_strjoin_multiple_zero_count, "cma_strjoin_multiple zero count")
{
    FT_ASSERT_EQ(ft_nullptr, cma_strjoin_multiple(0));
    return (1);
}

FT_TEST(test_strjoin_multiple_negative_count, "cma_strjoin_multiple negative count")
{
    FT_ASSERT_EQ(ft_nullptr, cma_strjoin_multiple(-5));
    return (1);
}

FT_TEST(test_strjoin_multiple_single, "cma_strjoin_multiple single string")
{
    char *joined = cma_strjoin_multiple(1, "solo");
    if (joined == ft_nullptr)
        return (0);
    int ok = ft_strcmp(joined, "solo") == 0;
    cma_free(joined);
    return (ok);
}

FT_TEST(test_strjoin_multiple_resets_errno_before_joining, "cma_strjoin_multiple clears ft_errno before processing")
{
    char *joined_string;
    int result;

    ft_errno = FT_ERR_OUT_OF_RANGE;
    joined_string = cma_strjoin_multiple(2, "foo", "bar");
    if (joined_string == ft_nullptr)
        return (0);
    result = (ft_errno == FT_ERR_SUCCESSS && ft_strcmp(joined_string, "foobar") == 0);
    cma_free(joined_string);
    return (result);
}

FT_TEST(test_strjoin_multiple_retains_empty_segments, "cma_strjoin_multiple preserves empty strings")
{
    char *joined;
    int result;

    joined = cma_strjoin_multiple(4, "", "alpha", "", "beta");
    if (joined == ft_nullptr)
        return (0);
    result = (ft_strcmp(joined, "alphabeta") == 0);
    cma_free(joined);
    return (result);
}

FT_TEST(test_strjoin_multiple_null_segment_recovers_errno, "cma_strjoin_multiple clears errno with null segments")
{
    char *joined;
    int result;

    ft_errno = FT_ERR_INVALID_ARGUMENT;
    joined = cma_strjoin_multiple(3, "alpha", static_cast<const char *>(ft_nullptr), "beta");
    if (joined == ft_nullptr)
        return (0);
    result = (ft_errno == FT_ERR_SUCCESSS && ft_strcmp(joined, "alphabeta") == 0);
    cma_free(joined);
    return (result);
}
