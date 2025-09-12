#include "yaml.hpp"
#include "yaml_reader_utils.hpp"
#include "../System_utils/system_utils.hpp"
#include "../Libft/libft.hpp"
#include "../Errno/errno.hpp"

static yaml_value *parse_value(const ft_vector<ft_string> &lines, size_t &index, int indent) noexcept
{
    size_t lines_count = lines.size();
    if (lines.get_error() != ER_SUCCESS)
    {
        ft_errno = lines.get_error();
        return (ft_nullptr);
    }
    if (index >= lines_count)
        return (ft_nullptr);
    size_t line_indent = yaml_count_indent(lines[index]);
    if (line_indent != static_cast<size_t>(indent))
        return (ft_nullptr);
    ft_string line = yaml_substr_from(lines[index], line_indent);
    if (line.get_error() != ER_SUCCESS)
        return (ft_nullptr);
    yaml_trim(line);
    if (line.get_error() != ER_SUCCESS)
        return (ft_nullptr);
    const char *line_data = line.c_str();
    if (line.size() >= 1 && line_data[0] == '-')
    {
        yaml_value *list_value = new yaml_value();
        list_value->set_type(YAML_LIST);
        while (index < lines_count)
        {
            size_t current_indent = yaml_count_indent(lines[index]);
            if (current_indent != static_cast<size_t>(indent))
                break;
            ft_string item_line = yaml_substr_from(lines[index], current_indent);
            if (item_line.get_error() != ER_SUCCESS)
            {
                delete list_value;
                return (ft_nullptr);
            }
            yaml_trim(item_line);
            if (item_line.get_error() != ER_SUCCESS)
            {
                delete list_value;
                return (ft_nullptr);
            }
            const char *item_data = item_line.c_str();
            if (item_line.size() == 0 || item_data[0] != '-')
                break;
            item_line = yaml_substr_from(item_line, 1);
            if (item_line.get_error() != ER_SUCCESS)
            {
                delete list_value;
                return (ft_nullptr);
            }
            yaml_trim(item_line);
            if (item_line.get_error() != ER_SUCCESS)
            {
                delete list_value;
                return (ft_nullptr);
            }
            if (item_line.size() == 0)
            {
                index++;
                yaml_value *child = parse_value(lines, index, indent + 2);
                if (!child)
                    child = new yaml_value();
                list_value->add_list_item(child);
                if (list_value->get_error() != ER_SUCCESS)
                    break;
            }
            else
            {
                yaml_value *child = new yaml_value();
                child->set_scalar(item_line);
                list_value->add_list_item(child);
                if (list_value->get_error() != ER_SUCCESS || child->get_error() != ER_SUCCESS)
                    break;
                index++;
            }
        }
        return (list_value);
    }
    size_t colon_position = yaml_find_char(line, ':');
    if (colon_position != static_cast<size_t>(-1))
    {
        yaml_value *map_value = new yaml_value();
        map_value->set_type(YAML_MAP);
        while (index < lines_count)
        {
            size_t current_indent = yaml_count_indent(lines[index]);
            if (current_indent != static_cast<size_t>(indent))
                break;
            ft_string pair_line = yaml_substr_from(lines[index], current_indent);
            if (pair_line.get_error() != ER_SUCCESS)
            {
                delete map_value;
                return (ft_nullptr);
            }
            size_t pair_colon = yaml_find_char(pair_line, ':');
            if (pair_colon == static_cast<size_t>(-1))
                break;
            ft_string key = yaml_substr(pair_line, 0, pair_colon);
            if (key.get_error() != ER_SUCCESS)
            {
                delete map_value;
                return (ft_nullptr);
            }
            yaml_trim(key);
            if (key.get_error() != ER_SUCCESS)
            {
                delete map_value;
                return (ft_nullptr);
            }
            ft_string value_part = yaml_substr_from(pair_line, pair_colon + 1);
            if (value_part.get_error() != ER_SUCCESS)
            {
                delete map_value;
                return (ft_nullptr);
            }
            yaml_trim(value_part);
            if (value_part.get_error() != ER_SUCCESS)
            {
                delete map_value;
                return (ft_nullptr);
            }
            if (value_part.size() == 0)
            {
                index++;
                yaml_value *child = parse_value(lines, index, indent + 2);
                if (!child)
                    child = new yaml_value();
                map_value->add_map_item(key, child);
                if (map_value->get_error() != ER_SUCCESS)
                    break;
            }
            else
            {
                yaml_value *child = new yaml_value();
                child->set_scalar(value_part);
                map_value->add_map_item(key, child);
                if (map_value->get_error() != ER_SUCCESS || child->get_error() != ER_SUCCESS)
                    break;
                index++;
            }
        }
        return (map_value);
    }
    yaml_value *scalar_value = new yaml_value();
    scalar_value->set_scalar(line);
    if (scalar_value->get_error() != ER_SUCCESS)
        return (scalar_value);
    index++;
    return (scalar_value);
}

yaml_value *yaml_read_from_string(const ft_string &content) noexcept
{
    if (content.get_error() != ER_SUCCESS)
    {
        ft_errno = content.get_error();
        return (ft_nullptr);
    }
    ft_vector<ft_string> lines;
    yaml_split_lines(content, lines);
    if (lines.get_error() != ER_SUCCESS)
        return (ft_nullptr);
    size_t index = 0;
    yaml_value *root = parse_value(lines, index, 0);
    return (root);
}

yaml_value *yaml_read_from_file(const char *file_path) noexcept
{
    su_file *file = su_fopen(file_path);
    if (file == ft_nullptr)
        return (ft_nullptr);
    ft_string content;
    char buffer[1024];
    size_t read_count = su_fread(buffer, 1, sizeof(buffer), file);
    while (read_count > 0)
    {
        size_t buffer_index = 0;
        while (buffer_index < read_count)
        {
            content.append(buffer[buffer_index]);
            if (content.get_error() != ER_SUCCESS)
            {
                su_fclose(file);
                return (ft_nullptr);
            }
            buffer_index++;
        }
        read_count = su_fread(buffer, 1, sizeof(buffer), file);
    }
    su_fclose(file);
    return (yaml_read_from_string(content));
}

void yaml_free(yaml_value *value) noexcept
{
    delete value;
    return ;
}

