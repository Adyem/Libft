#ifndef YAML_HPP
#define YAML_HPP

#include "../CPP_class/class_string_class.hpp"
#include "../Template/vector.hpp"
#include "../Template/map.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Parser/document_backend.hpp"

class pt_mutex;

enum yaml_type
{
    YAML_SCALAR,
    YAML_LIST,
    YAML_MAP
};

class yaml_value
{
    private:
        mutable int _error_code;
        void set_error(int error_code) const noexcept;

        yaml_type _type;
        ft_string _scalar;
        ft_vector<yaml_value*> _list;
        ft_map<ft_string, yaml_value*> _map;
        ft_vector<ft_string> _map_keys;

        mutable pt_mutex *_mutex;
        mutable bool _thread_safe_enabled;

        int prepare_thread_safety() noexcept;
        void teardown_thread_safety() noexcept;
        int lock(bool *lock_acquired) const noexcept;
        void unlock(bool lock_acquired) const noexcept;

    public:
        class thread_guard
        {
            private:
                const yaml_value *_value;
                bool _lock_acquired;
                int _status;
                int _entry_errno;

            public:
                thread_guard(const yaml_value *value) noexcept;
                ~thread_guard() noexcept;

                int get_status() const noexcept;
                bool lock_acquired() const noexcept;
        };

        yaml_value() noexcept;
        ~yaml_value() noexcept;

        yaml_value(const yaml_value &) = delete;
        yaml_value &operator=(const yaml_value &) = delete;
        yaml_value(yaml_value &&) = delete;
        yaml_value &operator=(yaml_value &&) = delete;

        int get_error() const noexcept;
        const char *get_error_str() const noexcept;

        void set_type(yaml_type type) noexcept;
        yaml_type get_type() const noexcept;

        void set_scalar(const ft_string &value) noexcept;
        const ft_string &get_scalar() const noexcept;

        void add_list_item(yaml_value *item) noexcept;
        const ft_vector<yaml_value*> &get_list() const noexcept;

        void add_map_item(const ft_string &key, yaml_value *value) noexcept;
        const ft_map<ft_string, yaml_value*> &get_map() const noexcept;
        const ft_vector<ft_string> &get_map_keys() const noexcept;

        bool is_thread_safe_enabled() const noexcept;
};

size_t      yaml_find_char(const ft_string &string, char character) noexcept;
ft_string   yaml_substr(const ft_string &string, size_t start, size_t length) noexcept;
ft_string   yaml_substr_from(const ft_string &string, size_t start) noexcept;
size_t      yaml_count_indent(const ft_string &line) noexcept;
void        yaml_trim(ft_string &string) noexcept;
int         yaml_split_lines(const ft_string &content, ft_vector<ft_string> &lines) noexcept;

yaml_value    *yaml_read_from_string(const ft_string &content) noexcept;
yaml_value    *yaml_read_from_file(const char *file_path) noexcept;
yaml_value    *yaml_read_from_backend(ft_document_source &source) noexcept;
ft_string      yaml_write_to_string(const yaml_value *value) noexcept;
int           yaml_write_to_file(const char *file_path, const yaml_value *value) noexcept;
int           yaml_write_to_backend(ft_document_sink &sink, const yaml_value *value) noexcept;
void          yaml_free(yaml_value *value) noexcept;

#endif
