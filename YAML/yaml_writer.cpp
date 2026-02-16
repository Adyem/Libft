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
    yaml_type value_type;

    value_type = value->get_type();
    if (value_type == YAML_SCALAR)
    {
        int indent_error;

        indent_error = write_indent(output, indent);
        if (indent_error != FT_ERR_SUCCESS)
            return (indent_error);
        const ft_string &scalar = value->get_scalar();
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
        size_t index = 0;
        size_t list_count = list_ref.size();
        while (index < list_count)
        {
            const yaml_value *item = list_ref[index];
            bool item_is_scalar = false;
            if (item)
            {
                yaml_type item_type = item->get_type();
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
        const ft_vector<ft_string> &keys = value->get_map_keys();
        size_t key_index = 0;
        size_t key_count = keys.size();
        while (key_index < key_count)
        {
            const ft_string &key = keys[key_index];
            const yaml_value *child = map_ref.at(key);
            bool child_is_scalar = false;
            if (child)
            {
                yaml_type child_type = child->get_type();
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
        return (ft_string(write_error));
    return (output);
}

int yaml_write_to_file(const char *file_path, const yaml_value *value) noexcept
{
    ft_string output = yaml_write_to_string(value);
    su_file *file;
    const char *data;
    size_t expected_size;
    size_t written;

    if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
        return (-1);
    file = su_fopen(file_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file == ft_nullptr)
        return (-1);
    data = output.c_str();
    expected_size = output.size();
    written = su_fwrite(data, 1, expected_size, file);
    if (written != expected_size)
    {
        (void)su_fclose(file);
        return (-1);
    }
    if (su_fclose(file) != 0)
        return (-1);
    return (0);
}

int yaml_write_to_backend(ft_document_sink &sink, const yaml_value *value) noexcept
{
    ft_string serialized;
    int write_result;
    int serialize_error;

    serialized = yaml_write_to_string(value);
    serialize_error = ft_string::last_operation_error();
    if (serialize_error != FT_ERR_SUCCESS)
        return (-1);
    write_result = sink.write_all(serialized.c_str(), serialized.size());
    if (write_result != FT_ERR_SUCCESS)
        return (-1);
    return (0);
}
