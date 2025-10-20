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
        html_node *nextNode;
        html_node *childrenList;
        html_attr *attributeList;
        bool       node_lock_acquired;
        int        lock_status;

        nextNode = nodeList->next;
        childrenList = nodeList->children;
        attributeList = nodeList->attributes;
        node_lock_acquired = false;
        lock_status = html_node_lock(nodeList, &node_lock_acquired);
        if (lock_status == 0 && node_lock_acquired)
        {
            childrenList = nodeList->children;
            attributeList = nodeList->attributes;
            nodeList->children = ft_nullptr;
            nodeList->attributes = ft_nullptr;
            nodeList->next = ft_nullptr;
            html_node_unlock(nodeList, node_lock_acquired);
        }
        html_attr *currentAttribute;

        currentAttribute = attributeList;
        while (currentAttribute)
        {
            html_attr *nextAttribute;
            bool       attribute_lock_acquired;
            int        lock_attribute_status;

            attribute_lock_acquired = false;
            lock_attribute_status = html_attr_lock(currentAttribute, &attribute_lock_acquired);
            if (lock_attribute_status == 0 && attribute_lock_acquired)
            {
                nextAttribute = currentAttribute->next;
                currentAttribute->next = ft_nullptr;
                html_attr_unlock(currentAttribute, attribute_lock_acquired);
            }
            else
                nextAttribute = currentAttribute->next;
            release_html_string(currentAttribute->key);
            release_html_string(currentAttribute->value);
            html_attr_teardown_thread_safety(currentAttribute);
            delete currentAttribute;
            currentAttribute = nextAttribute;
        }
        html_free_nodes(childrenList);
        release_html_string(nodeList->tag);
        release_html_string(nodeList->text);
        html_node_teardown_thread_safety(nodeList);
        delete nodeList;
        nodeList = nextNode;
    }
    return ;
}

void html_remove_nodes_by_tag(html_node **nodeList, const char *tagName)
{
    html_node *currentNode;
    html_node *previousNode;
    bool       previous_lock_acquired;

    if (nodeList == ft_nullptr || tagName == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return ;
    }
    currentNode = *nodeList;
    previousNode = ft_nullptr;
    previous_lock_acquired = false;
    while (currentNode)
    {
        html_node *nextNode;
        bool       current_lock_acquired;
        int        lock_status;

        nextNode = currentNode->next;
        current_lock_acquired = false;
        lock_status = html_node_lock(currentNode, &current_lock_acquired);
        if (lock_status != 0)
        {
            if (previousNode)
            {
                html_node_unlock(previousNode, previous_lock_acquired);
                previousNode = ft_nullptr;
                previous_lock_acquired = false;
            }
            currentNode = nextNode;
            continue ;
        }
        if (currentNode->tag && ft_strcmp(currentNode->tag, tagName) == 0)
        {
            if (previousNode)
                previousNode->next = nextNode;
            else
                *nodeList = nextNode;
            currentNode->next = ft_nullptr;
            html_node_unlock(currentNode, current_lock_acquired);
            html_free_nodes(currentNode);
            currentNode = nextNode;
            continue ;
        }
        html_remove_nodes_by_tag(&currentNode->children, tagName);
        if (previousNode)
            html_node_unlock(previousNode, previous_lock_acquired);
        previousNode = currentNode;
        previous_lock_acquired = current_lock_acquired;
        currentNode = nextNode;
    }
    if (previousNode)
        html_node_unlock(previousNode, previous_lock_acquired);
    ft_errno = ER_SUCCESS;
}

