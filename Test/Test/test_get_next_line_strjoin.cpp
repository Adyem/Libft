#include "../test_internal.hpp"
#include "../../GetNextLine/get_next_line.hpp"
#include "../../CMA/CMA.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../Basic/basic.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

static char *duplicate_literal(const char *literal)
{
    char *duplicate;
    ft_size_t index;

    duplicate = static_cast<char *>(cma_malloc(ft_strlen_size_t(literal) + 1));
    if (!duplicate)
        return (ft_nullptr);
    index = 0;
    while (literal[index] != '\0')
    {
        duplicate[index] = literal[index];
        index++;
    }
    duplicate[index] = '\0';
    return (duplicate);
}

FT_TEST(test_ft_strjoin_gnl_concatenates_inputs, "ft_strjoin_gnl concatenates two buffers and frees the first")
{
    char *first;
    char *second;
    char *joined;

    first = duplicate_literal("Hello");
    second = duplicate_literal(" world");
    if (!first || !second)
    {
        cma_free(first);
        cma_free(second);
        return (0);
    }
    joined = ft_strjoin_gnl(first, second);
    cma_free(second);
    if (!joined)
        return (0);
    FT_ASSERT_EQ(0, ft_strcmp(joined, "Hello world"));
    cma_free(joined);
    return (1);
}

FT_TEST(test_ft_strjoin_gnl_accepts_null_first, "ft_strjoin_gnl handles null first buffer")
{
    char *second;
    char *joined;

    second = duplicate_literal("line");
    if (!second)
        return (0);
    joined = ft_strjoin_gnl(ft_nullptr, second);
    cma_free(second);
    if (!joined)
        return (0);
    FT_ASSERT_EQ(0, ft_strcmp(joined, "line"));
    cma_free(joined);
    return (1);
}

FT_TEST(test_ft_strjoin_gnl_null_inputs_fail, "ft_strjoin_gnl rejects when both buffers are null")
{
    char *joined;

    joined = ft_strjoin_gnl(ft_nullptr, ft_nullptr);
    FT_ASSERT_EQ(ft_nullptr, joined);
    return (1);
}

FT_TEST(test_ft_strjoin_gnl_second_null_copies_first, "ft_strjoin_gnl returns duplicate when second buffer is null")
{
    char *first;
    char *joined;

    first = duplicate_literal("partial");
    if (!first)
        return (0);
    joined = ft_strjoin_gnl(first, ft_nullptr);
    if (!joined)
        return (0);
    FT_ASSERT_EQ(0, ft_strcmp(joined, "partial"));
    cma_free(joined);
    return (1);
}
