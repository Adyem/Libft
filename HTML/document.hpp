#ifndef HTML_DOCUMENT_HPP
#define HTML_DOCUMENT_HPP

#include "parser.hpp"

class html_document
{
    public:
        html_document() noexcept;
        ~html_document() noexcept;

        html_node   *create_node(const char *tag_name, const char *text_content) noexcept;
        html_attr   *create_attr(const char *key, const char *value) noexcept;
        void        add_attr(html_node *target_node, html_attr *new_attribute) noexcept;
        void        remove_attr(html_node *target_node, const char *key) noexcept;
        void        add_child(html_node *parent_node, html_node *child_node) noexcept;
        void        append_node(html_node *new_node) noexcept;
        int         write_to_file(const char *file_path) const noexcept;
        char        *write_to_string() const noexcept;
        void        remove_nodes_by_tag(const char *tag_name) noexcept;
        void        remove_nodes_by_attr(const char *key, const char *value) noexcept;
        void        remove_nodes_by_text(const char *text_content) noexcept;
        html_node   *find_by_tag(const char *tag_name) const noexcept;
        html_node   *find_by_attr(const char *key, const char *value) const noexcept;
        html_node   *find_by_text(const char *text_content) const noexcept;
        size_t      count_nodes_by_tag(const char *tag_name) const noexcept;
        void        clear() noexcept;

    private:
        html_node *_root;
};

#endif
