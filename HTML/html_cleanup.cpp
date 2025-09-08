#include "html_parser.hpp"
#include "../Libft/libft.hpp"
#include "../CPP_class/cpp_class_nullptr.hpp"

void html_free_nodes(html_node *nodeList)
{
    while (nodeList)
    {
        html_node *nextNode = nodeList->next;
        if (nodeList->tag)
            delete[] nodeList->tag;
        if (nodeList->text)
            delete[] nodeList->text;
        html_attr *attribute = nodeList->attributes;
        while (attribute)
        {
            html_attr *nextAttr = attribute->next;
            if (attribute->key)
                delete[] attribute->key;
            if (attribute->value)
                delete[] attribute->value;
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
    html_node *current = *nodeList;
    html_node *prev = ft_nullptr;
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
}

void html_remove_nodes_by_attr(html_node **nodeList, const char *key, const char *value)
{
    html_node *current = *nodeList;
    html_node *prev = ft_nullptr;
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
}

void html_remove_nodes_by_text(html_node **nodeList, const char *textContent)
{
    html_node *current = *nodeList;
    html_node *prev = ft_nullptr;
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
}
