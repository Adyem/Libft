#include "yaml.hpp"
#include "../System_utils/system_utils.hpp"
#include "../Libft/libft.hpp"
#include "../Errno/errno.hpp"
#include <new>

static yaml_value *parse_value(const ft_vector<ft_string> &lines, size_t &index, int indent) noexcept
{
    int error_code = ER_SUCCESS;
    yaml_value *result = ft_nullptr;
    yaml_value *list_value = ft_nullptr;
    yaml_value *map_value = ft_nullptr;
    size_t lines_count = lines.size();
    size_t line_indent = 0;
    if (lines.get_error() != ER_SUCCESS)
    {
        error_code = lines.get_error();
        goto error;
    }
    if (index >= lines_count)
    {
        error_code = FT_EINVAL;
        goto error;
    }
    line_indent = yaml_count_indent(lines[index]);
    if (line_indent != static_cast<size_t>(indent))
    {
        error_code = FT_EINVAL;
        goto error;
    }
    {
        ft_string line = yaml_substr_from(lines[index], line_indent);
        if (line.get_error() != ER_SUCCESS)
        {
            error_code = line.get_error();
            goto error;
        }
        yaml_trim(line);
        if (line.get_error() != ER_SUCCESS)
        {
            error_code = line.get_error();
            goto error;
        }
        const char *line_data = line.c_str();
        if (line.size() >= 1 && line_data[0] == '-')
        {
            list_value = new (std::nothrow) yaml_value();
            if (list_value == ft_nullptr)
            {
                error_code = FT_EALLOC;
                goto error;
            }
            list_value->set_type(YAML_LIST);
            while (index < lines_count)
            {
                size_t current_indent = yaml_count_indent(lines[index]);
                if (current_indent != static_cast<size_t>(indent))
                    break;
                ft_string item_line = yaml_substr_from(lines[index], current_indent);
                if (item_line.get_error() != ER_SUCCESS)
                {
                    error_code = item_line.get_error();
                    goto list_cleanup;
                }
                yaml_trim(item_line);
                if (item_line.get_error() != ER_SUCCESS)
                {
                    error_code = item_line.get_error();
                    goto list_cleanup;
                }
                const char *item_data = item_line.c_str();
                if (item_line.size() == 0 || item_data[0] != '-')
                    break;
                item_line = yaml_substr_from(item_line, 1);
                if (item_line.get_error() != ER_SUCCESS)
                {
                    error_code = item_line.get_error();
                    goto list_cleanup;
                }
                yaml_trim(item_line);
                if (item_line.get_error() != ER_SUCCESS)
                {
                    error_code = item_line.get_error();
                    goto list_cleanup;
                }
                if (item_line.size() == 0)
                {
                    index++;
                    yaml_value *child = parse_value(lines, index, indent + 2);
                    if (child == ft_nullptr)
                    {
                        error_code = ft_errno;
                        goto list_cleanup;
                    }
                    list_value->add_list_item(child);
                    if (list_value->get_error() != ER_SUCCESS)
                    {
                        error_code = list_value->get_error();
                        goto list_cleanup;
                    }
                }
                else
                {
                    yaml_value *child = new (std::nothrow) yaml_value();
                    if (child == ft_nullptr)
                    {
                        error_code = FT_EALLOC;
                        goto list_cleanup;
                    }
                    child->set_scalar(item_line);
                    if (child->get_error() != ER_SUCCESS)
                    {
                        error_code = child->get_error();
                        delete child;
                        goto list_cleanup;
                    }
                    list_value->add_list_item(child);
                    if (list_value->get_error() != ER_SUCCESS)
                    {
                        error_code = list_value->get_error();
                        goto list_cleanup;
                    }
                    index++;
                }
            }
            result = list_value;
            list_value = ft_nullptr;
            goto success;
        }
        size_t colon_position = yaml_find_char(line, ':');
        if (colon_position != static_cast<size_t>(-1))
        {
            map_value = new (std::nothrow) yaml_value();
            if (map_value == ft_nullptr)
            {
                error_code = FT_EALLOC;
                goto error;
            }
            map_value->set_type(YAML_MAP);
            while (index < lines_count)
            {
                size_t current_indent = yaml_count_indent(lines[index]);
                if (current_indent != static_cast<size_t>(indent))
                    break;
                ft_string pair_line = yaml_substr_from(lines[index], current_indent);
                if (pair_line.get_error() != ER_SUCCESS)
                {
                    error_code = pair_line.get_error();
                    goto map_cleanup;
                }
                size_t pair_colon = yaml_find_char(pair_line, ':');
                if (pair_colon == static_cast<size_t>(-1))
                    break;
                ft_string key = yaml_substr(pair_line, 0, pair_colon);
                if (key.get_error() != ER_SUCCESS)
                {
                    error_code = key.get_error();
                    goto map_cleanup;
                }
                yaml_trim(key);
                if (key.get_error() != ER_SUCCESS)
                {
                    error_code = key.get_error();
                    goto map_cleanup;
                }
                ft_string value_part = yaml_substr_from(pair_line, pair_colon + 1);
                if (value_part.get_error() != ER_SUCCESS)
                {
                    error_code = value_part.get_error();
                    goto map_cleanup;
                }
                yaml_trim(value_part);
                if (value_part.get_error() != ER_SUCCESS)
                {
                    error_code = value_part.get_error();
                    goto map_cleanup;
                }
                if (value_part.size() == 0)
                {
                    index++;
                    yaml_value *child = parse_value(lines, index, indent + 2);
                    if (child == ft_nullptr)
                    {
                        error_code = ft_errno;
                        goto map_cleanup;
                    }
                    map_value->add_map_item(key, child);
                    if (map_value->get_error() != ER_SUCCESS)
                    {
                        error_code = map_value->get_error();
                        goto map_cleanup;
                    }
                }
                else
                {
                    yaml_value *child = new (std::nothrow) yaml_value();
                    if (child == ft_nullptr)
                    {
                        error_code = FT_EALLOC;
                        goto map_cleanup;
                    }
                    child->set_scalar(value_part);
                    if (child->get_error() != ER_SUCCESS)
                    {
                        error_code = child->get_error();
                        delete child;
                        goto map_cleanup;
                    }
                    map_value->add_map_item(key, child);
                    if (map_value->get_error() != ER_SUCCESS)
                    {
                        error_code = map_value->get_error();
                        goto map_cleanup;
                    }
                    index++;
                }
            }
            result = map_value;
            map_value = ft_nullptr;
            goto success;
        }
    yaml_value *scalar_value = new (std::nothrow) yaml_value();
    if (scalar_value == ft_nullptr)
    {
        error_code = FT_EALLOC;
        goto error;
    }
    scalar_value->set_scalar(line);
    if (scalar_value->get_error() != ER_SUCCESS)
    {
        error_code = scalar_value->get_error();
        delete scalar_value;
        scalar_value = ft_nullptr;
        goto error;
    }
    index++;
    result = scalar_value;
    scalar_value = ft_nullptr;
    goto success;
}
success:
    ft_errno = ER_SUCCESS;
    return (result);
list_cleanup:
    if (list_value != ft_nullptr)
    {
        delete list_value;
        list_value = ft_nullptr;
    }
    goto error;
map_cleanup:
    if (map_value != ft_nullptr)
    {
        delete map_value;
        map_value = ft_nullptr;
    }
    goto error;
error:
    if (error_code == ER_SUCCESS)
        error_code = FT_EINVAL;
    ft_errno = error_code;
    return (ft_nullptr);
}