void html_remove_nodes_by_attr(html_node **nodeList, const char *key, const char *value)
{
    html_node *currentNode;
    html_node *previousNode;
    bool       previous_lock_acquired;

    if (nodeList == ft_nullptr || key == ft_nullptr || value == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return ;
    }
    currentNode = *nodeList;
    previousNode = ft_nullptr;
    previous_lock_acquired = false;
    while (currentNode)
    {
        html_node *nextNode;
        html_attr *attributeNode;
        int        match_found;
        bool       current_lock_acquired;
        int        lock_status;

        nextNode = currentNode->next;
        current_lock_acquired = false;
        lock_status = html_node_lock(currentNode, &current_lock_acquired);
        if (lock_status != 0)
        {
            if (previousNode)
            {
                html_node_unlock(previousNode, previous_lock_acquired);
                previousNode = ft_nullptr;
                previous_lock_acquired = false;
            }
            currentNode = nextNode;
            continue ;
        }
        match_found = 0;
        attributeNode = currentNode->attributes;
        while (attributeNode)
        {
            bool       attribute_lock_acquired;
            int        attribute_lock_status;
            html_attr *next_attribute;

            attribute_lock_acquired = false;
            attribute_lock_status = html_attr_lock(attributeNode, &attribute_lock_acquired);
            if (attribute_lock_status != 0)
            {
                attributeNode = attributeNode->next;
                continue ;
            }
            next_attribute = attributeNode->next;
            if (attributeNode->key && attributeNode->value &&
                ft_strcmp(attributeNode->key, key) == 0 &&
                ft_strcmp(attributeNode->value, value) == 0)
            {
                match_found = 1;
                html_attr_unlock(attributeNode, attribute_lock_acquired);
                break;
            }
            html_attr_unlock(attributeNode, attribute_lock_acquired);
            attributeNode = next_attribute;
        }
        if (match_found)
        {
            if (previousNode)
                previousNode->next = nextNode;
            else
                *nodeList = nextNode;
            currentNode->next = ft_nullptr;
            html_node_unlock(currentNode, current_lock_acquired);
            html_free_nodes(currentNode);
            currentNode = nextNode;
            continue ;
        }
        html_remove_nodes_by_attr(&currentNode->children, key, value);
        if (previousNode)
            html_node_unlock(previousNode, previous_lock_acquired);
        previousNode = currentNode;
        previous_lock_acquired = current_lock_acquired;
        currentNode = nextNode;
    }
    if (previousNode)
        html_node_unlock(previousNode, previous_lock_acquired);
    ft_errno = ER_SUCCESS;
}

void html_remove_nodes_by_text(html_node **nodeList, const char *textContent)
{
    html_node *currentNode;
    html_node *previousNode;
    bool       previous_lock_acquired;

    if (nodeList == ft_nullptr || textContent == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return ;
    }
    currentNode = *nodeList;
    previousNode = ft_nullptr;
    previous_lock_acquired = false;
    while (currentNode)
    {
        html_node *nextNode;
        bool       current_lock_acquired;
        int        lock_status;

        nextNode = currentNode->next;
        current_lock_acquired = false;
        lock_status = html_node_lock(currentNode, &current_lock_acquired);
        if (lock_status != 0)
        {
            if (previousNode)
            {
                html_node_unlock(previousNode, previous_lock_acquired);
                previousNode = ft_nullptr;
                previous_lock_acquired = false;
            }
            currentNode = nextNode;
            continue ;
        }
        if (currentNode->text && ft_strcmp(currentNode->text, textContent) == 0)
        {
            if (previousNode)
                previousNode->next = nextNode;
            else
                *nodeList = nextNode;
            currentNode->next = ft_nullptr;
            html_node_unlock(currentNode, current_lock_acquired);
            html_free_nodes(currentNode);
            currentNode = nextNode;
            continue ;
        }
        html_remove_nodes_by_text(&currentNode->children, textContent);
        if (previousNode)
            html_node_unlock(previousNode, previous_lock_acquired);
        previousNode = currentNode;
        previous_lock_acquired = current_lock_acquired;
        currentNode = nextNode;
    }
    if (previousNode)
        html_node_unlock(previousNode, previous_lock_acquired);
    ft_errno = ER_SUCCESS;
}
