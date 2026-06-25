#include <cstdlib>
#include "html_parser.hpp"
#include "../Basic/basic.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../CMA/CMA.hpp"
#include "../Basic/limits.hpp"

static void release_html_string(char *string)
{
    if (!string)
        return ;
    int32_t release_result = cma_checked_free(string);
    if (release_result != 0)
        return ;
    return ;
}

void html_free_nodes(html_node *node_list)
{
    while (node_list)
    {
        html_node *next_node;
        html_node *children_list;
        html_attr *attribute_list;
        next_node = node_list->next;
        children_list = node_list->children;
        attribute_list = node_list->attributes;
        node_list->children = ft_nullptr;
        node_list->attributes = ft_nullptr;
        node_list->next = ft_nullptr;
        html_attr *current_attribute;

        current_attribute = attribute_list;
        while (current_attribute)
        {
            html_attr *next_attribute;
            next_attribute = current_attribute->next;
            current_attribute->next = ft_nullptr;
            release_html_string(current_attribute->key);
            release_html_string(current_attribute->value);
            delete current_attribute;
            current_attribute = next_attribute;
        }
        html_free_nodes(children_list);
        release_html_string(node_list->tag);
        release_html_string(node_list->text);
        delete node_list;
        node_list = next_node;
    }
    return ;
}

void html_remove_nodes_by_tag(html_node **node_list, const char *tag_name)
{
    html_node *current_node;
    html_node *previous_node;

    if (node_list == ft_nullptr || tag_name == ft_nullptr)
        return ;
    current_node = *node_list;
    previous_node = ft_nullptr;
    while (current_node)
    {
        html_node *next_node;

        next_node = current_node->next;
        if (current_node->tag && ft_strcmp(current_node->tag, tag_name) == 0)
        {
            if (previous_node)
                previous_node->next = next_node;
            else
                *node_list = next_node;
            current_node->next = ft_nullptr;
            html_free_nodes(current_node);
            current_node = next_node;
            continue ;
        }
        html_remove_nodes_by_tag(&current_node->children, tag_name);
        previous_node = current_node;
        current_node = next_node;
    }
    return ;
}

void html_remove_nodes_by_attr(html_node **node_list, const char *key, const char *value)
{
    html_node *current_node;
    html_node *previous_node;

    if (node_list == ft_nullptr || key == ft_nullptr || value == ft_nullptr)
        return ;
    current_node = *node_list;
    previous_node = ft_nullptr;
    while (current_node)
    {
        html_node *next_node;
        html_attr *attribute_node;
        int32_t        match_found;

        next_node = current_node->next;
        match_found = 0;
        attribute_node = current_node->attributes;
        while (attribute_node)
        {
            if (attribute_node->key && attribute_node->value &&
                ft_strcmp(attribute_node->key, key) == 0 &&
                ft_strcmp(attribute_node->value, value) == 0)
            {
                match_found = 1;
                break ;
            }
            attribute_node = attribute_node->next;
        }
        if (match_found)
        {
            if (previous_node)
                previous_node->next = next_node;
            else
                *node_list = next_node;
            current_node->next = ft_nullptr;
            html_free_nodes(current_node);
            current_node = next_node;
            continue ;
        }
        html_remove_nodes_by_attr(&current_node->children, key, value);
        previous_node = current_node;
        current_node = next_node;
    }
    return ;
}

void html_remove_nodes_by_text(html_node **node_list, const char *text_content)
{
    html_node *current_node;
    html_node *previous_node;

    if (node_list == ft_nullptr || text_content == ft_nullptr)
        return ;
    current_node = *node_list;
    previous_node = ft_nullptr;
    while (current_node)
    {
        html_node *next_node;

        next_node = current_node->next;
        if (current_node->text && ft_strcmp(current_node->text, text_content) == 0)
        {
            if (previous_node)
                previous_node->next = next_node;
            else
                *node_list = next_node;
            current_node->next = ft_nullptr;
            html_free_nodes(current_node);
            current_node = next_node;
            continue ;
        }
        html_remove_nodes_by_text(&current_node->children, text_content);
        previous_node = current_node;
        current_node = next_node;
    }
    return ;
}
