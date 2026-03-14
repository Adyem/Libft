#include <cstdlib>
#include <new>
#include "html_parser.hpp"
#include "../CMA/CMA.hpp"
#include "../Basic/basic.hpp"
#include "../Advanced/advanced.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

static void html_release_string(char *string)
{
    if (!string)
        return ;
    int32_t release_result = cma_checked_free(string);
    if (release_result != 0)
        return ;
    return ;
}


html_node *html_create_node(const char *tag_name, const char *text_content)
{
    html_node *new_node;

    new_node = new(std::nothrow) html_node;
    if (!new_node)
        return (ft_nullptr);
    new_node->mutex = ft_nullptr;
    new_node->thread_safe_enabled = FT_FALSE;
    new_node->tag = adv_strdup(tag_name);
    if (!new_node->tag)
    {
        delete new_node;
        return (ft_nullptr);
    }
    if (text_content)
        new_node->text = adv_strdup(text_content);
    else
        new_node->text = ft_nullptr;
    if (text_content && !new_node->text)
    {
        html_release_string(new_node->tag);
        delete new_node;
        return (ft_nullptr);
    }
    new_node->attributes = ft_nullptr;
    new_node->children = ft_nullptr;
    new_node->next = ft_nullptr;
    return (new_node);
}

html_attr *html_create_attr(const char *key, const char *value)
{
    html_attr *new_attr = new(std::nothrow) html_attr;
    if (!new_attr)
        return (ft_nullptr);
    new_attr->mutex = ft_nullptr;
    new_attr->thread_safe_enabled = FT_FALSE;
    new_attr->key = adv_strdup(key);
    if (!new_attr->key)
    {
        delete new_attr;
        return (ft_nullptr);
    }
    new_attr->value = adv_strdup(value);
    if (!new_attr->value)
    {
        html_release_string(new_attr->key);
        delete new_attr;
        return (ft_nullptr);
    }
    new_attr->next = ft_nullptr;
    return (new_attr);
}

void html_add_attr(html_node *target_node, html_attr *new_attribute)
{
    ft_bool lock_acquired;
    int32_t  lock_status;

    lock_acquired = FT_FALSE;
    lock_status = html_node_lock(target_node, &lock_acquired);
    if (lock_status != 0)
        return ;
    if (!target_node->attributes)
        target_node->attributes = new_attribute;
    else
    {
        html_attr *current_attribute;
        ft_bool       attribute_lock_acquired;
        int32_t        attribute_lock_status;

        current_attribute = target_node->attributes;
        attribute_lock_acquired = FT_FALSE;
        attribute_lock_status = html_attr_lock(current_attribute, &attribute_lock_acquired);
        if (attribute_lock_status != 0)
        {
            html_node_unlock(target_node, lock_acquired);
            return ;
        }
        while (current_attribute->next)
        {
            html_attr *next_attribute;
            ft_bool       next_lock_acquired;

            next_attribute = current_attribute->next;
            next_lock_acquired = FT_FALSE;
            attribute_lock_status = html_attr_lock(next_attribute, &next_lock_acquired);
            if (attribute_lock_status != 0)
            {
                html_attr_unlock(current_attribute, attribute_lock_acquired);
                html_node_unlock(target_node, lock_acquired);
                return ;
            }
            html_attr_unlock(current_attribute, attribute_lock_acquired);
            current_attribute = next_attribute;
            attribute_lock_acquired = next_lock_acquired;
        }
        current_attribute->next = new_attribute;
        html_attr_unlock(current_attribute, attribute_lock_acquired);
    }
    html_node_unlock(target_node, lock_acquired);
    return ;
}

