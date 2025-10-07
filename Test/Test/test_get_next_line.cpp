#include "../../GetNextLine/get_next_line.hpp"
#include "../../CMA/CMA.hpp"
#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_istringstream.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"
#include <fstream>
#include <unistd.h>

class ft_failing_istream : public ft_istream
{
    private:
        bool _triggered;

    public:
        ft_failing_istream(void)
        : ft_istream(), _triggered(false)
        {
            return ;
        }

        ~ft_failing_istream(void)
        {
            return ;
        }

    protected:
        std::size_t do_read(char *buffer, std::size_t count)
        {
            (void)buffer;
            (void)count;
            if (!this->_triggered)
            {
                this->_triggered = true;
                this->set_error(FT_EIO);
                return (0);
            }
            return (0);
        }
};

int test_get_next_line_basic(void)
{
    ft_istringstream input("Hello\nWorld\n");
    char *line_one = get_next_line(input, 2);
    char *line_two = get_next_line(input, 2);
    char *line_three = get_next_line(input, 2);
    int ok = line_one && line_two && !line_three &&
             ft_strcmp(line_one, "Hello\n") == 0 &&
             ft_strcmp(line_two, "World\n") == 0;
    if (line_one)
        cma_free(line_one);
    if (line_two)
        cma_free(line_two);
    return (ok);
}

int test_get_next_line_empty(void)
{
    ft_istringstream input("");
    char *line = get_next_line(input, 4);
    if (line)
    {
        cma_free(line);
        return (0);
    }
    return (1);
}

int test_get_next_line_custom_buffer(void)
{
    ft_istringstream input("A long line without newline");
    char *line = get_next_line(input, 3);
    int ok = line && ft_strcmp(line, "A long line without newline") == 0;
    if (line)
        cma_free(line);
    return (ok);
}

int test_ft_open_and_read_file(void)
{
    const char *fname = "tmp_gnl_readlines.txt";
    std::ofstream out(fname);
    if (!out.is_open())
        return (0);
    out << "A\nB\nC\n";
    out.close();
    char **lines = ft_open_and_read_file(fname, 4);
    ::unlink(fname);
    if (!lines)
        return (0);
    int ok = lines[0] && lines[1] && lines[2] && !lines[3] &&
             ft_strcmp(lines[0], "A\n") == 0 &&
             ft_strcmp(lines[1], "B\n") == 0 &&
             ft_strcmp(lines[2], "C\n") == 0;
    cma_free_double(lines);
    return (ok);
}

FT_TEST(test_get_next_line_zero_buffer_sets_errno, "get_next_line reports FT_EINVAL when buffer size is zero")
{
    ft_istringstream input("Hello\n");
    char *line;

    ft_errno = ER_SUCCESS;
    line = get_next_line(input, 0);
    FT_ASSERT(line == ft_nullptr);
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_get_next_line_stream_error_sets_errno, "get_next_line propagates stream errors via ft_errno")
{
    ft_failing_istream input;
    char *line;

    ft_errno = ER_SUCCESS;
    line = get_next_line(input, 4);
    FT_ASSERT(line == ft_nullptr);
    FT_ASSERT_EQ(FT_EIO, ft_errno);
    return (1);
}

FT_TEST(test_get_next_line_allocator_failure_sets_errno, "get_next_line reports FT_EALLOC when allocations fail")
{
    ft_istringstream input("data\n");
    char *line;

    ft_errno = ER_SUCCESS;
    cma_set_alloc_limit(4);
    line = get_next_line(input, 4);
    cma_set_alloc_limit(0);
    if (line)
    {
        cma_free(line);
        return (0);
    }
    FT_ASSERT_EQ(FT_EALLOC, ft_errno);
    return (1);
}

FT_TEST(test_get_next_line_map_failure_sets_errno, "get_next_line surfaces hash map allocation failures")
{
    ft_istringstream stream_one("A\nB\n");
    ft_istringstream stream_two("C\nD\n");
    ft_istringstream stream_three("E\nF\n");
    ft_istringstream stream_four("G\nH\n");
    ft_istringstream stream_five("I\nJ\n");
    ft_istringstream stream_six("K\nL\n");
    char *line;

    ft_errno = ER_SUCCESS;
    cma_set_alloc_limit(0);
    line = get_next_line(stream_one, 2);
    if (!line)
    {
        cma_set_alloc_limit(0);
        return (0);
    }
    cma_free(line);
    line = get_next_line(stream_two, 2);
    if (!line)
    {
        cma_set_alloc_limit(0);
        return (0);
    }
    cma_free(line);
    line = get_next_line(stream_three, 2);
    if (!line)
    {
        cma_set_alloc_limit(0);
        return (0);
    }
    cma_free(line);
    line = get_next_line(stream_four, 2);
    if (!line)
    {
        cma_set_alloc_limit(0);
        return (0);
    }
    cma_free(line);
    line = get_next_line(stream_five, 2);
    if (!line)
    {
        cma_set_alloc_limit(0);
        return (0);
    }
    cma_free(line);
    cma_set_alloc_limit(64);
    line = get_next_line(stream_six, 2);
    cma_set_alloc_limit(0);
    FT_ASSERT(line == ft_nullptr);
    FT_ASSERT_EQ(UNORD_MAP_MEMORY, ft_errno);
    return (1);
}

static void    *gnl_fail_leftover_alloc(ft_size_t size)
{
    (void)size;
    ft_errno = FT_EALLOC;
    return (ft_nullptr);
}

FT_TEST(test_get_next_line_leftover_alloc_failure_frees_buffer, "get_next_line releases buffered data when leftover allocation fails")
{
    ft_istringstream input("one\ntwo\nthree\n");
    char *line;
    ft_size_t allocation_before;
    ft_size_t free_before;
    ft_size_t allocation_after;
    ft_size_t free_after;

    gnl_reset_leftover_alloc_hook();
    line = get_next_line(input, 32);
    FT_ASSERT(line != ft_nullptr);
    cma_free(line);
    cma_get_stats(&allocation_before, &free_before);
    gnl_set_leftover_alloc_hook(gnl_fail_leftover_alloc);
    line = get_next_line(input, 32);
    gnl_reset_leftover_alloc_hook();
    FT_ASSERT(line == ft_nullptr);
    FT_ASSERT_EQ(FT_EALLOC, ft_errno);
    cma_get_stats(&allocation_after, &free_after);
    FT_ASSERT_EQ(allocation_before + 1, allocation_after);
    FT_ASSERT_EQ(free_before + 2, free_after);
    return (1);
}
