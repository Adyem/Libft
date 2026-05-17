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

static void yaml_delete_string(ft_string *string) noexcept
{
    if (string == ft_nullptr)
        return ;
    (void)string->destroy();
    delete string;
    return ;
}

ft_string *yaml_substr(const ft_string &string, ft_size_t start, ft_size_t length) noexcept
{
    ft_string *output;
    int32_t initialize_error;

    output = new (std::nothrow) ft_string();
    if (output == ft_nullptr)
        return (ft_nullptr);
    initialize_error = output->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete output;
        return (ft_nullptr);
    }
    const char *data = string.c_str();
    ft_size_t index = 0;
    while (index < length && start + index < string.size())
    {
        output->append(data[start + index]);
        if (output->get_error() != FT_ERR_SUCCESS)
        {
            yaml_delete_string(output);
            return (ft_nullptr);
        }
        index++;
    }
    return (output);
}

ft_string *yaml_substr_from(const ft_string &string, ft_size_t start) noexcept
{
    if (start > string.size())
        return (ft_nullptr);
    return (yaml_substr(string, start, string.size() - start));
}

static int32_t yaml_assign_substr(const ft_string &string, ft_size_t start, ft_size_t length,
    ft_string &output) noexcept
{
    ft_string *substring;

    substring = yaml_substr(string, start, length);
    if (substring == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    output = *substring;
    if (output.get_error() != FT_ERR_SUCCESS)
    {
        int32_t output_error = output.get_error();

        yaml_delete_string(substring);
        return (output_error);
    }
    yaml_delete_string(substring);
    return (FT_ERR_SUCCESS);
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
    ft_string trimmed;
    int32_t substr_error;

    substr_error = yaml_assign_substr(string, start_index, end_index - start_index, trimmed);
    if (substr_error != FT_ERR_SUCCESS)
    {
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
        ft_string part;
        int32_t substr_error;

        substr_error = yaml_assign_substr(content, start_index, end_index - start_index, part);
        if (substr_error != FT_ERR_SUCCESS)
            return (substr_error);
        lines.push_back(part);
        if (lines.get_error() != FT_ERR_SUCCESS)
            return (lines.get_error());
        start_index = end_index + 1;
    }
    return (FT_ERR_SUCCESS);
}
