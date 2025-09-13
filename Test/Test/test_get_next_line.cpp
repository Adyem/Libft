#include "../../GetNextLine/get_next_line.hpp"
#include "../../CMA/CMA.hpp"
#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_istringstream.hpp"
#include <fstream>
#include <unistd.h>

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
