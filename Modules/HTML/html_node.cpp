#include <cstdlib>
#include <new>
#include "html_parser.hpp"
#include "../CMA/CMA.hpp"
#include "../Basic/basic.hpp"
#include "../Advanced/advanced.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/limits.hpp"

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
    html_attr *current_attribute;

    if (target_node == ft_nullptr || new_attribute == ft_nullptr)
        return ;
    if (!target_node->attributes)
    {
        target_node->attributes = new_attribute;
        return ;
    }
    current_attribute = target_node->attributes;
    while (current_attribute->next)
        current_attribute = current_attribute->next;
    current_attribute->next = new_attribute;
    return ;
}

void html_remove_attr(html_node *target_node, const char *key)
{
    html_attr *previous_attribute;
    html_attr *current_attribute;

    if (target_node == ft_nullptr || key == ft_nullptr)
        return ;
    previous_attribute = ft_nullptr;
    current_attribute = target_node->attributes;
    while (current_attribute)
    {
        html_attr *next_attribute;

        next_attribute = current_attribute->next;
        if (current_attribute->key && ft_strcmp(current_attribute->key, key) == 0)
        {
            if (previous_attribute)
                previous_attribute->next = next_attribute;
            else
                target_node->attributes = next_attribute;
            html_release_string(current_attribute->key);
            html_release_string(current_attribute->value);
            delete current_attribute;
            return ;
        }
        previous_attribute = current_attribute;
        current_attribute = next_attribute;
    }
    return ;
}

void html_add_child(html_node *parent_node, html_node *child_node)
{
    html_node *last_child;

    if (parent_node == ft_nullptr || child_node == ft_nullptr)
        return ;
    if (!parent_node->children)
    {
        parent_node->children = child_node;
        return ;
    }
    last_child = parent_node->children;
    while (last_child->next)
        last_child = last_child->next;
    last_child->next = child_node;
    return ;
}

void html_append_node(html_node **head_node, html_node *new_node)
{
    html_node *current_node;

    if (!head_node || new_node == ft_nullptr)
        return ;
    if (!(*head_node))
    {
        *head_node = new_node;
        return ;
    }
    current_node = *head_node;
    while (current_node->next)
        current_node = current_node->next;
    current_node->next = new_node;
    return ;
}
