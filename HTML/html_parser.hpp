#ifndef HTML_PARSER_HPP
#define HTML_PARSER_HPP

#include "../Errno/errno.hpp"

class pt_mutex;

typedef struct html_attr
{
    pt_mutex *mutex;
    ft_bool thread_safe_enabled;
    char *key;
    char *value;
    struct html_attr *next;
} html_attr;

typedef struct html_node
{
    pt_mutex *mutex;
    ft_bool thread_safe_enabled;
    char *tag;
    char *text;
    html_attr *attributes;
    struct html_node *children;
    struct html_node *next;
} html_node;

html_node   *html_create_node(const char *tag_name, const char *text_content);
void        html_add_child(html_node *parent_node, html_node *child_node);
void        html_append_node(html_node **head_node, html_node *new_node);
html_attr   *html_create_attr(const char *key, const char *value);
void        html_add_attr(html_node *target_node, html_attr *new_attribute);
void        html_remove_attr(html_node *target_node, const char *key);
int32_t     html_write_to_file(const char *file_path, html_node *node_list);
char        *html_write_to_string(html_node *node_list);
void        html_free_nodes(html_node *node_list);
void        html_remove_nodes_by_tag(html_node **node_list, const char *tag_name);
void        html_remove_nodes_by_attr(html_node **node_list, const char *key, const char *value);
void        html_remove_nodes_by_text(html_node **node_list, const char *text_content);
html_node   *html_find_by_tag(html_node *node_list, const char *tag_name);
html_node   *html_find_by_attr(html_node *node_list, const char *key, const char *value);
html_node   *html_find_by_text(html_node *node_list, const char *text_content);
html_node   *html_find_by_selector(html_node *node_list, const char *selector);

html_node   *html_query_selector(html_node *node_list, const char *selector);
ft_size_t   html_count_nodes_by_tag(html_node *node_list, const char *tag_name);

int32_t     html_node_prepare_thread_safety(html_node *node);
void        html_node_teardown_thread_safety(html_node *node);
int32_t     html_node_lock(const html_node *node, ft_bool *lock_acquired);
void        html_node_unlock(const html_node *node, ft_bool lock_acquired);
ft_bool     html_node_is_thread_safe_enabled(const html_node *node);

int32_t     html_attr_prepare_thread_safety(html_attr *attribute);
void        html_attr_teardown_thread_safety(html_attr *attribute);
int32_t     html_attr_lock(const html_attr *attribute, ft_bool *lock_acquired);
void        html_attr_unlock(const html_attr *attribute, ft_bool lock_acquired);
ft_bool     html_attr_is_thread_safe_enabled(const html_attr *attribute);

#endif
