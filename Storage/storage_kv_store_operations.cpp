#include "kv_store.hpp"

#include <cstddef>
#include <cerrno>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <ctime>

#include "../Libft/libft.hpp"
#include "../Template/move.hpp"

const char *g_kv_store_ttl_prefix = "__ttl__";

int kv_store_init_set_operation(kv_store_operation &operation, const char *key_string, const char *value_string, long long ttl_seconds)
{
    if (key_string == ft_nullptr || value_string == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    operation._type = KV_STORE_OPERATION_TYPE_SET;
    operation._key = key_string;
    if (operation._key.get_error() != ER_SUCCESS)
    {
        ft_errno = operation._key.get_error();
        return (-1);
    }
    operation._value = value_string;
    if (operation._value.get_error() != ER_SUCCESS)
    {
        ft_errno = operation._value.get_error();
        return (-1);
    }
    operation._has_value = true;
    if (ttl_seconds >= 0)
    {
        operation._has_ttl = true;
        operation._ttl_seconds = ttl_seconds;
    }
    else
    {
        operation._has_ttl = false;
        operation._ttl_seconds = -1;
    }
    ft_errno = ER_SUCCESS;
    return (0);
}

int kv_store_init_delete_operation(kv_store_operation &operation, const char *key_string)
{
    if (key_string == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    operation._type = KV_STORE_OPERATION_TYPE_DELETE;
    operation._key = key_string;
    if (operation._key.get_error() != ER_SUCCESS)
    {
        ft_errno = operation._key.get_error();
        return (-1);
    }
    operation._value.clear();
    operation._has_value = false;
    operation._has_ttl = false;
    operation._ttl_seconds = -1;
    ft_errno = ER_SUCCESS;
    return (0);
}

void kv_store::set_error_unlocked(int error_code) const noexcept
{
    ft_errno = error_code;
    this->_error_code = error_code;
    return ;
}

void kv_store::set_error(int error_code) const noexcept
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;

    entry_errno = ft_errno;
    lock_error = this->lock_store(guard);
    if (lock_error != ER_SUCCESS)
    {
        ft_errno = lock_error;
        return ;
    }
    this->set_error_unlocked(error_code);
    kv_store::restore_errno(guard, entry_errno);
    return ;
}

int kv_store::lock_store(ft_unique_lock<pt_mutex> &guard) const noexcept
{
    int entry_errno;
    ft_unique_lock<pt_mutex> local_guard(this->_mutex);

    entry_errno = ft_errno;
    if (local_guard.get_error() != ER_SUCCESS)
    {
        ft_errno = entry_errno;
        guard = ft_unique_lock<pt_mutex>();
        return (local_guard.get_error());
    }
    ft_errno = entry_errno;
    guard = ft_move(local_guard);
    return (ER_SUCCESS);
}

void kv_store::restore_errno(ft_unique_lock<pt_mutex> &guard, int entry_errno) noexcept
{
    int current_errno;

    current_errno = ft_errno;
    if (guard.owns_lock())
        guard.unlock();
    if (guard.get_error() != ER_SUCCESS)
    {
        ft_errno = guard.get_error();
        return ;
    }
    if (current_errno != ER_SUCCESS)
    {
        ft_errno = current_errno;
        return ;
    }
    ft_errno = entry_errno;
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
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    int prune_result;

    entry_errno = ft_errno;
    lock_error = this->lock_store(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error_unlocked(lock_error);
        kv_store::restore_errno(guard, entry_errno);
        return (-1);
    }
    prune_result = this->prune_expired_locked(guard);
    kv_store::restore_errno(guard, entry_errno);
    return (prune_result);
}

int kv_store::prune_expired_locked(ft_unique_lock<pt_mutex> &guard)
{
    size_t map_size;
    long long current_time;
    size_t map_index;

    if (guard.owns_lock() == false)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    map_size = this->_data.size();
    if (this->_data.get_error() != ER_SUCCESS)
    {
        this->set_error_unlocked(this->_data.get_error());
        return (-1);
    }
    if (map_size == 0)
    {
        this->set_error_unlocked(ER_SUCCESS);
        return (0);
    }
    current_time = this->current_time_seconds();
    if (current_time < 0)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    map_index = 0;
    while (map_index < map_size)
    {
        Pair<ft_string, kv_store_entry> *map_end;
        Pair<ft_string, kv_store_entry> *map_begin;
        Pair<ft_string, kv_store_entry> *entry_pointer;
        bool has_expiration;

        map_end = this->_data.end();
        if (this->_data.get_error() != ER_SUCCESS)
        {
            this->set_error_unlocked(this->_data.get_error());
            return (-1);
        }
        map_begin = map_end - static_cast<std::ptrdiff_t>(map_size);
        entry_pointer = map_begin + map_index;
        if (entry_pointer == ft_nullptr)
        {
            this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
            return (-1);
        }
        has_expiration = false;
        if (entry_pointer->value.has_expiration(has_expiration) != 0)
        {
            this->set_error_unlocked(entry_pointer->value.get_error());
            return (-1);
        }
        if (has_expiration)
        {
            long long expiration_timestamp;

            if (entry_pointer->value.get_expiration(expiration_timestamp) != 0)
            {
                this->set_error_unlocked(entry_pointer->value.get_error());
                return (-1);
            }
            if (expiration_timestamp <= current_time)
            {
                this->_data.remove(entry_pointer->key);
                if (this->_data.get_error() != ER_SUCCESS)
                {
                    this->set_error_unlocked(this->_data.get_error());
                    return (-1);
                }
                map_size = this->_data.size();
                if (this->_data.get_error() != ER_SUCCESS)
                {
                    this->set_error_unlocked(this->_data.get_error());
                    return (-1);
                }
                continue ;
            }
        }
        map_index++;
    }
    this->set_error_unlocked(ER_SUCCESS);
    return (0);
}

int kv_store::kv_set(const char *key_string, const char *value_string, long long ttl_seconds)
{
    ft_string key_storage;
    ft_string value_storage;
    ft_unique_lock<pt_mutex> guard;
    Pair<ft_string, kv_store_entry> *existing_pair;
    bool has_expiration;
    long long expiration_timestamp;
    int entry_errno;
    int lock_error;

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
    entry_errno = ft_errno;
    lock_error = this->lock_store(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error_unlocked(lock_error);
        kv_store::restore_errno(guard, entry_errno);
        return (-1);
    }
    if (this->prune_expired_locked(guard) != 0)
    {
        kv_store::restore_errno(guard, entry_errno);
        return (-1);
    }
    existing_pair = this->_data.find(key_storage);
    if (this->_data.get_error() != ER_SUCCESS)
    {
        this->set_error_unlocked(this->_data.get_error());
        kv_store::restore_errno(guard, entry_errno);
        return (-1);
    }
    has_expiration = false;
    expiration_timestamp = 0;
    if (ttl_seconds >= 0)
    {
        if (this->compute_expiration(ttl_seconds, has_expiration, expiration_timestamp) != 0)
        {
            kv_store::restore_errno(guard, entry_errno);
            return (-1);
        }
        if (has_expiration)
        {
            long long current_time;

            current_time = this->current_time_seconds();
            if (current_time < 0)
            {
                this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
                kv_store::restore_errno(guard, entry_errno);
                return (-1);
            }
            if (expiration_timestamp <= current_time)
            {
                if (existing_pair != ft_nullptr)
                {
                    this->_data.remove(key_storage);
                    if (this->_data.get_error() != ER_SUCCESS)
                    {
                        this->set_error_unlocked(this->_data.get_error());
                        kv_store::restore_errno(guard, entry_errno);
                        return (-1);
                    }
                }
                this->set_error_unlocked(ER_SUCCESS);
                kv_store::restore_errno(guard, entry_errno);
                return (0);
            }
        }
    }
    else if (existing_pair != ft_nullptr)
    {
        bool existing_has_expiration;

        existing_has_expiration = false;
        if (existing_pair->value.has_expiration(existing_has_expiration) != 0)
        {
            this->set_error_unlocked(existing_pair->value.get_error());
            kv_store::restore_errno(guard, entry_errno);
            return (-1);
        }
        if (existing_has_expiration)
        {
            if (existing_pair->value.get_expiration(expiration_timestamp) != 0)
            {
                this->set_error_unlocked(existing_pair->value.get_error());
                kv_store::restore_errno(guard, entry_errno);
                return (-1);
            }
            has_expiration = true;
        }
    }
    if (has_expiration)
    {
        long long current_time;

        current_time = this->current_time_seconds();
        if (current_time < 0)
        {
            this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
            kv_store::restore_errno(guard, entry_errno);
            return (-1);
        }
        if (expiration_timestamp <= current_time)
        {
            if (existing_pair != ft_nullptr)
            {
                this->_data.remove(key_storage);
                if (this->_data.get_error() != ER_SUCCESS)
                {
                    this->set_error_unlocked(this->_data.get_error());
                    kv_store::restore_errno(guard, entry_errno);
                    return (-1);
                }
            }
            this->set_error_unlocked(ER_SUCCESS);
            kv_store::restore_errno(guard, entry_errno);
            return (0);
        }
    }
    if (existing_pair != ft_nullptr)
    {
        if (existing_pair->value.set_value(value_storage) != 0)
        {
            this->set_error_unlocked(existing_pair->value.get_error());
            kv_store::restore_errno(guard, entry_errno);
            return (-1);
        }
        if (existing_pair->value.configure_expiration(has_expiration, expiration_timestamp) != 0)
        {
            this->set_error_unlocked(existing_pair->value.get_error());
            kv_store::restore_errno(guard, entry_errno);
            return (-1);
        }
    }
    else
    {
        kv_store_entry entry;

        if (entry.set_value(value_storage) != 0)
        {
            this->set_error_unlocked(entry.get_error());
            kv_store::restore_errno(guard, entry_errno);
            return (-1);
        }
        if (entry.configure_expiration(has_expiration, expiration_timestamp) != 0)
        {
            this->set_error_unlocked(entry.get_error());
            kv_store::restore_errno(guard, entry_errno);
            return (-1);
        }
        this->_data.insert(key_storage, entry);
        if (this->_data.get_error() != ER_SUCCESS)
        {
            this->set_error_unlocked(this->_data.get_error());
            kv_store::restore_errno(guard, entry_errno);
            return (-1);
        }
    }
    this->set_error_unlocked(ER_SUCCESS);
    kv_store::restore_errno(guard, entry_errno);
    return (0);
}

const char *kv_store::kv_get(const char *key_string) const
{
    ft_string key_storage;
    const Pair<ft_string, kv_store_entry> *map_pair;
    ft_unique_lock<pt_mutex> guard;
    kv_store *mutable_this;
    int entry_errno;
    int lock_error;
    const char *value_pointer;

    if (key_string == ft_nullptr)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    key_storage = key_string;
    if (key_storage.get_error() != ER_SUCCESS)
    {
        this->set_error(key_storage.get_error());
        return (ft_nullptr);
    }
    entry_errno = ft_errno;
    lock_error = this->lock_store(guard);
    if (lock_error != ER_SUCCESS)
    {
        mutable_this = const_cast<kv_store *>(this);
        mutable_this->set_error_unlocked(lock_error);
        kv_store::restore_errno(guard, entry_errno);
        return (ft_nullptr);
    }
    mutable_this = const_cast<kv_store *>(this);
    if (mutable_this->prune_expired_locked(guard) != 0)
    {
        kv_store::restore_errno(guard, entry_errno);
        return (ft_nullptr);
    }
    map_pair = this->_data.find(key_storage);
    if (this->_data.get_error() != ER_SUCCESS)
    {
        mutable_this->set_error_unlocked(this->_data.get_error());
        kv_store::restore_errno(guard, entry_errno);
        return (ft_nullptr);
    }
    if (map_pair == ft_nullptr)
    {
        mutable_this->set_error_unlocked(FT_ERR_NOT_FOUND);
        kv_store::restore_errno(guard, entry_errno);
        return (ft_nullptr);
    }
    value_pointer = ft_nullptr;
    if (map_pair->value.get_value_pointer(&value_pointer) != 0)
    {
        mutable_this->set_error_unlocked(map_pair->value.get_error());
        kv_store::restore_errno(guard, entry_errno);
        return (ft_nullptr);
    }
    mutable_this->set_error_unlocked(ER_SUCCESS);
    kv_store::restore_errno(guard, entry_errno);
    return (value_pointer);
}

int kv_store::kv_delete(const char *key_string)
{
    ft_string key_storage;
    const Pair<ft_string, kv_store_entry> *map_pair;
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;

    if (key_string == ft_nullptr)
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
    entry_errno = ft_errno;
    lock_error = this->lock_store(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error_unlocked(lock_error);
        kv_store::restore_errno(guard, entry_errno);
        return (-1);
    }
    if (this->prune_expired_locked(guard) != 0)
    {
        kv_store::restore_errno(guard, entry_errno);
        return (-1);
    }
    map_pair = this->_data.find(key_storage);
    if (this->_data.get_error() != ER_SUCCESS)
    {
        this->set_error_unlocked(this->_data.get_error());
        kv_store::restore_errno(guard, entry_errno);
        return (-1);
    }
    if (map_pair == ft_nullptr)
    {
        this->set_error_unlocked(FT_ERR_NOT_FOUND);
        kv_store::restore_errno(guard, entry_errno);
        return (-1);
    }
    this->_data.remove(key_storage);
    if (this->_data.get_error() != ER_SUCCESS)
    {
        this->set_error_unlocked(this->_data.get_error());
        kv_store::restore_errno(guard, entry_errno);
        return (-1);
    }
    this->set_error_unlocked(ER_SUCCESS);
    kv_store::restore_errno(guard, entry_errno);
    return (0);
}

int kv_store::kv_flush() const
{
    json_group *store_group;
    json_group *head_group;
    json_item *item_pointer;
    ft_string stored_value;
    ft_string plain_value;
    const Pair<ft_string, kv_store_entry> *map_end;
    const Pair<ft_string, kv_store_entry> *map_begin;
    size_t map_size;
    size_t map_index;
    int result;
    int error_code;
    kv_store *mutable_this;
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;

    mutable_this = const_cast<kv_store *>(this);
    entry_errno = ft_errno;
    lock_error = this->lock_store(guard);
    if (lock_error != ER_SUCCESS)
    {
        mutable_this->set_error_unlocked(lock_error);
        kv_store::restore_errno(guard, entry_errno);
        return (-1);
    }
    if (mutable_this->prune_expired_locked(guard) != 0)
    {
        kv_store::restore_errno(guard, entry_errno);
        return (-1);
    }
    store_group = json_create_json_group("kv_store");
    if (store_group == ft_nullptr)
    {
        mutable_this->set_error_unlocked(FT_ERR_NO_MEMORY);
        kv_store::restore_errno(guard, entry_errno);
        return (-1);
    }
    if (this->_encryption_enabled)
    {
        item_pointer = json_create_item("__encryption__", "aes-128-ecb-base64");
        if (item_pointer == ft_nullptr)
        {
            json_free_groups(store_group);
            mutable_this->set_error_unlocked(FT_ERR_NO_MEMORY);
            kv_store::restore_errno(guard, entry_errno);
            return (-1);
        }
        json_add_item_to_group(store_group, item_pointer);
    }
    map_size = this->_data.size();
    if (this->_data.get_error() != ER_SUCCESS)
    {
        json_free_groups(store_group);
        mutable_this->set_error_unlocked(this->_data.get_error());
        kv_store::restore_errno(guard, entry_errno);
        return (-1);
    }
    map_end = this->_data.end();
    if (this->_data.get_error() != ER_SUCCESS)
    {
        json_free_groups(store_group);
        mutable_this->set_error_unlocked(this->_data.get_error());
        kv_store::restore_errno(guard, entry_errno);
        return (-1);
    }
    map_begin = map_end - static_cast<std::ptrdiff_t>(map_size);
    map_index = 0;
    while (map_index < map_size)
    {
        const Pair<ft_string, kv_store_entry> &entry = map_begin[map_index];
        const kv_store_entry &entry_value = entry.value;
        bool entry_has_expiration;
        long long expiration_timestamp;

        plain_value.clear();
        if (plain_value.get_error() != ER_SUCCESS)
        {
            json_free_groups(store_group);
            mutable_this->set_error_unlocked(plain_value.get_error());
            kv_store::restore_errno(guard, entry_errno);
            return (-1);
        }
        if (const_cast<kv_store_entry &>(entry_value).copy_value(plain_value) != 0)
        {
            json_free_groups(store_group);
            mutable_this->set_error_unlocked(entry_value.get_error());
            kv_store::restore_errno(guard, entry_errno);
            return (-1);
        }
        if (this->_encryption_enabled)
        {
            stored_value.clear();
            if (stored_value.get_error() != ER_SUCCESS)
            {
                json_free_groups(store_group);
                mutable_this->set_error_unlocked(stored_value.get_error());
                kv_store::restore_errno(guard, entry_errno);
                return (-1);
            }
            if (this->encrypt_value(plain_value, stored_value) != 0)
            {
                json_free_groups(store_group);
                kv_store::restore_errno(guard, entry_errno);
                return (-1);
            }
            item_pointer = json_create_item(entry.key.c_str(), stored_value.c_str());
        }
        else
            item_pointer = json_create_item(entry.key.c_str(), plain_value.c_str());
        if (item_pointer == ft_nullptr)
        {
            json_free_groups(store_group);
            mutable_this->set_error_unlocked(FT_ERR_NO_MEMORY);
            kv_store::restore_errno(guard, entry_errno);
            return (-1);
        }
        json_add_item_to_group(store_group, item_pointer);
        entry_has_expiration = false;
        if (entry_value.has_expiration(entry_has_expiration) != 0)
        {
            json_free_groups(store_group);
            mutable_this->set_error_unlocked(entry_value.get_error());
            kv_store::restore_errno(guard, entry_errno);
            return (-1);
        }
        if (entry_has_expiration)
        {
            ft_string ttl_key;
            char expiration_buffer[32];
            int written_length;

            if (entry_value.get_expiration(expiration_timestamp) != 0)
            {
                json_free_groups(store_group);
                mutable_this->set_error_unlocked(entry_value.get_error());
                kv_store::restore_errno(guard, entry_errno);
                return (-1);
            }
            ttl_key = g_kv_store_ttl_prefix;
            if (ttl_key.get_error() != ER_SUCCESS)
            {
                json_free_groups(store_group);
                mutable_this->set_error_unlocked(ttl_key.get_error());
                kv_store::restore_errno(guard, entry_errno);
                return (-1);
            }
            ttl_key += entry.key;
            if (ttl_key.get_error() != ER_SUCCESS)
            {
                json_free_groups(store_group);
                mutable_this->set_error_unlocked(ttl_key.get_error());
                kv_store::restore_errno(guard, entry_errno);
                return (-1);
            }
            written_length = std::snprintf(expiration_buffer, sizeof(expiration_buffer), "%lld", expiration_timestamp);
            if (written_length < 0 || static_cast<size_t>(written_length) >= sizeof(expiration_buffer))
            {
                json_free_groups(store_group);
                mutable_this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
                kv_store::restore_errno(guard, entry_errno);
                return (-1);
            }
            item_pointer = json_create_item(ttl_key.c_str(), expiration_buffer);
            if (item_pointer == ft_nullptr)
            {
                json_free_groups(store_group);
                mutable_this->set_error_unlocked(FT_ERR_NO_MEMORY);
                kv_store::restore_errno(guard, entry_errno);
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
            mutable_this->set_error_unlocked(error_code);
        else
            mutable_this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        kv_store::restore_errno(guard, entry_errno);
        return (-1);
    }
    mutable_this->set_error_unlocked(ER_SUCCESS);
    kv_store::restore_errno(guard, entry_errno);
    return (0);
}

int kv_store::kv_apply(const ft_vector<kv_store_operation> &operations)
{
    ft_unique_lock<pt_mutex> guard;
    ft_map<ft_string, kv_store_entry> staged_map;
    size_t operation_count;
    size_t operation_index;
    int entry_errno;
    int lock_error;

    if (operations.get_error() != ER_SUCCESS)
    {
        this->set_error(operations.get_error());
        return (-1);
    }
    entry_errno = ft_errno;
    lock_error = this->lock_store(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error_unlocked(lock_error);
        kv_store::restore_errno(guard, entry_errno);
        return (-1);
    }
    if (this->prune_expired_locked(guard) != 0)
    {
        kv_store::restore_errno(guard, entry_errno);
        return (-1);
    }
    staged_map = ft_map<ft_string, kv_store_entry>(this->_data);
    if (staged_map.get_error() != ER_SUCCESS)
    {
        this->set_error_unlocked(staged_map.get_error());
        kv_store::restore_errno(guard, entry_errno);
        return (-1);
    }
    operation_count = operations.size();
    operation_index = 0;
    while (operation_index < operation_count)
    {
        const kv_store_operation &operation = operations[operation_index];

        if (operation._key.c_str() == ft_nullptr || operation._key.size() == 0)
        {
            this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
            kv_store::restore_errno(guard, entry_errno);
            return (-1);
        }
        if (operation._type == KV_STORE_OPERATION_TYPE_DELETE)
        {
            Pair<ft_string, kv_store_entry> *existing_pair;

            existing_pair = staged_map.find(operation._key);
            if (staged_map.get_error() != ER_SUCCESS)
            {
                this->set_error_unlocked(staged_map.get_error());
                kv_store::restore_errno(guard, entry_errno);
                return (-1);
            }
            if (existing_pair == ft_nullptr)
            {
                this->set_error_unlocked(FT_ERR_NOT_FOUND);
                kv_store::restore_errno(guard, entry_errno);
                return (-1);
            }
            staged_map.remove(operation._key);
            if (staged_map.get_error() != ER_SUCCESS)
            {
                this->set_error_unlocked(staged_map.get_error());
                kv_store::restore_errno(guard, entry_errno);
                return (-1);
            }
        }
        else
        {
            Pair<ft_string, kv_store_entry> *existing_pair;
            kv_store_entry new_entry;
            bool has_expiration;
            long long expiration_timestamp;

            if (operation._has_value == false)
            {
                this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
                kv_store::restore_errno(guard, entry_errno);
                return (-1);
            }
            if (new_entry.set_value(operation._value) != 0)
            {
                this->set_error_unlocked(new_entry.get_error());
                kv_store::restore_errno(guard, entry_errno);
                return (-1);
            }
            existing_pair = staged_map.find(operation._key);
            if (staged_map.get_error() != ER_SUCCESS)
            {
                this->set_error_unlocked(staged_map.get_error());
                kv_store::restore_errno(guard, entry_errno);
                return (-1);
            }
            has_expiration = false;
            expiration_timestamp = 0;
            if (operation._has_ttl)
            {
                if (this->compute_expiration(operation._ttl_seconds, has_expiration, expiration_timestamp) != 0)
                {
                    kv_store::restore_errno(guard, entry_errno);
                    return (-1);
                }
                if (has_expiration)
                {
                    long long current_time;

                    current_time = this->current_time_seconds();
                    if (current_time < 0)
                    {
                        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
                        kv_store::restore_errno(guard, entry_errno);
                        return (-1);
                    }
                    if (expiration_timestamp <= current_time)
                    {
                        staged_map.remove(operation._key);
                        if (staged_map.get_error() != ER_SUCCESS)
                        {
                            this->set_error_unlocked(staged_map.get_error());
                            kv_store::restore_errno(guard, entry_errno);
                            return (-1);
                        }
                        operation_index++;
                        continue ;
                    }
                }
            }
            else if (existing_pair != ft_nullptr)
            {
                bool existing_has_expiration;

                existing_has_expiration = false;
                if (existing_pair->value.has_expiration(existing_has_expiration) != 0)
                {
                    this->set_error_unlocked(existing_pair->value.get_error());
                    kv_store::restore_errno(guard, entry_errno);
                    return (-1);
                }
                if (existing_has_expiration)
                {
                    if (existing_pair->value.get_expiration(expiration_timestamp) != 0)
                    {
                        this->set_error_unlocked(existing_pair->value.get_error());
                        kv_store::restore_errno(guard, entry_errno);
                        return (-1);
                    }
                    has_expiration = true;
                }
            }
            if (has_expiration)
            {
                long long current_time;

                current_time = this->current_time_seconds();
                if (current_time < 0)
                {
                    this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
                    kv_store::restore_errno(guard, entry_errno);
                    return (-1);
                }
                if (expiration_timestamp <= current_time)
                {
                    staged_map.remove(operation._key);
                    if (staged_map.get_error() != ER_SUCCESS)
                    {
                        this->set_error_unlocked(staged_map.get_error());
                        kv_store::restore_errno(guard, entry_errno);
                        return (-1);
                    }
                    operation_index++;
                    continue ;
                }
            }
            if (new_entry.configure_expiration(has_expiration, expiration_timestamp) != 0)
            {
                this->set_error_unlocked(new_entry.get_error());
                kv_store::restore_errno(guard, entry_errno);
                return (-1);
            }
            if (existing_pair != ft_nullptr)
            {
                existing_pair->value = new_entry;
                if (existing_pair->value.get_error() != ER_SUCCESS)
                {
                    this->set_error_unlocked(existing_pair->value.get_error());
                    kv_store::restore_errno(guard, entry_errno);
                    return (-1);
                }
            }
            else
            {
                staged_map.insert(operation._key, new_entry);
                if (staged_map.get_error() != ER_SUCCESS)
                {
                    this->set_error_unlocked(staged_map.get_error());
                    kv_store::restore_errno(guard, entry_errno);
                    return (-1);
                }
            }
        }
        operation_index++;
    }
    this->_data = ft_move(staged_map);
    if (this->_data.get_error() != ER_SUCCESS)
    {
        this->set_error_unlocked(this->_data.get_error());
        kv_store::restore_errno(guard, entry_errno);
        return (-1);
    }
    this->set_error_unlocked(ER_SUCCESS);
    kv_store::restore_errno(guard, entry_errno);
    return (0);
}

int kv_store::kv_compare_and_swap(const char *key_string, const char *expected_value, const char *new_value, long long ttl_seconds)
{
    ft_string key_storage;
    Pair<ft_string, kv_store_entry> *existing_pair;
    ft_unique_lock<pt_mutex> guard;
    bool has_expiration;
    long long expiration_timestamp;
    int entry_errno;
    int lock_error;

    if (key_string == ft_nullptr)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (expected_value == ft_nullptr && new_value == ft_nullptr)
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
    entry_errno = ft_errno;
    lock_error = this->lock_store(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error_unlocked(lock_error);
        kv_store::restore_errno(guard, entry_errno);
        return (-1);
    }
    if (this->prune_expired_locked(guard) != 0)
    {
        kv_store::restore_errno(guard, entry_errno);
        return (-1);
    }
    existing_pair = this->_data.find(key_storage);
    if (this->_data.get_error() != ER_SUCCESS)
    {
        this->set_error_unlocked(this->_data.get_error());
        kv_store::restore_errno(guard, entry_errno);
        return (-1);
    }
    if (expected_value == ft_nullptr)
    {
        if (existing_pair != ft_nullptr)
        {
            this->set_error_unlocked(FT_ERR_ALREADY_EXISTS);
            kv_store::restore_errno(guard, entry_errno);
            return (-1);
        }
    }
    else
    {
        ft_string existing_value;

        if (existing_pair == ft_nullptr)
        {
            this->set_error_unlocked(FT_ERR_NOT_FOUND);
            kv_store::restore_errno(guard, entry_errno);
            return (-1);
        }
        existing_value = ft_string();
        if (existing_pair->value.copy_value(existing_value) != 0)
        {
            this->set_error_unlocked(existing_pair->value.get_error());
            kv_store::restore_errno(guard, entry_errno);
            return (-1);
        }
        if (existing_value.c_str() == ft_nullptr || ft_strcmp(existing_value.c_str(), expected_value) != 0)
        {
            this->set_error_unlocked(FT_ERR_INVALID_OPERATION);
            kv_store::restore_errno(guard, entry_errno);
            return (-1);
        }
    }
    if (new_value == ft_nullptr)
    {
        if (existing_pair != ft_nullptr)
        {
            this->_data.remove(key_storage);
            if (this->_data.get_error() != ER_SUCCESS)
            {
                this->set_error_unlocked(this->_data.get_error());
                kv_store::restore_errno(guard, entry_errno);
                return (-1);
            }
        }
        this->set_error_unlocked(ER_SUCCESS);
        kv_store::restore_errno(guard, entry_errno);
        return (0);
    }
    if (ttl_seconds >= 0)
    {
        if (this->compute_expiration(ttl_seconds, has_expiration, expiration_timestamp) != 0)
        {
            kv_store::restore_errno(guard, entry_errno);
            return (-1);
        }
    }
    else if (existing_pair != ft_nullptr)
    {
        bool existing_has_expiration;

        existing_has_expiration = false;
        if (existing_pair->value.has_expiration(existing_has_expiration) != 0)
        {
            this->set_error_unlocked(existing_pair->value.get_error());
            kv_store::restore_errno(guard, entry_errno);
            return (-1);
        }
        if (existing_has_expiration)
        {
            if (existing_pair->value.get_expiration(expiration_timestamp) != 0)
            {
                this->set_error_unlocked(existing_pair->value.get_error());
                kv_store::restore_errno(guard, entry_errno);
                return (-1);
            }
            has_expiration = true;
        }
        else
        {
            has_expiration = false;
            expiration_timestamp = 0;
        }
    }
    else
    {
        has_expiration = false;
        expiration_timestamp = 0;
    }
    if (has_expiration)
    {
        long long current_time;

        current_time = this->current_time_seconds();
        if (current_time < 0)
        {
            this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
            kv_store::restore_errno(guard, entry_errno);
            return (-1);
        }
        if (expiration_timestamp <= current_time)
        {
            if (existing_pair != ft_nullptr)
            {
                this->_data.remove(key_storage);
                if (this->_data.get_error() != ER_SUCCESS)
                {
                    this->set_error_unlocked(this->_data.get_error());
                    kv_store::restore_errno(guard, entry_errno);
                    return (-1);
                }
            }
            this->set_error_unlocked(ER_SUCCESS);
            kv_store::restore_errno(guard, entry_errno);
            return (0);
        }
    }
    if (existing_pair != ft_nullptr)
    {
        if (existing_pair->value.set_value(new_value) != 0)
        {
            this->set_error_unlocked(existing_pair->value.get_error());
            kv_store::restore_errno(guard, entry_errno);
            return (-1);
        }
        if (existing_pair->value.configure_expiration(has_expiration, expiration_timestamp) != 0)
        {
            this->set_error_unlocked(existing_pair->value.get_error());
            kv_store::restore_errno(guard, entry_errno);
            return (-1);
        }
    }
    else
    {
        kv_store_entry new_entry;

        if (new_entry.set_value(new_value) != 0)
        {
            this->set_error_unlocked(new_entry.get_error());
            kv_store::restore_errno(guard, entry_errno);
            return (-1);
        }
        if (new_entry.configure_expiration(has_expiration, expiration_timestamp) != 0)
        {
            this->set_error_unlocked(new_entry.get_error());
            kv_store::restore_errno(guard, entry_errno);
            return (-1);
        }
        this->_data.insert(key_storage, new_entry);
        if (this->_data.get_error() != ER_SUCCESS)
        {
            this->set_error_unlocked(this->_data.get_error());
            kv_store::restore_errno(guard, entry_errno);
            return (-1);
        }
    }
    this->set_error_unlocked(ER_SUCCESS);
    kv_store::restore_errno(guard, entry_errno);
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

