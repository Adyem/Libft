#include "kv_store.hpp"

#include <cstddef>
#include <cstring>
#include <new>

#include "../Basic/basic.hpp"
#include "../Errno/errno_internal.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../Template/map.hpp"
#include "../Template/pair.hpp"
#include "../Template/vector.hpp"

kv_store::kv_store() noexcept
    : _initialised_state(FT_CLASS_STATE_UNINITIALISED)
    , _data()
    , _file_path()
    , _encryption_key()
    , _encryption_algorithm_name()
    , _encryption_enabled(FT_FALSE)
    , _backend_type(KV_STORE_BACKEND_JSON)
    , _background_thread_active(FT_FALSE)
    , _background_stop_requested(FT_FALSE)
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

int32_t kv_store::initialize(const kv_store &other) noexcept
{
    int32_t destroy_error;

    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_lifecycle(other._initialised_state,
            "kv_store::initialize(const kv_store &)",
            "source is uninitialised");
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        destroy_error = this->destroy();
        if (destroy_error != FT_ERR_SUCCESS)
            return (destroy_error);
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (FT_ERR_SUCCESS);
    }
    return (this->initialize(other._file_path.c_str(), other._encryption_key.c_str(),
        other._encryption_enabled, other._encryption_algorithm_name.c_str()));
}

int32_t kv_store::initialize(kv_store &&other) noexcept
{
    int32_t destroy_error;

    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_lifecycle(other._initialised_state,
            "kv_store::initialize(kv_store &&)", "source is uninitialised");
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        destroy_error = this->destroy();
        if (destroy_error != FT_ERR_SUCCESS)
            return (destroy_error);
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (FT_ERR_SUCCESS);
    }
    return (this->move(other));
}

int32_t kv_store::move(kv_store &other) noexcept
{
    int32_t destroy_error;
    int32_t initialize_error;

    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_lifecycle(other._initialised_state, "kv_store::move",
            "source is uninitialised");
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        {
            initialize_error = this->destroy();
            if (initialize_error != FT_ERR_SUCCESS)
                return (initialize_error);
        }
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (FT_ERR_SUCCESS);
    }
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        destroy_error = this->destroy();
        if (destroy_error != FT_ERR_SUCCESS)
            return (destroy_error);
    }
    initialize_error = this->initialize(other._file_path.c_str(),
        other._encryption_key.c_str(), other._encryption_enabled,
        other._encryption_algorithm_name.c_str());
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    (void)other.destroy();
    return (FT_ERR_SUCCESS);
}

int32_t kv_store::enable_thread_safety() noexcept
{
    pt_recursive_mutex *store_mutex;
    pt_recursive_mutex *background_mutex;
    pt_recursive_mutex *replication_mutex;
    int32_t mutex_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "kv_store::enable_thread_safety");
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

int32_t kv_store::disable_thread_safety() noexcept
{
    int32_t error_code;
    int32_t destroy_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "kv_store::disable_thread_safety");
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

