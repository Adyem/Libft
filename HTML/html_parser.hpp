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

html_node   *html_create_node(const char *tagName, const char *textContent);
void        html_add_child(html_node *parentNode, html_node *childNode);
void        html_append_node(html_node **headNode, html_node *newNode);
html_attr   *html_create_attr(const char *key, const char *value);
void        html_add_attr(html_node *targetNode, html_attr *newAttribute);
void        html_remove_attr(html_node *targetNode, const char *key);
int         html_write_to_file(const char *filePath, html_node *nodeList);
char        *html_write_to_string(html_node *nodeList);
void        html_free_nodes(html_node *nodeList);
html_node   *html_find_by_tag(html_node *nodeList, const char *tagName);
html_node   *html_find_by_attr(html_node *nodeList, const char *key, const char *value);

#endif
