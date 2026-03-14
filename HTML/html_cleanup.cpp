#include <cstdlib>
#include "html_parser.hpp"
#include "../Basic/basic.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../CMA/CMA.hpp"

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
        ft_bool       node_lock_acquired;
        int32_t        lock_status;

        next_node = node_list->next;
        children_list = node_list->children;
        attribute_list = node_list->attributes;
        node_lock_acquired = FT_FALSE;
        lock_status = html_node_lock(node_list, &node_lock_acquired);
        if (lock_status == 0 && node_lock_acquired)
        {
            children_list = node_list->children;
            attribute_list = node_list->attributes;
            node_list->children = ft_nullptr;
            node_list->attributes = ft_nullptr;
            node_list->next = ft_nullptr;
            html_node_unlock(node_list, node_lock_acquired);
        }
        html_attr *current_attribute;

        current_attribute = attribute_list;
        while (current_attribute)
        {
            html_attr *next_attribute;
            ft_bool       attribute_lock_acquired;
            int32_t        lock_attribute_status;

            attribute_lock_acquired = FT_FALSE;
            lock_attribute_status = html_attr_lock(current_attribute, &attribute_lock_acquired);
            if (lock_attribute_status == 0 && attribute_lock_acquired)
            {
                next_attribute = current_attribute->next;
                current_attribute->next = ft_nullptr;
                html_attr_unlock(current_attribute, attribute_lock_acquired);
            }
            else
                next_attribute = current_attribute->next;
            release_html_string(current_attribute->key);
            release_html_string(current_attribute->value);
            html_attr_teardown_thread_safety(current_attribute);
            delete current_attribute;
            current_attribute = next_attribute;
        }
        html_free_nodes(children_list);
        release_html_string(node_list->tag);
        release_html_string(node_list->text);
        html_node_teardown_thread_safety(node_list);
        delete node_list;
        node_list = next_node;
    }
    return ;
}

void html_remove_nodes_by_tag(html_node **node_list, const char *tag_name)
{
    html_node *current_node;
    html_node *previous_node;
    ft_bool       previous_lock_acquired;

    if (node_list == ft_nullptr || tag_name == ft_nullptr)
        return ;
    current_node = *node_list;
    previous_node = ft_nullptr;
    previous_lock_acquired = FT_FALSE;
    while (current_node)
    {
        html_node *next_node;
        ft_bool       current_lock_acquired;
        int32_t        lock_status;

        next_node = current_node->next;
        current_lock_acquired = FT_FALSE;
        lock_status = html_node_lock(current_node, &current_lock_acquired);
        if (lock_status != 0)
        {
            if (previous_node)
            {
                html_node_unlock(previous_node, previous_lock_acquired);
                previous_node = ft_nullptr;
                previous_lock_acquired = FT_FALSE;
            }
            current_node = next_node;
            continue ;
        }
        if (current_node->tag && ft_strcmp(current_node->tag, tag_name) == 0)
        {
            if (previous_node)
                previous_node->next = next_node;
            else
                *node_list = next_node;
            current_node->next = ft_nullptr;
            html_node_unlock(current_node, current_lock_acquired);
            html_free_nodes(current_node);
            current_node = next_node;
            continue ;
        }
        html_remove_nodes_by_tag(&current_node->children, tag_name);
        if (previous_node)
            html_node_unlock(previous_node, previous_lock_acquired);
        previous_node = current_node;
        previous_lock_acquired = current_lock_acquired;
        current_node = next_node;
    }
    if (previous_node)
        html_node_unlock(previous_node, previous_lock_acquired);
    return ;
}

