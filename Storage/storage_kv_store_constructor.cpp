#include "kv_store.hpp"

#include <cstddef>
#include <cstring>
#include <ctime>
#include <new>

#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"

void kv_store::abort_lifecycle_error(const char *method_name,
    const char *reason) const noexcept
{
    pf_printf_fd(2, "kv_store lifecycle error in %s: %s\n", method_name, reason);
    su_abort();
    return ;
}

void kv_store::abort_if_not_initialized(const char *method_name) const noexcept
{
    if (this->_initialized_state == kv_store::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name, "instance is not initialized");
    return ;
}

kv_store::kv_store(const char *, const char *, bool)
    : _initialized_state(kv_store::_state_uninitialized)
    , _data()
    , _file_path()
    , _encryption_key()
    , _encryption_enabled(false)
    , _backend_type(KV_STORE_BACKEND_JSON)
    , _background_thread_active(false)
    , _background_stop_requested(false)
    , _background_interval_seconds(0)
    , _background_thread()
    , _background_mutex(ft_nullptr)
    , _mutex(ft_nullptr)
    , _metrics_set_operations(0)
    , _metrics_delete_operations(0)
    , _metrics_get_hits(0)
    , _metrics_get_misses(0)
    , _metrics_prune_operations(0)
    , _metrics_pruned_entries(0)
    , _metrics_total_prune_duration_ms(0)
    , _metrics_last_prune_duration_ms(0)
    , _replication_sinks()
    , _replication_mutex(ft_nullptr)
{
    return ;
}

