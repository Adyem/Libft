#include <cstdlib>
#include <new>
#include "parser.hpp"
#include "../CMA/CMA.hpp"
#include "../Libft/libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

static void html_release_string(char *string)
{
    if (!string)
        return ;
    int release_result = cma_checked_free(string);
    if (release_result != 0)
        return ;
    return ;
}

#define HTML_MALLOC_FAIL 1001

html_node *html_create_node(const char *tagName, const char *textContent)
{
    html_node *newNode;

    newNode = new(std::nothrow) html_node;
    if (!newNode)
    {
        ft_errno = HTML_MALLOC_FAIL;
        return (ft_nullptr);
    }
    newNode->mutex = ft_nullptr;
    newNode->thread_safe_enabled = false;
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
        html_release_string(newNode->tag);
        delete newNode;
        ft_errno = HTML_MALLOC_FAIL;
        return (ft_nullptr);
    }
    newNode->attributes = ft_nullptr;
    newNode->children = ft_nullptr;
    newNode->next = ft_nullptr;
    if (html_node_prepare_thread_safety(newNode) != 0)
    {
        html_release_string(newNode->tag);
        html_release_string(newNode->text);
        delete newNode;
        return (ft_nullptr);
    }
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
        html_release_string(newAttr->key);
        delete newAttr;
        ft_errno = HTML_MALLOC_FAIL;
        return (ft_nullptr);
    }
    newAttr->next = ft_nullptr;
    return (newAttr);
}

void html_add_attr(html_node *targetNode, html_attr *newAttribute)
{
    bool lock_acquired;
    int  lock_status;

    lock_acquired = false;
    lock_status = html_node_lock(targetNode, &lock_acquired);
    if (lock_status != 0)
        return ;
    if (!targetNode->attributes)
        targetNode->attributes = newAttribute;
    else
    {
        html_attr *currentAttribute;

        currentAttribute = targetNode->attributes;
        while (currentAttribute->next)
            currentAttribute = currentAttribute->next;
        currentAttribute->next = newAttribute;
    }
    html_node_unlock(targetNode, lock_acquired);
    return ;
}

void html_remove_attr(html_node *targetNode, const char *key)
{
    html_attr *previousAttribute;
    html_attr *currentAttribute;
    bool       lock_acquired;
    int        lock_status;

    lock_acquired = false;
    lock_status = html_node_lock(targetNode, &lock_acquired);
    if (lock_status != 0)
        return ;
    previousAttribute = ft_nullptr;
    currentAttribute = targetNode->attributes;
    while (currentAttribute)
    {
        if (currentAttribute->key && ft_strcmp(currentAttribute->key, key) == 0)
        {
            if (previousAttribute)
                previousAttribute->next = currentAttribute->next;
            else
                targetNode->attributes = currentAttribute->next;
            html_release_string(currentAttribute->key);
            html_release_string(currentAttribute->value);
            delete currentAttribute;
            html_node_unlock(targetNode, lock_acquired);
            return ;
        }
        previousAttribute = currentAttribute;
        currentAttribute = currentAttribute->next;
    }
    html_node_unlock(targetNode, lock_acquired);
    return ;
}

void html_add_child(html_node *parentNode, html_node *childNode)
{
    bool parent_lock_acquired;
    int  lock_status;

    parent_lock_acquired = false;
    lock_status = html_node_lock(parentNode, &parent_lock_acquired);
    if (lock_status != 0)
        return ;
    if (!parentNode->children)
    {
        parentNode->children = childNode;
        html_node_unlock(parentNode, parent_lock_acquired);
        return ;
    }
    html_node *lastChild;
    bool       child_lock_acquired;

    lastChild = parentNode->children;
    child_lock_acquired = false;
    lock_status = html_node_lock(lastChild, &child_lock_acquired);
    if (lock_status != 0)
    {
        html_node_unlock(parentNode, parent_lock_acquired);
        return ;
    }
    while (lastChild->next)
    {
        html_node *nextChild;
        bool       next_lock_acquired;

        nextChild = lastChild->next;
        next_lock_acquired = false;
        lock_status = html_node_lock(nextChild, &next_lock_acquired);
        if (lock_status != 0)
        {
            html_node_unlock(lastChild, child_lock_acquired);
            html_node_unlock(parentNode, parent_lock_acquired);
            return ;
        }
        html_node_unlock(lastChild, child_lock_acquired);
        lastChild = nextChild;
        child_lock_acquired = next_lock_acquired;
    }
    lastChild->next = childNode;
    html_node_unlock(lastChild, child_lock_acquired);
    html_node_unlock(parentNode, parent_lock_acquired);
    return ;
}

void html_append_node(html_node **headNode, html_node *newNode)
{
    html_node *currentNode;
    bool       current_lock_acquired;
    int        lock_status;

    if (!headNode)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return ;
    }
    if (!(*headNode))
    {
        *headNode = newNode;
        return ;
    }
    currentNode = *headNode;
    current_lock_acquired = false;
    lock_status = html_node_lock(currentNode, &current_lock_acquired);
    if (lock_status != 0)
        return ;
    while (currentNode->next)
    {
        html_node *nextNode;
        bool       next_lock_acquired;

        nextNode = currentNode->next;
        next_lock_acquired = false;
        lock_status = html_node_lock(nextNode, &next_lock_acquired);
        if (lock_status != 0)
        {
            html_node_unlock(currentNode, current_lock_acquired);
            return ;
        }
        html_node_unlock(currentNode, current_lock_acquired);
        currentNode = nextNode;
        current_lock_acquired = next_lock_acquired;
    }
    currentNode->next = newNode;
    html_node_unlock(currentNode, current_lock_acquired);
    return ;
}