void html_remove_nodes_by_attr(html_node **node_list, const char *key, const char *value)
{
    html_node *current_node;
    html_node *previous_node;
    ft_bool       previous_lock_acquired;

    if (node_list == ft_nullptr || key == ft_nullptr || value == ft_nullptr)
        return ;
    current_node = *node_list;
    previous_node = ft_nullptr;
    previous_lock_acquired = FT_FALSE;
    while (current_node)
    {
        html_node *next_node;
        html_attr *attribute_node;
        int32_t        match_found;
        ft_bool       current_lock_acquired;
        int32_t        lock_status;

        next_node = current_node->next;
        current_lock_acquired = FT_FALSE;
        lock_status = html_node_lock(current_node, &current_lock_acquired);
        if (lock_status != 0)
        {
            if (previous_node)
            {
                html_node_unlock(previous_node, previous_lock_acquired);
                previous_node = ft_nullptr;
                previous_lock_acquired = FT_FALSE;
            }
            current_node = next_node;
            continue ;
        }
        match_found = 0;
        attribute_node = current_node->attributes;
        while (attribute_node)
        {
            ft_bool       attribute_lock_acquired;
            int32_t        attribute_lock_status;
            html_attr *next_attribute;

            attribute_lock_acquired = FT_FALSE;
            attribute_lock_status = html_attr_lock(attribute_node, &attribute_lock_acquired);
            if (attribute_lock_status != 0)
            {
                attribute_node = attribute_node->next;
                continue ;
            }
            next_attribute = attribute_node->next;
            if (attribute_node->key && attribute_node->value &&
                ft_strcmp(attribute_node->key, key) == 0 &&
                ft_strcmp(attribute_node->value, value) == 0)
            {
                match_found = 1;
                html_attr_unlock(attribute_node, attribute_lock_acquired);
                break ;
            }
            html_attr_unlock(attribute_node, attribute_lock_acquired);
            attribute_node = next_attribute;
        }
        if (match_found)
        {
            if (previous_node)
                previous_node->next = next_node;
            else
                *node_list = next_node;
            current_node->next = ft_nullptr;
            html_node_unlock(current_node, current_lock_acquired);
            html_free_nodes(current_node);
            current_node = next_node;
            continue ;
        }
        html_remove_nodes_by_attr(&current_node->children, key, value);
        if (previous_node)
            html_node_unlock(previous_node, previous_lock_acquired);
        previous_node = current_node;
        previous_lock_acquired = current_lock_acquired;
        current_node = next_node;
    }
    if (previous_node)
        html_node_unlock(previous_node, previous_lock_acquired);
    return ;
}

void html_remove_nodes_by_text(html_node **node_list, const char *text_content)
{
    html_node *current_node;
    html_node *previous_node;
    ft_bool       previous_lock_acquired;

    if (node_list == ft_nullptr || text_content == ft_nullptr)
        return ;
    current_node = *node_list;
    previous_node = ft_nullptr;
    previous_lock_acquired = FT_FALSE;
    while (current_node)
    {
        html_node *next_node;
        ft_bool       current_lock_acquired;
        int32_t        lock_status;

        next_node = current_node->next;
        current_lock_acquired = FT_FALSE;
        lock_status = html_node_lock(current_node, &current_lock_acquired);
        if (lock_status != 0)
        {
            if (previous_node)
            {
                html_node_unlock(previous_node, previous_lock_acquired);
                previous_node = ft_nullptr;
                previous_lock_acquired = FT_FALSE;
            }
            current_node = next_node;
            continue ;
        }
        if (current_node->text && ft_strcmp(current_node->text, text_content) == 0)
        {
            if (previous_node)
                previous_node->next = next_node;
            else
                *node_list = next_node;
            current_node->next = ft_nullptr;
            html_node_unlock(current_node, current_lock_acquired);
            html_free_nodes(current_node);
            current_node = next_node;
            continue ;
        }
        html_remove_nodes_by_text(&current_node->children, text_content);
        if (previous_node)
            html_node_unlock(previous_node, previous_lock_acquired);
        previous_node = current_node;
        previous_lock_acquired = current_lock_acquired;
        current_node = next_node;
    }
    if (previous_node)
        html_node_unlock(previous_node, previous_lock_acquired);
    return ;
}