void html_remove_attr(html_node *target_node, const char *key)
{
    html_attr *previous_attribute;
    html_attr *current_attribute;
    ft_bool       lock_acquired;
    int32_t        lock_status;
    ft_bool       previous_lock_acquired;

    lock_acquired = FT_FALSE;
    lock_status = html_node_lock(target_node, &lock_acquired);
    if (lock_status != 0)
        return ;
    previous_attribute = ft_nullptr;
    previous_lock_acquired = FT_FALSE;
    current_attribute = target_node->attributes;
    while (current_attribute)
    {
        ft_bool       current_lock_acquired;
        int32_t        attribute_lock_status;
        html_attr *next_attribute;

        current_lock_acquired = FT_FALSE;
        attribute_lock_status = html_attr_lock(current_attribute, &current_lock_acquired);
        if (attribute_lock_status != 0)
        {
            if (previous_attribute)
                html_attr_unlock(previous_attribute, previous_lock_acquired);
            html_node_unlock(target_node, lock_acquired);
            return ;
        }
        next_attribute = current_attribute->next;
        if (current_attribute->key && ft_strcmp(current_attribute->key, key) == 0)
        {
            if (previous_attribute)
            {
                previous_attribute->next = next_attribute;
                html_attr_unlock(previous_attribute, previous_lock_acquired);
            }
            else
                target_node->attributes = next_attribute;
            html_attr_unlock(current_attribute, current_lock_acquired);
            html_attr_teardown_thread_safety(current_attribute);
            html_release_string(current_attribute->key);
            html_release_string(current_attribute->value);
            delete current_attribute;
            html_node_unlock(target_node, lock_acquired);
            return ;
        }
        if (previous_attribute)
            html_attr_unlock(previous_attribute, previous_lock_acquired);
        previous_attribute = current_attribute;
        previous_lock_acquired = current_lock_acquired;
        current_attribute = next_attribute;
    }
    if (previous_attribute)
        html_attr_unlock(previous_attribute, previous_lock_acquired);
    html_node_unlock(target_node, lock_acquired);
    return ;
}

void html_add_child(html_node *parent_node, html_node *child_node)
{
    ft_bool parent_lock_acquired;
    int32_t  lock_status;

    parent_lock_acquired = FT_FALSE;
    lock_status = html_node_lock(parent_node, &parent_lock_acquired);
    if (lock_status != 0)
        return ;
    if (!parent_node->children)
    {
        parent_node->children = child_node;
        html_node_unlock(parent_node, parent_lock_acquired);
        return ;
    }
    html_node *last_child;
    ft_bool       child_lock_acquired;

    last_child = parent_node->children;
    child_lock_acquired = FT_FALSE;
    lock_status = html_node_lock(last_child, &child_lock_acquired);
    if (lock_status != 0)
    {
        html_node_unlock(parent_node, parent_lock_acquired);
        return ;
    }
    while (last_child->next)
    {
        html_node *next_child;
        ft_bool       next_lock_acquired;

        next_child = last_child->next;
        next_lock_acquired = FT_FALSE;
        lock_status = html_node_lock(next_child, &next_lock_acquired);
        if (lock_status != 0)
        {
            html_node_unlock(last_child, child_lock_acquired);
            html_node_unlock(parent_node, parent_lock_acquired);
            return ;
        }
        html_node_unlock(last_child, child_lock_acquired);
        last_child = next_child;
        child_lock_acquired = next_lock_acquired;
    }
    last_child->next = child_node;
    html_node_unlock(last_child, child_lock_acquired);
    html_node_unlock(parent_node, parent_lock_acquired);
    return ;
}

void html_append_node(html_node **head_node, html_node *new_node)
{
    html_node *current_node;
    ft_bool       current_lock_acquired;
    int32_t        lock_status;

    if (!head_node)
        return ;
    if (!(*head_node))
    {
        *head_node = new_node;
        return ;
    }
    current_node = *head_node;
    current_lock_acquired = FT_FALSE;
    lock_status = html_node_lock(current_node, &current_lock_acquired);
    if (lock_status != 0)
        return ;
    while (current_node->next)
    {
        html_node *next_node;
        ft_bool       next_lock_acquired;

        next_node = current_node->next;
        next_lock_acquired = FT_FALSE;
        lock_status = html_node_lock(next_node, &next_lock_acquired);
        if (lock_status != 0)
        {
            html_node_unlock(current_node, current_lock_acquired);
            return ;
        }
        html_node_unlock(current_node, current_lock_acquired);
        current_node = next_node;
        current_lock_acquired = next_lock_acquired;
    }
    current_node->next = new_node;
    html_node_unlock(current_node, current_lock_acquired);
    return ;
}
