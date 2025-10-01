#include "kv_store.hpp"

kv_store::kv_store(const char *file_path)
    : _data(), _file_path(), _error_code(ER_SUCCESS)
{
    json_group *group_head;
    json_group *store_group;
    json_item *item_pointer;
    int current_error;

    if (file_path != ft_nullptr)
        this->_file_path = file_path;
    else
        this->_file_path = "";
    if (file_path == ft_nullptr)
    {
        this->set_error(FT_EINVAL);
        return ;
    }

    group_head = json_read_from_file(file_path);
    if (group_head == ft_nullptr)
    {
        current_error = ft_errno;
        if (current_error == ER_SUCCESS)
            current_error = FT_EINVAL;
        this->set_error(current_error);
        return ;
    }
    store_group = json_find_group(group_head, "kv_store");
    if (store_group == ft_nullptr)
    {
        json_free_groups(group_head);
        this->set_error(FT_EINVAL);
        return ;
    }
    item_pointer = store_group->items;
    while (item_pointer != ft_nullptr)
    {
        this->_data[item_pointer->key] = item_pointer->value;
        item_pointer = item_pointer->next;
    }
    json_free_groups(group_head);
    this->set_error(ER_SUCCESS);
    return ;
}

kv_store::~kv_store()
{
    return ;
}

void kv_store::set_error(int error_code) const
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}

int kv_store::kv_set(const char *key_string, const char *value_string)
{
    if (key_string == ft_nullptr || value_string == ft_nullptr)
    {
        this->set_error(FT_EINVAL);
        return (-1);
    }
    this->_data[key_string] = value_string;
    this->set_error(ER_SUCCESS);
    return (0);
}

const char *kv_store::kv_get(const char *key_string) const
{
    std::map<std::string, std::string>::const_iterator map_iterator;

    if (key_string == ft_nullptr)
    {
        this->set_error(FT_EINVAL);
        return (ft_nullptr);
    }
    map_iterator = this->_data.find(key_string);
    if (map_iterator == this->_data.end())
    {
        this->set_error(MAP_KEY_NOT_FOUND);
        return (ft_nullptr);
    }
    this->set_error(ER_SUCCESS);
    return (map_iterator->second.c_str());
}

int kv_store::kv_delete(const char *key_string)
{
    std::map<std::string, std::string>::iterator map_iterator;

    if (key_string == ft_nullptr)
    {
        this->set_error(FT_EINVAL);
        return (-1);
    }
    map_iterator = this->_data.find(key_string);
    if (map_iterator == this->_data.end())
    {
        this->set_error(MAP_KEY_NOT_FOUND);
        return (-1);
    }
    this->_data.erase(map_iterator);
    this->set_error(ER_SUCCESS);
    return (0);
}

int kv_store::kv_flush() const
{
    json_group *store_group;
    json_group *head_group;
    json_item *item_pointer;
    std::map<std::string, std::string>::const_iterator map_iterator;
    int result;
    int error_code;

    store_group = json_create_json_group("kv_store");
    if (store_group == ft_nullptr)
    {
        this->set_error(JSON_MALLOC_FAIL);
        return (-1);
    }
    map_iterator = this->_data.begin();
    while (map_iterator != this->_data.end())
    {
        item_pointer = json_create_item(map_iterator->first.c_str(), map_iterator->second.c_str());
        if (item_pointer == ft_nullptr)
        {
            json_free_groups(store_group);
            this->set_error(JSON_MALLOC_FAIL);
            return (-1);
        }
        json_add_item_to_group(store_group, item_pointer);
        ++map_iterator;
    }
    head_group = ft_nullptr;
    json_append_group(&head_group, store_group);
    result = json_write_to_file(this->_file_path.c_str(), head_group);
    error_code = ft_errno;
    json_free_groups(head_group);
    if (result != 0)
    {
        if (error_code != ER_SUCCESS)
            this->set_error(error_code);
        else
            this->set_error(FT_EINVAL);
        return (-1);
    }
    this->set_error(ER_SUCCESS);
    return (0);
}

int kv_store::get_error() const
{
    return (this->_error_code);
}

const char *kv_store::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}

