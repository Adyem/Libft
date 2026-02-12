#include "yaml.hpp"
#include "../System_utils/system_utils.hpp"
#include "../Errno/errno.hpp"

static int write_indent(ft_string &output, int indent) noexcept
{
    int indent_index = 0;
    while (indent_index < indent)
    {
        output.append(' ');
        if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
        {
            return (ft_string::last_operation_error());
        }
        indent_index++;
    }
    return (FT_ERR_SUCCESS);
}

static int write_node(const yaml_value *value, ft_string &output, int indent) noexcept
{
    if (!value)
    {
        return (FT_ERR_SUCCESS);
    }
    yaml_value::thread_guard guard(value);

    if (guard.get_status() != 0)
        return (guard.get_status());
    yaml_type value_type;

    value_type = value->get_type();
    {
        int yaml_value_error = ft_global_error_stack_drop_last_error();

        if (yaml_value_error != FT_ERR_SUCCESS)
            return (yaml_value_error);
    }
    if (value_type == YAML_SCALAR)
    {
        int indent_error;

        indent_error = write_indent(output, indent);
        if (indent_error != FT_ERR_SUCCESS)
            return (indent_error);
        const ft_string &scalar = value->get_scalar();
            {
                int yaml_value_error = ft_global_error_stack_drop_last_error();

                if (yaml_value_error != FT_ERR_SUCCESS)
                    return (yaml_value_error);
            }
        output.append(scalar);
        if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
        {
            return (ft_string::last_operation_error());
        }
        output.append('\n');
        if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
        {
            return (ft_string::last_operation_error());
        }
        return (FT_ERR_SUCCESS);
    }
    if (value_type == YAML_LIST)
    {
        const ft_vector<yaml_value*> &list_ref = value->get_list();
        {
            int yaml_value_error = ft_global_error_stack_drop_last_error();

            if (yaml_value_error != FT_ERR_SUCCESS)
                return (yaml_value_error);
        }
        size_t index = 0;
        size_t list_count = list_ref.size();
        if (list_ref.get_error() != FT_ERR_SUCCESS)
        {
            return (list_ref.get_error());
        }
        while (index < list_count)
        {
            const yaml_value *item = list_ref[index];
            if (list_ref.get_error() != FT_ERR_SUCCESS)
            {
                return (list_ref.get_error());
            }
            bool item_is_scalar = false;
            if (item)
            {
                yaml_type item_type = item->get_type();
                {
                    int yaml_value_error = ft_global_error_stack_drop_last_error();

                    if (yaml_value_error != FT_ERR_SUCCESS)
                        return (yaml_value_error);
                }
                if (item_type == YAML_SCALAR)
                    item_is_scalar = true;
            }
            if (item_is_scalar)
            {
                int indent_error;

                indent_error = write_indent(output, indent);
                if (indent_error != FT_ERR_SUCCESS)
                    return (indent_error);
                output.append("- ");
                if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
                {
                    return (ft_string::last_operation_error());
                }
                const ft_string &scalar = item->get_scalar();
                {
                    int yaml_value_error = ft_global_error_stack_drop_last_error();

                    if (yaml_value_error != FT_ERR_SUCCESS)
                        return (yaml_value_error);
                }
                output.append(scalar);
                if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
                {
                    return (ft_string::last_operation_error());
                }
                output.append('\n');
                if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
                {
                    return (ft_string::last_operation_error());
                }
            }
            else
            {
                int indent_error;

                indent_error = write_indent(output, indent);
                if (indent_error != FT_ERR_SUCCESS)
                    return (indent_error);
                output.append("-\n");
                if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
                {
                    return (ft_string::last_operation_error());
                }
                int node_error;

                node_error = write_node(item, output, indent + 2);
                if (node_error != FT_ERR_SUCCESS)
                    return (node_error);
            }
            index++;
        }
        return (FT_ERR_SUCCESS);
    }
    if (value_type == YAML_MAP)
    {
        const ft_map<ft_string, yaml_value*> &map_ref = value->get_map();
        {
            int yaml_value_error = ft_global_error_stack_drop_last_error();

            if (yaml_value_error != FT_ERR_SUCCESS)
                return (yaml_value_error);
        }
        const ft_vector<ft_string> &keys = value->get_map_keys();
        {
            int yaml_value_error = ft_global_error_stack_drop_last_error();

            if (yaml_value_error != FT_ERR_SUCCESS)
                return (yaml_value_error);
        }
        size_t key_index = 0;
        size_t key_count = keys.size();
        if (keys.get_error() != FT_ERR_SUCCESS)
        {
            return (keys.get_error());
        }
        while (key_index < key_count)
        {
            const ft_string &key = keys[key_index];
            const yaml_value *child = map_ref.at(key);
            {
                int map_error = map_ref.last_operation_error();

                if (map_error != FT_ERR_SUCCESS)
                    return (map_error);
            }
            bool child_is_scalar = false;
            if (child)
            {
                yaml_type child_type = child->get_type();
                {
                    int yaml_value_error = ft_global_error_stack_drop_last_error();

                    if (yaml_value_error != FT_ERR_SUCCESS)
                        return (yaml_value_error);
                }
                if (child_type == YAML_SCALAR)
                    child_is_scalar = true;
            }
            if (child_is_scalar)
            {
                int indent_error;

                indent_error = write_indent(output, indent);
                if (indent_error != FT_ERR_SUCCESS)
                    return (indent_error);
                output.append(key);
                if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
                {
                    return (ft_string::last_operation_error());
                }
                output.append(": ");
                if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
                {
                    return (ft_string::last_operation_error());
                }
                const ft_string &scalar = child->get_scalar();
                {
                    int yaml_value_error = ft_global_error_stack_drop_last_error();

                    if (yaml_value_error != FT_ERR_SUCCESS)
                        return (yaml_value_error);
                }
                output.append(scalar);
                if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
                {
                    return (ft_string::last_operation_error());
                }
                output.append('\n');
                if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
                {
                    return (ft_string::last_operation_error());
                }
            }
            else
            {
                int indent_error;

                indent_error = write_indent(output, indent);
                if (indent_error != FT_ERR_SUCCESS)
                    return (indent_error);
                output.append(key);
                if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
                {
                    return (ft_string::last_operation_error());
                }
                output.append(":\n");
                if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
                {
                    return (ft_string::last_operation_error());
                }
                int node_error;

                node_error = write_node(child, output, indent + 2);
                if (node_error != FT_ERR_SUCCESS)
                    return (node_error);
            }
            key_index++;
        }
        return (FT_ERR_SUCCESS);
    }
    return (FT_ERR_SUCCESS);
}

