#ifndef XML_HPP
#define XML_HPP

#include "../Template/vector.hpp"
#include "../Template/unordered_map.hpp"
#include "../Errno/errno.hpp"
#include "../Errno/errno_internal.hpp"
#include "../CMA/CMA.hpp"
#include "../Parser/document_backend.hpp"

class pt_mutex;

struct xml_namespace_entry;

struct xml_node
{
    pt_mutex *mutex;
    bool thread_safe_enabled;
    char *name;
    char *namespace_prefix;
    char *local_name;
    const char *namespace_uri;
    xml_namespace_entry *namespace_bindings;
    char *text;
    ft_vector<xml_node*> children;
    ft_unordered_map<char*, char*> attributes;

    xml_node() noexcept;
    ~xml_node() noexcept;

    xml_node(const xml_node &other) noexcept = delete;
    xml_node &operator=(const xml_node &other) noexcept = delete;
    xml_node(xml_node &&other) noexcept = delete;
    xml_node &operator=(xml_node &&other) noexcept = delete;
};

class xml_document
{
    private:
        xml_node *_root;
        mutable pt_mutex *_mutex;
        mutable bool _thread_safe_enabled;
        mutable ft_operation_error_stack _operation_errors = {{}, {}, 0};

        void record_operation_error(int error_code) const noexcept;
        ft_operation_error_stack *operation_error_stack_handle() const noexcept;
        int prepare_thread_safety() noexcept;
        void teardown_thread_safety() noexcept;
        int lock(bool *lock_acquired) const noexcept;
        void unlock(bool lock_acquired) const noexcept;
        pt_mutex *get_mutex_for_validation() const noexcept;

    public:
        class thread_guard
        {
            private:
                const xml_document *_document;
                bool _lock_acquired;
                int _status;

            public:
                thread_guard(const xml_document *document) noexcept;
                ~thread_guard() noexcept;

                int get_status() const noexcept;
                bool lock_acquired() const noexcept;
        };

        xml_document() noexcept;
        ~xml_document() noexcept;

        int load_from_string(const char *xml) noexcept;
        int load_from_file(const char *file_path) noexcept;
        int load_from_backend(ft_document_source &source) noexcept;
        char *write_to_string() const noexcept;
        int write_to_file(const char *file_path) const noexcept;
        int write_to_backend(ft_document_sink &sink) const noexcept;
        xml_node *get_root() const noexcept;
        void set_manual_error(int error_code) noexcept;
        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
        bool is_thread_safe_enabled() const noexcept;

        xml_document(const xml_document &other) noexcept = delete;
        xml_document &operator=(const xml_document &other) noexcept = delete;
        xml_document(xml_document &&other) noexcept = delete;
        xml_document &operator=(xml_document &&other) noexcept = delete;
};

int  xml_node_prepare_thread_safety(xml_node *node) noexcept;
void xml_node_teardown_thread_safety(xml_node *node) noexcept;
int  xml_node_lock(const xml_node *node, bool *lock_acquired) noexcept;
void xml_node_unlock(const xml_node *node, bool lock_acquired) noexcept;
bool xml_node_is_thread_safe_enabled(const xml_node *node) noexcept;

#endif
