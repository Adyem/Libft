#ifndef PARSER_DOM_HPP
#define PARSER_DOM_HPP

#include "../CPP_class/class_string.hpp"
#include "../Template/vector.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include <stdint.h>

class pt_recursive_mutex;

typedef struct s_ft_operation_error_stack
{
    int last_error;
} ft_operation_error_stack;

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
        ft_dom_node_type _type;
        ft_string _name;
        ft_string _value;
        ft_vector<ft_dom_node*> _children;
        ft_vector<ft_string> _attribute_keys;
        ft_vector<ft_string> _attribute_values;
        pt_recursive_mutex *_mutex;
        uint8_t _initialized_state;

        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;

        void abort_lifecycle_error(const char *method_name,
            const char *reason) const;
        void abort_if_not_initialized(const char *method_name) const;
        int lock_internal(bool *lock_acquired) const noexcept;
        int unlock_internal(bool lock_acquired) const noexcept;

    public:
        ft_dom_node() noexcept;
        ~ft_dom_node() noexcept;

        ft_dom_node(const ft_dom_node &) = delete;
        ft_dom_node &operator=(const ft_dom_node &) = delete;
        ft_dom_node(ft_dom_node &&) = delete;
        ft_dom_node &operator=(ft_dom_node &&) = delete;

        int initialize() noexcept;
        int destroy() noexcept;
        int enable_thread_safety() noexcept;
        int disable_thread_safety() noexcept;
        bool is_thread_safe() const noexcept;
        int lock(bool *lock_acquired) const noexcept;
        void unlock(bool lock_acquired) const noexcept;

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

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_validation() const noexcept;
#endif
};

class ft_dom_document
{
    private:
        ft_dom_node *_root;
        pt_recursive_mutex *_mutex;
        uint8_t _initialized_state;

        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;

        void abort_lifecycle_error(const char *method_name,
            const char *reason) const;
        void abort_if_not_initialized(const char *method_name) const;
        int lock_internal(bool *lock_acquired) const noexcept;
        int unlock_internal(bool lock_acquired) const noexcept;

    public:

        ft_dom_document() noexcept;
        ~ft_dom_document() noexcept;

        ft_dom_document(const ft_dom_document &) = delete;
        ft_dom_document &operator=(const ft_dom_document &) = delete;
        ft_dom_document(ft_dom_document &&) = delete;
        ft_dom_document &operator=(ft_dom_document &&) = delete;

        int initialize() noexcept;
        int destroy() noexcept;
        int enable_thread_safety() noexcept;
        int disable_thread_safety() noexcept;
        bool is_thread_safe() const noexcept;
        int lock(bool *lock_acquired) const noexcept;
        void unlock(bool lock_acquired) const noexcept;

        void set_root(ft_dom_node *root) noexcept;
        ft_dom_node *get_root() const noexcept;
        void clear() noexcept;

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_validation() const noexcept;
#endif
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
        pt_recursive_mutex *_mutex;
        uint8_t _initialized_state;

        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;

        void abort_lifecycle_error(const char *method_name,
            const char *reason) const;
        void abort_if_not_initialized(const char *method_name) const;
        int lock_internal(bool *lock_acquired) const noexcept;
        int unlock_internal(bool lock_acquired) const noexcept;

    public:
        ft_dom_validation_report() noexcept;
        ~ft_dom_validation_report() noexcept;
        ft_dom_validation_report(const ft_dom_validation_report &) = delete;
        ft_dom_validation_report &operator=(const ft_dom_validation_report &) = delete;
        ft_dom_validation_report(ft_dom_validation_report &&) = delete;
        ft_dom_validation_report &operator=(ft_dom_validation_report &&) = delete;

        int initialize() noexcept;
        int destroy() noexcept;
        int enable_thread_safety() noexcept;
        int disable_thread_safety() noexcept;
        bool is_thread_safe() const noexcept;
        int lock(bool *lock_acquired) const noexcept;
        void unlock(bool lock_acquired) const noexcept;

        void mark_valid() noexcept;
        void mark_invalid() noexcept;
        bool valid() const noexcept;
        int add_error(const ft_string &path, const ft_string &message) noexcept;
        const ft_vector<ft_dom_validation_error> &errors() const noexcept;

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_validation() const noexcept;
#endif
};

class ft_dom_schema
{
    private:
        ft_vector<ft_dom_schema_rule> _rules;
        pt_recursive_mutex *_mutex;
        uint8_t _initialized_state;

        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;

        void abort_lifecycle_error(const char *method_name,
            const char *reason) const;
        void abort_if_not_initialized(const char *method_name) const;
        int lock_internal(bool *lock_acquired) const noexcept;
        int unlock_internal(bool lock_acquired) const noexcept;
        int validate_rule(const ft_dom_schema_rule &rule, const ft_dom_node *node,
            const ft_string &base_path, ft_dom_validation_report &report) const noexcept;

    public:
        ft_dom_schema() noexcept;
        ~ft_dom_schema() noexcept;
        ft_dom_schema(const ft_dom_schema &) = delete;
        ft_dom_schema &operator=(const ft_dom_schema &) = delete;
        ft_dom_schema(ft_dom_schema &&) = delete;
        ft_dom_schema &operator=(ft_dom_schema &&) = delete;

        int initialize() noexcept;
        int destroy() noexcept;
        int enable_thread_safety() noexcept;
        int disable_thread_safety() noexcept;
        bool is_thread_safe() const noexcept;
        int lock(bool *lock_acquired) const noexcept;
        void unlock(bool lock_acquired) const noexcept;

        int add_rule(const ft_string &path, ft_dom_node_type type, bool required) noexcept;
        int validate(const ft_dom_document &document, ft_dom_validation_report &report) const noexcept;

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_validation() const noexcept;
#endif
};

int ft_dom_find_path(const ft_dom_node *root, const ft_string &path, const ft_dom_node **out_node) noexcept;

#endif
