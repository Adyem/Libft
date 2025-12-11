#ifndef PARSER_DOM_HPP
#define PARSER_DOM_HPP

#include "../CPP_class/class_string.hpp"
#include "../Template/vector.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"

class pt_mutex;

enum ft_dom_node_type
{
    FT_DOM_NODE_NULL,
    FT_DOM_NODE_OBJECT,
    FT_DOM_NODE_ARRAY,
    FT_DOM_NODE_VALUE,
    FT_DOM_NODE_ELEMENT
};

class ft_dom_node
{
    private:
        mutable int _error_code;
        ft_dom_node_type _type;
        ft_string _name;
        ft_string _value;
        ft_vector<ft_dom_node*> _children;
        ft_vector<ft_string> _attribute_keys;
        ft_vector<ft_string> _attribute_values;
        mutable pt_mutex *_mutex;
        mutable bool _thread_safe_enabled;

        void set_error(int error_code) const noexcept;
        int prepare_thread_safety() noexcept;
        void teardown_thread_safety() noexcept;
        int lock(bool *lock_acquired) const noexcept;
        void unlock(bool lock_acquired) const noexcept;

    public:
        class thread_guard
        {
            private:
                const ft_dom_node *_node;
                bool _lock_acquired;
                int _status;

            public:
                thread_guard(const ft_dom_node *node) noexcept;
                ~thread_guard() noexcept;

                int get_status() const noexcept;
                bool lock_acquired() const noexcept;
        };

        ft_dom_node() noexcept;
        ~ft_dom_node() noexcept;

        ft_dom_node(const ft_dom_node &) = delete;
        ft_dom_node &operator=(const ft_dom_node &) = delete;
        ft_dom_node(ft_dom_node &&) = delete;
        ft_dom_node &operator=(ft_dom_node &&) = delete;

        ft_dom_node_type get_type() const noexcept;
        void set_type(ft_dom_node_type type) noexcept;
        int set_name(const ft_string &name) noexcept;
        int set_name(const char *name) noexcept;
        const ft_string &get_name() const noexcept;
        int set_value(const ft_string &value) noexcept;
        int set_value(const char *value) noexcept;
        const ft_string &get_value() const noexcept;
        int add_child(ft_dom_node *child) noexcept;
        const ft_vector<ft_dom_node*> &get_children() const noexcept;
        int add_attribute(const ft_string &key, const ft_string &value) noexcept;
        int add_attribute(const char *key, const char *value) noexcept;
        bool has_attribute(const ft_string &key) const noexcept;
        ft_string get_attribute(const ft_string &key) const noexcept;
        const ft_vector<ft_string> &get_attribute_keys() const noexcept;
        const ft_vector<ft_string> &get_attribute_values() const noexcept;
        ft_dom_node *find_child(const ft_string &name) const noexcept;
        bool is_thread_safe_enabled() const noexcept;
        int get_error() const noexcept;
};

class ft_dom_document
{
    private:
        ft_dom_node *_root;
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
                const ft_dom_document *_document;
                bool _lock_acquired;
                int _status;

            public:
                thread_guard(const ft_dom_document *document) noexcept;
                ~thread_guard() noexcept;

                int get_status() const noexcept;
                bool lock_acquired() const noexcept;
        };

        ft_dom_document() noexcept;
        ~ft_dom_document() noexcept;

        ft_dom_document(const ft_dom_document &) = delete;
        ft_dom_document &operator=(const ft_dom_document &) = delete;
        ft_dom_document(ft_dom_document &&) = delete;
        ft_dom_document &operator=(ft_dom_document &&) = delete;

        void set_root(ft_dom_node *root) noexcept;
        ft_dom_node *get_root() const noexcept;
        void clear() noexcept;
        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
        bool is_thread_safe_enabled() const noexcept;
};

struct ft_dom_schema_rule
{
    ft_string path;
    ft_dom_node_type type;
    bool required;

    ft_dom_schema_rule() noexcept;
    ~ft_dom_schema_rule() noexcept;
};

struct ft_dom_validation_error
{
    ft_string path;
    ft_string message;

    ft_dom_validation_error() noexcept;
    ~ft_dom_validation_error() noexcept;
};

class ft_dom_validation_report
{
    private:
        bool _valid;
        ft_vector<ft_dom_validation_error> _errors;
        mutable int _error_code;

        void set_error(int error_code) const noexcept;

    public:
        ft_dom_validation_report() noexcept;
        ~ft_dom_validation_report() noexcept;

        void mark_valid() noexcept;
        void mark_invalid() noexcept;
        bool valid() const noexcept;
        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
        int add_error(const ft_string &path, const ft_string &message) noexcept;
        const ft_vector<ft_dom_validation_error> &errors() const noexcept;
};

class ft_dom_schema
{
    private:
        ft_vector<ft_dom_schema_rule> _rules;
        mutable int _error_code;

        void set_error(int error_code) const noexcept;
        int validate_rule(const ft_dom_schema_rule &rule, const ft_dom_node *node,
            const ft_string &base_path, ft_dom_validation_report &report) const noexcept;

    public:
        ft_dom_schema() noexcept;
        ~ft_dom_schema() noexcept;

        int add_rule(const ft_string &path, ft_dom_node_type type, bool required) noexcept;
        int validate(const ft_dom_document &document, ft_dom_validation_report &report) const noexcept;
};

int ft_dom_find_path(const ft_dom_node *root, const ft_string &path, const ft_dom_node **out_node) noexcept;

#endif
