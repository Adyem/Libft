#include "kv_store.hpp"

#include <cstddef>
#include <cerrno>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <vector>

const char *g_kv_store_ttl_prefix = "__ttl__";

void kv_store::set_error(int error_code) const
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}

long long kv_store::current_time_seconds() const
{
    std::time_t current_time;

    current_time = std::time(ft_nullptr);
    if (current_time == static_cast<std::time_t>(-1))
        return (-1);
    return (static_cast<long long>(current_time));
}

int kv_store::compute_expiration(long long ttl_seconds, bool &has_expiration, long long &expiration_timestamp) const
{
    long long current_time;

    has_expiration = false;
    expiration_timestamp = 0;
    if (ttl_seconds < 0)
        return (0);
    current_time = this->current_time_seconds();
    if (current_time < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    has_expiration = true;
    if (ttl_seconds > LLONG_MAX - current_time)
        expiration_timestamp = LLONG_MAX;
    else
        expiration_timestamp = current_time + ttl_seconds;
    return (0);
}

int kv_store::parse_expiration_timestamp(const char *value_string, long long &expiration_timestamp) const
{
    char *end_pointer;
    long long parsed_value;

    if (value_string == ft_nullptr)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    errno = 0;
    parsed_value = std::strtoll(value_string, &end_pointer, 10);
    if (errno != 0 || end_pointer == value_string || *end_pointer != '\0')
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    expiration_timestamp = parsed_value;
    return (0);
}

int kv_store::prune_expired()
{
    size_t map_size;
    long long current_time;
    std::vector<ft_string> keys_to_remove;

    map_size = this->_data.size();
    if (this->_data.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_data.get_error());
        return (-1);
    }
    if (map_size == 0)
        return (0);
    current_time = this->current_time_seconds();
    if (current_time < 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    Pair<ft_string, kv_store_entry> *map_end;
    Pair<ft_string, kv_store_entry> *map_begin;
    size_t map_index;

    map_end = this->_data.end();
    if (this->_data.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_data.get_error());
        return (-1);
    }
    map_begin = map_end - static_cast<std::ptrdiff_t>(map_size);
    map_index = 0;
    while (map_index < map_size)
    {
        Pair<ft_string, kv_store_entry> &entry = map_begin[map_index];

        if (entry.value._has_expiration && entry.value._expiration_timestamp <= current_time)
        {
            keys_to_remove.push_back(entry.key);
            if (keys_to_remove.back().get_error() != ER_SUCCESS)
            {
                this->set_error(keys_to_remove.back().get_error());
                return (-1);
            }
        }
        map_index++;
    }
    size_t removal_index;

    removal_index = 0;
    while (removal_index < keys_to_remove.size())
    {
        this->_data.remove(keys_to_remove[removal_index]);
        if (this->_data.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_data.get_error());
            return (-1);
        }
        removal_index++;
    }
    return (0);
}

int kv_store::kv_set(const char *key_string, const char *value_string, long long ttl_seconds)
{
    ft_string key_storage;
    ft_string value_storage;
    Pair<ft_string, kv_store_entry> *existing_pair;
    bool has_expiration;
    long long expiration_timestamp;

    if (key_string == ft_nullptr || value_string == ft_nullptr)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    key_storage = key_string;
    if (key_storage.get_error() != ER_SUCCESS)
    {
        this->set_error(key_storage.get_error());
        return (-1);
    }
    value_storage = value_string;
    if (value_storage.get_error() != ER_SUCCESS)
    {
        this->set_error(value_storage.get_error());
        return (-1);
    }
    if (this->prune_expired() != 0)
        return (-1);
    if (this->compute_expiration(ttl_seconds, has_expiration, expiration_timestamp) != 0)
        return (-1);
    existing_pair = this->_data.find(key_storage);
    if (this->_data.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_data.get_error());
        return (-1);
    }
    if (existing_pair != ft_nullptr)
    {
        existing_pair->value._value = value_storage;
        if (existing_pair->value._value.get_error() != ER_SUCCESS)
        {
            this->set_error(existing_pair->value._value.get_error());
            return (-1);
        }
        existing_pair->value._has_expiration = has_expiration;
        if (has_expiration)
            existing_pair->value._expiration_timestamp = expiration_timestamp;
        else
            existing_pair->value._expiration_timestamp = 0;
    }
    else
    {
        kv_store_entry new_entry;

        new_entry._value = value_storage;
        if (new_entry._value.get_error() != ER_SUCCESS)
        {
            this->set_error(new_entry._value.get_error());
            return (-1);
        }
        new_entry._has_expiration = has_expiration;
        if (has_expiration)
            new_entry._expiration_timestamp = expiration_timestamp;
        else
            new_entry._expiration_timestamp = 0;
        this->_data.insert(key_storage, new_entry);
        if (this->_data.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_data.get_error());
            return (-1);
        }
    }
    if (has_expiration)
    {
        long long current_time;

        current_time = this->current_time_seconds();
        if (current_time < 0)
        {
            this->set_error(FT_ERR_INVALID_ARGUMENT);
            return (-1);
        }
        if (expiration_timestamp <= current_time)
        {
            if (this->prune_expired() != 0)
                return (-1);
        }
    }
    this->set_error(ER_SUCCESS);
    return (0);
}

