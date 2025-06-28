#ifndef HTML_PARSER_HPP
#define HTML_PARSER_HPP

#include <cstddef>

typedef struct html_attr
{
    char *key;
    char *value;
    struct html_attr *next;
} html_attr;

typedef struct html_node
{
    char *tag;
    char *text;
    html_attr *attributes;
    struct html_node *children;
    struct html_node *next;
} html_node;

html_node   *html_create_node(const char *tag, const char *text);
void        html_add_child(html_node *parent, html_node *child);
void        html_append_node(html_node **head, html_node *new_node);
html_attr   *html_create_attr(const char *key, const char *value);
void        html_add_attr(html_node *node, html_attr *attr);
int         html_write_to_file(const char *filename, html_node *nodes);
void        html_free_nodes(html_node *node);

#endif
