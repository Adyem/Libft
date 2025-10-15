#include "../../GetNextLine/get_next_line.hpp"
#include "../../CMA/CMA.hpp"
#include "../../Libft/libft.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

static int  create_temp_fd_with_content(const char *content)
{
    char        template_path[] = "/tmp/gnl_testXXXXXX";
    int         file_descriptor;
    ft_size_t   total_written;
    ft_size_t   content_length;
    ssize_t     write_result;

    file_descriptor = mkstemp(template_path);
    if (file_descriptor < 0)
        return (-1);
    unlink(template_path);
    content_length = ft_strlen(content);
    total_written = 0;
    while (total_written < content_length)
    {
        write_result = write(file_descriptor, content + total_written,
                content_length - total_written);
        if (write_result <= 0)
        {
            close(file_descriptor);
            return (-1);
        }
        total_written += write_result;
    }
    if (lseek(file_descriptor, 0, SEEK_SET) < 0)
    {
        close(file_descriptor);
        return (-1);
    }
    return (file_descriptor);
}

static void close_temp_fd(int file_descriptor)
{
    gnl_clear_stream(file_descriptor);
    close(file_descriptor);
    return ;
}

int test_get_next_line_basic(void)
{
    int     file_descriptor;
    char    *line_one;
    char    *line_two;
    char    *line_three;
    int     success;

    file_descriptor = create_temp_fd_with_content("Hello\nWorld\n");
    if (file_descriptor < 0)
        return (0);
    line_one = get_next_line(file_descriptor, 2);
    line_two = get_next_line(file_descriptor, 2);
    line_three = get_next_line(file_descriptor, 2);
    success = line_one && line_two && !line_three
        && ft_strcmp(line_one, "Hello\n") == 0
        && ft_strcmp(line_two, "World\n") == 0;
    if (line_one)
        cma_free(line_one);
    if (line_two)
        cma_free(line_two);
    if (line_three)
        cma_free(line_three);
    close_temp_fd(file_descriptor);
    return (success);
}

int test_get_next_line_empty(void)
{
    int     file_descriptor;
    char    *line;

    file_descriptor = create_temp_fd_with_content("");
    if (file_descriptor < 0)
        return (0);
    line = get_next_line(file_descriptor, 4);
    if (line)
    {
        cma_free(line);
        close_temp_fd(file_descriptor);
        return (0);
    }
    close_temp_fd(file_descriptor);
    return (1);
}

int test_get_next_line_custom_buffer(void)
{
    int     file_descriptor;
    char    *line;
    int     success;

    file_descriptor = create_temp_fd_with_content("A long line without newline");
    if (file_descriptor < 0)
        return (0);
    line = get_next_line(file_descriptor, 3);
    success = line && ft_strcmp(line, "A long line without newline") == 0;
    if (line)
        cma_free(line);
    close_temp_fd(file_descriptor);
    return (success);
}

FT_TEST(test_get_next_line_zero_buffer_sets_errno, "get_next_line reports FT_ERR_INVALID_ARGUMENT when buffer size is zero")
{
    int     file_descriptor;
    char    *line;

    file_descriptor = create_temp_fd_with_content("Hello\n");
    FT_ASSERT(file_descriptor >= 0);
    ft_errno = ER_SUCCESS;
    line = get_next_line(file_descriptor, 0);
    FT_ASSERT(line == ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    close_temp_fd(file_descriptor);
    return (1);
}

FT_TEST(test_get_next_line_stream_error_sets_errno, "get_next_line propagates stream errors via ft_errno")
{
    int     file_descriptor;
    char    *line;

    file_descriptor = create_temp_fd_with_content("data\n");
    FT_ASSERT(file_descriptor >= 0);
    close(file_descriptor);
    ft_errno = ER_SUCCESS;
    line = get_next_line(file_descriptor, 4);
    FT_ASSERT(line == ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_IO, ft_errno);
    gnl_clear_stream(file_descriptor);
    return (1);
}

FT_TEST(test_get_next_line_allocator_failure_sets_errno, "get_next_line reports FT_ERR_NO_MEMORY when allocations fail")
{
    int     file_descriptor;
    char    *line;

    file_descriptor = create_temp_fd_with_content("data\n");
    FT_ASSERT(file_descriptor >= 0);
    ft_errno = ER_SUCCESS;
    cma_set_alloc_limit(4);
    line = get_next_line(file_descriptor, 4);
    cma_set_alloc_limit(0);
    FT_ASSERT(line == ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, ft_errno);
    close_temp_fd(file_descriptor);
    return (1);
}


