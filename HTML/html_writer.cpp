#include <fcntl.h>
#include "html_parser.hpp"
#include "../Printf/printf.hpp"
#include "../Linux/linux_file.hpp"

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
    for (int i = 0; i < indent; ++i)
        pf_printf_fd(fd, "  ");
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
        for (int i = 0; i < indent; ++i)
            pf_printf_fd(fd, "  ");
    }
    pf_printf_fd(fd, "</%s>\n", htmlNode->tag);
}

int html_write_to_file(const char *filePath, html_node *nodeList)
{
    int fd = ft_open(filePath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0)
        return (-1);
    html_node *currentNode = nodeList;
    while (currentNode)
    {
        html_write_node(fd, currentNode, 0);
        currentNode = currentNode->next;
    }
    ft_close(fd);
    return (0);
}
