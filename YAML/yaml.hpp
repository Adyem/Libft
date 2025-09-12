#ifndef YAML_HPP
#define YAML_HPP

#include "../CPP_class/class_string_class.hpp"
#include "../Template/vector.hpp"
#include "../Template/map.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"

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

    public:
        yaml_value() noexcept;
        ~yaml_value() noexcept;

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
};

yaml_value    *yaml_read_from_string(const ft_string &content) noexcept;
yaml_value    *yaml_read_from_file(const char *file_path) noexcept;
ft_string      yaml_write_to_string(const yaml_value *value) noexcept;
int           yaml_write_to_file(const char *file_path, const yaml_value *value) noexcept;
void          yaml_free(yaml_value *value) noexcept;

#endif