ft_bool kv_store::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int32_t kv_store::initialize(const char *file_path, const char *encryption_key,
    ft_bool enable_encryption, const char *encryption_algorithm_name)
{
    json_group *group_head;
    json_group *store_group;
    json_item *item_pointer;
    ft_map<ft_string, int64_t> ttl_metadata;
    ft_size_t ttl_prefix_length;
    int32_t member_error;
    ft_bool data_initialised = FT_FALSE;
    ft_bool file_path_initialised = FT_FALSE;
    ft_bool encryption_key_initialised = FT_FALSE;
    ft_bool encryption_algorithm_name_initialised = FT_FALSE;
    ft_bool replication_sinks_initialised = FT_FALSE;

    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        errno_abort_lifecycle(this->_initialised_state, "kv_store::initialize", "initialize called on initialised instance");
    member_error = this->_data.initialize();
    if (member_error != FT_ERR_SUCCESS)
        return (this->cleanup_partial_initialization(FT_FALSE, FT_FALSE,
            FT_FALSE, FT_FALSE, member_error));
    data_initialised = FT_TRUE;
    member_error = this->_file_path.initialize();
    if (member_error != FT_ERR_SUCCESS)
        return (this->cleanup_partial_initialization(data_initialised, FT_FALSE,
            FT_FALSE, FT_FALSE, member_error));
    file_path_initialised = FT_TRUE;
    member_error = this->_encryption_key.initialize();
    if (member_error != FT_ERR_SUCCESS)
        return (this->cleanup_partial_initialization(data_initialised, file_path_initialised,
            FT_FALSE, FT_FALSE, member_error));
    encryption_key_initialised = FT_TRUE;
    member_error = this->_encryption_algorithm_name.initialize();
    if (member_error != FT_ERR_SUCCESS)
        return (this->cleanup_partial_initialization(data_initialised, file_path_initialised,
            encryption_key_initialised, FT_FALSE, member_error));
    encryption_algorithm_name_initialised = FT_TRUE;
    member_error = this->_replication_sinks.initialize();
    if (member_error != FT_ERR_SUCCESS)
        return (this->cleanup_partial_initialization(data_initialised, file_path_initialised,
            encryption_key_initialised, encryption_algorithm_name_initialised, member_error));
    replication_sinks_initialised = FT_TRUE;
    member_error = ttl_metadata.initialize();
    if (member_error != FT_ERR_SUCCESS)
        return (this->cleanup_partial_initialization(data_initialised, file_path_initialised,
            encryption_key_initialised, encryption_algorithm_name_initialised,
            replication_sinks_initialised, member_error));
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    this->_data.clear();
    this->_file_path.clear();
    this->_encryption_key.clear();
    this->_encryption_algorithm_name.clear();
    this->_backend_type = KV_STORE_BACKEND_JSON;
    this->_encryption_enabled = FT_FALSE;
    this->_background_thread_active = FT_FALSE;
    this->_background_stop_requested = FT_FALSE;
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
        return (this->cleanup_partial_initialization(data_initialised, file_path_initialised,
            encryption_key_initialised, encryption_algorithm_name_initialised,
            replication_sinks_initialised, FT_ERR_INVALID_ARGUMENT));
    }
    this->_file_path = file_path;
    if (enable_encryption)
    {
        if (encryption_key == ft_nullptr)
        {
            return (this->cleanup_partial_initialization(data_initialised, file_path_initialised,
                encryption_key_initialised, encryption_algorithm_name_initialised,
                replication_sinks_initialised, FT_ERR_INVALID_ARGUMENT));
        }
        this->_encryption_key = encryption_key;
        if (this->_encryption_key.size() != 16)
        {
            return (this->cleanup_partial_initialization(data_initialised, file_path_initialised,
                encryption_key_initialised, encryption_algorithm_name_initialised,
                replication_sinks_initialised, FT_ERR_INVALID_ARGUMENT));
        }
        if (encryption_algorithm_name == ft_nullptr)
            this->_encryption_algorithm_name = "aes-128-ecb-base64";
        else
            this->_encryption_algorithm_name = encryption_algorithm_name;
        if (this->_encryption_algorithm_name.size() == 0)
        {
            return (this->cleanup_partial_initialization(data_initialised, file_path_initialised,
                encryption_key_initialised, encryption_algorithm_name_initialised,
                replication_sinks_initialised, FT_ERR_INVALID_ARGUMENT));
        }
        this->_encryption_enabled = FT_TRUE;
    }
    else
    {
        this->_encryption_enabled = FT_FALSE;
        if (encryption_key != ft_nullptr)
        {
            this->_encryption_key = encryption_key;
        }
        else
            this->_encryption_key.clear();
        if (encryption_algorithm_name == ft_nullptr)
            this->_encryption_algorithm_name = "aes-128-ecb-base64";
        else
            this->_encryption_algorithm_name = encryption_algorithm_name;
    }

    group_head = json_read_from_file(file_path);
    if (group_head == ft_nullptr)
    {
        return (this->cleanup_partial_initialization(data_initialised, file_path_initialised,
            encryption_key_initialised, replication_sinks_initialised, FT_ERR_IO));
    }
    store_group = json_find_group(group_head, "kv_store");
    if (store_group == ft_nullptr)
    {
        json_free_groups(group_head);
        return (this->cleanup_partial_initialization(data_initialised, file_path_initialised,
            encryption_key_initialised, replication_sinks_initialised, FT_ERR_INVALID_ARGUMENT));
    }
    ttl_prefix_length = std::strlen(g_kv_store_ttl_prefix);
    item_pointer = store_group->items;
    while (item_pointer != ft_nullptr)
    {
        if (ft_strcmp(item_pointer->key, "__encryption__") == 0)
        {
            if (ft_strcmp(item_pointer->value, this->_encryption_algorithm_name.c_str()) == 0)
            {
                if (this->_encryption_enabled == FT_FALSE)
                {
                    json_free_groups(group_head);
                    return (this->cleanup_partial_initialization(data_initialised,
                        file_path_initialised, encryption_key_initialised,
                        replication_sinks_initialised, FT_ERR_INVALID_ARGUMENT));
                }
            }
            else
            {
                json_free_groups(group_head);
                return (this->cleanup_partial_initialization(data_initialised,
                    file_path_initialised, encryption_key_initialised,
                    replication_sinks_initialised, FT_ERR_INVALID_ARGUMENT));
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
                return (this->cleanup_partial_initialization(data_initialised,
                    file_path_initialised, encryption_key_initialised,
                    replication_sinks_initialised, FT_ERR_INVALID_OPERATION));
            }
            ttl_key = ttl_suffix;
            int64_t expiration_timestamp;
            if (this->parse_expiration_timestamp(item_pointer->value, expiration_timestamp) != 0)
            {
                json_free_groups(group_head);
                return (this->cleanup_partial_initialization(data_initialised,
                    file_path_initialised, encryption_key_initialised,
                    replication_sinks_initialised, FT_ERR_INVALID_ARGUMENT));
            }
            ttl_metadata.insert(ttl_key, expiration_timestamp);
            item_pointer = item_pointer->next;
            continue;
        }
        ft_string key_storage;
        if (key_storage.initialize(item_pointer->key) != FT_ERR_SUCCESS)
        {
            json_free_groups(group_head);
            return (this->cleanup_partial_initialization(data_initialised,
                file_path_initialised, encryption_key_initialised,
                replication_sinks_initialised, FT_ERR_INVALID_OPERATION));
        }
        kv_store_entry entry;
        if (entry.initialize() != FT_ERR_SUCCESS)
        {
            json_free_groups(group_head);
            return (this->cleanup_partial_initialization(data_initialised,
                file_path_initialised, encryption_key_initialised,
                replication_sinks_initialised, FT_ERR_INVALID_OPERATION));
        }
        if (entry.configure_expiration(FT_FALSE, 0) != 0)
        {
            json_free_groups(group_head);
            return (this->cleanup_partial_initialization(data_initialised,
                file_path_initialised, encryption_key_initialised,
                replication_sinks_initialised, FT_ERR_INVALID_OPERATION));
        }
        if (this->_encryption_enabled)
        {
            ft_string encoded_value;
            if (encoded_value.initialize(item_pointer->value) != FT_ERR_SUCCESS)
            {
                json_free_groups(group_head);
                return (this->cleanup_partial_initialization(data_initialised,
                    file_path_initialised, encryption_key_initialised,
                    replication_sinks_initialised, FT_ERR_INVALID_OPERATION));
            }
            ft_string decrypted_value;
            if (decrypted_value.initialize() != FT_ERR_SUCCESS)
            {
                json_free_groups(group_head);
                return (this->cleanup_partial_initialization(data_initialised,
                    file_path_initialised, encryption_key_initialised,
                    replication_sinks_initialised, FT_ERR_INVALID_OPERATION));
            }
            if (this->decrypt_value(encoded_value, decrypted_value) != 0)
            {
                json_free_groups(group_head);
                return (this->cleanup_partial_initialization(data_initialised,
                    file_path_initialised, encryption_key_initialised,
                    replication_sinks_initialised, FT_ERR_INVALID_ARGUMENT));
            }
            if (entry.set_value(decrypted_value) != 0)
            {
                json_free_groups(group_head);
                return (this->cleanup_partial_initialization(data_initialised,
                    file_path_initialised, encryption_key_initialised,
                    replication_sinks_initialised, FT_ERR_INVALID_OPERATION));
            }
        }
        else
        {
            ft_string plain_value;
            if (plain_value.initialize(item_pointer->value) != FT_ERR_SUCCESS)
            {
                json_free_groups(group_head);
                return (this->cleanup_partial_initialization(data_initialised,
                    file_path_initialised, encryption_key_initialised,
                    replication_sinks_initialised, FT_ERR_INVALID_OPERATION));
            }
            if (entry.set_value(plain_value) != 0)
            {
                json_free_groups(group_head);
                return (this->cleanup_partial_initialization(data_initialised,
                    file_path_initialised, encryption_key_initialised,
                    replication_sinks_initialised, FT_ERR_INVALID_OPERATION));
            }
        }
        this->_data.insert(key_storage, entry);
        item_pointer = item_pointer->next;
    }
    ft_size_t ttl_size;

    ttl_size = ttl_metadata.size();
    if (ttl_size > 0)
    {
        const Pair<ft_string, int64_t> *ttl_end;
        const Pair<ft_string, int64_t> *ttl_begin;
        ft_size_t ttl_index;

        ttl_end = ttl_metadata.end();
        ttl_begin = ttl_end - static_cast<std::ptrdiff_t>(ttl_size);
        ttl_index = 0;
        while (ttl_index < ttl_size)
        {
            const Pair<ft_string, int64_t> &ttl_entry = ttl_begin[ttl_index];
            Pair<ft_string, kv_store_entry> *data_pair;

            data_pair = this->_data.find(ttl_entry.key);
            if (data_pair != ft_nullptr)
            {
                if (data_pair->value.configure_expiration(FT_TRUE, ttl_entry.value) != 0)
                {
                    json_free_groups(group_head);
                    return (this->cleanup_partial_initialization(data_initialised,
                        file_path_initialised, encryption_key_initialised,
                        replication_sinks_initialised, FT_ERR_INVALID_OPERATION));
                }
            }
            ttl_index++;
        }
    }
    if (this->prune_expired() != 0)
    {
        json_free_groups(group_head);
        return (this->cleanup_partial_initialization(data_initialised, file_path_initialised,
            encryption_key_initialised, replication_sinks_initialised, FT_ERR_INVALID_OPERATION));
    }
    json_free_groups(group_head);
    return (FT_ERR_SUCCESS);
}

