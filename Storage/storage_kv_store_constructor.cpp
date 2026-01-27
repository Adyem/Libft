#include "kv_store.hpp"

#include <cstddef>
#include <cstring>
#include <ctime>

static int storage_kv_move_string_error(ft_string &value) noexcept
{
    unsigned long long operation_id = value.last_operation_id();

    if (operation_id == 0)
        return (FT_ERR_SUCCESSS);
    return (value.pop_operation_error(operation_id));
}

template <typename ContainerType>
static int storage_kv_pop_newest_error(ContainerType &container) noexcept
{
    return (container.pop_newest_operation_error());
}

kv_store::kv_store(const char *file_path, const char *encryption_key, bool enable_encryption)
    : _data()
    , _file_path()
    , _encryption_key()
    , _encryption_enabled(false)
    , _backend_type(KV_STORE_BACKEND_JSON)
    , _background_thread_active(false)
    , _background_stop_requested(false)
    , _background_interval_seconds(0)
    , _background_thread()
    , _background_mutex()
    , _error_code(FT_ERR_SUCCESSS)
    , _mutex()
    , _metrics_set_operations(0)
    , _metrics_delete_operations(0)
    , _metrics_get_hits(0)
    , _metrics_get_misses(0)
    , _metrics_prune_operations(0)
    , _metrics_pruned_entries(0)
    , _metrics_total_prune_duration_ms(0)
    , _metrics_last_prune_duration_ms(0)
    , _replication_sinks()
    , _replication_mutex()
{
    json_group *group_head;
    json_group *store_group;
    json_item *item_pointer;
    ft_map<ft_string, long long> ttl_metadata;
    size_t ttl_prefix_length;
    if (file_path == ft_nullptr)
    {
        this->_file_path.clear();
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    this->_file_path = file_path;
    int file_path_error = storage_kv_move_string_error(this->_file_path);
    if (file_path_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(file_path_error);
        return ;
    }
    if (enable_encryption)
    {
        if (encryption_key == ft_nullptr)
        {
            ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
            return ;
        }
        this->_encryption_key = encryption_key;
        int encryption_key_error = storage_kv_move_string_error(this->_encryption_key);
        if (encryption_key_error != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(encryption_key_error);
            return ;
        }
        if (this->_encryption_key.size() != 16)
        {
            ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
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
            int encryption_key_error = storage_kv_move_string_error(this->_encryption_key);
            if (encryption_key_error != FT_ERR_SUCCESSS)
            {
                ft_global_error_stack_push(encryption_key_error);
                return ;
            }
        }
        else
            this->_encryption_key.clear();
    }

    group_head = json_read_from_file(file_path);
    if (group_head == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_IO);
        return ;
    }
    store_group = json_find_group(group_head, "kv_store");
    if (store_group == ft_nullptr)
    {
        json_free_groups(group_head);
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
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
                    ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
                    return ;
                }
            }
            else
            {
                json_free_groups(group_head);
                ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
                return ;
            }
            item_pointer = item_pointer->next;
            continue;
        }
        if (ttl_prefix_length > 0 && std::strncmp(item_pointer->key, g_kv_store_ttl_prefix, ttl_prefix_length) == 0)
        {
            ft_string ttl_key(item_pointer->key + ttl_prefix_length);
            long long expiration_timestamp;

            int ttl_key_error = storage_kv_move_string_error(ttl_key);
            if (ttl_key_error != FT_ERR_SUCCESSS)
            {
                json_free_groups(group_head);
                ft_global_error_stack_push(ttl_key_error);
                return ;
            }
            if (this->parse_expiration_timestamp(item_pointer->value, expiration_timestamp) != 0)
            {
                json_free_groups(group_head);
                return ;
            }
            ttl_metadata.insert(ttl_key, expiration_timestamp);
            if (storage_kv_pop_newest_error(ttl_metadata) != FT_ERR_SUCCESSS)
            {
                json_free_groups(group_head);
                ft_global_error_stack_push(ft_errno);
                return ;
            }
            item_pointer = item_pointer->next;
            continue;
        }
        ft_string key_storage(item_pointer->key);
        kv_store_entry entry;

        int key_storage_error = storage_kv_move_string_error(key_storage);
        if (key_storage_error != FT_ERR_SUCCESSS)
        {
            json_free_groups(group_head);
            ft_global_error_stack_push(key_storage_error);
            return ;
        }
        if (entry.configure_expiration(false, 0) != 0)
        {
            json_free_groups(group_head);
            ft_global_error_stack_push(entry.get_error());
            return ;
        }
        if (this->_encryption_enabled)
        {
            ft_string encoded_value(item_pointer->value);
            ft_string decrypted_value;

            int encoded_value_error = storage_kv_move_string_error(encoded_value);
            if (encoded_value_error != FT_ERR_SUCCESSS)
            {
                json_free_groups(group_head);
                ft_global_error_stack_push(encoded_value_error);
                return ;
            }
            if (this->decrypt_value(encoded_value, decrypted_value) != 0)
            {
                json_free_groups(group_head);
                return ;
            }
            if (entry.set_value(decrypted_value) != 0)
            {
                json_free_groups(group_head);
                ft_global_error_stack_push(entry.get_error());
                return ;
            }
        }
        else
        {
            ft_string plain_value(item_pointer->value);

            int plain_value_error = storage_kv_move_string_error(plain_value);
            if (plain_value_error != FT_ERR_SUCCESSS)
            {
                json_free_groups(group_head);
                ft_global_error_stack_push(plain_value_error);
                return ;
            }
            if (entry.set_value(plain_value) != 0)
            {
                json_free_groups(group_head);
                ft_global_error_stack_push(entry.get_error());
                return ;
            }
        }
        this->_data.insert(key_storage, entry);
        if (storage_kv_pop_newest_error(this->_data) != FT_ERR_SUCCESSS)
        {
            json_free_groups(group_head);
            ft_global_error_stack_push(ft_errno);
            return ;
        }
        item_pointer = item_pointer->next;
    }
    size_t ttl_size;

    ttl_size = ttl_metadata.size();
    if (storage_kv_pop_newest_error(ttl_metadata) != FT_ERR_SUCCESSS)
    {
        json_free_groups(group_head);
        ft_global_error_stack_push(ft_errno);
        return ;
    }
    if (ttl_size > 0)
    {
        const Pair<ft_string, long long> *ttl_end;
        const Pair<ft_string, long long> *ttl_begin;
        size_t ttl_index;

        ttl_end = ttl_metadata.end();
        if (storage_kv_pop_newest_error(ttl_metadata) != FT_ERR_SUCCESSS)
        {
            json_free_groups(group_head);
            ft_global_error_stack_push(ft_errno);
            return ;
        }
        ttl_begin = ttl_end - static_cast<std::ptrdiff_t>(ttl_size);
        ttl_index = 0;
        while (ttl_index < ttl_size)
        {
            const Pair<ft_string, long long> &ttl_entry = ttl_begin[ttl_index];
            Pair<ft_string, kv_store_entry> *data_pair;

            data_pair = this->_data.find(ttl_entry.key);
            if (storage_kv_pop_newest_error(this->_data) != FT_ERR_SUCCESSS)
            {
                json_free_groups(group_head);
                ft_global_error_stack_push(ft_errno);
                return ;
            }
            if (data_pair != ft_nullptr)
            {
                if (data_pair->value.configure_expiration(true, ttl_entry.value) != 0)
                {
                    json_free_groups(group_head);
                    ft_global_error_stack_push(data_pair->value.get_error());
                    return ;
                }
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
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

kv_store::~kv_store()
{
    this->stop_background_compaction();
    return ;
}
