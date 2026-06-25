#include "html_parser.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../Basic/basic.hpp"
#include "../Advanced/advanced.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/limits.hpp"

static int32_t normalize_selector_value(char *value_string)
{
    ft_size_t value_length;
    char opening_character;
    char closing_character;

    if (!value_string)
        return (FT_ERR_SUCCESS);
    value_length = ft_strlen_size_t(value_string);
    if (value_length < 2)
        return (FT_ERR_SUCCESS);
    opening_character = value_string[0];
    closing_character = value_string[value_length - 1];
    if ((opening_character == '"' && closing_character == '"')
        || (opening_character == '\'' && closing_character == '\''))
    {
        ft_memmove(value_string, value_string + 1, value_length - 1);
        value_string[value_length - 2] = '\0';
    }
    return (FT_ERR_SUCCESS);
}

html_node *html_find_by_tag(html_node *node_list, const char *tag_name)
{
    html_node *current_node;

    current_node = node_list;
    while (current_node)
    {
        html_node *next_node;

        next_node = current_node->next;
        if (current_node->tag && ft_strcmp(current_node->tag, tag_name) == 0)
            return (current_node);
        html_node *found = html_find_by_tag(current_node->children, tag_name);

        if (found)
            return (found);
        current_node = next_node;
    }
    return (ft_nullptr);
}

html_node *html_find_by_attr(html_node *node_list, const char *key, const char *value)
{
    html_node *current_node;

    current_node = node_list;
    while (current_node)
    {
        html_node *next_node;

        next_node = current_node->next;
        html_attr *attribute = current_node->attributes;

        while (attribute)
        {
            html_attr *next_attribute;

            next_attribute = attribute->next;
            if (attribute->key && ft_strcmp(attribute->key, key) == 0)
            {
                if (!value || (attribute->value && ft_strcmp(attribute->value, value) == 0))
                    return (current_node);
            }
            attribute = next_attribute;
        }
        html_node *found = html_find_by_attr(current_node->children, key, value);

        if (found)
            return (found);
        current_node = next_node;
    }
    return (ft_nullptr);
}

html_node *html_find_by_text(html_node *node_list, const char *text_content)
{
    html_node *current_node;

    current_node = node_list;
    while (current_node)
    {
        html_node *next_node;

        next_node = current_node->next;
        if (current_node->text && ft_strcmp(current_node->text, text_content) == 0)
            return (current_node);
        html_node *found = html_find_by_text(current_node->children, text_content);

        if (found)
            return (found);
        current_node = next_node;
    }
    return (ft_nullptr);
}

ft_size_t html_count_nodes_by_tag(html_node *node_list, const char *tag_name)
{
    ft_size_t count;
    html_node *current_node;

    count = 0;
    current_node = node_list;
    while (current_node)
    {
        html_node *next_node;

        next_node = current_node->next;
        if (current_node->tag && ft_strcmp(current_node->tag, tag_name) == 0)
            ++count;
        count += html_count_nodes_by_tag(current_node->children, tag_name);
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
            key = adv_strndup(selector + 1, static_cast<ft_size_t>(equal_sign - selector - 1));
            if (!key)
                return (ft_nullptr);
            value = adv_strndup(equal_sign + 1, static_cast<ft_size_t>(close_bracket - equal_sign - 1));
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
        key = adv_strndup(selector + 1, static_cast<ft_size_t>(close_bracket - selector - 1));
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