yaml_value *yaml_read_from_string(const ft_string &content) noexcept
{
    if (content.get_error() != ER_SUCCESS)
    {
        ft_errno = content.get_error();
        return (ft_nullptr);
    }
    yaml_value *root = ft_nullptr;
    int parse_error = ER_SUCCESS;
    {
        ft_vector<ft_string> lines;
        int split_error = yaml_split_lines(content, lines);
        if (split_error != ER_SUCCESS)
            return (ft_nullptr);
        size_t local_index = 0;
        root = parse_value(lines, local_index, 0);
        parse_error = ft_errno;
    }
    if (root == ft_nullptr)
    {
        if (parse_error == ER_SUCCESS)
            parse_error = FT_EINVAL;
        ft_errno = parse_error;
        return (ft_nullptr);
    }
    ft_errno = ER_SUCCESS;
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
                int append_error = content.get_error();
                su_fclose(file);
                ft_errno = append_error;
                return (ft_nullptr);
            }
            buffer_index++;
        }
        read_count = su_fread(buffer, 1, sizeof(buffer), file);
    }
    su_fclose(file);
    yaml_value *result = yaml_read_from_string(content);
    int parse_error = ft_errno;
    if (result == ft_nullptr)
    {
        if (parse_error == ER_SUCCESS)
            parse_error = FT_EINVAL;
        ft_errno = parse_error;
        return (ft_nullptr);
    }
    ft_errno = ER_SUCCESS;
    return (result);
}

void yaml_free(yaml_value *value) noexcept
{
    delete value;
    return ;
}