int kv_store::enable_thread_safety() noexcept
{
    pt_recursive_mutex *store_mutex;
    pt_recursive_mutex *background_mutex;
    pt_recursive_mutex *replication_mutex;
    int mutex_error;

    this->abort_if_not_initialized("kv_store::enable_thread_safety");
    if (this->_mutex != ft_nullptr && this->_background_mutex != ft_nullptr
        && this->_replication_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    store_mutex = new (std::nothrow) pt_recursive_mutex();
    if (store_mutex == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    mutex_error = store_mutex->initialize();
    if (mutex_error != FT_ERR_SUCCESS)
    {
        delete store_mutex;
        return (mutex_error);
    }
    background_mutex = new (std::nothrow) pt_recursive_mutex();
    if (background_mutex == ft_nullptr)
    {
        (void)store_mutex->destroy();
        delete store_mutex;
        return (FT_ERR_NO_MEMORY);
    }
    mutex_error = background_mutex->initialize();
    if (mutex_error != FT_ERR_SUCCESS)
    {
        delete background_mutex;
        (void)store_mutex->destroy();
        delete store_mutex;
        return (mutex_error);
    }
    replication_mutex = new (std::nothrow) pt_recursive_mutex();
    if (replication_mutex == ft_nullptr)
    {
        (void)background_mutex->destroy();
        delete background_mutex;
        (void)store_mutex->destroy();
        delete store_mutex;
        return (FT_ERR_NO_MEMORY);
    }
    mutex_error = replication_mutex->initialize();
    if (mutex_error != FT_ERR_SUCCESS)
    {
        delete replication_mutex;
        (void)background_mutex->destroy();
        delete background_mutex;
        (void)store_mutex->destroy();
        delete store_mutex;
        return (mutex_error);
    }
    this->_mutex = store_mutex;
    this->_background_mutex = background_mutex;
    this->_replication_mutex = replication_mutex;
    return (FT_ERR_SUCCESS);
}

int kv_store::disable_thread_safety() noexcept
{
    int error_code;
    int destroy_error;

    this->abort_if_not_initialized("kv_store::disable_thread_safety");
    error_code = FT_ERR_SUCCESS;
    if (this->_replication_mutex != ft_nullptr)
    {
        destroy_error = this->_replication_mutex->destroy();
        if (destroy_error != FT_ERR_SUCCESS && error_code == FT_ERR_SUCCESS)
            error_code = destroy_error;
        delete this->_replication_mutex;
        this->_replication_mutex = ft_nullptr;
    }
    if (this->_background_mutex != ft_nullptr)
    {
        destroy_error = this->_background_mutex->destroy();
        if (destroy_error != FT_ERR_SUCCESS && error_code == FT_ERR_SUCCESS)
            error_code = destroy_error;
        delete this->_background_mutex;
        this->_background_mutex = ft_nullptr;
    }
    if (this->_mutex != ft_nullptr)
    {
        destroy_error = this->_mutex->destroy();
        if (destroy_error != FT_ERR_SUCCESS && error_code == FT_ERR_SUCCESS)
            error_code = destroy_error;
        delete this->_mutex;
        this->_mutex = ft_nullptr;
    }
    return (error_code);
}

bool kv_store::is_thread_safe() const noexcept
{
    if (this->_mutex == ft_nullptr)
        return (false);
    if (this->_background_mutex == ft_nullptr)
        return (false);
    if (this->_replication_mutex == ft_nullptr)
        return (false);
    return (true);
}

int kv_store::initialize(const char *file_path, const char *encryption_key, bool enable_encryption)
{
    json_group *group_head;
    json_group *store_group;
    json_item *item_pointer;
    ft_map<ft_string, long long> ttl_metadata;
    size_t ttl_prefix_length;

    if (this->_initialized_state == kv_store::_state_initialized)
        this->abort_lifecycle_error("kv_store::initialize", "initialize called on initialized instance");
    this->_initialized_state = kv_store::_state_initialized;
    this->_data.clear();
    this->_file_path.clear();
    this->_encryption_key.clear();
    this->_backend_type = KV_STORE_BACKEND_JSON;
    this->_encryption_enabled = false;
    this->_background_thread_active = false;
    this->_background_stop_requested = false;
    this->_background_interval_seconds = 0;
    this->_metrics_set_operations = 0;
    this->_metrics_delete_operations = 0;
    this->_metrics_get_hits = 0;
    this->_metrics_get_misses = 0;
    this->_metrics_prune_operations = 0;
    this->_metrics_pruned_entries = 0;
    this->_metrics_total_prune_duration_ms = 0;
    this->_metrics_last_prune_duration_ms = 0;
    this->_replication_sinks.clear();
    if (file_path == ft_nullptr)
    {
        this->_file_path.clear();
        this->_initialized_state = kv_store::_state_destroyed;
        return (FT_ERR_INVALID_ARGUMENT);
    }
    this->_file_path = file_path;
    if (enable_encryption)
    {
        if (encryption_key == ft_nullptr)
        {
            this->_initialized_state = kv_store::_state_destroyed;
            return (FT_ERR_INVALID_ARGUMENT);
        }
        this->_encryption_key = encryption_key;
        if (this->_encryption_key.size() != 16)
        {
            this->_initialized_state = kv_store::_state_destroyed;
            return (FT_ERR_INVALID_ARGUMENT);
        }
        this->_encryption_enabled = true;
    }
    else
    {
        this->_encryption_enabled = false;
        if (encryption_key != ft_nullptr)
        {
            this->_encryption_key = encryption_key;
        }
        else
            this->_encryption_key.clear();
    }

    group_head = json_read_from_file(file_path);
    if (group_head == ft_nullptr)
    {
        this->_initialized_state = kv_store::_state_destroyed;
        return (FT_ERR_IO);
    }
    store_group = json_find_group(group_head, "kv_store");
    if (store_group == ft_nullptr)
    {
        json_free_groups(group_head);
        this->_initialized_state = kv_store::_state_destroyed;
        return (FT_ERR_INVALID_ARGUMENT);
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
                    this->_initialized_state = kv_store::_state_destroyed;
                    return (FT_ERR_INVALID_ARGUMENT);
                }
            }
            else
            {
                json_free_groups(group_head);
                this->_initialized_state = kv_store::_state_destroyed;
                return (FT_ERR_INVALID_ARGUMENT);
            }
            item_pointer = item_pointer->next;
            continue;
        }
        if (ttl_prefix_length > 0 && std::strncmp(item_pointer->key, g_kv_store_ttl_prefix, ttl_prefix_length) == 0)
        {
            ft_string ttl_key;
            ft_string ttl_suffix;
            if (ttl_suffix.initialize(item_pointer->key + ttl_prefix_length) != FT_ERR_SUCCESS)
            {
                json_free_groups(group_head);
                this->_initialized_state = kv_store::_state_destroyed;
                return (FT_ERR_INVALID_OPERATION);
            }
            ttl_key = ttl_suffix;
            long long expiration_timestamp;
            if (this->parse_expiration_timestamp(item_pointer->value, expiration_timestamp) != 0)
            {
                json_free_groups(group_head);
                this->_initialized_state = kv_store::_state_destroyed;
                return (FT_ERR_INVALID_ARGUMENT);
            }
            ttl_metadata.insert(ttl_key, expiration_timestamp);
            item_pointer = item_pointer->next;
            continue;
        }
        ft_string key_storage;
        if (key_storage.initialize(item_pointer->key) != FT_ERR_SUCCESS)
        {
            json_free_groups(group_head);
            this->_initialized_state = kv_store::_state_destroyed;
            return (FT_ERR_INVALID_OPERATION);
        }
        kv_store_entry entry;
        if (entry.initialize() != FT_ERR_SUCCESS)
        {
            json_free_groups(group_head);
            this->_initialized_state = kv_store::_state_destroyed;
            return (FT_ERR_INVALID_OPERATION);
        }
        if (entry.configure_expiration(false, 0) != 0)
        {
            json_free_groups(group_head);
            this->_initialized_state = kv_store::_state_destroyed;
            return (FT_ERR_INVALID_OPERATION);
        }
        if (this->_encryption_enabled)
        {
            ft_string encoded_value;
            if (encoded_value.initialize(item_pointer->value) != FT_ERR_SUCCESS)
            {
                json_free_groups(group_head);
                this->_initialized_state = kv_store::_state_destroyed;
                return (FT_ERR_INVALID_OPERATION);
            }
            ft_string decrypted_value;
            if (this->decrypt_value(encoded_value, decrypted_value) != 0)
            {
                json_free_groups(group_head);
                this->_initialized_state = kv_store::_state_destroyed;
                return (FT_ERR_INVALID_ARGUMENT);
            }
            if (entry.set_value(decrypted_value) != 0)
            {
                json_free_groups(group_head);
                this->_initialized_state = kv_store::_state_destroyed;
                return (FT_ERR_INVALID_OPERATION);
            }
        }
        else
        {
            ft_string plain_value;
            if (plain_value.initialize(item_pointer->value) != FT_ERR_SUCCESS)
            {
                json_free_groups(group_head);
                this->_initialized_state = kv_store::_state_destroyed;
                return (FT_ERR_INVALID_OPERATION);
            }
            if (entry.set_value(plain_value) != 0)
            {
                json_free_groups(group_head);
                this->_initialized_state = kv_store::_state_destroyed;
                return (FT_ERR_INVALID_OPERATION);
            }
        }
        this->_data.insert(key_storage, entry);
        item_pointer = item_pointer->next;
    }
    size_t ttl_size;

    ttl_size = ttl_metadata.size();
    if (ttl_size > 0)
    {
        const Pair<ft_string, long long> *ttl_end;
        const Pair<ft_string, long long> *ttl_begin;
        size_t ttl_index;

        ttl_end = ttl_metadata.end();
        ttl_begin = ttl_end - static_cast<std::ptrdiff_t>(ttl_size);
        ttl_index = 0;
        while (ttl_index < ttl_size)
        {
            const Pair<ft_string, long long> &ttl_entry = ttl_begin[ttl_index];
            Pair<ft_string, kv_store_entry> *data_pair;

            data_pair = this->_data.find(ttl_entry.key);
            if (data_pair != ft_nullptr)
            {
                if (data_pair->value.configure_expiration(true, ttl_entry.value) != 0)
                {
                    json_free_groups(group_head);
                    this->_initialized_state = kv_store::_state_destroyed;
                    return (FT_ERR_INVALID_OPERATION);
                }
            }
            ttl_index++;
        }
    }
    if (this->prune_expired() != 0)
    {
        json_free_groups(group_head);
        this->_initialized_state = kv_store::_state_destroyed;
        return (FT_ERR_INVALID_OPERATION);
    }
    json_free_groups(group_head);
    return (FT_ERR_SUCCESS);
}

int kv_store::destroy()
{
    int disable_error;

    if (this->_initialized_state != kv_store::_state_initialized)
        return (FT_ERR_INVALID_STATE);
    (void)this->stop_background_compaction();
    this->_data.clear();
    this->_replication_sinks.clear();
    this->_file_path.clear();
    this->_encryption_key.clear();
    this->_encryption_enabled = false;
    this->_background_thread_active = false;
    this->_background_stop_requested = false;
    this->_background_interval_seconds = 0;
    this->_metrics_set_operations = 0;
    this->_metrics_delete_operations = 0;
    this->_metrics_get_hits = 0;
    this->_metrics_get_misses = 0;
    this->_metrics_prune_operations = 0;
    this->_metrics_pruned_entries = 0;
    this->_metrics_total_prune_duration_ms = 0;
    this->_metrics_last_prune_duration_ms = 0;
    disable_error = this->disable_thread_safety();
    if (disable_error != FT_ERR_SUCCESS)
        return (disable_error);
    this->_initialized_state = kv_store::_state_destroyed;
    return (FT_ERR_SUCCESS);
}

kv_store::~kv_store()
{
    if (this->_initialized_state == kv_store::_state_initialized)
        (void)this->destroy();
    return ;
}
