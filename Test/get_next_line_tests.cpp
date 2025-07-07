#include "../GetNextLine/get_next_line.hpp"
#include "../CPP_class/file.hpp"
#include "../CMA/CMA.hpp"
#include "../Libft/libft.hpp"
#include <fcntl.h>
#include <unistd.h>

int test_get_next_line_basic(void)
{
    const char *fname = "tmp_gnl_basic.txt";
    int fd = ::open(fname, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd < 0)
        return 0;
    ::write(fd, "Hello\nWorld\n", 12);
    ::close(fd);

    ft_file file(fname, O_RDONLY);
    if (file.get_fd() < 0)
    {
        ::unlink(fname);
        return 0;
    }
    char *line1 = get_next_line(file);
    char *line2 = get_next_line(file);
    char *line3 = get_next_line(file);
    file.close();
    ::unlink(fname);
    int ok = line1 && line2 && !line3 &&
             ft_strcmp(line1, "Hello\n") == 0 &&
             ft_strcmp(line2, "World\n") == 0;
    if (line1)
        cma_free(line1);
    if (line2)
        cma_free(line2);
    return ok;
}

int test_get_next_line_empty(void)
{
    const char *fname = "tmp_gnl_empty.txt";
    int fd = ::open(fname, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd < 0)
        return 0;
    ::close(fd);

    ft_file file(fname, O_RDONLY);
    if (file.get_fd() < 0)
    {
        ::unlink(fname);
        return 0;
    }
    char *line = get_next_line(file);
    file.close();
    ::unlink(fname);
    if (line)
    {
        cma_free(line);
        return 0;
    }
    return 1;
}

int test_ft_open_and_read_file(void)
{
    const char *fname = "tmp_gnl_readlines.txt";
    int fd = ::open(fname, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd < 0)
        return 0;
    ::write(fd, "A\nB\nC\n", 6);
    ::close(fd);

    char **lines = ft_open_and_read_file(fname);
    ::unlink(fname);
    if (!lines)
        return 0;
    int ok = lines[0] && lines[1] && lines[2] && !lines[3] &&
             ft_strcmp(lines[0], "A\n") == 0 &&
             ft_strcmp(lines[1], "B\n") == 0 &&
             ft_strcmp(lines[2], "C\n") == 0;
    cma_free_double(lines);
    return ok;
}
