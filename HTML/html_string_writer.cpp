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

static char *html_escape_attribute_value(const char *value)
{
    size_t  input_length;
    size_t  input_index;
    size_t  output_length;
    char    *escaped_value;
    size_t  output_index;

    if (!value)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
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
    if (!escaped_value)
    {
        ft_errno = FT_ERR_NO_MEMORY;
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
    ft_errno = FT_ERR_SUCCESSS;
    return (escaped_value);
}

static char *html_attrs_to_string(html_attr *attribute)
{
    char *result;

    result = cma_strdup("");
    if (!result)
    {
        ft_errno = FT_ERR_NO_MEMORY;
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
        if (lock_status != 0)
        {
            cma_free(result);
            return (ft_nullptr);
        }
        if (!attribute->key || !attribute->value)
        {
            html_attr_unlock(attribute, lock_acquired);
            cma_free(result);
            ft_errno = FT_ERR_INVALID_ARGUMENT;
            return (ft_nullptr);
        }
        key_copy = cma_strdup(attribute->key);
        if (!key_copy)
        {
            html_attr_unlock(attribute, lock_acquired);
            cma_free(result);
            ft_errno = FT_ERR_NO_MEMORY;
            return (ft_nullptr);
        }
        value_copy = cma_strdup(attribute->value);
        if (!value_copy)
        {
            cma_free(key_copy);
            html_attr_unlock(attribute, lock_acquired);
            cma_free(result);
            ft_errno = FT_ERR_NO_MEMORY;
            return (ft_nullptr);
        }
        next_attribute = attribute->next;
        html_attr_unlock(attribute, lock_acquired);
        escaped_value = html_escape_attribute_value(value_copy);
        cma_free(value_copy);
        if (!escaped_value)
        {
            cma_free(key_copy);
            cma_free(result);
            return (ft_nullptr);
        }
        attr = cma_strjoin_multiple(5, " ", key_copy, "=\"", escaped_value, "\"");
        cma_free(key_copy);
        cma_free(escaped_value);
        if (!attr)
        {
            cma_free(result);
            ft_errno = FT_ERR_NO_MEMORY;
            return (ft_nullptr);
        }
        char *tmp;

        tmp = cma_strjoin(result, attr);
        cma_free(result);
        cma_free(attr);
        if (!tmp)
        {
            ft_errno = FT_ERR_NO_MEMORY;
            return (ft_nullptr);
        }
        result = tmp;
        attribute = next_attribute;
    }
    ft_errno = FT_ERR_SUCCESSS;
    return (result);
}

static char *html_indent(int indent)
{
    int spaces = indent * 2;
    char *result = static_cast<char*>(cma_calloc(spaces + 1, sizeof(char)));
    if (!result)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        return (ft_nullptr);
    }
    int index = 0;
    while (index < spaces)
    {
        result[index] = ' ';
        ++index;
    }
    ft_errno = FT_ERR_SUCCESSS;
    return (result);
}

