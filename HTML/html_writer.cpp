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
        pf_printf_fd(fd, " %s=\"%s\"", attribute->key, attribute->value);
        attribute = attribute->next;
    }
}

static void html_write_node(int fd, html_node *htmlNode, int indent)
{
    int indent_index = 0;
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
        return ;
    }
    pf_printf_fd(fd, ">");
    if (htmlNode->text)
        pf_printf_fd(fd, "%s", htmlNode->text);
    if (htmlNode->children)
    {
        pf_printf_fd(fd, "\n");
        html_node *childNode = htmlNode->children;
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
}

int html_write_to_file(const char *file_path, html_node *node_list)
{
    int file_descriptor;
    html_node *current_node;

    if (!file_path)
    {
        ft_errno = FT_EINVAL;
        return (-1);
    }
    file_descriptor = su_open(file_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file_descriptor < 0)
        return (-1);
    current_node = node_list;
    while (current_node)
    {
        html_write_node(file_descriptor, current_node, 0);
        current_node = current_node->next;
    }
    if (cmp_close(file_descriptor) != 0)
        return (-1);
    ft_errno = ER_SUCCESS;
    return (0);
}