ft_string yaml_write_to_string(const yaml_value *value) noexcept
{
    ft_string output;
    int write_error;

    write_error = write_node(value, output, 0);
    if (write_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(write_error);
        return (ft_string(write_error));
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (output);
}

int yaml_write_to_file(const char *file_path, const yaml_value *value) noexcept
{
    int final_error;
    int result;

    final_error = FT_ERR_SUCCESS;
    result = 0;
    {
        ft_string output = yaml_write_to_string(value);
        int serialize_error;

        serialize_error = ft_global_error_stack_drop_last_error();
        if (serialize_error != FT_ERR_SUCCESS)
        {
            final_error = serialize_error;
            result = -1;
        }
        else
        {
            su_file *file;

            file = su_fopen(file_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            int open_error = ft_global_error_stack_drop_last_error();
            if (file == ft_nullptr)
            {
                if (open_error != FT_ERR_SUCCESS)
                    final_error = open_error;
                else
                    final_error = FT_ERR_INVALID_ARGUMENT;
                result = -1;
            }
            else
            {
                const char *data;
                size_t expected_size;
                size_t written;

                data = output.c_str();
                expected_size = output.size();
                written = su_fwrite(data, 1, expected_size, file);
                int write_error = ft_global_error_stack_drop_last_error();
                if (written != expected_size)
                {
                    int close_error;

                    close_error = su_fclose(file);
                    int close_status = ft_global_error_stack_drop_last_error();
                    if (close_error != 0 && write_error == FT_ERR_SUCCESS)
                        write_error = close_status;
                    if (write_error != FT_ERR_SUCCESS)
                        final_error = write_error;
                    else if (close_status != FT_ERR_SUCCESS)
                        final_error = close_status;
                    else
                        final_error = FT_ERR_INVALID_ARGUMENT;
                    result = -1;
                }
                else
                {
                    int close_result;

                    close_result = su_fclose(file);
                    int close_error = ft_global_error_stack_drop_last_error();
                    if (close_result != 0)
                    {
                        if (close_error != FT_ERR_SUCCESS)
                            final_error = close_error;
                        else
                            final_error = FT_ERR_INVALID_ARGUMENT;
                        result = -1;
                    }
                }
            }
        }
    }
    if (result == 0)
        final_error = FT_ERR_SUCCESS;
    ft_global_error_stack_push(final_error);
    return (result);
}

int yaml_write_to_backend(ft_document_sink &sink, const yaml_value *value) noexcept
{
    ft_string serialized;
    int write_result;
    int serialize_error;

    serialized = yaml_write_to_string(value);
    serialize_error = ft_global_error_stack_drop_last_error();
    if (serialize_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(serialize_error);
        return (-1);
    }
    write_result = sink.write_all(serialized.c_str(), serialized.size());
    if (write_result != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(write_result);
        return (-1);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (0);
}
