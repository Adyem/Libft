#include "yaml.hpp"
#include "../Basic/basic.hpp"
#include "../Errno/errno.hpp"

ft_size_t yaml_find_char(const ft_string &string, char character) noexcept
{
    const char *data = string.c_str();
    ft_size_t index = 0;
    ft_size_t length = string.size();
    while (index < length)
    {
        if (data[index] == character)
            return (index);
        index++;
    }
    return (static_cast<ft_size_t>(-1));
}

ft_string yaml_substr(const ft_string &string, ft_size_t start, ft_size_t length) noexcept
{
    int32_t initialize_error;

    ft_string result;
    initialize_error = result.initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        return (ft_string());
    }
    const char *data = string.c_str();
    ft_size_t index = 0;
    while (index < length && start + index < string.size())
    {
        result.append(data[start + index]);
        if (result.get_error() != FT_ERR_SUCCESS)
        {
            return (ft_string());
        }
        index++;
    }
    return (result);
}

ft_string yaml_substr_from(const ft_string &string, ft_size_t start) noexcept
{
    if (start >= string.size())
        return (ft_string());
    ft_string part = yaml_substr(string, start, string.size() - start);
    return (part);
}

ft_size_t yaml_count_indent(const ft_string &line) noexcept
{
    const char *data = line.c_str();
    ft_size_t index = 0;
    ft_size_t length = line.size();
    while (index < length && data[index] == ' ')
        index++;
    return (index);
}

void yaml_trim(ft_string &string) noexcept
{
    const char *data = string.c_str();
    ft_size_t start_index = 0;
    ft_size_t string_length = string.size();
    while (start_index < string_length && ft_isspace(static_cast<unsigned char>(data[start_index])))
        start_index++;
    ft_size_t end_index = string_length;
    while (end_index > start_index && ft_isspace(static_cast<unsigned char>(data[end_index - 1])))
        end_index--;
    if (start_index == 0 && end_index == string_length)
        return ;
    ft_string trimmed = yaml_substr(string, start_index, end_index - start_index);
    if (trimmed.is_initialised() != FT_CLASS_STATE_INITIALISED)
    {
        string = ft_string();
        return ;
    }
    string = trimmed;
    return ;
}

int32_t yaml_split_lines(const ft_string &content, ft_vector<ft_string> &lines) noexcept
{
    int32_t lines_initialize_error;

    if (lines.is_initialised() != FT_CLASS_STATE_INITIALISED)
    {
        lines_initialize_error = lines.initialize();
        if (lines_initialize_error != FT_ERR_SUCCESS)
            return (lines_initialize_error);
    }
    const char *data = content.c_str();
    ft_size_t start_index = 0;
    ft_size_t content_length = content.size();
    while (start_index < content_length)
    {
        ft_size_t end_index = start_index;
        while (end_index < content_length && data[end_index] != '\n')
            end_index++;
        ft_string part = yaml_substr(content, start_index, end_index - start_index);
        if (part.is_initialised() != FT_CLASS_STATE_INITIALISED)
            return (FT_ERR_NO_MEMORY);
        lines.push_back(part);
        if (lines.get_error() != FT_ERR_SUCCESS)
            return (static_cast<int32_t>(lines.get_error()));
        start_index = end_index + 1;
    }
    return (FT_ERR_SUCCESS);
}
