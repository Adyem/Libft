#include "dumb_console.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_string.hpp"
#include "../GetNextLine/get_next_line.hpp"
#include <iostream>
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"

void ft_print_char(char character)
{
    std::cout << character;
    return ;
}

void ft_print_string(const char *string)
{
    std::cout << string;
    return ;
}

void ft_print_line(const char *string)
{
    std::cout << string << std::endl;
    return ;
}

int32_t ft_read_line(char *buffer, ft_size_t buffer_size)
{
    char *raw_line;
    ft_string line_string;
    int32_t initialize_error;
    int32_t erase_error;

    if (!buffer || buffer_size == 0)
        return (FT_ERR_INVALID_ARGUMENT);
    raw_line = get_next_line(0, buffer_size);
    if (!raw_line)
        return (FT_ERR_END_OF_FILE);
    initialize_error = line_string.initialize(raw_line);
    if (initialize_error != FT_ERR_SUCCESS)
    {
        cma_free(raw_line);
        return (initialize_error);
    }
    cma_free(raw_line);
    if (line_string.size() > 0 && line_string.back() == '\n')
    {
        erase_error = line_string.erase(line_string.size() - 1, 1);
        if (erase_error != FT_ERR_SUCCESS)
            return (erase_error);
    }
    if (line_string.size() >= buffer_size)
        return (FT_ERR_INVALID_ARGUMENT);
    ft_memcpy(buffer, line_string.c_str(), line_string.size());
    buffer[line_string.size()] = '\0';
    return (FT_ERR_SUCCESS);
}
