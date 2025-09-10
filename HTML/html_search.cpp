#include "parser.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Libft/libft.hpp"
#include "../CMA/CMA.hpp"

html_node *html_find_by_tag(html_node *nodeList, const char *tagName)
{
    html_node *currentNode = nodeList;
    while (currentNode)
    {
        if (currentNode->tag && ft_strcmp(currentNode->tag, tagName) == 0)
            return (currentNode);
        html_node *found = html_find_by_tag(currentNode->children, tagName);
        if (found)
            return (found);
        currentNode = currentNode->next;
    }
    return (ft_nullptr);
}

html_node *html_find_by_attr(html_node *nodeList, const char *key, const char *value)
{
    html_node *currentNode = nodeList;
    while (currentNode)
    {
        html_attr *attribute = currentNode->attributes;
        while (attribute)
        {
            if (attribute->key && ft_strcmp(attribute->key, key) == 0)
            {
                if (!value || (attribute->value && ft_strcmp(attribute->value, value) == 0))
                    return (currentNode);
            }
            attribute = attribute->next;
        }
        html_node *found = html_find_by_attr(currentNode->children, key, value);
        if (found)
            return (found);
        currentNode = currentNode->next;
    }
    return (ft_nullptr);
}

html_node *html_find_by_text(html_node *nodeList, const char *textContent)
{
    html_node *currentNode = nodeList;
    while (currentNode)
    {
        if (currentNode->text && ft_strcmp(currentNode->text, textContent) == 0)
            return (currentNode);
        html_node *found = html_find_by_text(currentNode->children, textContent);
        if (found)
            return (found);
        currentNode = currentNode->next;
    }
    return (ft_nullptr);
}

size_t html_count_nodes_by_tag(html_node *nodeList, const char *tagName)
{
    size_t count = 0;
    html_node *currentNode = nodeList;
    while (currentNode)
    {
        if (currentNode->tag && ft_strcmp(currentNode->tag, tagName) == 0)
            ++count;
        count += html_count_nodes_by_tag(currentNode->children, tagName);
        currentNode = currentNode->next;
    }
    return (count);
}

html_node *html_find_by_selector(html_node *node_list, const char *selector)
{
    const char *close_bracket;
    const char *equal_sign;
    char *key;
    char *value;
    html_node *result;

    if (!selector)
        return (ft_nullptr);
    if (selector[0] == '#')
        return (html_find_by_attr(node_list, "id", selector + 1));
    if (selector[0] == '.')
        return (html_find_by_attr(node_list, "class", selector + 1));
    if (selector[0] == '[')
    {
        close_bracket = ft_strchr(selector + 1, ']');
        if (!close_bracket)
            return (ft_nullptr);
        equal_sign = ft_strchr(selector + 1, '=');
        if (equal_sign && equal_sign < close_bracket)
        {
            key = cma_substr(selector + 1, 0, static_cast<size_t>(equal_sign - selector - 1));
            value = cma_substr(equal_sign + 1, 0, static_cast<size_t>(close_bracket - equal_sign - 1));
            result = html_find_by_attr(node_list, key, value);
            if (key)
                cma_free(key);
            if (value)
                cma_free(value);
            return (result);
        }
        key = cma_substr(selector + 1, 0, static_cast<size_t>(close_bracket - selector - 1));
        result = html_find_by_attr(node_list, key, ft_nullptr);
        if (key)
            cma_free(key);
        return (result);
    }
    return (html_find_by_tag(node_list, selector));
}
