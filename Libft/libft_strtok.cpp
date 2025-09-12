#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"

char    *ft_strtok(char *string, const char *delimiters)
{
    static char *saved_string = ft_nullptr;
    char    *token_start;
    char    *current_pointer;
    int     is_delimiter;
    size_t  delimiter_index;

    if (string != ft_nullptr)
        saved_string = string;
    if (saved_string == ft_nullptr || delimiters == ft_nullptr)
        return (ft_nullptr);
    current_pointer = saved_string;
    while (*current_pointer != '\0')
    {
        is_delimiter = 0;
        delimiter_index = 0;
        while (delimiters[delimiter_index] != '\0')
        {
            if (*current_pointer == delimiters[delimiter_index])
            {
                is_delimiter = 1;
                break;
            }
            delimiter_index++;
        }
        if (is_delimiter == 0)
            break;
        current_pointer++;
    }
    if (*current_pointer == '\0')
    {
        saved_string = ft_nullptr;
        return (ft_nullptr);
    }
    token_start = current_pointer;
    while (*current_pointer != '\0')
    {
        is_delimiter = 0;
        delimiter_index = 0;
        while (delimiters[delimiter_index] != '\0')
        {
            if (*current_pointer == delimiters[delimiter_index])
            {
                is_delimiter = 1;
                break;
            }
            delimiter_index++;
        }
        if (is_delimiter)
            break;
        current_pointer++;
    }
    if (*current_pointer == '\0')
    {
        saved_string = ft_nullptr;
    }
    else
    {
        *current_pointer = '\0';
        saved_string = current_pointer + 1;
    }
    return (token_start);
}

