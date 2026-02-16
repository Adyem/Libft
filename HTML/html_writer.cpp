#include <fcntl.h>
#include "parser.hpp"
#include "../Errno/errno.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include "../Compatebility/compatebility_internal.hpp"

static void html_write_attrs(int fd, html_attr *attribute)
{
    while (attribute)
    {
        bool       lock_acquired;
        int        lock_status;
        html_attr *next_attribute;

        lock_acquired = false;
        lock_status = html_attr_lock(attribute, &lock_acquired);
        if (lock_status != 0)
            return ;
        next_attribute = attribute->next;
        if (attribute->key && attribute->value)
            pf_printf_fd(fd, " %s=\"%s\"", attribute->key, attribute->value);
        html_attr_unlock(attribute, lock_acquired);
        attribute = next_attribute;
    }
}

static void html_write_node(int fd, html_node *htmlNode, int indent)
{
    bool node_lock_acquired;
    int  lock_status;
    int  indent_index;

    node_lock_acquired = false;
    lock_status = html_node_lock(htmlNode, &node_lock_acquired);
    if (lock_status != 0)
        return ;
    indent_index = 0;
    while (indent_index < indent)
    {
        pf_printf_fd(fd, "  ");
        ++indent_index;
    }
    pf_printf_fd(fd, "<%s", htmlNode->tag);
    html_write_attrs(fd, htmlNode->attributes);
    if (!htmlNode->text && !htmlNode->children)
    {
        pf_printf_fd(fd, "/>\n");
        html_node_unlock(htmlNode, node_lock_acquired);
        return ;
    }
    pf_printf_fd(fd, ">");
    if (htmlNode->text)
        pf_printf_fd(fd, "%s", htmlNode->text);
    if (htmlNode->children)
    {
        html_node *childNode;

        pf_printf_fd(fd, "\n");
        childNode = htmlNode->children;
        while (childNode)
        {
            html_write_node(fd, childNode, indent + 1);
            childNode = childNode->next;
        }
        indent_index = 0;
        while (indent_index < indent)
        {
            pf_printf_fd(fd, "  ");
            ++indent_index;
        }
    }
    pf_printf_fd(fd, "</%s>\n", htmlNode->tag);
    html_node_unlock(htmlNode, node_lock_acquired);
}

int html_write_to_file(const char *file_path, html_node *node_list)
{
    int file_descriptor;
    html_node *current_node;
    html_node *next_node;
    bool       node_lock_acquired;
    int        lock_status;

    if (!file_path)
        return (-1);
    file_descriptor = su_open(file_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file_descriptor < 0)
        return (-1);
    current_node = node_list;
    while (current_node)
    {
        node_lock_acquired = false;
        lock_status = html_node_lock(current_node, &node_lock_acquired);
        if (lock_status == 0 && node_lock_acquired)
        {
            next_node = current_node->next;
            html_node_unlock(current_node, node_lock_acquired);
        }
        else
            next_node = current_node->next;
        html_write_node(file_descriptor, current_node, 0);
        current_node = next_node;
    }
    if (su_close(file_descriptor) != 0)
        return (-1);
    return (0);
}
