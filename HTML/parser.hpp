#ifndef HTML_PARSER_HPP
#define HTML_PARSER_HPP

#include <cstddef>

class pt_mutex;

typedef struct html_attr
{
    pt_mutex *mutex;
    bool thread_safe_enabled;
    char *key;
    char *value;
    struct html_attr *next;
} html_attr;

typedef struct html_node
{
    pt_mutex *mutex;
    bool thread_safe_enabled;
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
void        html_remove_nodes_by_tag(html_node **nodeList, const char *tagName);
void        html_remove_nodes_by_attr(html_node **nodeList, const char *key, const char *value);
void        html_remove_nodes_by_text(html_node **nodeList, const char *textContent);
html_node   *html_find_by_tag(html_node *nodeList, const char *tagName);
html_node   *html_find_by_attr(html_node *nodeList, const char *key, const char *value);
html_node   *html_find_by_text(html_node *nodeList, const char *textContent);
html_node   *html_find_by_selector(html_node *node_list, const char *selector);

html_node   *html_query_selector(html_node *node_list, const char *selector);
size_t      html_count_nodes_by_tag(html_node *nodeList, const char *tagName);

int         html_node_prepare_thread_safety(html_node *node);
void        html_node_teardown_thread_safety(html_node *node);
int         html_node_lock(const html_node *node, bool *lock_acquired);
void        html_node_unlock(const html_node *node, bool lock_acquired);
bool        html_node_is_thread_safe_enabled(const html_node *node);

int         html_attr_prepare_thread_safety(html_attr *attribute);
void        html_attr_teardown_thread_safety(html_attr *attribute);
int         html_attr_lock(const html_attr *attribute, bool *lock_acquired);
void        html_attr_unlock(const html_attr *attribute, bool lock_acquired);
bool        html_attr_is_thread_safe_enabled(const html_attr *attribute);

#endif
