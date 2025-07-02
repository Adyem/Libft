#include "html_parser.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/nullptr.hpp"
#include "../Libft/libft.hpp"

static char *html_attrs_to_string(html_attr *attribute)
{
    char *result = cma_strdup("");
    if (!result)
        return (ft_nullptr);
    while (attribute)
    {
        char *attr = cma_strjoin_multiple(5, " ", attribute->key, "=\"", attribute->value, "\"");
        if (!attr)
        {
            cma_free(result);
            return (ft_nullptr);
        }
        char *tmp = cma_strjoin(result, attr);
        cma_free(result);
        cma_free(attr);
        if (!tmp)
            return (ft_nullptr);
        result = tmp;
        attribute = attribute->next;
    }
    return (result);
}

static char *html_indent(int indent)
{
    int spaces = indent * 2;
    char *result = static_cast<char*>(cma_calloc(spaces + 1, sizeof(char)));
    if (!result)
        return (ft_nullptr);
    for (int index = 0; index < spaces; ++index)
        result[index] = ' ';
    return (result);
}

static char *html_node_to_string(html_node *node, int indent)
{
    char *result = cma_strdup("");
    if (!result)
        return (ft_nullptr);
    char *pad = html_indent(indent);
    if (!pad)
    {
        cma_free(result);
        return (ft_nullptr);
    }
    char *tmp = cma_strjoin(result, pad);
    cma_free(result);
    cma_free(pad);
    if (!tmp)
        return (ft_nullptr);
    result = tmp;
    char *attrs = html_attrs_to_string(node->attributes);
    if (!attrs)
    {
        cma_free(result);
        return (ft_nullptr);
    }
    char *open = cma_strjoin_multiple(3, "<", node->tag, attrs);
    cma_free(attrs);
    if (!open)
    {
        cma_free(result);
        return (ft_nullptr);
    }
    if (!node->text && !node->children)
    {
        char *line = cma_strjoin_multiple(3, open, "/>", "\n");
        cma_free(open);
        if (!line)
        {
            cma_free(result);
            return (ft_nullptr);
        }
        tmp = cma_strjoin(result, line);
        cma_free(result);
        cma_free(line);
        if (!tmp)
            return (ft_nullptr);
        result = tmp;
        return (result);
    }
    tmp = cma_strjoin_multiple(2, open, ">");
    cma_free(open);
    if (!tmp)
    {
        cma_free(result);
        return (ft_nullptr);
    }
    char *joined = cma_strjoin(result, tmp);
    cma_free(result);
    cma_free(tmp);
    if (!joined)
        return (ft_nullptr);
    result = joined;
    if (node->text)
    {
        tmp = cma_strjoin(result, node->text);
        cma_free(result);
        if (!tmp)
            return (ft_nullptr);
        result = tmp;
    }
    if (node->children)
    {
        tmp = cma_strjoin(result, "\n");
        cma_free(result);
        if (!tmp)
            return (ft_nullptr);
        result = tmp;
        html_node *child = node->children;
        while (child)
        {
            char *child_str = html_node_to_string(child, indent + 1);
            if (!child_str)
            {
                cma_free(result);
                return (ft_nullptr);
            }
            tmp = cma_strjoin(result, child_str);
            cma_free(result);
            cma_free(child_str);
            if (!tmp)
                return (ft_nullptr);
            result = tmp;
            child = child->next;
        }
        pad = html_indent(indent);
        if (!pad)
        {
            cma_free(result);
            return (ft_nullptr);
        }
        tmp = cma_strjoin(result, pad);
        cma_free(result);
        cma_free(pad);
        if (!tmp)
            return (ft_nullptr);
        result = tmp;
    }
    char *close_start = cma_strjoin_multiple(2, "</", node->tag);
    if (!close_start)
    {
        cma_free(result);
        return (ft_nullptr);
    }
    char *close = cma_strjoin_multiple(3, close_start, ">", "\n");
    cma_free(close_start);
    if (!close)
    {
        cma_free(result);
        return (ft_nullptr);
    }
    tmp = cma_strjoin(result, close);
    cma_free(result);
    cma_free(close);
    if (!tmp)
        return (ft_nullptr);
    result = tmp;
    return (result);
}

char *html_write_to_string(html_node *nodeList)
{
    char *result = cma_strdup("");
    if (!result)
        return (ft_nullptr);
    html_node *current = nodeList;
    while (current)
    {
        char *node_str = html_node_to_string(current, 0);
        if (!node_str)
        {
            cma_free(result);
            return (ft_nullptr);
        }
        char *tmp = cma_strjoin(result, node_str);
        cma_free(result);
        cma_free(node_str);
        if (!tmp)
            return (ft_nullptr);
        result = tmp;
        current = current->next;
    }
    return (result);
}

