#include <fcntl.h>
#include "html_parser.hpp"
#include "../Errno/errno.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include "../Compatebility/compatebility_internal.hpp"

static void html_write_attrs(int32_t file_descriptor, html_attr *attribute)
{
    while (attribute)
    {
        html_attr *next_attribute;

        next_attribute = attribute->next;
        if (attribute->key && attribute->value)
            pf_printf_fd(file_descriptor, " %s=\"%s\"", attribute->key, attribute->value);
        attribute = next_attribute;
    }
}

static void html_write_node(int32_t file_descriptor, html_node *node_item, int32_t indent)
{
    int32_t  indent_index;

    indent_index = 0;
    while (indent_index < indent)
    {
        pf_printf_fd(file_descriptor, "  ");
        ++indent_index;
    }
    pf_printf_fd(file_descriptor, "<%s", node_item->tag);
    html_write_attrs(file_descriptor, node_item->attributes);
    if (!node_item->text && !node_item->children)
    {
        pf_printf_fd(file_descriptor, "/>\n");
        return ;
    }
    pf_printf_fd(file_descriptor, ">");
    if (node_item->text)
        pf_printf_fd(file_descriptor, "%s", node_item->text);
    if (node_item->children)
    {
        html_node *child_node;

        pf_printf_fd(file_descriptor, "\n");
        child_node = node_item->children;
        while (child_node)
        {
            html_write_node(file_descriptor, child_node, indent + 1);
            child_node = child_node->next;
        }
        indent_index = 0;
        while (indent_index < indent)
        {
            pf_printf_fd(file_descriptor, "  ");
            ++indent_index;
        }
    }
    pf_printf_fd(file_descriptor, "</%s>\n", node_item->tag);
}

int32_t html_write_to_file(const char *file_path, html_node *node_list)
{
    int32_t file_descriptor;
    html_node *current_node;
    html_node *next_node;

    if (!file_path)
        return (FT_ERR_INTERNAL);
    file_descriptor = su_open(file_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file_descriptor < 0)
        return (FT_ERR_INTERNAL);
    current_node = node_list;
    while (current_node)
    {
        next_node = current_node->next;
        html_write_node(file_descriptor, current_node, 0);
        current_node = next_node;
    }
    if (su_close(file_descriptor) != 0)
        return (FT_ERR_INTERNAL);
    return (FT_ERR_SUCCESS);
}
