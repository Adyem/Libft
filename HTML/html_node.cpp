#include <new>
#include "html_parser.hpp"
#include "../CMA/CMA.hpp"
#include "../Libft/libft.hpp"
#include "../CPP_class/nullptr.hpp"
#include "../Errno/errno.hpp"

#define HTML_MALLOC_FAIL 1001

html_node *html_create_node(const char *tagName, const char *textContent)
{
    html_node *newNode = new(std::nothrow) html_node;
    if (!newNode)
    {
        ft_errno = HTML_MALLOC_FAIL;
        return (ft_nullptr);
    }
    newNode->tag = cma_strdup(tagName);
    if (!newNode->tag)
    {
        delete newNode;
        ft_errno = HTML_MALLOC_FAIL;
        return (ft_nullptr);
    }
    if (textContent)
        newNode->text = cma_strdup(textContent);
    else
        newNode->text = ft_nullptr;
    if (textContent && !newNode->text)
    {
        delete[] newNode->tag;
        delete newNode;
        ft_errno = HTML_MALLOC_FAIL;
        return (ft_nullptr);
    }
    newNode->attributes = ft_nullptr;
    newNode->children = ft_nullptr;
    newNode->next = ft_nullptr;
    return (newNode);
}

html_attr *html_create_attr(const char *key, const char *value)
{
    html_attr *newAttr = new(std::nothrow) html_attr;
    if (!newAttr)
    {
        ft_errno = HTML_MALLOC_FAIL;
        return (ft_nullptr);
    }
    newAttr->key = cma_strdup(key);
    if (!newAttr->key)
    {
        delete newAttr;
        ft_errno = HTML_MALLOC_FAIL;
        return (ft_nullptr);
    }
    newAttr->value = cma_strdup(value);
    if (!newAttr->value)
    {
        delete[] newAttr->key;
        delete newAttr;
        ft_errno = HTML_MALLOC_FAIL;
        return (ft_nullptr);
    }
    newAttr->next = ft_nullptr;
    return (newAttr);
}

void html_add_attr(html_node *targetNode, html_attr *newAttribute)
{
    if (!targetNode->attributes)
        targetNode->attributes = newAttribute;
    else
    {
        html_attr *currentAttribute = targetNode->attributes;
        while (currentAttribute->next)
            currentAttribute = currentAttribute->next;
        currentAttribute->next = newAttribute;
    }
    return ;
}

void html_remove_attr(html_node *targetNode, const char *key)
{
    html_attr *prev = ft_nullptr;
    html_attr *current = targetNode->attributes;
    while (current)
    {
        if (current->key && ft_strcmp(current->key, key) == 0)
        {
            if (prev)
                prev->next = current->next;
            else
                targetNode->attributes = current->next;
            if (current->key)
                delete[] current->key;
            if (current->value)
                delete[] current->value;
            delete current;
            return ;
        }
        prev = current;
        current = current->next;
    }
    return ;
}

void html_add_child(html_node *parentNode, html_node *childNode)
{
    if (!parentNode->children)
        parentNode->children = childNode;
    else
    {
        html_node *lastChild = parentNode->children;
        while (lastChild->next)
            lastChild = lastChild->next;
        lastChild->next = childNode;
    }
    return ;
}

void html_append_node(html_node **headNode, html_node *newNode)
{
    if (!(*headNode))
        *headNode = newNode;
    else
    {
        html_node *lastNode = *headNode;
        while (lastNode->next)
            lastNode = lastNode->next;
        lastNode->next = newNode;
    }
    return ;
}
