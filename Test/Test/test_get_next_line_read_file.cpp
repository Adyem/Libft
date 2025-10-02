#include "../../GetNextLine/get_next_line.hpp"
#include "../../CMA/CMA.hpp"
#include "../../CPP_class/class_istringstream.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../Libft/libft.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_ft_read_file_lines_basic, "ft_read_file_lines collects lines from stream")
{
    ft_istringstream input("First\nSecond\n");
    char **lines;

    ft_errno = FT_EINVAL;
    lines = ft_read_file_lines(input, 4);
    if (lines == ft_nullptr)
        return (0);
    if (lines[0] == ft_nullptr || lines[1] == ft_nullptr)
    {
        cma_free_double(lines);
        return (0);
    }
    if (lines[2] != ft_nullptr)
    {
        cma_free_double(lines);
        return (0);
    }
    if (ft_strcmp(lines[0], "First\n") != 0)
    {
        cma_free_double(lines);
        return (0);
    }
    if (ft_strcmp(lines[1], "Second\n") != 0)
    {
        cma_free_double(lines);
        return (0);
    }
    if (ft_errno != ER_SUCCESS)
    {
        cma_free_double(lines);
        return (0);
    }
    cma_free_double(lines);
    return (1);
}

FT_TEST(test_ft_read_file_lines_empty_stream_returns_null, "ft_read_file_lines returns nullptr for empty input")
{
    ft_istringstream input("");
    char **lines;

    ft_errno = FT_EINVAL;
    lines = ft_read_file_lines(input, 8);
    FT_ASSERT_EQ(ft_nullptr, lines);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_ft_read_file_lines_allocation_failure_sets_errno, "ft_read_file_lines reports allocation failures")
{
    ft_istringstream input("alpha\n");
    char **lines;

    cma_set_alloc_limit(1);
    ft_errno = ER_SUCCESS;
    lines = ft_read_file_lines(input, 4);
    cma_set_alloc_limit(0);
    FT_ASSERT_EQ(ft_nullptr, lines);
    FT_ASSERT_EQ(FT_EALLOC, ft_errno);
    return (1);
}