int32_t kv_store::destroy()
{
    int32_t first_error;
    int32_t disable_error;
    int32_t stop_error;

    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED
        || this->_initialised_state == FT_CLASS_STATE_DESTROYED)
        return (FT_ERR_SUCCESS);
    first_error = FT_ERR_SUCCESS;
    disable_error = this->disable_thread_safety();
    if (disable_error != FT_ERR_SUCCESS && first_error == FT_ERR_SUCCESS)
        first_error = disable_error;
    stop_error = this->stop_background_compaction();
    if (stop_error != FT_ERR_SUCCESS && first_error == FT_ERR_SUCCESS)
        first_error = stop_error;
    this->_data.clear();
    this->_replication_sinks.clear();
    this->_file_path.clear();
    this->_encryption_key.clear();
    this->_encryption_algorithm_name.clear();
    this->_encryption_enabled = FT_FALSE;
    this->_background_thread_active = FT_FALSE;
    this->_background_stop_requested = FT_FALSE;
    this->_background_interval_seconds = 0;
    this->_metrics_set_operations = 0;
    this->_metrics_delete_operations = 0;
    this->_metrics_get_hits = 0;
    this->_metrics_get_misses = 0;
    this->_metrics_prune_operations = 0;
    this->_metrics_pruned_entries = 0;
    this->_metrics_total_prune_duration_ms = 0;
    this->_metrics_last_prune_duration_ms = 0;
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return (first_error);
}

