#include "libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

static char *report_strtok_result(int error_code, char *result)
{
    ft_global_error_stack_push(error_code);
    return (result);
}

char    *ft_strtok(char *string, const char *delimiters)
{
    static thread_local char *saved_string = ft_nullptr;
    static thread_local bool delimiter_lookup[256];
    static thread_local const char *cached_delimiters = ft_nullptr;
    static thread_local size_t cached_delimiters_length = 0;
    static thread_local size_t cached_delimiters_hash = 0;
    static thread_local bool delimiter_lookup_initialized = false;
    bool            delimiter_table_rebuild_required;
    char            *token_start;
    char            *current_pointer;
    size_t          delimiter_index;
    size_t          delimiter_table_index;
    unsigned char   current_character_value;
    size_t          new_delimiters_length;
    size_t          new_delimiters_hash;
    unsigned char   delimiter_character_value;
    int             error_code;

    if (string != ft_nullptr)
    {
        saved_string = string;
        delimiter_lookup_initialized = false;
        cached_delimiters = ft_nullptr;
        cached_delimiters_length = 0;
        cached_delimiters_hash = 0;
    }
    if (delimiters == ft_nullptr)
        return (report_strtok_result(FT_ERR_INVALID_ARGUMENT, ft_nullptr));
    if (saved_string == ft_nullptr)
    {
        error_code = FT_ERR_SUCCESSS;
        return (report_strtok_result(error_code, ft_nullptr));
    }
    new_delimiters_hash = 5381;
    delimiter_index = 0;
    while (delimiters[delimiter_index] != '\0')
    {
        delimiter_character_value = static_cast<unsigned char>(delimiters[delimiter_index]);
        new_delimiters_hash = ((new_delimiters_hash << 5) + new_delimiters_hash) + delimiter_character_value;
        delimiter_index++;
    }
    new_delimiters_length = delimiter_index;
    delimiter_table_rebuild_required = true;
    if (delimiter_lookup_initialized == true)
        if (cached_delimiters == delimiters)
            if (cached_delimiters_length == new_delimiters_length)
                if (cached_delimiters_hash == new_delimiters_hash)
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
        cached_delimiters_length = new_delimiters_length;
        cached_delimiters_hash = new_delimiters_hash;
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
        error_code = FT_ERR_SUCCESSS;
        return (report_strtok_result(error_code, ft_nullptr));
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
        saved_string = ft_nullptr;
    else
    {
        *current_pointer = '\0';
        saved_string = current_pointer + 1;
    }
    error_code = FT_ERR_SUCCESSS;
    return (report_strtok_result(error_code, token_start));
}
