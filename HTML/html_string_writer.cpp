#include "parser.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"

static void html_append_literal(char *destination, size_t *output_index, const char *literal)
{
    size_t literal_index;

    literal_index = 0;
    while (literal[literal_index])
    {
        destination[*output_index] = literal[literal_index];
        *output_index += 1;
        literal_index += 1;
    }
    return ;
}

static char *html_escape_attribute_value(const char *value, int *error_code)
{
    size_t  input_length;
    size_t  input_index;
    size_t  output_length;
    char    *escaped_value;
    size_t  output_index;
    int     local_error;

    if (!value)
    {
        if (error_code != ft_nullptr)
            *error_code = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    input_length = ft_strlen(value);
    output_length = input_length;
    input_index = 0;
    while (input_index < input_length)
    {
        char current_character;

        current_character = value[input_index];
        if (current_character == '&')
            output_length += 4;
        else if (current_character == '"')
            output_length += 5;
        else if (current_character == static_cast<char>(39))
            output_length += 4;
        else if (current_character == '<' || current_character == '>')
            output_length += 3;
        input_index += 1;
    }
    escaped_value = static_cast<char *>(cma_malloc(output_length + 1));
    local_error = ft_global_error_stack_drop_last_error();
    if (!escaped_value)
    {
        if (local_error == FT_ERR_SUCCESSS)
            local_error = FT_ERR_NO_MEMORY;
        if (error_code != ft_nullptr)
            *error_code = local_error;
        return (ft_nullptr);
    }
    input_index = 0;
    output_index = 0;
    while (input_index < input_length)
    {
        char current_character;

        current_character = value[input_index];
        if (current_character == '&')
            html_append_literal(escaped_value, &output_index, "&amp;");
        else if (current_character == '"')
            html_append_literal(escaped_value, &output_index, "&quot;");
        else if (current_character == static_cast<char>(39))
            html_append_literal(escaped_value, &output_index, "&#39;");
        else if (current_character == '<')
            html_append_literal(escaped_value, &output_index, "&lt;");
        else if (current_character == '>')
            html_append_literal(escaped_value, &output_index, "&gt;");
        else
        {
            escaped_value[output_index] = current_character;
            output_index += 1;
        }
        input_index += 1;
    }
    escaped_value[output_index] = '\0';
    if (error_code != ft_nullptr)
        *error_code = FT_ERR_SUCCESSS;
    return (escaped_value);
}

static char *html_attrs_to_string(html_attr *attribute, int *error_code)
{
    char *result;
    int  local_error;

    result = cma_strdup("");
    local_error = ft_global_error_stack_drop_last_error();
    if (!result)
    {
        if (local_error == FT_ERR_SUCCESSS)
            local_error = FT_ERR_NO_MEMORY;
        if (error_code != ft_nullptr)
            *error_code = local_error;
        return (ft_nullptr);
    }
    while (attribute)
    {
        bool       lock_acquired;
        int        lock_status;
        char      *escaped_value;
        char      *attr;
        char      *key_copy;
        char      *value_copy;
        html_attr *next_attribute;

        lock_acquired = false;
        lock_status = html_attr_lock(attribute, &lock_acquired);
        local_error = ft_global_error_stack_drop_last_error();
        if (lock_status != 0)
        {
            cma_free(result);
            ft_global_error_stack_drop_last_error();
            if (error_code != ft_nullptr)
                *error_code = local_error;
            return (ft_nullptr);
        }
        if (!attribute->key || !attribute->value)
        {
            html_attr_unlock(attribute, lock_acquired);
            ft_global_error_stack_drop_last_error();
            cma_free(result);
            ft_global_error_stack_drop_last_error();
            if (error_code != ft_nullptr)
                *error_code = FT_ERR_INVALID_ARGUMENT;
            return (ft_nullptr);
        }
        key_copy = cma_strdup(attribute->key);
        local_error = ft_global_error_stack_drop_last_error();
        if (!key_copy)
        {
            html_attr_unlock(attribute, lock_acquired);
            ft_global_error_stack_drop_last_error();
            cma_free(result);
            ft_global_error_stack_drop_last_error();
            if (local_error == FT_ERR_SUCCESSS)
                local_error = FT_ERR_NO_MEMORY;
            if (error_code != ft_nullptr)
                *error_code = local_error;
            return (ft_nullptr);
        }
        value_copy = cma_strdup(attribute->value);
        local_error = ft_global_error_stack_drop_last_error();
        if (!value_copy)
        {
            cma_free(key_copy);
            ft_global_error_stack_drop_last_error();
            html_attr_unlock(attribute, lock_acquired);
            ft_global_error_stack_drop_last_error();
            cma_free(result);
            ft_global_error_stack_drop_last_error();
            if (local_error == FT_ERR_SUCCESSS)
                local_error = FT_ERR_NO_MEMORY;
            if (error_code != ft_nullptr)
                *error_code = local_error;
            return (ft_nullptr);
        }
        next_attribute = attribute->next;
        html_attr_unlock(attribute, lock_acquired);
        ft_global_error_stack_drop_last_error();
        escaped_value = html_escape_attribute_value(value_copy, &local_error);
        cma_free(value_copy);
        ft_global_error_stack_drop_last_error();
        if (!escaped_value)
        {
            cma_free(key_copy);
            ft_global_error_stack_drop_last_error();
            cma_free(result);
            ft_global_error_stack_drop_last_error();
            if (error_code != ft_nullptr)
                *error_code = local_error;
            return (ft_nullptr);
        }
        attr = cma_strjoin_multiple(5, " ", key_copy, "=\"", escaped_value, "\"");
        local_error = ft_global_error_stack_drop_last_error();
        cma_free(key_copy);
        ft_global_error_stack_drop_last_error();
        cma_free(escaped_value);
        ft_global_error_stack_drop_last_error();
        if (!attr)
        {
            cma_free(result);
            ft_global_error_stack_drop_last_error();
            if (local_error == FT_ERR_SUCCESSS)
                local_error = FT_ERR_NO_MEMORY;
            if (error_code != ft_nullptr)
                *error_code = local_error;
            return (ft_nullptr);
        }
        char *tmp;

        tmp = cma_strjoin(result, attr);
        local_error = ft_global_error_stack_drop_last_error();
        cma_free(result);
        ft_global_error_stack_drop_last_error();
        cma_free(attr);
        ft_global_error_stack_drop_last_error();
        if (!tmp)
        {
            if (local_error == FT_ERR_SUCCESSS)
                local_error = FT_ERR_NO_MEMORY;
            if (error_code != ft_nullptr)
                *error_code = local_error;
            return (ft_nullptr);
        }
        result = tmp;
        attribute = next_attribute;
    }
    if (error_code != ft_nullptr)
        *error_code = FT_ERR_SUCCESSS;
    return (result);
}

static char *html_indent(int indent, int *error_code)
{
    int spaces;
    char *result;
    int local_error;

    spaces = indent * 2;
    result = static_cast<char*>(cma_calloc(spaces + 1, sizeof(char)));
    local_error = ft_global_error_stack_drop_last_error();
    if (!result)
    {
        if (local_error == FT_ERR_SUCCESSS)
            local_error = FT_ERR_NO_MEMORY;
        if (error_code != ft_nullptr)
            *error_code = local_error;
        return (ft_nullptr);
    }
    int index = 0;
    while (index < spaces)
    {
        result[index] = ' ';
        ++index;
    }
    if (error_code != ft_nullptr)
        *error_code = FT_ERR_SUCCESSS;
    return (result);
}

static char *html_node_to_string(html_node *node, int indent, int *error_code)
{
    bool node_lock_acquired;
    int  lock_status;
    char *result;
    int  local_error;

    node_lock_acquired = false;
    lock_status = html_node_lock(node, &node_lock_acquired);
    local_error = ft_global_error_stack_drop_last_error();
    if (lock_status != 0)
    {
        if (error_code != ft_nullptr)
            *error_code = local_error;
        return (ft_nullptr);
    }
    result = cma_strdup("");
    local_error = ft_global_error_stack_drop_last_error();
    if (!result)
    {
        html_node_unlock(node, node_lock_acquired);
        ft_global_error_stack_drop_last_error();
        if (local_error == FT_ERR_SUCCESSS)
            local_error = FT_ERR_NO_MEMORY;
        if (error_code != ft_nullptr)
            *error_code = local_error;
        return (ft_nullptr);
    }
    char *pad = html_indent(indent, &local_error);
    if (!pad)
    {
        cma_free(result);
        ft_global_error_stack_drop_last_error();
        html_node_unlock(node, node_lock_acquired);
        ft_global_error_stack_drop_last_error();
        if (error_code != ft_nullptr)
            *error_code = local_error;
        return (ft_nullptr);
    }
    char *tmp = cma_strjoin(result, pad);
    local_error = ft_global_error_stack_drop_last_error();
    cma_free(result);
    ft_global_error_stack_drop_last_error();
    cma_free(pad);
    ft_global_error_stack_drop_last_error();
    if (!tmp)
    {
        html_node_unlock(node, node_lock_acquired);
        ft_global_error_stack_drop_last_error();
        if (local_error == FT_ERR_SUCCESSS)
            local_error = FT_ERR_NO_MEMORY;
        if (error_code != ft_nullptr)
            *error_code = local_error;
        return (ft_nullptr);
    }
    result = tmp;
    char *attrs = html_attrs_to_string(node->attributes, &local_error);
    if (!attrs)
    {
        cma_free(result);
        ft_global_error_stack_drop_last_error();
        html_node_unlock(node, node_lock_acquired);
        ft_global_error_stack_drop_last_error();
        if (error_code != ft_nullptr)
            *error_code = local_error;
        return (ft_nullptr);
    }
    char *open = cma_strjoin_multiple(3, "<", node->tag, attrs);
    local_error = ft_global_error_stack_drop_last_error();
    cma_free(attrs);
    ft_global_error_stack_drop_last_error();
    if (!open)
    {
        cma_free(result);
        ft_global_error_stack_drop_last_error();
        html_node_unlock(node, node_lock_acquired);
        ft_global_error_stack_drop_last_error();
        if (local_error == FT_ERR_SUCCESSS)
            local_error = FT_ERR_NO_MEMORY;
        if (error_code != ft_nullptr)
            *error_code = local_error;
        return (ft_nullptr);
    }
    if (!node->text && !node->children)
    {
        char *line = cma_strjoin_multiple(3, open, "/>", "\n");
        local_error = ft_global_error_stack_drop_last_error();
        cma_free(open);
        ft_global_error_stack_drop_last_error();
        if (!line)
        {
            cma_free(result);
            ft_global_error_stack_drop_last_error();
            html_node_unlock(node, node_lock_acquired);
            ft_global_error_stack_drop_last_error();
            if (local_error == FT_ERR_SUCCESSS)
                local_error = FT_ERR_NO_MEMORY;
            if (error_code != ft_nullptr)
                *error_code = local_error;
            return (ft_nullptr);
        }
        tmp = cma_strjoin(result, line);
        local_error = ft_global_error_stack_drop_last_error();
        cma_free(result);
        ft_global_error_stack_drop_last_error();
        cma_free(line);
        ft_global_error_stack_drop_last_error();
        if (!tmp)
        {
            html_node_unlock(node, node_lock_acquired);
            ft_global_error_stack_drop_last_error();
            if (local_error == FT_ERR_SUCCESSS)
                local_error = FT_ERR_NO_MEMORY;
            if (error_code != ft_nullptr)
                *error_code = local_error;
            return (ft_nullptr);
        }
        result = tmp;
        html_node_unlock(node, node_lock_acquired);
        ft_global_error_stack_drop_last_error();
        if (error_code != ft_nullptr)
            *error_code = FT_ERR_SUCCESSS;
        return (result);
    }
    tmp = cma_strjoin_multiple(2, open, ">");
    local_error = ft_global_error_stack_drop_last_error();
    cma_free(open);
    ft_global_error_stack_drop_last_error();
    if (!tmp)
    {
        cma_free(result);
        ft_global_error_stack_drop_last_error();
        html_node_unlock(node, node_lock_acquired);
        ft_global_error_stack_drop_last_error();
        if (local_error == FT_ERR_SUCCESSS)
            local_error = FT_ERR_NO_MEMORY;
        if (error_code != ft_nullptr)
            *error_code = local_error;
        return (ft_nullptr);
    }
    char *joined = cma_strjoin(result, tmp);
    local_error = ft_global_error_stack_drop_last_error();
    cma_free(result);
    ft_global_error_stack_drop_last_error();
    cma_free(tmp);
    ft_global_error_stack_drop_last_error();
    if (!joined)
    {
        html_node_unlock(node, node_lock_acquired);
        ft_global_error_stack_drop_last_error();
        if (local_error == FT_ERR_SUCCESSS)
            local_error = FT_ERR_NO_MEMORY;
        if (error_code != ft_nullptr)
            *error_code = local_error;
        return (ft_nullptr);
    }
    result = joined;
    if (node->text)
    {
        tmp = cma_strjoin(result, node->text);
        local_error = ft_global_error_stack_drop_last_error();
        cma_free(result);
        ft_global_error_stack_drop_last_error();
        if (!tmp)
        {
            html_node_unlock(node, node_lock_acquired);
            ft_global_error_stack_drop_last_error();
            if (local_error == FT_ERR_SUCCESSS)
                local_error = FT_ERR_NO_MEMORY;
            if (error_code != ft_nullptr)
                *error_code = local_error;
            return (ft_nullptr);
        }
        result = tmp;
    }
    if (node->children)
    {
        tmp = cma_strjoin(result, "\n");
        local_error = ft_global_error_stack_drop_last_error();
        cma_free(result);
        ft_global_error_stack_drop_last_error();
        if (!tmp)
        {
            html_node_unlock(node, node_lock_acquired);
            ft_global_error_stack_drop_last_error();
            if (local_error == FT_ERR_SUCCESSS)
                local_error = FT_ERR_NO_MEMORY;
            if (error_code != ft_nullptr)
                *error_code = local_error;
            return (ft_nullptr);
        }
        result = tmp;
        html_node *child = node->children;
        while (child)
        {
            char *child_str = html_node_to_string(child, indent + 1, &local_error);
            if (!child_str)
            {
                cma_free(result);
                ft_global_error_stack_drop_last_error();
                html_node_unlock(node, node_lock_acquired);
                ft_global_error_stack_drop_last_error();
                if (error_code != ft_nullptr)
                    *error_code = local_error;
                return (ft_nullptr);
            }
            tmp = cma_strjoin(result, child_str);
            local_error = ft_global_error_stack_drop_last_error();
            cma_free(result);
            ft_global_error_stack_drop_last_error();
            cma_free(child_str);
            ft_global_error_stack_drop_last_error();
            if (!tmp)
            {
                html_node_unlock(node, node_lock_acquired);
                ft_global_error_stack_drop_last_error();
                if (local_error == FT_ERR_SUCCESSS)
                    local_error = FT_ERR_NO_MEMORY;
                if (error_code != ft_nullptr)
                    *error_code = local_error;
                return (ft_nullptr);
            }
            result = tmp;
            child = child->next;
        }
        pad = html_indent(indent, &local_error);
        if (!pad)
        {
            cma_free(result);
            ft_global_error_stack_drop_last_error();
            html_node_unlock(node, node_lock_acquired);
            ft_global_error_stack_drop_last_error();
            if (error_code != ft_nullptr)
                *error_code = local_error;
            return (ft_nullptr);
        }
        tmp = cma_strjoin(result, pad);
        local_error = ft_global_error_stack_drop_last_error();
        cma_free(result);
        ft_global_error_stack_drop_last_error();
        cma_free(pad);
        ft_global_error_stack_drop_last_error();
        if (!tmp)
        {
            html_node_unlock(node, node_lock_acquired);
            ft_global_error_stack_drop_last_error();
            if (local_error == FT_ERR_SUCCESSS)
                local_error = FT_ERR_NO_MEMORY;
            if (error_code != ft_nullptr)
                *error_code = local_error;
            return (ft_nullptr);
        }
        result = tmp;
    }
    char *close_start = cma_strjoin_multiple(2, "</", node->tag);
    local_error = ft_global_error_stack_drop_last_error();
    if (!close_start)
    {
        cma_free(result);
        ft_global_error_stack_drop_last_error();
        html_node_unlock(node, node_lock_acquired);
        ft_global_error_stack_drop_last_error();
        if (local_error == FT_ERR_SUCCESSS)
            local_error = FT_ERR_NO_MEMORY;
        if (error_code != ft_nullptr)
            *error_code = local_error;
        return (ft_nullptr);
    }
    char *close = cma_strjoin_multiple(3, close_start, ">", "\n");
    local_error = ft_global_error_stack_drop_last_error();
    cma_free(close_start);
    ft_global_error_stack_drop_last_error();
    if (!close)
    {
        cma_free(result);
        ft_global_error_stack_drop_last_error();
        html_node_unlock(node, node_lock_acquired);
        ft_global_error_stack_drop_last_error();
        if (local_error == FT_ERR_SUCCESSS)
            local_error = FT_ERR_NO_MEMORY;
        if (error_code != ft_nullptr)
            *error_code = local_error;
        return (ft_nullptr);
    }
    tmp = cma_strjoin(result, close);
    local_error = ft_global_error_stack_drop_last_error();
    cma_free(result);
    ft_global_error_stack_drop_last_error();
    cma_free(close);
    ft_global_error_stack_drop_last_error();
    if (!tmp)
    {
        html_node_unlock(node, node_lock_acquired);
        ft_global_error_stack_drop_last_error();
        if (local_error == FT_ERR_SUCCESSS)
            local_error = FT_ERR_NO_MEMORY;
        if (error_code != ft_nullptr)
            *error_code = local_error;
        return (ft_nullptr);
    }
    result = tmp;
    html_node_unlock(node, node_lock_acquired);
    ft_global_error_stack_drop_last_error();
    if (error_code != ft_nullptr)
        *error_code = FT_ERR_SUCCESSS;
    return (result);
}

char *html_write_to_string(html_node *nodeList)
{
    char *result;
    int error_code;

    result = cma_strdup("");
    error_code = ft_global_error_stack_drop_last_error();
    if (!result)
    {
        if (error_code == FT_ERR_SUCCESSS)
            error_code = FT_ERR_NO_MEMORY;
        ft_global_error_stack_push(error_code);
        return (ft_nullptr);
    }
    html_node *current = nodeList;
    while (current)
    {
        html_node *next_node;
        bool       node_lock_acquired;
        int        lock_status;

        node_lock_acquired = false;
        lock_status = html_node_lock(current, &node_lock_acquired);
        ft_global_error_stack_drop_last_error();
        if (lock_status == 0 && node_lock_acquired)
        {
            next_node = current->next;
            html_node_unlock(current, node_lock_acquired);
            ft_global_error_stack_drop_last_error();
        }
        else
            next_node = current->next;
        char *node_str = html_node_to_string(current, 0, &error_code);
        if (!node_str)
        {
            cma_free(result);
            ft_global_error_stack_drop_last_error();
            if (error_code == FT_ERR_SUCCESSS)
                error_code = FT_ERR_NO_MEMORY;
            ft_global_error_stack_push(error_code);
            return (ft_nullptr);
        }
        char *tmp = cma_strjoin(result, node_str);
        error_code = ft_global_error_stack_drop_last_error();
        cma_free(result);
        ft_global_error_stack_drop_last_error();
        cma_free(node_str);
        ft_global_error_stack_drop_last_error();
        if (!tmp)
        {
            if (error_code == FT_ERR_SUCCESSS)
                error_code = FT_ERR_NO_MEMORY;
            ft_global_error_stack_push(error_code);
            return (ft_nullptr);
        }
        result = tmp;
        current = next_node;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}