int32_t kv_store::cleanup_partial_initialization(ft_bool data_initialised, ft_bool file_path_initialised,
    ft_bool encryption_key_initialised, ft_bool encryption_algorithm_name_initialised,
    ft_bool replication_sinks_initialised, int32_t error_code) noexcept
{
    if (replication_sinks_initialised)
        (void)this->_replication_sinks.destroy();
    if (encryption_key_initialised)
        (void)this->_encryption_key.destroy();
    if (encryption_algorithm_name_initialised)
        (void)this->_encryption_algorithm_name.destroy();
    if (file_path_initialised)
        (void)this->_file_path.destroy();
    if (data_initialised)
        (void)this->_data.destroy();
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return (error_code);
}

int32_t kv_store::cleanup_partial_initialization(ft_bool data_initialised, ft_bool file_path_initialised,
    ft_bool encryption_key_initialised, ft_bool replication_sinks_initialised, int32_t error_code) noexcept
{
    int32_t cleanup_error;

    cleanup_error = this->cleanup_partial_initialization(data_initialised, file_path_initialised,
        encryption_key_initialised, FT_FALSE, replication_sinks_initialised, error_code);
    (void)this->_encryption_algorithm_name.destroy();
    return (cleanup_error);
}

kv_store::~kv_store() noexcept
{
    (void)this->destroy();
    return ;
}
