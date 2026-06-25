#include "basic.hpp"
#include "../Basic/class_nullptr.hpp"

char *ft_strtrim_left_in_place(char *string)
{
    ft_size_t source_index;
    ft_size_t destination_index;

    if (string == ft_nullptr)
        return (ft_nullptr);
    source_index = 0;
    while (string[source_index] != '\0' && ft_isspace(static_cast<int32_t>(static_cast<unsigned char>(string[source_index]))))
        ++source_index;
    if (source_index == 0)
        return (string);
    destination_index = 0;
    while (string[source_index] != '\0')
    {
        string[destination_index] = string[source_index];
        ++destination_index;
        ++source_index;
    }
    string[destination_index] = '\0';
    return (string);
}

char *ft_strtrim_right_in_place(char *string)
{
    ft_size_t string_length;

    if (string == ft_nullptr)
        return (ft_nullptr);
    string_length = ft_strlen_size_t(string);
    while (string_length > 0 && ft_isspace(static_cast<int32_t>(static_cast<unsigned char>(string[string_length - 1]))))
        --string_length;
    string[string_length] = '\0';
    return (string);
}

char *ft_strtrim_in_place(char *string)
{
    if (string == ft_nullptr)
        return (ft_nullptr);
    if (ft_strtrim_left_in_place(string) == ft_nullptr)
        return (ft_nullptr);
    return (ft_strtrim_right_in_place(string));
}