const char *kv_store::kv_get(const char *key_string) const
{
    ft_string key_storage;
    const Pair<ft_string, kv_store_entry> *map_pair;
    kv_store *mutable_this;

    if (key_string == ft_nullptr)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    mutable_this = const_cast<kv_store *>(this);
    if (mutable_this->prune_expired() != 0)
        return (ft_nullptr);
    key_storage = key_string;
    if (key_storage.get_error() != ER_SUCCESS)
    {
        this->set_error(key_storage.get_error());
        return (ft_nullptr);
    }
    map_pair = this->_data.find(key_storage);
    if (this->_data.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_data.get_error());
        return (ft_nullptr);
    }
    if (map_pair == ft_nullptr)
    {
        this->set_error(FT_ERR_NOT_FOUND);
        return (ft_nullptr);
    }
    this->set_error(ER_SUCCESS);
    return (map_pair->value._value.c_str());
}

int kv_store::kv_delete(const char *key_string)
{
    ft_string key_storage;
    const Pair<ft_string, kv_store_entry> *map_pair;

    if (key_string == ft_nullptr)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (this->prune_expired() != 0)
        return (-1);
    key_storage = key_string;
    if (key_storage.get_error() != ER_SUCCESS)
    {
        this->set_error(key_storage.get_error());
        return (-1);
    }
    map_pair = this->_data.find(key_storage);
    if (this->_data.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_data.get_error());
        return (-1);
    }
    if (map_pair == ft_nullptr)
    {
        this->set_error(FT_ERR_NOT_FOUND);
        return (-1);
    }
    this->_data.remove(key_storage);
    if (this->_data.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_data.get_error());
        return (-1);
    }
    this->set_error(ER_SUCCESS);
    return (0);
}

int kv_store::kv_flush() const
{
    json_group *store_group;
    json_group *head_group;
    json_item *item_pointer;
    ft_string stored_value;
    const Pair<ft_string, kv_store_entry> *map_end;
    const Pair<ft_string, kv_store_entry> *map_begin;
    size_t map_size;
    size_t map_index;
    int result;
    int error_code;
    kv_store *mutable_this;

    mutable_this = const_cast<kv_store *>(this);
    if (mutable_this->prune_expired() != 0)
        return (-1);
    store_group = json_create_json_group("kv_store");
    if (store_group == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return (-1);
    }
    if (this->_encryption_enabled)
    {
        item_pointer = json_create_item("__encryption__", "aes-128-ecb-base64");
        if (item_pointer == ft_nullptr)
        {
            json_free_groups(store_group);
            this->set_error(FT_ERR_NO_MEMORY);
            return (-1);
        }
        json_add_item_to_group(store_group, item_pointer);
    }
    map_size = this->_data.size();
    if (this->_data.get_error() != ER_SUCCESS)
    {
        json_free_groups(store_group);
        this->set_error(this->_data.get_error());
        return (-1);
    }
    map_end = this->_data.end();
    if (this->_data.get_error() != ER_SUCCESS)
    {
        json_free_groups(store_group);
        this->set_error(this->_data.get_error());
        return (-1);
    }
    map_begin = map_end - static_cast<std::ptrdiff_t>(map_size);
    map_index = 0;
    while (map_index < map_size)
    {
        const Pair<ft_string, kv_store_entry> &entry = map_begin[map_index];
        const kv_store_entry &entry_value = entry.value;

        if (this->_encryption_enabled)
        {
            stored_value.clear();
            if (this->encrypt_value(entry_value._value, stored_value) != 0)
            {
                json_free_groups(store_group);
                return (-1);
            }
            item_pointer = json_create_item(entry.key.c_str(), stored_value.c_str());
        }
        else
            item_pointer = json_create_item(entry.key.c_str(), entry_value._value.c_str());
        if (item_pointer == ft_nullptr)
        {
            json_free_groups(store_group);
            this->set_error(FT_ERR_NO_MEMORY);
            return (-1);
        }
        json_add_item_to_group(store_group, item_pointer);
        if (entry_value._has_expiration)
        {
            ft_string ttl_key;
            char expiration_buffer[32];
            int written_length;

            ttl_key = g_kv_store_ttl_prefix;
            if (ttl_key.get_error() != ER_SUCCESS)
            {
                json_free_groups(store_group);
                this->set_error(ttl_key.get_error());
                return (-1);
            }
            ttl_key += entry.key;
            if (ttl_key.get_error() != ER_SUCCESS)
            {
                json_free_groups(store_group);
                this->set_error(ttl_key.get_error());
                return (-1);
            }
            written_length = std::snprintf(expiration_buffer, sizeof(expiration_buffer), "%lld", entry_value._expiration_timestamp);
            if (written_length < 0 || static_cast<size_t>(written_length) >= sizeof(expiration_buffer))
            {
                json_free_groups(store_group);
                this->set_error(FT_ERR_INVALID_ARGUMENT);
                return (-1);
            }
            item_pointer = json_create_item(ttl_key.c_str(), expiration_buffer);
            if (item_pointer == ft_nullptr)
            {
                json_free_groups(store_group);
                this->set_error(FT_ERR_NO_MEMORY);
                return (-1);
            }
            json_add_item_to_group(store_group, item_pointer);
        }
        map_index++;
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
            this->set_error(FT_ERR_INVALID_ARGUMENT);
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

