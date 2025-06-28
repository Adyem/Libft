#include <new>
#include <fcntl.h>
#include "html_parser.hpp"
#include "../CMA/CMA.hpp"
#include "../Printf/printf.hpp"
#include "../Linux/linux_file.hpp"
#include "../CPP_class/nullptr.hpp"
#include "../Errno/errno.hpp"

#define HTML_MALLOC_FAIL 1001

html_node *html_create_node(const char *tag, const char *text)
{
    html_node *node = new(std::nothrow) html_node;
    if (!node)
    {
        ft_errno = HTML_MALLOC_FAIL;
        return (ft_nullptr);
    }
    node->tag = cma_strdup(tag);
    if (!node->tag)
    {
        delete node;
        ft_errno = HTML_MALLOC_FAIL;
        return (ft_nullptr);
    }
    if (text)
        node->text = cma_strdup(text);
    else
        node->text = ft_nullptr;
    if (text && !node->text)
    {
        delete[] node->tag;
        delete node;
        ft_errno = HTML_MALLOC_FAIL;
        return (ft_nullptr);
    }
    node->attributes = ft_nullptr;
    node->children = ft_nullptr;
    node->next = ft_nullptr;
    return (node);
}

html_attr *html_create_attr(const char *key, const char *value)
{
    html_attr *attr = new(std::nothrow) html_attr;
    if (!attr)
    {
        ft_errno = HTML_MALLOC_FAIL;
        return (ft_nullptr);
    }
    attr->key = cma_strdup(key);
    if (!attr->key)
    {
        delete attr;
        ft_errno = HTML_MALLOC_FAIL;
        return (ft_nullptr);
    }
    attr->value = cma_strdup(value);
    if (!attr->value)
    {
        delete[] attr->key;
        delete attr;
        ft_errno = HTML_MALLOC_FAIL;
        return (ft_nullptr);
    }
    attr->next = ft_nullptr;
    return (attr);
}

void html_add_attr(html_node *node, html_attr *attr)
{
    if (!node->attributes)
        node->attributes = attr;
    else
    {
        html_attr *current = node->attributes;
        while (current->next)
            current = current->next;
        current->next = attr;
    }
    return ;
}

void html_add_child(html_node *parent, html_node *child)
{
    if (!parent->children)
        parent->children = child;
    else
    {
        html_node *current = parent->children;
        while (current->next)
            current = current->next;
        current->next = child;
    }
    return ;
}

void html_append_node(html_node **head, html_node *new_node)
{
    if (!(*head))
        *head = new_node;
    else
    {
        html_node *current = *head;
        while (current->next)
            current = current->next;
        current->next = new_node;
    }
    return ;
}

static void html_write_attrs(int fd, html_attr *attr)
{
    while (attr)
    {
        pf_printf_fd(fd, " %s=\"%s\"", attr->key, attr->value);
        attr = attr->next;
    }
}

static void html_write_node(int fd, html_node *node, int indent)
{
    for (int i = 0; i < indent; ++i)
        pf_printf_fd(fd, "  ");
    pf_printf_fd(fd, "<%s", node->tag);
    html_write_attrs(fd, node->attributes);
    if (!node->text && !node->children)
    {
        pf_printf_fd(fd, "/>\n");
        return ;
    }
    pf_printf_fd(fd, ">");
    if (node->text)
        pf_printf_fd(fd, "%s", node->text);
    if (node->children)
    {
        pf_printf_fd(fd, "\n");
        html_node *child = node->children;
        while (child)
        {
            html_write_node(fd, child, indent + 1);
            child = child->next;
        }
        for (int i = 0; i < indent; ++i)
            pf_printf_fd(fd, "  ");
    }
    pf_printf_fd(fd, "</%s>\n", node->tag);
}

int html_write_to_file(const char *filename, html_node *nodes)
{
    int fd = ft_open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0)
        return (-1);
    html_node *current = nodes;
    while (current)
    {
        html_write_node(fd, current, 0);
        current = current->next;
    }
    ft_close(fd);
    return (0);
}

void html_free_nodes(html_node *node)
{
    while (node)
    {
        html_node *next = node->next;
        if (node->tag)
            delete[] node->tag;
        if (node->text)
            delete[] node->text;
        html_attr *attr = node->attributes;
        while (attr)
        {
            html_attr *next_attr = attr->next;
            if (attr->key)
                delete[] attr->key;
            if (attr->value)
                delete[] attr->value;
            delete attr;
            attr = next_attr;
        }
        html_free_nodes(node->children);
        delete node;
        node = next;
    }
}

