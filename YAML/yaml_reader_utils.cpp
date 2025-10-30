#include "yaml.hpp"
#include "../Libft/libft.hpp"

size_t yaml_find_char(const ft_string &string, char character) noexcept
{
    if (string.get_error() != ER_SUCCESS)
    {
        ft_errno = string.get_error();
        return (static_cast<size_t>(-1));
    }
    const char *data = string.c_str();
    size_t index = 0;
    size_t length = string.size();
    while (index < length)
    {
        if (data[index] == character)
        {
            ft_errno = ER_SUCCESS;
            return (index);
        }
        index++;
    }
    ft_errno = ER_SUCCESS;
    return (static_cast<size_t>(-1));
}

ft_string yaml_substr(const ft_string &string, size_t start, size_t length) noexcept
{
    if (string.get_error() != ER_SUCCESS)
    {
        ft_string error_string(string.get_error());

        ft_errno = string.get_error();
        return (error_string);
    }
    ft_string result;
    const char *data = string.c_str();
    size_t index = 0;
    while (index < length && start + index < string.size())
    {
        result.append(data[start + index]);
        if (result.get_error() != ER_SUCCESS)
        {
            ft_string error_string(result.get_error());

            ft_errno = result.get_error();
            return (error_string);
        }
        index++;
    }
    ft_errno = ER_SUCCESS;
    return (result);
}

ft_string yaml_substr_from(const ft_string &string, size_t start) noexcept
{
    if (string.get_error() != ER_SUCCESS)
    {
        ft_string error_string(string.get_error());

        ft_errno = string.get_error();
        return (error_string);
    }
    if (start >= string.size())
    {
        ft_errno = ER_SUCCESS;
        return (ft_string());
    }
    ft_string part = yaml_substr(string, start, string.size() - start);
    if (part.get_error() != ER_SUCCESS)
    {
        ft_string error_string(part.get_error());

        ft_errno = part.get_error();
        return (error_string);
    }
    ft_errno = ER_SUCCESS;
    return (part);
}

size_t yaml_count_indent(const ft_string &line) noexcept
{
    if (line.get_error() != ER_SUCCESS)
    {
        ft_errno = line.get_error();
        return (0);
    }
    const char *data = line.c_str();
    size_t index = 0;
    size_t length = line.size();
    while (index < length && data[index] == ' ')
        index++;
    ft_errno = ER_SUCCESS;
    return (index);
}

void yaml_trim(ft_string &string) noexcept
{
    if (string.get_error() != ER_SUCCESS)
    {
        ft_errno = string.get_error();
        return ;
    }
    const char *data = string.c_str();
    size_t start_index = 0;
    size_t string_length = string.size();
    while (start_index < string_length && ft_isspace(static_cast<unsigned char>(data[start_index])))
        start_index++;
    size_t end_index = string_length;
    while (end_index > start_index && ft_isspace(static_cast<unsigned char>(data[end_index - 1])))
        end_index--;
    if (start_index == 0 && end_index == string_length)
    {
        ft_errno = ER_SUCCESS;
        return ;
    }
    ft_string trimmed = yaml_substr(string, start_index, end_index - start_index);
    if (trimmed.get_error() != ER_SUCCESS)
    {
        string = ft_string(trimmed.get_error());
        ft_errno = trimmed.get_error();
        return ;
    }
    string = trimmed;
    ft_errno = ER_SUCCESS;
    return ;
}

int yaml_split_lines(const ft_string &content, ft_vector<ft_string> &lines) noexcept
{
    if (content.get_error() != ER_SUCCESS)
    {
        ft_errno = content.get_error();
        return (content.get_error());
    }
    const char *data = content.c_str();
    size_t start_index = 0;
    size_t content_length = content.size();
    while (start_index < content_length)
    {
        size_t end_index = start_index;
        while (end_index < content_length && data[end_index] != '\n')
            end_index++;
        ft_string part = yaml_substr(content, start_index, end_index - start_index);
        if (part.get_error() != ER_SUCCESS)
        {
            ft_errno = part.get_error();
            return (part.get_error());
        }
        lines.push_back(part);
        if (lines.get_error() != ER_SUCCESS)
        {
            ft_errno = lines.get_error();
            return (lines.get_error());
        }
        start_index = end_index + 1;
    }
    ft_errno = ER_SUCCESS;
    return (ER_SUCCESS);
}

