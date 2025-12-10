#ifndef HTML_DOCUMENT_HPP
#define HTML_DOCUMENT_HPP

#include "parser.hpp"

class pt_mutex;

class html_document
{
    private:
        html_node *_root;
        mutable pt_mutex *_mutex;
        mutable bool _thread_safe_enabled;
        mutable int _error_code;

        void set_error(int error_code) const noexcept;

        int prepare_thread_safety() noexcept;
        void teardown_thread_safety() noexcept;
        int lock(bool *lock_acquired) const noexcept;
        void unlock(bool lock_acquired) const noexcept;

    public:
        class thread_guard
        {
            private:
                const html_document *_document;
                bool _lock_acquired;
                int _status;

            public:
                thread_guard(const html_document *document) noexcept;
                ~thread_guard() noexcept;

                int get_status() const noexcept;
                bool lock_acquired() const noexcept;
        };

        html_document() noexcept;
        ~html_document() noexcept;

        html_document(const html_document &) = delete;
        html_document &operator=(const html_document &) = delete;
        html_document(html_document &&) = delete;
        html_document &operator=(html_document &&) = delete;

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
};

#endif
