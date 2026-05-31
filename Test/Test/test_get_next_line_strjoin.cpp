#include "../test_internal.hpp"
#include "../../Modules/GetNextLine/get_next_line.hpp"
#include "../../Modules/CMA/CMA.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/Basic/basic.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

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

FT_TEST(test_ft_strjoin_gnl_concatenates_inputs)
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

FT_TEST(test_ft_strjoin_gnl_accepts_null_first)
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

FT_TEST(test_ft_strjoin_gnl_null_inputs_fail)
{
    char *joined;

    joined = ft_strjoin_gnl(ft_nullptr, ft_nullptr);
    FT_ASSERT_EQ(ft_nullptr, joined);
    return (1);
}

FT_TEST(test_ft_strjoin_gnl_second_null_copies_first)
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
