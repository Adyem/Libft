#include "dumb_console.hpp"
#include <iostream>
#include <string>

void ft_print_char(char c)
{
    std::cout << c;
    return ;
}

void ft_print_string(const char *str)
{
    std::cout << str;
    return ;
}

void ft_print_line(const char *str)
{
    std::cout << str << std::endl;
    return ;
}

int ft_read_line(char *buffer, size_t buffer_size)
{
    std::string line;
    if (std::getline(std::cin, line))
    {
        if (line.length() < buffer_size)
        {
            line.copy(buffer, line.length());
            buffer[line.length()] = '\0';
            return (0);
        }
    }
    return (-1);
}
