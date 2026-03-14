#include "yaml.hpp"
#include "../System_utils/system_utils.hpp"
#include "../Errno/errno.hpp"

static int32_t write_indent(ft_string &output, int32_t indent) noexcept
{
    int32_t indent_index = 0;
    while (indent_index < indent)
    {
        output.append(' ');
        if (ft_string::get_error() != FT_ERR_SUCCESS)
        {
            return (ft_string::get_error());
        }
        indent_index++;
    }
    return (FT_ERR_SUCCESS);
}

static int32_t write_node(const yaml_value *value, ft_string &output, int32_t indent) noexcept
{
    if (!value)
    {
        return (FT_ERR_SUCCESS);
    }
    yaml_type value_type;

    value_type = value->get_type();
    if (value_type == YAML_SCALAR)
    {
        int32_t indent_error;

        indent_error = write_indent(output, indent);
        if (indent_error != FT_ERR_SUCCESS)
            return (indent_error);
        const ft_string &scalar = value->get_scalar();
        output.append(scalar);
        if (ft_string::get_error() != FT_ERR_SUCCESS)
        {
            return (ft_string::get_error());
        }
        output.append('\n');
        if (ft_string::get_error() != FT_ERR_SUCCESS)
        {
            return (ft_string::get_error());
        }
        return (FT_ERR_SUCCESS);
    }
    if (value_type == YAML_LIST)
    {
        const ft_vector<yaml_value*> &list_ref = value->get_list();
        ft_size_t index = 0;
        ft_size_t list_count = list_ref.size();
        while (index < list_count)
        {
            const yaml_value *item = list_ref[index];
            ft_bool item_is_scalar = FT_FALSE;
            if (item)
            {
                yaml_type item_type = item->get_type();
                if (item_type == YAML_SCALAR)
                    item_is_scalar = FT_TRUE;
            }
            if (item_is_scalar)
            {
                int32_t indent_error;

                indent_error = write_indent(output, indent);
                if (indent_error != FT_ERR_SUCCESS)
                    return (indent_error);
                output.append("- ");
                if (ft_string::get_error() != FT_ERR_SUCCESS)
                {
                    return (ft_string::get_error());
                }
                const ft_string &scalar = item->get_scalar();
                output.append(scalar);
                if (ft_string::get_error() != FT_ERR_SUCCESS)
                {
                    return (ft_string::get_error());
                }
                output.append('\n');
                if (ft_string::get_error() != FT_ERR_SUCCESS)
                {
                    return (ft_string::get_error());
                }
            }
            else
            {
                int32_t indent_error;

                indent_error = write_indent(output, indent);
                if (indent_error != FT_ERR_SUCCESS)
                    return (indent_error);
                output.append("-\n");
                if (ft_string::get_error() != FT_ERR_SUCCESS)
                {
                    return (ft_string::get_error());
                }
                int32_t node_error;

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
        ft_size_t key_index = 0;
        ft_size_t key_count = keys.size();
        while (key_index < key_count)
        {
            const ft_string &key = keys[key_index];
            const yaml_value *child = map_ref.at(key);
            ft_bool child_is_scalar = FT_FALSE;
            if (child)
            {
                yaml_type child_type = child->get_type();
                if (child_type == YAML_SCALAR)
                    child_is_scalar = FT_TRUE;
            }
            if (child_is_scalar)
            {
                int32_t indent_error;

                indent_error = write_indent(output, indent);
                if (indent_error != FT_ERR_SUCCESS)
                    return (indent_error);
                output.append(key);
                if (ft_string::get_error() != FT_ERR_SUCCESS)
                {
                    return (ft_string::get_error());
                }
                output.append(": ");
                if (ft_string::get_error() != FT_ERR_SUCCESS)
                {
                    return (ft_string::get_error());
                }
                const ft_string &scalar = child->get_scalar();
                output.append(scalar);
                if (ft_string::get_error() != FT_ERR_SUCCESS)
                {
                    return (ft_string::get_error());
                }
                output.append('\n');
                if (ft_string::get_error() != FT_ERR_SUCCESS)
                {
                    return (ft_string::get_error());
                }
            }
            else
            {
                int32_t indent_error;

                indent_error = write_indent(output, indent);
                if (indent_error != FT_ERR_SUCCESS)
                    return (indent_error);
                output.append(key);
                if (ft_string::get_error() != FT_ERR_SUCCESS)
                {
                    return (ft_string::get_error());
                }
                output.append(":\n");
                if (ft_string::get_error() != FT_ERR_SUCCESS)
                {
                    return (ft_string::get_error());
                }
                int32_t node_error;

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
    int32_t write_error;

    write_error = write_node(value, output, 0);
    if (write_error != FT_ERR_SUCCESS)
        return (ft_string());
    return (output);
}

int32_t yaml_write_to_file(const char *file_path, const yaml_value *value) noexcept
{
    ft_string output = yaml_write_to_string(value);
    su_file *file;
    const char *data;
    ft_size_t expected_size;
    ft_size_t written;

    if (ft_string::get_error() != FT_ERR_SUCCESS)
        return (FT_ERR_INVALID_STATE);
    file = su_fopen(file_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file == ft_nullptr)
        return (FT_ERR_IO);
    data = output.c_str();
    expected_size = output.size();
    written = su_fwrite(data, 1, expected_size, file);
    if (written != expected_size)
    {
        (void)su_fclose(file);
        return (FT_ERR_IO);
    }
    if (su_fclose(file) != 0)
        return (FT_ERR_IO);
    return (FT_ERR_SUCCESS);
}

int32_t yaml_write_to_backend(ft_document_sink &sink, const yaml_value *value) noexcept
{
    ft_string serialized;
    int32_t write_result;
    int32_t serialize_error;

    serialized = yaml_write_to_string(value);
    serialize_error = ft_string::get_error();
    if (serialize_error != FT_ERR_SUCCESS)
        return (serialize_error);
    write_result = sink.write_all(serialized.c_str(), serialized.size());
    if (write_result != FT_ERR_SUCCESS)
        return (write_result);
    return (FT_ERR_SUCCESS);
}
