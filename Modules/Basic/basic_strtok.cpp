#include "basic.hpp"
#include "../Basic/class_nullptr.hpp"

char    *ft_strtok(char *string, const char *delimiters)
{
    static thread_local char *saved_string = ft_nullptr;
    static thread_local ft_bool delimiter_lookup[256];
    static thread_local const char *cached_delimiters = ft_nullptr;
    static thread_local ft_size_t cached_delimiters_length = 0;
    static thread_local ft_size_t cached_delimiters_hash = 0;
    static thread_local ft_bool delimiter_lookup_initialised = FT_FALSE;
    ft_bool         delimiter_table_rebuild_required;
    char            *token_start;
    char            *current_pointer;
    ft_size_t       delimiter_index;
    ft_size_t       delimiter_table_index;
    unsigned char   current_character_value;
    ft_size_t       new_delimiters_length;
    ft_size_t       new_delimiters_hash;
    unsigned char   delimiter_character_value;

    if (string != ft_nullptr)
    {
        saved_string = string;
        delimiter_lookup_initialised = FT_FALSE;
        cached_delimiters = ft_nullptr;
        cached_delimiters_length = 0;
        cached_delimiters_hash = 0;
    }
    if (delimiters == ft_nullptr || saved_string == ft_nullptr)
        return (ft_nullptr);
    new_delimiters_hash = 5381;
    delimiter_index = 0;
    while (delimiters[delimiter_index] != '\0')
    {
        delimiter_character_value
            = static_cast<unsigned char>(delimiters[delimiter_index]);
        new_delimiters_hash = ((new_delimiters_hash << 5)
                + new_delimiters_hash) + delimiter_character_value;
        delimiter_index++;
    }
    new_delimiters_length = delimiter_index;
    delimiter_table_rebuild_required = FT_TRUE;
    if (delimiter_lookup_initialised == FT_TRUE)
        if (cached_delimiters == delimiters)
            if (cached_delimiters_length == new_delimiters_length)
                if (cached_delimiters_hash == new_delimiters_hash)
                    delimiter_table_rebuild_required = FT_FALSE;
    if (delimiter_table_rebuild_required == FT_TRUE)
    {
        delimiter_table_index = 0;
        while (delimiter_table_index < 256)
        {
            delimiter_lookup[delimiter_table_index] = FT_FALSE;
            delimiter_table_index++;
        }
        delimiter_index = 0;
        while (delimiters[delimiter_index] != '\0')
        {
            current_character_value
                = static_cast<unsigned char>(delimiters[delimiter_index]);
            delimiter_lookup[current_character_value] = FT_TRUE;
            delimiter_index++;
        }
        cached_delimiters = delimiters;
        cached_delimiters_length = new_delimiters_length;
        cached_delimiters_hash = new_delimiters_hash;
        delimiter_lookup_initialised = FT_TRUE;
    }
    current_pointer = saved_string;
    while (*current_pointer != '\0')
    {
        current_character_value = static_cast<unsigned char>(*current_pointer);
        if (delimiter_lookup[current_character_value] == FT_FALSE)
            break ;
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
        if (delimiter_lookup[current_character_value] == FT_TRUE)
            break ;
        current_pointer++;
    }
    if (*current_pointer == '\0')
        saved_string = ft_nullptr;
    else
    {
        *current_pointer = '\0';
        saved_string = current_pointer + 1;
    }
    return (token_start);
}
