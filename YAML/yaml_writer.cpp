#include "yaml.hpp"
#include "../System_utils/system_utils.hpp"
#include "../Errno/errno.hpp"

static void write_indent(ft_string &output, int indent) noexcept
{
    int indent_index = 0;
    while (indent_index < indent)
    {
        output.append(' ');
        if (output.get_error() != FT_ER_SUCCESSS)
        {
            ft_errno = output.get_error();
            return ;
        }
        indent_index++;
    }
    ft_errno = FT_ER_SUCCESSS;
    return ;
}

static void write_node(const yaml_value *value, ft_string &output, int indent) noexcept
{
    if (!value)
    {
        ft_errno = FT_ER_SUCCESSS;
        return ;
    }
    yaml_value::thread_guard guard(value);

    if (guard.get_status() != 0)
        return ;
    if (value->get_error() != FT_ER_SUCCESSS)
    {
        ft_errno = value->get_error();
        return ;
    }
    yaml_type value_type;

    value_type = value->get_type();
    if (value->get_error() != FT_ER_SUCCESSS)
        return ;
    if (value_type == YAML_SCALAR)
    {
        write_indent(output, indent);
        if (output.get_error() != FT_ER_SUCCESSS)
        {
            ft_errno = output.get_error();
            return ;
        }
        output.append(value->get_scalar());
        if (output.get_error() != FT_ER_SUCCESSS)
        {
            ft_errno = output.get_error();
            return ;
        }
        output.append('\n');
        if (output.get_error() != FT_ER_SUCCESSS)
        {
            ft_errno = output.get_error();
            return ;
        }
        ft_errno = FT_ER_SUCCESSS;
        return ;
    }
    if (value_type == YAML_LIST)
    {
        const ft_vector<yaml_value*> &list_ref = value->get_list();
        size_t index = 0;
        size_t list_count = list_ref.size();
        if (list_ref.get_error() != FT_ER_SUCCESSS)
        {
            ft_errno = list_ref.get_error();
            return ;
        }
        while (index < list_count)
        {
            const yaml_value *item = list_ref[index];
            if (list_ref.get_error() != FT_ER_SUCCESSS)
            {
                ft_errno = list_ref.get_error();
                return ;
            }
            if (item && item->get_type() == YAML_SCALAR)
            {
                write_indent(output, indent);
                if (output.get_error() != FT_ER_SUCCESSS)
                {
                    ft_errno = output.get_error();
                    return ;
                }
                output.append("- ");
                if (output.get_error() != FT_ER_SUCCESSS)
                {
                    ft_errno = output.get_error();
                    return ;
                }
                output.append(item->get_scalar());
                if (output.get_error() != FT_ER_SUCCESSS)
                {
                    ft_errno = output.get_error();
                    return ;
                }
                output.append('\n');
                if (output.get_error() != FT_ER_SUCCESSS)
                {
                    ft_errno = output.get_error();
                    return ;
                }
            }
            else
            {
                write_indent(output, indent);
                if (output.get_error() != FT_ER_SUCCESSS)
                {
                    ft_errno = output.get_error();
                    return ;
                }
                output.append("-\n");
                if (output.get_error() != FT_ER_SUCCESSS)
                {
                    ft_errno = output.get_error();
                    return ;
                }
                write_node(item, output, indent + 2);
                if (ft_errno != FT_ER_SUCCESSS)
                    return ;
            }
            index++;
        }
        ft_errno = FT_ER_SUCCESSS;
        return ;
    }
    if (value_type == YAML_MAP)
    {
        const ft_map<ft_string, yaml_value*> &map_ref = value->get_map();
        const ft_vector<ft_string> &keys = value->get_map_keys();
        size_t key_index = 0;
        size_t key_count = keys.size();
        if (keys.get_error() != FT_ER_SUCCESSS)
        {
            ft_errno = keys.get_error();
            return ;
        }
        while (key_index < key_count)
        {
            const ft_string &key = keys[key_index];
            const yaml_value *child = map_ref.at(key);
            if (map_ref.get_error() != FT_ER_SUCCESSS)
            {
                ft_errno = map_ref.get_error();
                return ;
            }
            if (child && child->get_type() == YAML_SCALAR)
            {
                write_indent(output, indent);
                if (output.get_error() != FT_ER_SUCCESSS)
                {
                    ft_errno = output.get_error();
                    return ;
                }
                output.append(key);
                if (output.get_error() != FT_ER_SUCCESSS)
                {
                    ft_errno = output.get_error();
                    return ;
                }
                output.append(": ");
                if (output.get_error() != FT_ER_SUCCESSS)
                {
                    ft_errno = output.get_error();
                    return ;
                }
                output.append(child->get_scalar());
                if (output.get_error() != FT_ER_SUCCESSS)
                {
                    ft_errno = output.get_error();
                    return ;
                }
                output.append('\n');
                if (output.get_error() != FT_ER_SUCCESSS)
                {
                    ft_errno = output.get_error();
                    return ;
                }
            }
            else
            {
                write_indent(output, indent);
                if (output.get_error() != FT_ER_SUCCESSS)
                {
                    ft_errno = output.get_error();
                    return ;
                }
                output.append(key);
                if (output.get_error() != FT_ER_SUCCESSS)
                {
                    ft_errno = output.get_error();
                    return ;
                }
                output.append(":\n");
                if (output.get_error() != FT_ER_SUCCESSS)
                {
                    ft_errno = output.get_error();
                    return ;
                }
                write_node(child, output, indent + 2);
                if (ft_errno != FT_ER_SUCCESSS)
                    return ;
            }
            key_index++;
        }
        ft_errno = FT_ER_SUCCESSS;
        return ;
    }
    ft_errno = FT_ER_SUCCESSS;
    return ;
}

