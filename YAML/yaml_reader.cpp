#include "yaml.hpp"
#include "../System_utils/system_utils.hpp"
#include "../Basic/basic.hpp"
#include "../Errno/errno.hpp"
#include <new>

static int yaml_string_pop_error(const ft_string &string_value) noexcept
{
    unsigned long long operation_id;
    int               error_code;

    operation_id = string_value.last_operation_id();
    if (operation_id == 0)
        return (FT_ERR_SUCCESS);
    error_code = string_value.pop_operation_error(operation_id);
    if (error_code != FT_ERR_SUCCESS)
        ft_global_error_stack_push_entry_with_id(error_code, operation_id);
    return (error_code);
}

static yaml_value *parse_value(const ft_vector<ft_string> &lines, size_t &index, int indent,
    int *error_code_out) noexcept
{
    int error_code = FT_ERR_SUCCESS;
    yaml_value *result = ft_nullptr;
    yaml_value *list_value = ft_nullptr;
    yaml_value *map_value = ft_nullptr;
    size_t lines_count = lines.size();
    size_t line_indent = 0;
    int last_error;
    if (error_code_out != ft_nullptr)
        *error_code_out = FT_ERR_SUCCESS;
    if (lines.get_error() != FT_ERR_SUCCESS)
    {
        error_code = lines.get_error();
        goto error;
    }
    if (index >= lines_count)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        goto error;
    }
    line_indent = yaml_count_indent(lines[index]);
    last_error = ft_global_error_stack_drop_last_error();
    if (last_error != FT_ERR_SUCCESS)
    {
        error_code = last_error;
        goto error;
    }
    if (line_indent != static_cast<size_t>(indent))
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        goto error;
    }
    {
        ft_string line = yaml_substr_from(lines[index], line_indent);
        last_error = ft_global_error_stack_drop_last_error();
        if (last_error != FT_ERR_SUCCESS)
        {
            error_code = last_error;
            goto error;
        }
        {
            int string_error = yaml_string_pop_error(line);

            if (string_error != FT_ERR_SUCCESS)
            {
                error_code = string_error;
                ft_global_error_stack_push(string_error);
                goto error;
            }
        }
        yaml_trim(line);
        last_error = ft_global_error_stack_drop_last_error();
        if (last_error != FT_ERR_SUCCESS)
        {
            error_code = last_error;
            goto error;
        }
        {
            int string_error = yaml_string_pop_error(line);

            if (string_error != FT_ERR_SUCCESS)
            {
                error_code = string_error;
                goto error;
            }
        }
        const char *line_data = line.c_str();
        if (line.size() >= 1 && line_data[0] == '-')
        {
            list_value = new (std::nothrow) yaml_value();
            if (list_value == ft_nullptr)
            {
                error_code = FT_ERR_NO_MEMORY;
                goto error;
            }
            list_value->set_type(YAML_LIST);
            {
                int yaml_value_error = ft_global_error_stack_drop_last_error();

                if (yaml_value_error != FT_ERR_SUCCESS)
                {
                    error_code = yaml_value_error;
                    goto list_cleanup;
                }
            }
            while (index < lines_count)
            {
                size_t current_indent = yaml_count_indent(lines[index]);
                last_error = ft_global_error_stack_drop_last_error();
                if (last_error != FT_ERR_SUCCESS)
                {
                    error_code = last_error;
                    goto list_cleanup;
                }
                if (current_indent != static_cast<size_t>(indent))
                    break;
                ft_string item_line = yaml_substr_from(lines[index], current_indent);
                last_error = ft_global_error_stack_drop_last_error();
                if (last_error != FT_ERR_SUCCESS)
                {
                    error_code = last_error;
                    goto list_cleanup;
                }
                {
                    int string_error = yaml_string_pop_error(item_line);

                    if (string_error != FT_ERR_SUCCESS)
                    {
                        error_code = string_error;
                        goto list_cleanup;
                    }
                }
                yaml_trim(item_line);
                last_error = ft_global_error_stack_drop_last_error();
                if (last_error != FT_ERR_SUCCESS)
                {
                    error_code = last_error;
                    goto list_cleanup;
                }
                {
                    int string_error = yaml_string_pop_error(item_line);

                    if (string_error != FT_ERR_SUCCESS)
                    {
                        error_code = string_error;
                        goto list_cleanup;
                    }
                }
                const char *item_data = item_line.c_str();
                if (item_line.size() == 0 || item_data[0] != '-')
                    break;
                item_line = yaml_substr_from(item_line, 1);
                last_error = ft_global_error_stack_drop_last_error();
                if (last_error != FT_ERR_SUCCESS)
                {
                    error_code = last_error;
                    goto list_cleanup;
                }
                {
                    int string_error = yaml_string_pop_error(item_line);

                    if (string_error != FT_ERR_SUCCESS)
                    {
                        error_code = string_error;
                        goto list_cleanup;
                    }
                }
                yaml_trim(item_line);
                last_error = ft_global_error_stack_drop_last_error();
                if (last_error != FT_ERR_SUCCESS)
                {
                    error_code = last_error;
                    goto list_cleanup;
                }
                {
                    int string_error = yaml_string_pop_error(item_line);

                    if (string_error != FT_ERR_SUCCESS)
                    {
                        error_code = string_error;
                        goto list_cleanup;
                    }
                }
                if (item_line.size() == 0)
                {
                    index++;
                    yaml_value *child = parse_value(lines, index, indent + 2, &error_code);
                    if (child == ft_nullptr)
                    {
                        goto list_cleanup;
                    }
                    list_value->add_list_item(child);
                    {
                        int yaml_value_error = ft_global_error_stack_drop_last_error();

                        if (yaml_value_error != FT_ERR_SUCCESS)
                        {
                            error_code = yaml_value_error;
                            goto list_cleanup;
                        }
                    }
                }
                else
                {
                    size_t inline_colon = yaml_find_char(item_line, ':');
                    last_error = ft_global_error_stack_drop_last_error();
                    if (last_error != FT_ERR_SUCCESS)
                    {
                        error_code = last_error;
                        goto list_cleanup;
                    }
                    if (inline_colon != static_cast<size_t>(-1))
                    {
                        yaml_value *map_child = new (std::nothrow) yaml_value();
                        bool processing_current_line = true;
                        int map_indent = indent + 2;

                        if (map_child == ft_nullptr)
                        {
                            error_code = FT_ERR_NO_MEMORY;
                            goto list_cleanup;
                        }
                        map_child->set_type(YAML_MAP);
                        {
                            int yaml_value_error = ft_global_error_stack_drop_last_error();

                            if (yaml_value_error != FT_ERR_SUCCESS)
                            {
                                error_code = yaml_value_error;
                                delete map_child;
                                goto list_cleanup;
                            }
                        }
                        while (true)
                        {
                            ft_string map_line;

                            if (processing_current_line == true)
                                map_line = item_line;
                            else
                            {
                                size_t child_indent;

                                if (index >= lines_count)
                                    break;
                                child_indent = yaml_count_indent(lines[index]);
                                last_error = ft_global_error_stack_drop_last_error();
                                if (last_error != FT_ERR_SUCCESS)
                                {
                                    error_code = last_error;
                                    delete map_child;
                                    goto list_cleanup;
                                }
                                if (child_indent != static_cast<size_t>(map_indent))
                                    break;
                                map_line = yaml_substr_from(lines[index], child_indent);
                                last_error = ft_global_error_stack_drop_last_error();
                                if (last_error != FT_ERR_SUCCESS)
                                {
                                    error_code = last_error;
                                    delete map_child;
                                    goto list_cleanup;
                                }
                                {
                                    int string_error = yaml_string_pop_error(map_line);

                                    if (string_error != FT_ERR_SUCCESS)
                                    {
                                        error_code = string_error;
                                        delete map_child;
                                        goto list_cleanup;
                                    }
                                }
                            }
                            size_t map_colon = yaml_find_char(map_line, ':');
                            last_error = ft_global_error_stack_drop_last_error();
                            if (last_error != FT_ERR_SUCCESS)
                            {
                                error_code = last_error;
                                delete map_child;
                                goto list_cleanup;
                            }
                            if (map_colon == static_cast<size_t>(-1))
                            {
                                error_code = FT_ERR_INVALID_ARGUMENT;
                                delete map_child;
                                goto list_cleanup;
                            }
                            ft_string key = yaml_substr(map_line, 0, map_colon);
                            last_error = ft_global_error_stack_drop_last_error();
                            if (last_error != FT_ERR_SUCCESS)
                            {
                                error_code = last_error;
                                delete map_child;
                                goto list_cleanup;
                            }
                            {
                                int string_error = yaml_string_pop_error(key);

                                if (string_error != FT_ERR_SUCCESS)
                                {
                                    error_code = string_error;
                                    delete map_child;
                                    goto list_cleanup;
                                }
                            }
                            yaml_trim(key);
                            last_error = ft_global_error_stack_drop_last_error();
                            if (last_error != FT_ERR_SUCCESS)
                            {
                                error_code = last_error;
                                delete map_child;
                                goto list_cleanup;
                            }
                            {
                                int string_error = yaml_string_pop_error(key);

                                if (string_error != FT_ERR_SUCCESS)
                                {
                                    error_code = string_error;
                                    delete map_child;
                                    goto list_cleanup;
                                }
                            }
                            ft_string value_part = yaml_substr_from(map_line, map_colon + 1);
                            last_error = ft_global_error_stack_drop_last_error();
                            if (last_error != FT_ERR_SUCCESS)
                            {
                                error_code = last_error;
                                delete map_child;
                                goto list_cleanup;
                            }
                            {
                                int string_error = yaml_string_pop_error(value_part);

                                if (string_error != FT_ERR_SUCCESS)
                                {
                                    error_code = string_error;
                                    delete map_child;
                                    goto list_cleanup;
                                }
                            }
                            yaml_trim(value_part);
                            last_error = ft_global_error_stack_drop_last_error();
                            if (last_error != FT_ERR_SUCCESS)
                            {
                                error_code = last_error;
                                delete map_child;
                                goto list_cleanup;
                            }
                            {
                                int string_error = yaml_string_pop_error(value_part);

                                if (string_error != FT_ERR_SUCCESS)
                                {
                                    error_code = string_error;
                                    delete map_child;
                                    goto list_cleanup;
                                }
                            }
                            if (value_part.size() == 0)
                            {
                                yaml_value *nested_child;

                                index++;
                                nested_child = parse_value(lines, index, map_indent + 2, &error_code);
                                if (nested_child == ft_nullptr)
                                {
                                    delete map_child;
                                    goto list_cleanup;
                                }
                                map_child->add_map_item(key, nested_child);
                                {
                                    int yaml_value_error = ft_global_error_stack_drop_last_error();

                                    if (yaml_value_error != FT_ERR_SUCCESS)
                                    {
                                        error_code = yaml_value_error;
                                        delete map_child;
                                        goto list_cleanup;
                                    }
                                }
                            }
                            else
                            {
                                yaml_value *scalar_child = new (std::nothrow) yaml_value();

                                if (scalar_child == ft_nullptr)
                                {
                                    error_code = FT_ERR_NO_MEMORY;
                                    delete map_child;
                                    goto list_cleanup;
                                }
                                scalar_child->set_scalar(value_part);
                                {
                                    int yaml_value_error = ft_global_error_stack_drop_last_error();

                                    if (yaml_value_error != FT_ERR_SUCCESS)
                                    {
                                        error_code = yaml_value_error;
                                        delete scalar_child;
                                        delete map_child;
                                        goto list_cleanup;
                                    }
                                }
                                map_child->add_map_item(key, scalar_child);
                                {
                                    int yaml_value_error = ft_global_error_stack_drop_last_error();

                                    if (yaml_value_error != FT_ERR_SUCCESS)
                                    {
                                        error_code = yaml_value_error;
                                        delete map_child;
                                        goto list_cleanup;
                                    }
                                }
                                index++;
                            }
                            if (processing_current_line == true)
                                processing_current_line = false;
                            else if (index >= lines_count)
                                break;
                        }
                        list_value->add_list_item(map_child);
                        {
                            int yaml_value_error = ft_global_error_stack_drop_last_error();

                            if (yaml_value_error != FT_ERR_SUCCESS)
                            {
                                error_code = yaml_value_error;
                                delete map_child;
                                goto list_cleanup;
                            }
                        }
                        continue;
                    }
                    yaml_value *child = new (std::nothrow) yaml_value();
                    if (child == ft_nullptr)
                    {
                        error_code = FT_ERR_NO_MEMORY;
                        goto list_cleanup;
                    }
                    child->set_scalar(item_line);
                    {
                        int yaml_value_error = ft_global_error_stack_drop_last_error();

                        if (yaml_value_error != FT_ERR_SUCCESS)
                        {
                            error_code = yaml_value_error;
                            delete child;
                            goto list_cleanup;
                        }
                    }
                    list_value->add_list_item(child);
                    {
                        int yaml_value_error = ft_global_error_stack_drop_last_error();

                        if (yaml_value_error != FT_ERR_SUCCESS)
                        {
                            error_code = yaml_value_error;
                            goto list_cleanup;
                        }
                    }
                    index++;
                }
            }
            result = list_value;
            list_value = ft_nullptr;
            goto success;
        }
        size_t colon_position = yaml_find_char(line, ':');
        last_error = ft_global_error_stack_drop_last_error();
        if (last_error != FT_ERR_SUCCESS)
        {
            error_code = last_error;
            goto error;
        }
        if (colon_position != static_cast<size_t>(-1))
        {
            map_value = new (std::nothrow) yaml_value();
            if (map_value == ft_nullptr)
            {
                error_code = FT_ERR_NO_MEMORY;
                goto error;
            }
            map_value->set_type(YAML_MAP);
            while (index < lines_count)
            {
                size_t current_indent = yaml_count_indent(lines[index]);
                last_error = ft_global_error_stack_drop_last_error();
                if (last_error != FT_ERR_SUCCESS)
                {
                    error_code = last_error;
                    goto map_cleanup;
                }
                if (current_indent != static_cast<size_t>(indent))
                    break;
                ft_string pair_line = yaml_substr_from(lines[index], current_indent);
                last_error = ft_global_error_stack_drop_last_error();
                if (last_error != FT_ERR_SUCCESS)
                {
                    error_code = last_error;
                    goto map_cleanup;
                }
                {
                    int string_error = yaml_string_pop_error(pair_line);

                    if (string_error != FT_ERR_SUCCESS)
                    {
                        error_code = string_error;
                        goto map_cleanup;
                    }
                }
                size_t pair_colon = yaml_find_char(pair_line, ':');
                last_error = ft_global_error_stack_drop_last_error();
                if (last_error != FT_ERR_SUCCESS)
                {
                    error_code = last_error;
                    goto map_cleanup;
                }
                if (pair_colon == static_cast<size_t>(-1))
                    break;
                ft_string key = yaml_substr(pair_line, 0, pair_colon);
                last_error = ft_global_error_stack_drop_last_error();
                if (last_error != FT_ERR_SUCCESS)
                {
                    error_code = last_error;
                    goto map_cleanup;
                }
                {
                    int string_error = yaml_string_pop_error(key);

                    if (string_error != FT_ERR_SUCCESS)
                    {
                        error_code = string_error;
                        goto map_cleanup;
                    }
                }
                yaml_trim(key);
                last_error = ft_global_error_stack_drop_last_error();
                if (last_error != FT_ERR_SUCCESS)
                {
                    error_code = last_error;
                    goto map_cleanup;
                }
                {
                    int string_error = yaml_string_pop_error(key);

                    if (string_error != FT_ERR_SUCCESS)
                    {
                        error_code = string_error;
                        goto map_cleanup;
                    }
                }
                ft_string value_part = yaml_substr_from(pair_line, pair_colon + 1);
                last_error = ft_global_error_stack_drop_last_error();
                if (last_error != FT_ERR_SUCCESS)
                {
                    error_code = last_error;
                    goto map_cleanup;
                }
                {
                    int string_error = yaml_string_pop_error(value_part);

                    if (string_error != FT_ERR_SUCCESS)
                    {
                        error_code = string_error;
                        goto map_cleanup;
                    }
                }
                yaml_trim(value_part);
                last_error = ft_global_error_stack_drop_last_error();
                if (last_error != FT_ERR_SUCCESS)
                {
                    error_code = last_error;
                    goto map_cleanup;
                }
                {
                    int string_error = yaml_string_pop_error(value_part);

                    if (string_error != FT_ERR_SUCCESS)
                    {
                        error_code = string_error;
                        goto map_cleanup;
                    }
                }
                if (value_part.size() == 0)
                {
                    index++;
                    yaml_value *child = parse_value(lines, index, indent + 2, &error_code);
                    if (child == ft_nullptr)
                    {
                        goto map_cleanup;
                    }
                    map_value->add_map_item(key, child);
                    {
                        int yaml_value_error = ft_global_error_stack_drop_last_error();

                        if (yaml_value_error != FT_ERR_SUCCESS)
                        {
                            error_code = yaml_value_error;
                            delete child;
                            goto map_cleanup;
                        }
                    }
                }
                else
                {
                    yaml_value *child = new (std::nothrow) yaml_value();
                    if (child == ft_nullptr)
                    {
                        error_code = FT_ERR_NO_MEMORY;
                        goto map_cleanup;
                    }
                    child->set_scalar(value_part);
                    {
                        int yaml_value_error = ft_global_error_stack_drop_last_error();

                        if (yaml_value_error != FT_ERR_SUCCESS)
                        {
                            error_code = yaml_value_error;
                            delete child;
                            goto map_cleanup;
                        }
                    }
                    map_value->add_map_item(key, child);
                    {
                        int yaml_value_error = ft_global_error_stack_drop_last_error();

                        if (yaml_value_error != FT_ERR_SUCCESS)
                        {
                            error_code = yaml_value_error;
                            delete child;
                            goto map_cleanup;
                        }
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
        error_code = FT_ERR_NO_MEMORY;
        goto error;
    }
    scalar_value->set_scalar(line);
    {
        int yaml_value_error = ft_global_error_stack_drop_last_error();

        if (yaml_value_error != FT_ERR_SUCCESS)
        {
            error_code = yaml_value_error;
            delete scalar_value;
            scalar_value = ft_nullptr;
            goto error;
        }
    }
    index++;
    result = scalar_value;
    scalar_value = ft_nullptr;
    goto success;
}
success:
    if (error_code_out != ft_nullptr)
        *error_code_out = FT_ERR_SUCCESS;
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
    if (error_code == FT_ERR_SUCCESS)
        error_code = FT_ERR_INVALID_ARGUMENT;
    if (error_code_out != ft_nullptr)
        *error_code_out = error_code;
    return (ft_nullptr);
}

yaml_value *yaml_read_from_string(const ft_string &content) noexcept
{
    {
        int string_error = yaml_string_pop_error(content);

        if (string_error != FT_ERR_SUCCESS)
            return (ft_nullptr);
    }
    yaml_value *root = ft_nullptr;
    int parse_error = FT_ERR_SUCCESS;
    {
        ft_vector<ft_string> lines;
        int split_error;

        split_error = yaml_split_lines(content, lines);
        ft_global_error_stack_drop_last_error();
        if (split_error != FT_ERR_SUCCESS)
        {
            parse_error = split_error;
            root = ft_nullptr;
        }
        else
        {
            size_t local_index;

            local_index = 0;
            root = parse_value(lines, local_index, 0, &parse_error);
        }
    }
    if (root == ft_nullptr)
    {
        if (parse_error == FT_ERR_SUCCESS)
            parse_error = FT_ERR_INVALID_ARGUMENT;
        ft_global_error_stack_push(parse_error);
        return (ft_nullptr);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (root);
}

yaml_value *yaml_read_from_file(const char *file_path) noexcept
{
    su_file *file = su_fopen(file_path);
    int file_error = ft_global_error_stack_drop_last_error();
    if (file == ft_nullptr)
    {
        if (file_error == FT_ERR_SUCCESS)
            file_error = FT_ERR_FILE_OPEN_FAILED;
        ft_global_error_stack_push(file_error);
        return (ft_nullptr);
    }
    ft_string content;
    char buffer[1024];
    int read_error;
    size_t read_count;

    read_error = FT_ERR_SUCCESS;
    read_count = su_fread(buffer, 1, sizeof(buffer), file);
    read_error = ft_global_error_stack_drop_last_error();
    while (read_count > 0)
    {
        size_t buffer_index = 0;
        while (buffer_index < read_count)
        {
            content.append(buffer[buffer_index]);
            {
                int string_error = yaml_string_pop_error(content);

                if (string_error != FT_ERR_SUCCESS)
                {
                    su_fclose(file);
                    return (ft_nullptr);
                }
            }
            buffer_index++;
        }
        read_count = su_fread(buffer, 1, sizeof(buffer), file);
        read_error = ft_global_error_stack_drop_last_error();
    }
    su_fclose(file);
    ft_global_error_stack_drop_last_error();
    if (read_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(read_error);
        return (ft_nullptr);
    }
    yaml_value *result = yaml_read_from_string(content);
    int parse_error = ft_global_error_stack_drop_last_error();
    if (result == ft_nullptr)
    {
        if (parse_error == FT_ERR_SUCCESS)
            parse_error = FT_ERR_INVALID_ARGUMENT;
        ft_global_error_stack_push(parse_error);
        return (ft_nullptr);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (result);
}

yaml_value *yaml_read_from_backend(ft_document_source &source) noexcept
{
    ft_string content;
    int read_result;

    read_result = source.read_all(content);
    if (read_result != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(read_result);
        return (ft_nullptr);
    }
    return (yaml_read_from_string(content));
}

void yaml_free(yaml_value *value) noexcept
{
    delete value;
    return ;
}
