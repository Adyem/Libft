#include "../test_internal.hpp"
#include "../../Advanced/advanced.hpp"
#include "../../Basic/basic.hpp"
#include "../../CMA/CMA.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

static ft_size_t advanced_current_allocated_bytes(void)
{
    ft_size_t allocation_count;
    ft_size_t free_count;
    ft_size_t current_bytes;
    ft_size_t peak_bytes;
    int32_t status;

    allocation_count = 0;
    free_count = 0;
    current_bytes = 0;
    peak_bytes = 0;
    status = cma_get_extended_stats(&allocation_count, &free_count,
            &current_bytes, &peak_bytes);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, status);
    return (current_bytes);
}

FT_TEST(test_strjoin_multiple_basic, "adv_strjoin_multiple basic concatenation")
{
    char *joined = adv_strjoin_multiple(3, "foo", "bar", "baz");
    if (joined == ft_nullptr)
        return (0);
    int result = (ft_strcmp(joined, "foobarbaz") == 0);
    cma_free(joined);
    return (result);
}

FT_TEST(test_strjoin_multiple_null_argument, "adv_strjoin_multiple treats null as empty")
{
    char *joined = adv_strjoin_multiple(2, static_cast<const char *>(ft_nullptr), "bar");
    if (joined == ft_nullptr)
        return (0);
    int result = (ft_strcmp(joined, "bar") == 0);
    cma_free(joined);
    return (result);
}

FT_TEST(test_strjoin_multiple_zero_count, "adv_strjoin_multiple rejects zero count")
{
    FT_ASSERT_EQ(ft_nullptr, adv_strjoin_multiple(0));
    return (1);
}

FT_TEST(test_strjoin_multiple_negative_count, "adv_strjoin_multiple rejects negative count")
{
    FT_ASSERT_EQ(ft_nullptr, adv_strjoin_multiple(-5));
    return (1);
}

FT_TEST(test_strjoin_multiple_single, "adv_strjoin_multiple handles one string")
{
    char *joined = adv_strjoin_multiple(1, "solo");
    if (joined == ft_nullptr)
        return (0);
    int result = (ft_strcmp(joined, "solo") == 0);
    cma_free(joined);
    return (result);
}

FT_TEST(test_strjoin_multiple_reusable_after_failure, "adv_strjoin_multiple still works after invalid call")
{
    char *joined;
    int result;

    FT_ASSERT_EQ(ft_nullptr, adv_strjoin_multiple(0));
    joined = adv_strjoin_multiple(2, "foo", "bar");
    if (joined == ft_nullptr)
        return (0);
    result = (ft_strcmp(joined, "foobar") == 0);
    cma_free(joined);
    return (result);
}

FT_TEST(test_strjoin_multiple_retains_empty_segments, "adv_strjoin_multiple preserves empty strings")
{
    char *joined;
    int result;

    joined = adv_strjoin_multiple(4, "", "alpha", "", "beta");
    if (joined == ft_nullptr)
        return (0);
    result = (ft_strcmp(joined, "alphabeta") == 0);
    cma_free(joined);
    return (result);
}

FT_TEST(test_strjoin_multiple_all_null_arguments, "adv_strjoin_multiple returns empty string for all null arguments")
{
    char *joined;
    int result;

    joined = adv_strjoin_multiple(2, static_cast<const char *>(ft_nullptr),
            static_cast<const char *>(ft_nullptr));
    if (joined == ft_nullptr)
        return (0);
    result = (ft_strcmp(joined, "") == 0);
    cma_free(joined);
    return (result);
}

FT_TEST(test_strjoin_multiple_allocation_failure, "adv_strjoin_multiple returns null when allocation fails")
{
    char *joined;

    cma_set_alloc_limit(1);
    joined = adv_strjoin_multiple(2, "foo", "bar");
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(ft_nullptr, joined);
    return (1);
}

FT_TEST(test_strjoin_multiple_late_allocation_failure_releases_memory,
    "adv_strjoin_multiple frees the length cache when result allocation fails")
{
    char *joined;
    ft_size_t bytes_before;
    ft_size_t bytes_after;
    const char *long_text;

    long_text = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    joined = adv_strjoin_multiple(2, "warm", "up");
    if (joined != ft_nullptr)
        cma_free(joined);
    bytes_before = advanced_current_allocated_bytes();
    cma_set_alloc_limit(16);
    joined = adv_strjoin_multiple(1, long_text);
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(ft_nullptr, joined);
    bytes_after = advanced_current_allocated_bytes();
    FT_ASSERT_EQ(bytes_before, bytes_after);
    return (1);
}