ft_string yaml_write_to_string(const yaml_value *value) noexcept
{
    ft_string output;
    write_node(value, output, 0);
    if (output.get_error() != FT_ER_SUCCESSS)
    {
        ft_errno = output.get_error();
        return (ft_string(output.get_error()));
    }
    ft_errno = FT_ER_SUCCESSS;
    return (output);
}

int yaml_write_to_file(const char *file_path, const yaml_value *value) noexcept
{
    int final_error;
    int result;

    final_error = FT_ER_SUCCESSS;
    result = 0;
    {
        ft_string output = yaml_write_to_string(value);
        if (output.get_error() != FT_ER_SUCCESSS)
        {
            final_error = output.get_error();
            result = -1;
        }
        else
        {
            su_file *file;

            file = su_fopen(file_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (file == ft_nullptr)
            {
                if (ft_errno != FT_ER_SUCCESSS)
                    final_error = ft_errno;
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
                if (written != expected_size)
                {
                    int write_error;
                    int close_error;

                    write_error = ft_errno;
                    close_error = su_fclose(file);
                    if (close_error != 0 && write_error == FT_ER_SUCCESSS)
                    {
                        if (ft_errno != FT_ER_SUCCESSS)
                            write_error = ft_errno;
                        else
                            write_error = FT_ERR_INVALID_ARGUMENT;
                    }
                    if (write_error != FT_ER_SUCCESSS)
                        final_error = write_error;
                    else if (ft_errno != FT_ER_SUCCESSS)
                        final_error = ft_errno;
                    else
                        final_error = FT_ERR_INVALID_ARGUMENT;
                    result = -1;
                }
                else if (su_fclose(file) != 0)
                {
                    if (ft_errno != FT_ER_SUCCESSS)
                        final_error = ft_errno;
                    else
                        final_error = FT_ERR_INVALID_ARGUMENT;
                    result = -1;
                }
            }
        }
    }
    if (result == 0)
        final_error = FT_ER_SUCCESSS;
    ft_errno = final_error;
    return (result);
}

int yaml_write_to_backend(ft_document_sink &sink, const yaml_value *value) noexcept
{
    ft_string serialized;
    int write_result;

    serialized = yaml_write_to_string(value);
    if (serialized.get_error() != FT_ER_SUCCESSS)
    {
        ft_errno = serialized.get_error();
        return (-1);
    }
    write_result = sink.write_all(serialized.c_str(), serialized.size());
    if (write_result != FT_ER_SUCCESSS)
    {
        if (ft_errno == FT_ER_SUCCESSS)
            ft_errno = write_result;
        return (-1);
    }
    ft_errno = FT_ER_SUCCESSS;
    return (0);
}
