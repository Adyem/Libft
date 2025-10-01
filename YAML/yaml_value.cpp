#include "yaml.hpp"

yaml_value::yaml_value() noexcept
{
    this->set_error(ER_SUCCESS);
    this->_type = YAML_SCALAR;
    this->_scalar = "";
    if (this->_scalar.get_error() != ER_SUCCESS)
        this->set_error(this->_scalar.get_error());
    return ;
}

yaml_value::~yaml_value() noexcept
{
    if (this->_type == YAML_LIST)
    {
        size_t list_index = 0;
        size_t list_size = this->_list.size();
        if (this->_list.get_error() != ER_SUCCESS)
            this->set_error(this->_list.get_error());
        while (list_index < list_size)
        {
            delete this->_list[list_index];
            list_index++;
        }
        return ;
    }
    if (this->_type == YAML_MAP)
    {
        size_t key_index = 0;
        size_t key_count = this->_map_keys.size();
        if (this->_map_keys.get_error() != ER_SUCCESS)
            this->set_error(this->_map_keys.get_error());
        while (key_index < key_count)
        {
            const ft_string &key = this->_map_keys[key_index];
            yaml_value *child = this->_map.at(key);
            if (this->_map.get_error() != ER_SUCCESS)
            {
                this->set_error(this->_map.get_error());
                break;
            }
            delete child;
            key_index++;
        }
        return ;
    }
    return ;
}

void yaml_value::set_error(int error_code) const noexcept
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}

int yaml_value::get_error() const noexcept
{
    return (this->_error_code);
}

const char *yaml_value::get_error_str() const noexcept
{
    return (ft_strerror(this->_error_code));
}

void yaml_value::set_type(yaml_type type) noexcept
{
    this->_type = type;
    this->set_error(ER_SUCCESS);
    return ;
}

yaml_type yaml_value::get_type() const noexcept
{
    return (this->_type);
}

void yaml_value::set_scalar(const ft_string &value) noexcept
{
    this->_type = YAML_SCALAR;
    this->_scalar = value;
    if (this->_scalar.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_scalar.get_error());
        return ;
    }
    this->set_error(ER_SUCCESS);
    return ;
}

const ft_string &yaml_value::get_scalar() const noexcept
{
    return (this->_scalar);
}

void yaml_value::add_list_item(yaml_value *item) noexcept
{
    this->_type = YAML_LIST;
    this->_list.push_back(item);
    if (this->_list.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_list.get_error());
        return ;
    }
    this->set_error(ER_SUCCESS);
    return ;
}

const ft_vector<yaml_value*> &yaml_value::get_list() const noexcept
{
    return (this->_list);
}

void yaml_value::add_map_item(const ft_string &key, yaml_value *value) noexcept
{
    this->_type = YAML_MAP;
    this->_map.insert(key, value);
    if (this->_map.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_map.get_error());
        return ;
    }
    this->_map_keys.push_back(key);
    if (this->_map_keys.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_map_keys.get_error());
        return ;
    }
    this->set_error(ER_SUCCESS);
    return ;
}

const ft_map<ft_string, yaml_value*> &yaml_value::get_map() const noexcept
{
    return (this->_map);
}

const ft_vector<ft_string> &yaml_value::get_map_keys() const noexcept
{
    return (this->_map_keys);
}

