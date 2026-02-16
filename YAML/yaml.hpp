#ifndef YAML_HPP
#define YAML_HPP

#include "../CPP_class/class_string.hpp"
#include "../Template/vector.hpp"
#include "../Template/map.hpp"
#include "../Parser/document_backend.hpp"
#include "../PThread/recursive_mutex.hpp"
#include <cstdint>

enum yaml_type
{
    YAML_SCALAR,
    YAML_LIST,
    YAML_MAP
};

class yaml_value
{
    private:
        uint8_t _state;
        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;
        yaml_type _type;
        ft_string _scalar;
        ft_vector<yaml_value*> _list;
        ft_map<ft_string, yaml_value*> _map;
        ft_vector<ft_string> _map_keys;

        mutable pt_recursive_mutex *_mutex;

        int lock(bool *lock_acquired) const noexcept;
        int unlock(bool lock_acquired) const noexcept;
        void abort_lifecycle_error(const char *method_name,
            const char *reason) const noexcept;
        void abort_if_not_initialized(const char *method_name) const noexcept;

    public:
        yaml_value() noexcept;
        ~yaml_value() noexcept;

        yaml_value(const yaml_value &) = delete;
        yaml_value &operator=(const yaml_value &) = delete;
        yaml_value(yaml_value &&) = delete;
        yaml_value &operator=(yaml_value &&) = delete;

        int initialize() noexcept;
        int destroy() noexcept;
        int enable_thread_safety() noexcept;
        int disable_thread_safety() noexcept;
        bool is_thread_safe() const noexcept;

        void set_type(yaml_type type) noexcept;
        yaml_type get_type() const noexcept;

        void set_scalar(const ft_string &value) noexcept;
        const ft_string &get_scalar() const noexcept;

        void add_list_item(yaml_value *item) noexcept;
        const ft_vector<yaml_value*> &get_list() const noexcept;

        void add_map_item(const ft_string &key, yaml_value *value) noexcept;
        const ft_map<ft_string, yaml_value*> &get_map() const noexcept;
        const ft_vector<ft_string> &get_map_keys() const noexcept;

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_validation() const noexcept;
#endif
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
