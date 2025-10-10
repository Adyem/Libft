#include <cstdlib>
#include "parser.hpp"
#include "../Libft/libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../CMA/CMA.hpp"

static void release_html_string(char *string)
{
    if (!string)
        return ;
    int release_result = cma_checked_free(string);
    if (release_result != 0)
        return ;
    return ;
}

void html_free_nodes(html_node *nodeList)
{
    while (nodeList)
    {
        html_node *nextNode = nodeList->next;
        release_html_string(nodeList->tag);
        release_html_string(nodeList->text);
        html_attr *attribute = nodeList->attributes;
        while (attribute)
        {
            html_attr *nextAttr = attribute->next;
            release_html_string(attribute->key);
            release_html_string(attribute->value);
            delete attribute;
            attribute = nextAttr;
        }
        html_free_nodes(nodeList->children);
        delete nodeList;
        nodeList = nextNode;
    }
    return ;
}

void html_remove_nodes_by_tag(html_node **nodeList, const char *tagName)
{
    html_node *current;
    html_node *prev;

    if (nodeList == ft_nullptr || tagName == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return ;
    }
    current = *nodeList;
    prev = ft_nullptr;
    while (current)
    {
        html_node *next = current->next;
        if (current->tag && ft_strcmp(current->tag, tagName) == 0)
        {
            if (prev)
                prev->next = next;
            else
                *nodeList = next;
            current->next = ft_nullptr;
            html_free_nodes(current);
        }
        else
        {
            html_remove_nodes_by_tag(&current->children, tagName);
            prev = current;
        }
        current = next;
    }
    ft_errno = ER_SUCCESS;
}

void html_remove_nodes_by_attr(html_node **nodeList, const char *key, const char *value)
{
    html_node *current;
    html_node *prev;

    if (nodeList == ft_nullptr || key == ft_nullptr || value == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return ;
    }
    current = *nodeList;
    prev = ft_nullptr;
    while (current)
    {
        html_node *next = current->next;
        html_attr *attr = current->attributes;
        int match = 0;
        while (attr)
        {
            if (attr->key && attr->value &&
                ft_strcmp(attr->key, key) == 0 &&
                ft_strcmp(attr->value, value) == 0)
            {
                match = 1;
                break;
            }
            attr = attr->next;
        }
        if (match)
        {
            if (prev)
                prev->next = next;
            else
                *nodeList = next;
            current->next = ft_nullptr;
            html_free_nodes(current);
        }
        else
        {
            html_remove_nodes_by_attr(&current->children, key, value);
            prev = current;
        }
        current = next;
    }
    ft_errno = ER_SUCCESS;
}

void html_remove_nodes_by_text(html_node **nodeList, const char *textContent)
{
    html_node *current;
    html_node *prev;

    if (nodeList == ft_nullptr || textContent == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return ;
    }
    current = *nodeList;
    prev = ft_nullptr;
    while (current)
    {
        html_node *next = current->next;
        if (current->text && ft_strcmp(current->text, textContent) == 0)
        {
            if (prev)
                prev->next = next;
            else
                *nodeList = next;
            current->next = ft_nullptr;
            html_free_nodes(current);
        }
        else
        {
            html_remove_nodes_by_text(&current->children, textContent);
            prev = current;
        }
        current = next;
    }
    ft_errno = ER_SUCCESS;
}