static char *html_node_to_string(html_node *node, int indent)
{
    bool node_lock_acquired;
    int  lock_status;
    char *result;

    node_lock_acquired = false;
    lock_status = html_node_lock(node, &node_lock_acquired);
    if (lock_status != 0)
        return (ft_nullptr);
    result = cma_strdup("");
    if (!result)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        html_node_unlock(node, node_lock_acquired);
        return (ft_nullptr);
    }
    char *pad = html_indent(indent);
    if (!pad)
    {
        cma_free(result);
        ft_errno = FT_ERR_NO_MEMORY;
        html_node_unlock(node, node_lock_acquired);
        return (ft_nullptr);
    }
    char *tmp = cma_strjoin(result, pad);
    cma_free(result);
    cma_free(pad);
    if (!tmp)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        html_node_unlock(node, node_lock_acquired);
        return (ft_nullptr);
    }
    result = tmp;
    char *attrs = html_attrs_to_string(node->attributes);
    if (!attrs)
    {
        cma_free(result);
        ft_errno = FT_ERR_NO_MEMORY;
        html_node_unlock(node, node_lock_acquired);
        return (ft_nullptr);
    }
    char *open = cma_strjoin_multiple(3, "<", node->tag, attrs);
    cma_free(attrs);
    if (!open)
    {
        cma_free(result);
        ft_errno = FT_ERR_NO_MEMORY;
        html_node_unlock(node, node_lock_acquired);
        return (ft_nullptr);
    }
    if (!node->text && !node->children)
    {
        char *line = cma_strjoin_multiple(3, open, "/>", "\n");
        cma_free(open);
        if (!line)
        {
            cma_free(result);
            ft_errno = FT_ERR_NO_MEMORY;
            html_node_unlock(node, node_lock_acquired);
            return (ft_nullptr);
        }
        tmp = cma_strjoin(result, line);
        cma_free(result);
        cma_free(line);
        if (!tmp)
        {
            ft_errno = FT_ERR_NO_MEMORY;
            html_node_unlock(node, node_lock_acquired);
            return (ft_nullptr);
        }
        result = tmp;
        ft_errno = FT_ERR_SUCCESSS;
        html_node_unlock(node, node_lock_acquired);
        return (result);
    }
    tmp = cma_strjoin_multiple(2, open, ">");
    cma_free(open);
    if (!tmp)
    {
        cma_free(result);
        ft_errno = FT_ERR_NO_MEMORY;
        html_node_unlock(node, node_lock_acquired);
        return (ft_nullptr);
    }
    char *joined = cma_strjoin(result, tmp);
    cma_free(result);
    cma_free(tmp);
    if (!joined)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        html_node_unlock(node, node_lock_acquired);
        return (ft_nullptr);
    }
    result = joined;
    if (node->text)
    {
        tmp = cma_strjoin(result, node->text);
        cma_free(result);
        if (!tmp)
        {
            ft_errno = FT_ERR_NO_MEMORY;
            html_node_unlock(node, node_lock_acquired);
            return (ft_nullptr);
        }
        result = tmp;
    }
    if (node->children)
    {
        tmp = cma_strjoin(result, "\n");
        cma_free(result);
        if (!tmp)
        {
            ft_errno = FT_ERR_NO_MEMORY;
            html_node_unlock(node, node_lock_acquired);
            return (ft_nullptr);
        }
        result = tmp;
        html_node *child = node->children;
        while (child)
        {
            char *child_str = html_node_to_string(child, indent + 1);
            if (!child_str)
            {
                cma_free(result);
                ft_errno = FT_ERR_NO_MEMORY;
                html_node_unlock(node, node_lock_acquired);
                return (ft_nullptr);
            }
            tmp = cma_strjoin(result, child_str);
            cma_free(result);
            cma_free(child_str);
            if (!tmp)
            {
                ft_errno = FT_ERR_NO_MEMORY;
                html_node_unlock(node, node_lock_acquired);
                return (ft_nullptr);
            }
            result = tmp;
            child = child->next;
        }
        pad = html_indent(indent);
        if (!pad)
        {
            cma_free(result);
            ft_errno = FT_ERR_NO_MEMORY;
            html_node_unlock(node, node_lock_acquired);
            return (ft_nullptr);
        }
        tmp = cma_strjoin(result, pad);
        cma_free(result);
        cma_free(pad);
        if (!tmp)
        {
            ft_errno = FT_ERR_NO_MEMORY;
            html_node_unlock(node, node_lock_acquired);
            return (ft_nullptr);
        }
        result = tmp;
    }
    char *close_start = cma_strjoin_multiple(2, "</", node->tag);
    if (!close_start)
    {
        cma_free(result);
        ft_errno = FT_ERR_NO_MEMORY;
        html_node_unlock(node, node_lock_acquired);
        return (ft_nullptr);
    }
    char *close = cma_strjoin_multiple(3, close_start, ">", "\n");
    cma_free(close_start);
    if (!close)
    {
        cma_free(result);
        ft_errno = FT_ERR_NO_MEMORY;
        html_node_unlock(node, node_lock_acquired);
        return (ft_nullptr);
    }
    tmp = cma_strjoin(result, close);
    cma_free(result);
    cma_free(close);
    if (!tmp)
    {
        ft_errno = FT_ERR_NO_MEMORY;
        html_node_unlock(node, node_lock_acquired);
        return (ft_nullptr);
    }
    result = tmp;
    ft_errno = FT_ERR_SUCCESSS;
    html_node_unlock(node, node_lock_acquired);
    return (result);
}

char *html_write_to_string(html_node *nodeList)
{
    char *result = cma_strdup("");
    if (!result)
    {
        ft_errno = FT_ERR_NO_MEMORY;
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
        if (lock_status == 0 && node_lock_acquired)
        {
            next_node = current->next;
            html_node_unlock(current, node_lock_acquired);
        }
        else
            next_node = current->next;
        char *node_str = html_node_to_string(current, 0);
        if (!node_str)
        {
            cma_free(result);
            ft_errno = FT_ERR_NO_MEMORY;
            return (ft_nullptr);
        }
        char *tmp = cma_strjoin(result, node_str);
        cma_free(result);
        cma_free(node_str);
        if (!tmp)
        {
            ft_errno = FT_ERR_NO_MEMORY;
            return (ft_nullptr);
        }
        result = tmp;
        current = next_node;
    }
    ft_errno = FT_ERR_SUCCESSS;
    return (result);
}

