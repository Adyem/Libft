#include "parser.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Libft/libft.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"

static int normalize_selector_value(char *value_string)
{
    size_t value_length;
    char opening_character;
    char closing_character;
    int length_error;

    if (!value_string)
        return (0);
    value_length = ft_strlen_size_t(value_string);
    length_error = ft_global_error_stack_pop_newest();
    if (length_error != FT_ERR_SUCCESSS)
    {
        return (-1);
    }
    if (value_length < 2)
        return (0);
    opening_character = value_string[0];
    closing_character = value_string[value_length - 1];
    if ((opening_character == '"' && closing_character == '"')
        || (opening_character == '\'' && closing_character == '\''))
    {
        ft_memmove(value_string, value_string + 1, value_length - 1);
        value_string[value_length - 2] = '\0';
    }
    return (0);
}

html_node *html_find_by_tag(html_node *nodeList, const char *tagName)
{
    html_node *current_node;

    current_node = nodeList;
    while (current_node)
    {
        html_node *next_node;
        bool       lock_acquired;
        int        lock_status;

        lock_acquired = false;
        lock_status = html_node_lock(current_node, &lock_acquired);
        next_node = current_node->next;
        if (lock_status != 0)
        {
            current_node = next_node;
            continue ;
        }
        if (current_node->tag && ft_strcmp(current_node->tag, tagName) == 0)
        {
            html_node_unlock(current_node, lock_acquired);
            return (current_node);
        }
        html_node *found = html_find_by_tag(current_node->children, tagName);

        html_node_unlock(current_node, lock_acquired);
        if (found)
            return (found);
        current_node = next_node;
    }
    return (ft_nullptr);
}

html_node *html_find_by_attr(html_node *nodeList, const char *key, const char *value)
{
    html_node *current_node;

    current_node = nodeList;
    while (current_node)
    {
        html_node *next_node;
        bool       lock_acquired;
        int        lock_status;

        lock_acquired = false;
        lock_status = html_node_lock(current_node, &lock_acquired);
        next_node = current_node->next;
        if (lock_status != 0)
        {
            current_node = next_node;
            continue ;
        }
        html_attr *attribute = current_node->attributes;

        while (attribute)
        {
            bool       attribute_lock_acquired;
            int        attribute_lock_status;
            html_attr *next_attribute;

            attribute_lock_acquired = false;
            attribute_lock_status = html_attr_lock(attribute, &attribute_lock_acquired);
            if (attribute_lock_status != 0)
            {
                attribute = attribute->next;
                continue ;
            }
            next_attribute = attribute->next;
            if (attribute->key && ft_strcmp(attribute->key, key) == 0)
            {
                if (!value || (attribute->value && ft_strcmp(attribute->value, value) == 0))
                {
                    html_attr_unlock(attribute, attribute_lock_acquired);
                    html_node_unlock(current_node, lock_acquired);
                    return (current_node);
                }
            }
            html_attr_unlock(attribute, attribute_lock_acquired);
            attribute = next_attribute;
        }
        html_node *found = html_find_by_attr(current_node->children, key, value);

        html_node_unlock(current_node, lock_acquired);
        if (found)
            return (found);
        current_node = next_node;
    }
    return (ft_nullptr);
}

html_node *html_find_by_text(html_node *nodeList, const char *textContent)
{
    html_node *current_node;

    current_node = nodeList;
    while (current_node)
    {
        html_node *next_node;
        bool       lock_acquired;
        int        lock_status;

        lock_acquired = false;
        lock_status = html_node_lock(current_node, &lock_acquired);
        next_node = current_node->next;
        if (lock_status != 0)
        {
            current_node = next_node;
            continue ;
        }
        if (current_node->text && ft_strcmp(current_node->text, textContent) == 0)
        {
            html_node_unlock(current_node, lock_acquired);
            return (current_node);
        }
        html_node *found = html_find_by_text(current_node->children, textContent);

        html_node_unlock(current_node, lock_acquired);
        if (found)
            return (found);
        current_node = next_node;
    }
    return (ft_nullptr);
}

size_t html_count_nodes_by_tag(html_node *nodeList, const char *tagName)
{
    size_t count;
    html_node *current_node;

    count = 0;
    current_node = nodeList;
    while (current_node)
    {
        html_node *next_node;
        bool       lock_acquired;
        int        lock_status;

        lock_acquired = false;
        lock_status = html_node_lock(current_node, &lock_acquired);
        next_node = current_node->next;
        if (lock_status != 0)
        {
            current_node = next_node;
            continue ;
        }
        if (current_node->tag && ft_strcmp(current_node->tag, tagName) == 0)
            ++count;
        count += html_count_nodes_by_tag(current_node->children, tagName);
        html_node_unlock(current_node, lock_acquired);
        current_node = next_node;
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
            if (!key)
                return (ft_nullptr);
            value = cma_substr(equal_sign + 1, 0, static_cast<size_t>(close_bracket - equal_sign - 1));
            if (!value)
            {
                cma_free(key);
                return (ft_nullptr);
            }
            if (normalize_selector_value(value) != 0)
            {
                cma_free(key);
                cma_free(value);
                return (ft_nullptr);
            }
            result = html_find_by_attr(node_list, key, value);
            cma_free(key);
            cma_free(value);
            return (result);
        }
        key = cma_substr(selector + 1, 0, static_cast<size_t>(close_bracket - selector - 1));
        if (!key)
            return (ft_nullptr);
        result = html_find_by_attr(node_list, key, ft_nullptr);
        cma_free(key);
        return (result);
    }
    return (html_find_by_tag(node_list, selector));
}

html_node *html_query_selector(html_node *node_list, const char *selector)
{
    if (!selector)
        return (ft_nullptr);
    if (selector[0] == '#')
        return (html_find_by_attr(node_list, "id", selector + 1));
    if (selector[0] == '.')
        return (html_find_by_attr(node_list, "class", selector + 1));
    return (html_find_by_tag(node_list, selector));
}
