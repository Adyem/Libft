#ifndef HTML_DOCUMENT_HPP
#define HTML_DOCUMENT_HPP

#include "parser.hpp"
#include <cstdint>

class pt_mutex;

class html_document
{
    private:
        html_node *_root;
        pt_mutex *_mutex;
        uint8_t _state;
        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;

        void abort_lifecycle_error(const char *method_name,
                    const char *reason) const noexcept;
        void abort_if_not_initialized(const char *method_name) const noexcept;
        int lock_document(bool *lock_acquired) const noexcept;
        int unlock_document(bool lock_acquired) const noexcept;

    public:
        html_document() noexcept;
        ~html_document() noexcept;

        html_document(const html_document &) = delete;
        html_document &operator=(const html_document &) = delete;
        html_document(html_document &&) = delete;
        html_document &operator=(html_document &&) = delete;

        int         initialize() noexcept;
        int         destroy() noexcept;
        int         enable_thread_safety() noexcept;
        int         disable_thread_safety() noexcept;
        bool        is_thread_safe() const noexcept;
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
        html_node   *find_by_selector(const char *selector) const noexcept;
        size_t      count_nodes_by_tag(const char *tag_name) const noexcept;
        html_node   *get_root() const noexcept;
        int         get_error() const noexcept;
        const char  *get_error_str() const noexcept;
        void        clear() noexcept;
        bool        is_thread_safe_enabled() const noexcept;
#ifdef LIBFT_TEST_BUILD
        pt_mutex    *get_mutex_for_validation() const noexcept;
#endif
};

#endif
