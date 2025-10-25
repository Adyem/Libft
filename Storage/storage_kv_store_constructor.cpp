#include "kv_store.hpp"

#include <cstddef>
#include <cstring>
#include <ctime>

kv_store::kv_store(const char *file_path, const char *encryption_key, bool enable_encryption)
    : _data(), _file_path(), _encryption_key(), _encryption_enabled(false), _error_code(ER_SUCCESS)
{
    json_group *group_head;
    json_group *store_group;
    json_item *item_pointer;
    ft_map<ft_string, long long> ttl_metadata;
    size_t ttl_prefix_length;
    int current_error;

    if (file_path == ft_nullptr)
    {
        this->_file_path.clear();
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    this->_file_path = file_path;
    if (this->_file_path.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_file_path.get_error());
        return ;
    }
    if (enable_encryption)
    {
        if (encryption_key == ft_nullptr)
        {
            this->set_error(FT_ERR_INVALID_ARGUMENT);
            return ;
        }
        this->_encryption_key = encryption_key;
        if (this->_encryption_key.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_encryption_key.get_error());
            return ;
        }
        if (this->_encryption_key.size() != 16)
        {
            this->set_error(FT_ERR_INVALID_ARGUMENT);
            return ;
        }
        this->_encryption_enabled = true;
    }
    else
    {
        this->_encryption_enabled = false;
        if (encryption_key != ft_nullptr)
        {
            this->_encryption_key = encryption_key;
            if (this->_encryption_key.get_error() != ER_SUCCESS)
            {
                this->set_error(this->_encryption_key.get_error());
                return ;
            }
        }
        else
            this->_encryption_key.clear();
    }

    group_head = json_read_from_file(file_path);
    if (group_head == ft_nullptr)
    {
        current_error = ft_errno;
        if (current_error == ER_SUCCESS)
        {
            this->set_error(ER_SUCCESS);
            return ;
        }
        this->set_error(current_error);
        return ;
    }
    store_group = json_find_group(group_head, "kv_store");
    if (store_group == ft_nullptr)
    {
        json_free_groups(group_head);
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    ttl_prefix_length = std::strlen(g_kv_store_ttl_prefix);
    item_pointer = store_group->items;
    while (item_pointer != ft_nullptr)
    {
        if (std::strcmp(item_pointer->key, "__encryption__") == 0)
        {
            if (std::strcmp(item_pointer->value, "aes-128-ecb-base64") == 0)
            {
                if (this->_encryption_enabled == false)
                {
                    json_free_groups(group_head);
                    this->set_error(FT_ERR_INVALID_ARGUMENT);
                    return ;
                }
            }
            else
            {
                json_free_groups(group_head);
                this->set_error(FT_ERR_INVALID_ARGUMENT);
                return ;
            }
            item_pointer = item_pointer->next;
            continue;
        }
        if (ttl_prefix_length > 0 && std::strncmp(item_pointer->key, g_kv_store_ttl_prefix, ttl_prefix_length) == 0)
        {
            ft_string ttl_key(item_pointer->key + ttl_prefix_length);
            long long expiration_timestamp;

            if (ttl_key.get_error() != ER_SUCCESS)
            {
                json_free_groups(group_head);
                this->set_error(ttl_key.get_error());
                return ;
            }
            if (this->parse_expiration_timestamp(item_pointer->value, expiration_timestamp) != 0)
            {
                json_free_groups(group_head);
                return ;
            }
            ttl_metadata.insert(ttl_key, expiration_timestamp);
            if (ttl_metadata.get_error() != ER_SUCCESS)
            {
                json_free_groups(group_head);
                this->set_error(ttl_metadata.get_error());
                return ;
            }
            item_pointer = item_pointer->next;
            continue;
        }
        ft_string key_storage(item_pointer->key);
        kv_store_entry entry;

        if (key_storage.get_error() != ER_SUCCESS)
        {
            json_free_groups(group_head);
            this->set_error(key_storage.get_error());
            return ;
        }
        entry._has_expiration = false;
        entry._expiration_timestamp = 0;
        if (this->_encryption_enabled)
        {
            ft_string encoded_value(item_pointer->value);
            ft_string decrypted_value;

            if (encoded_value.get_error() != ER_SUCCESS)
            {
                json_free_groups(group_head);
                this->set_error(encoded_value.get_error());
                return ;
            }
            if (this->decrypt_value(encoded_value, decrypted_value) != 0)
            {
                json_free_groups(group_head);
                return ;
            }
            entry._value = decrypted_value;
        }
        else
        {
            ft_string plain_value(item_pointer->value);

            if (plain_value.get_error() != ER_SUCCESS)
            {
                json_free_groups(group_head);
                this->set_error(plain_value.get_error());
                return ;
            }
            entry._value = plain_value;
        }
        if (entry._value.get_error() != ER_SUCCESS)
        {
            json_free_groups(group_head);
            this->set_error(entry._value.get_error());
            return ;
        }
        this->_data.insert(key_storage, entry);
        if (this->_data.get_error() != ER_SUCCESS)
        {
            json_free_groups(group_head);
            this->set_error(this->_data.get_error());
            return ;
        }
        item_pointer = item_pointer->next;
    }
    size_t ttl_size;

    ttl_size = ttl_metadata.size();
    if (ttl_metadata.get_error() != ER_SUCCESS)
    {
        json_free_groups(group_head);
        this->set_error(ttl_metadata.get_error());
        return ;
    }
    if (ttl_size > 0)
    {
        const Pair<ft_string, long long> *ttl_end;
        const Pair<ft_string, long long> *ttl_begin;
        size_t ttl_index;

        ttl_end = ttl_metadata.end();
        if (ttl_metadata.get_error() != ER_SUCCESS)
        {
            json_free_groups(group_head);
            this->set_error(ttl_metadata.get_error());
            return ;
        }
        ttl_begin = ttl_end - static_cast<std::ptrdiff_t>(ttl_size);
        ttl_index = 0;
        while (ttl_index < ttl_size)
        {
            const Pair<ft_string, long long> &ttl_entry = ttl_begin[ttl_index];
            Pair<ft_string, kv_store_entry> *data_pair;

            data_pair = this->_data.find(ttl_entry.key);
            if (this->_data.get_error() != ER_SUCCESS)
            {
                json_free_groups(group_head);
                this->set_error(this->_data.get_error());
                return ;
            }
            if (data_pair != ft_nullptr)
            {
                data_pair->value._has_expiration = true;
                data_pair->value._expiration_timestamp = ttl_entry.value;
            }
            ttl_index++;
        }
    }
    if (this->prune_expired() != 0)
    {
        json_free_groups(group_head);
        return ;
    }
    json_free_groups(group_head);
    this->set_error(ER_SUCCESS);
    return ;
}

kv_store::~kv_store()
{
    return ;
}

