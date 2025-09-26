#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"

char    *ft_strtok(char *string, const char *delimiters)
{
    static thread_local char *saved_string = ft_nullptr;
    static thread_local bool delimiter_lookup[256];
    static thread_local const char *cached_delimiters = ft_nullptr;
    static thread_local bool delimiter_lookup_initialized = false;
    bool            delimiter_table_rebuild_required;
    char            *token_start;
    char            *current_pointer;
    size_t          delimiter_index;
    size_t          delimiter_table_index;
    unsigned char   current_character_value;

    if (string != ft_nullptr)
        saved_string = string;
    if (saved_string == ft_nullptr || delimiters == ft_nullptr)
        return (ft_nullptr);
    delimiter_table_rebuild_required = true;
    if (delimiter_lookup_initialized == true)
        if (cached_delimiters == delimiters)
            delimiter_table_rebuild_required = false;
    if (delimiter_table_rebuild_required == true)
    {
        delimiter_table_index = 0;
        while (delimiter_table_index < 256)
        {
            delimiter_lookup[delimiter_table_index] = false;
            delimiter_table_index++;
        }
        delimiter_index = 0;
        while (delimiters[delimiter_index] != '\0')
        {
            current_character_value = static_cast<unsigned char>(delimiters[delimiter_index]);
            delimiter_lookup[current_character_value] = true;
            delimiter_index++;
        }
        cached_delimiters = delimiters;
        delimiter_lookup_initialized = true;
    }
    current_pointer = saved_string;
    while (*current_pointer != '\0')
    {
        current_character_value = static_cast<unsigned char>(*current_pointer);
        if (delimiter_lookup[current_character_value] == false)
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
        current_character_value = static_cast<unsigned char>(*current_pointer);
        if (delimiter_lookup[current_character_value] == true)
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

