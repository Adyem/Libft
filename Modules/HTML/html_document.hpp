#ifndef HTML_DOCUMENT_HPP
#define HTML_DOCUMENT_HPP

#include "html_parser.hpp"
#include <cstdint>

class pt_recursive_mutex;

class html_document
{
#ifdef LIBFT_TEST_BUILD
    public:
#else
    private:
#endif
        html_node *_root;
        static thread_local int32_t _last_error;
        pt_recursive_mutex *_mutex;
        uint8_t _initialised_state;
        static int32_t set_error(int32_t error_code) noexcept;

    public:
        html_document() noexcept;
        html_document(const html_document &other) noexcept = delete;
        html_document(html_document &&other) noexcept = delete;
        ~html_document() noexcept;

        html_document &operator=(const html_document &) = delete;
        html_document &operator=(html_document &&) = delete;

        int32_t         initialize() noexcept;
        int32_t         initialize(const html_document &other) noexcept;
        int32_t         initialize(html_document &&other) noexcept;
        int32_t         destroy() noexcept;
        int32_t         move(html_document &other) noexcept;
        int32_t         enable_thread_safety() noexcept;
        int32_t         disable_thread_safety() noexcept;
        ft_bool         is_thread_safe() const noexcept;
        html_node   *create_node(const char *tag_name, const char *text_content) noexcept;
        html_attr   *create_attr(const char *key, const char *value) noexcept;
        void        add_attr(html_node *target_node, html_attr *new_attribute) noexcept;
        void        remove_attr(html_node *target_node, const char *key) noexcept;
        void        add_child(html_node *parent_node, html_node *child_node) noexcept;
        void        append_node(html_node *new_node) noexcept;
        int32_t         write_to_file(const char *file_path) const noexcept;
        char        *write_to_string() const noexcept;
        void        remove_nodes_by_tag(const char *tag_name) noexcept;
        void        remove_nodes_by_attr(const char *key, const char *value) noexcept;
        void        remove_nodes_by_text(const char *text_content) noexcept;
        html_node   *find_by_tag(const char *tag_name) const noexcept;
        html_node   *find_by_attr(const char *key, const char *value) const noexcept;
        html_node   *find_by_text(const char *text_content) const noexcept;
        html_node   *find_by_selector(const char *selector) const noexcept;
        ft_size_t      count_nodes_by_tag(const char *tag_name) const noexcept;
        html_node   *get_root() const noexcept;
        int32_t         get_error() const noexcept;
        const char  *get_error_str() const noexcept;
        void        clear() noexcept;
};

#endif
