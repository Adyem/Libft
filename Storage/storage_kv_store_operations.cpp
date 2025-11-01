#include "kv_store.hpp"

#include <cstddef>
#include <cerrno>
#include <climits>
#include <limits>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <vector>

#ifdef _WIN32
# include <windows.h>
#else
# include <fcntl.h>
# include <sys/mman.h>
# include <sys/stat.h>
# include <unistd.h>
#endif

#include <sqlite3.h>

#include "../Libft/libft.hpp"
#include "../Template/move.hpp"
#include "../Time/time.hpp"
#include "../GetNextLine/get_next_line.hpp"

const char *g_kv_store_ttl_prefix = "__ttl__";

int kv_store::lock_replication(ft_unique_lock<pt_mutex> &guard) const noexcept
{
    int entry_errno;
    ft_unique_lock<pt_mutex> local_guard(this->_replication_mutex);

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

int kv_store::dispatch_snapshot_to_sink(kv_store_replication_snapshot_callback snapshot_callback, void *user_data) const
{
    ft_vector<kv_store_snapshot_entry> snapshot_entries;
    kv_store *mutable_this;
    int callback_error;

    if (snapshot_callback == ft_nullptr)
    {
        mutable_this = const_cast<kv_store *>(this);
        mutable_this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    snapshot_entries = ft_vector<kv_store_snapshot_entry>();
    if (snapshot_entries.get_error() != ER_SUCCESS)
    {
        mutable_this = const_cast<kv_store *>(this);
        mutable_this->set_error(snapshot_entries.get_error());
        return (-1);
    }
    mutable_this = const_cast<kv_store *>(this);
    if (mutable_this->export_snapshot(snapshot_entries) != 0)
        return (-1);
    callback_error = snapshot_callback(snapshot_entries, user_data);
    if (callback_error != 0)
    {
        mutable_this->set_error(callback_error);
        ft_errno = callback_error;
        return (-1);
    }
    mutable_this->set_error(ER_SUCCESS);
    return (0);
}

int kv_store::notify_replication_listeners(const ft_vector<kv_store_operation> &operations) const
{
    kv_store *mutable_this;
    ft_unique_lock<pt_mutex> guard;
    ft_vector<kv_store_replication_sink> listeners_copy;
    size_t listener_count;
    size_t listener_index;
    int entry_errno;
    int lock_error;

    mutable_this = const_cast<kv_store *>(this);
    if (operations.get_error() != ER_SUCCESS)
    {
        mutable_this->set_error(operations.get_error());
        return (-1);
    }
    listener_count = operations.size();
    if (operations.get_error() != ER_SUCCESS)
    {
        mutable_this->set_error(operations.get_error());
        return (-1);
    }
    if (listener_count == 0)
    {
        mutable_this->set_error(ER_SUCCESS);
        return (0);
    }
    entry_errno = ft_errno;
    lock_error = mutable_this->lock_replication(guard);
    if (lock_error != ER_SUCCESS)
    {
        mutable_this->set_error_unlocked(lock_error);
        kv_store::restore_errno(guard, entry_errno);
        return (-1);
    }
    listeners_copy = ft_vector<kv_store_replication_sink>();
    if (listeners_copy.get_error() != ER_SUCCESS)
    {
        mutable_this->set_error_unlocked(listeners_copy.get_error());
        kv_store::restore_errno(guard, entry_errno);
        return (-1);
    }
    listener_count = mutable_this->_replication_sinks.size();
    if (mutable_this->_replication_sinks.get_error() != ER_SUCCESS)
    {
        mutable_this->set_error_unlocked(mutable_this->_replication_sinks.get_error());
        kv_store::restore_errno(guard, entry_errno);
        return (-1);
    }
    listener_index = 0;
    while (listener_index < listener_count)
    {
        kv_store_replication_sink sink_entry;

        sink_entry = mutable_this->_replication_sinks[listener_index];
        if (mutable_this->_replication_sinks.get_error() != ER_SUCCESS)
        {
            mutable_this->set_error_unlocked(mutable_this->_replication_sinks.get_error());
            kv_store::restore_errno(guard, entry_errno);
            return (-1);
        }
        listeners_copy.push_back(sink_entry);
        if (listeners_copy.get_error() != ER_SUCCESS)
        {
            mutable_this->set_error_unlocked(listeners_copy.get_error());
            kv_store::restore_errno(guard, entry_errno);
            return (-1);
        }
        listener_index++;
    }
    mutable_this->set_error_unlocked(ER_SUCCESS);
    kv_store::restore_errno(guard, entry_errno);
    listener_count = listeners_copy.size();
    if (listeners_copy.get_error() != ER_SUCCESS)
    {
        mutable_this->set_error(listeners_copy.get_error());
        return (-1);
    }
    listener_index = 0;
    while (listener_index < listener_count)
    {
        const kv_store_replication_sink &sink = listeners_copy[listener_index];
        int callback_error;

        if (listeners_copy.get_error() != ER_SUCCESS)
        {
            mutable_this->set_error(listeners_copy.get_error());
            return (-1);
        }
        if (sink._operations_callback != ft_nullptr)
        {
            callback_error = sink._operations_callback(operations, sink._user_data);
            if (callback_error != 0)
            {
                mutable_this->set_error(callback_error);
                ft_errno = callback_error;
                return (-1);
            }
        }
        listener_index++;
    }
    mutable_this->set_error(ER_SUCCESS);
    return (0);
}

int kv_store::register_replication_sink(kv_store_replication_operations_callback operations_callback,
        kv_store_replication_snapshot_callback snapshot_callback, void *user_data,
        bool ship_initial_snapshot)
{
    ft_unique_lock<pt_mutex> guard;
    kv_store_replication_sink sink_entry;
    size_t listener_count;
    size_t listener_index;
    int entry_errno;
    int lock_error;

    if (operations_callback == ft_nullptr && snapshot_callback == ft_nullptr)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    entry_errno = ft_errno;
    lock_error = this->lock_replication(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error_unlocked(lock_error);
        kv_store::restore_errno(guard, entry_errno);
        return (-1);
    }
    listener_count = this->_replication_sinks.size();
    if (this->_replication_sinks.get_error() != ER_SUCCESS)
    {
        this->set_error_unlocked(this->_replication_sinks.get_error());
        kv_store::restore_errno(guard, entry_errno);
        return (-1);
    }
    listener_index = 0;
    while (listener_index < listener_count)
    {
        const kv_store_replication_sink &existing = this->_replication_sinks[listener_index];

        if (this->_replication_sinks.get_error() != ER_SUCCESS)
        {
            this->set_error_unlocked(this->_replication_sinks.get_error());
            kv_store::restore_errno(guard, entry_errno);
            return (-1);
        }
        if (existing._operations_callback == operations_callback
            && existing._snapshot_callback == snapshot_callback
            && existing._user_data == user_data)
        {
            this->set_error_unlocked(FT_ERR_ALREADY_EXISTS);
            kv_store::restore_errno(guard, entry_errno);
            return (-1);
        }
        listener_index++;
    }
    sink_entry._operations_callback = operations_callback;
    sink_entry._snapshot_callback = snapshot_callback;
    sink_entry._user_data = user_data;
    this->_replication_sinks.push_back(sink_entry);
    if (this->_replication_sinks.get_error() != ER_SUCCESS)
    {
        this->set_error_unlocked(this->_replication_sinks.get_error());
        kv_store::restore_errno(guard, entry_errno);
        return (-1);
    }
    this->set_error_unlocked(ER_SUCCESS);
    kv_store::restore_errno(guard, entry_errno);
    if (ship_initial_snapshot && snapshot_callback != ft_nullptr)
        return (this->dispatch_snapshot_to_sink(snapshot_callback, user_data));
    return (0);
}

int kv_store::unregister_replication_sink(kv_store_replication_operations_callback operations_callback,
        kv_store_replication_snapshot_callback snapshot_callback, void *user_data)
{
    ft_unique_lock<pt_mutex> guard;
    size_t listener_count;
    size_t listener_index;
    bool removed;
    int entry_errno;
    int lock_error;

    entry_errno = ft_errno;
    lock_error = this->lock_replication(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error_unlocked(lock_error);
        kv_store::restore_errno(guard, entry_errno);
        return (-1);
    }
    listener_count = this->_replication_sinks.size();
    if (this->_replication_sinks.get_error() != ER_SUCCESS)
    {
        this->set_error_unlocked(this->_replication_sinks.get_error());
        kv_store::restore_errno(guard, entry_errno);
        return (-1);
    }
    listener_index = 0;
    removed = false;
    while (listener_index < listener_count)
    {
        const kv_store_replication_sink &existing = this->_replication_sinks[listener_index];

        if (this->_replication_sinks.get_error() != ER_SUCCESS)
        {
            this->set_error_unlocked(this->_replication_sinks.get_error());
            kv_store::restore_errno(guard, entry_errno);
            return (-1);
        }
        if (existing._operations_callback == operations_callback
            && existing._snapshot_callback == snapshot_callback
            && existing._user_data == user_data)
        {
            this->_replication_sinks.erase(this->_replication_sinks.begin() + static_cast<std::ptrdiff_t>(listener_index));
            if (this->_replication_sinks.get_error() != ER_SUCCESS)
            {
                this->set_error_unlocked(this->_replication_sinks.get_error());
                kv_store::restore_errno(guard, entry_errno);
                return (-1);
            }
            removed = true;
            break ;
        }
        listener_index++;
    }
    if (removed == false)
        this->set_error_unlocked(FT_ERR_NOT_FOUND);
    else
        this->set_error_unlocked(ER_SUCCESS);
    kv_store::restore_errno(guard, entry_errno);
    if (removed == false)
        return (-1);
    return (0);
}

int kv_store::ship_replication_snapshot(kv_store_replication_snapshot_callback snapshot_callback, void *user_data) const
{
    return (this->dispatch_snapshot_to_sink(snapshot_callback, user_data));
}

int kv_store::assign_backend_location(const char *location)
{
    if (location == ft_nullptr)
    {
        this->_file_path.clear();
        if (this->_file_path.get_error() != ER_SUCCESS)
            this->set_error(this->_file_path.get_error());
        else
            this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    this->_file_path = location;
    if (this->_file_path.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_file_path.get_error());
        return (-1);
    }
    this->set_error(ER_SUCCESS);
    return (0);
}

int kv_store::parse_json_groups(json_group *group_head, ft_vector<kv_store_snapshot_entry> &out_entries)
{
    json_group *store_group;
    json_item *item_pointer;
    ft_map<ft_string, long long> ttl_metadata;
    size_t ttl_prefix_length;

    if (group_head == ft_nullptr)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    store_group = json_find_group(group_head, "kv_store");
    if (store_group == ft_nullptr)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    ttl_metadata = ft_map<ft_string, long long>();
    if (ttl_metadata.get_error() != ER_SUCCESS)
    {
        this->set_error(ttl_metadata.get_error());
        return (-1);
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
                    this->set_error(FT_ERR_INVALID_ARGUMENT);
                    return (-1);
                }
            }
            else
            {
                this->set_error(FT_ERR_INVALID_ARGUMENT);
                return (-1);
            }
            item_pointer = item_pointer->next;
            continue ;
        }
        if (ttl_prefix_length > 0
            && std::strncmp(item_pointer->key, g_kv_store_ttl_prefix, ttl_prefix_length) == 0)
        {
            ft_string ttl_key(item_pointer->key + ttl_prefix_length);
            long long expiration_timestamp;

            if (ttl_key.get_error() != ER_SUCCESS)
            {
                this->set_error(ttl_key.get_error());
                return (-1);
            }
            if (this->parse_expiration_timestamp(item_pointer->value, expiration_timestamp) != 0)
                return (-1);
            ttl_metadata.insert(ttl_key, expiration_timestamp);
            if (ttl_metadata.get_error() != ER_SUCCESS)
            {
                this->set_error(ttl_metadata.get_error());
                return (-1);
            }
            item_pointer = item_pointer->next;
            continue ;
        }
        kv_store_snapshot_entry snapshot_entry;

        snapshot_entry.has_expiration = false;
        snapshot_entry.expiration_timestamp = 0;
        snapshot_entry.key = item_pointer->key;
        if (snapshot_entry.key.get_error() != ER_SUCCESS)
        {
            this->set_error(snapshot_entry.key.get_error());
            return (-1);
        }
        if (this->_encryption_enabled)
        {
            ft_string encoded_value(item_pointer->value);
            ft_string decrypted_value;

            if (encoded_value.get_error() != ER_SUCCESS)
            {
                this->set_error(encoded_value.get_error());
                return (-1);
            }
            if (this->decrypt_value(encoded_value, decrypted_value) != 0)
                return (-1);
            snapshot_entry.value = decrypted_value;
        }
        else
            snapshot_entry.value = item_pointer->value;
        if (snapshot_entry.value.get_error() != ER_SUCCESS)
        {
            this->set_error(snapshot_entry.value.get_error());
            return (-1);
        }
        out_entries.push_back(ft_move(snapshot_entry));
        if (out_entries.get_error() != ER_SUCCESS)
        {
            this->set_error(out_entries.get_error());
            return (-1);
        }
        item_pointer = item_pointer->next;
    }
    size_t ttl_size;

    ttl_size = ttl_metadata.size();
    if (ttl_metadata.get_error() != ER_SUCCESS)
    {
        this->set_error(ttl_metadata.get_error());
        return (-1);
    }
    if (ttl_size > 0)
    {
        const Pair<ft_string, long long> *ttl_end;
        const Pair<ft_string, long long> *ttl_begin;
        size_t ttl_index;

        ttl_end = ttl_metadata.end();
        if (ttl_metadata.get_error() != ER_SUCCESS)
        {
            this->set_error(ttl_metadata.get_error());
            return (-1);
        }
        ttl_begin = ttl_end - static_cast<std::ptrdiff_t>(ttl_size);
        ttl_index = 0;
        while (ttl_index < ttl_size)
        {
            const Pair<ft_string, long long> &ttl_entry = ttl_begin[ttl_index];
            kv_store_snapshot_entry *entry_begin;
            kv_store_snapshot_entry *entry_end;
            kv_store_snapshot_entry *entry_cursor;

            entry_begin = out_entries.begin();
            if (out_entries.get_error() != ER_SUCCESS)
            {
                this->set_error(out_entries.get_error());
                return (-1);
            }
            entry_end = out_entries.end();
            if (out_entries.get_error() != ER_SUCCESS)
            {
                this->set_error(out_entries.get_error());
                return (-1);
            }
            entry_cursor = entry_begin;
            while (entry_cursor != entry_end)
            {
                if (entry_cursor->key == ttl_entry.key)
                {
                    entry_cursor->has_expiration = true;
                    entry_cursor->expiration_timestamp = ttl_entry.value;
                    break ;
                }
                entry_cursor++;
            }
            ttl_index++;
        }
    }
    this->set_error(ER_SUCCESS);
    return (0);
}

int kv_store::load_json_entries(const char *location, ft_vector<kv_store_snapshot_entry> &out_entries)
{
    json_group *group_head;
    int current_error;

    if (location == ft_nullptr)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    out_entries.clear();
    if (out_entries.get_error() != ER_SUCCESS)
    {
        this->set_error(out_entries.get_error());
        return (-1);
    }
    group_head = json_read_from_file(location);
    if (group_head == ft_nullptr)
    {
        current_error = ft_errno;
        if (current_error == ER_SUCCESS)
        {
            this->set_error(ER_SUCCESS);
            return (1);
        }
        this->set_error(current_error);
        return (-1);
    }
    if (this->parse_json_groups(group_head, out_entries) != 0)
    {
        json_free_groups(group_head);
        return (-1);
    }
    json_free_groups(group_head);
    this->set_error(ER_SUCCESS);
    return (0);
}

int kv_store::flush_json_entries(const ft_vector<kv_store_snapshot_entry> &entries) const
{
    json_group *store_group;
    json_group *head_group;
    kv_store *mutable_this;
    const kv_store_snapshot_entry *entry_begin;
    const kv_store_snapshot_entry *entry_end;
    const kv_store_snapshot_entry *entry_cursor;

    mutable_this = const_cast<kv_store *>(this);
    store_group = json_create_json_group("kv_store");
    if (store_group == ft_nullptr)
    {
        mutable_this->set_error_unlocked(FT_ERR_NO_MEMORY);
        return (-1);
    }
    if (this->_encryption_enabled)
    {
        json_item *metadata_item;

        metadata_item = json_create_item("__encryption__", "aes-128-ecb-base64");
        if (metadata_item == ft_nullptr)
        {
            json_free_groups(store_group);
            mutable_this->set_error_unlocked(FT_ERR_NO_MEMORY);
            return (-1);
        }
        json_add_item_to_group(store_group, metadata_item);
    }
    entry_begin = entries.begin();
    if (entries.get_error() != ER_SUCCESS)
    {
        json_free_groups(store_group);
        mutable_this->set_error_unlocked(entries.get_error());
        return (-1);
    }
    entry_end = entries.end();
    if (entries.get_error() != ER_SUCCESS)
    {
        json_free_groups(store_group);
        mutable_this->set_error_unlocked(entries.get_error());
        return (-1);
    }
    entry_cursor = entry_begin;
    while (entry_cursor != entry_end)
    {
        json_item *value_item;
        ft_string stored_value;
        const char *value_pointer;

        value_pointer = entry_cursor->value.c_str();
        if (this->_encryption_enabled)
        {
            ft_string encoded_value;

            encoded_value = ft_string();
            if (encoded_value.get_error() != ER_SUCCESS)
            {
                json_free_groups(store_group);
                mutable_this->set_error_unlocked(encoded_value.get_error());
                return (-1);
            }
            if (mutable_this->encrypt_value(entry_cursor->value, encoded_value) != 0)
            {
                json_free_groups(store_group);
                return (-1);
            }
            stored_value = encoded_value;
            if (stored_value.get_error() != ER_SUCCESS)
            {
                json_free_groups(store_group);
                mutable_this->set_error_unlocked(stored_value.get_error());
                return (-1);
            }
            value_pointer = stored_value.c_str();
        }
        value_item = json_create_item(entry_cursor->key.c_str(), value_pointer);
        if (value_item == ft_nullptr)
        {
            json_free_groups(store_group);
            mutable_this->set_error_unlocked(FT_ERR_NO_MEMORY);
            return (-1);
        }
        json_add_item_to_group(store_group, value_item);
        if (entry_cursor->has_expiration)
        {
            ft_string ttl_key;
            char expiration_buffer[32];
            int written_length;

            ttl_key = g_kv_store_ttl_prefix;
            if (ttl_key.get_error() != ER_SUCCESS)
            {
                json_free_groups(store_group);
                mutable_this->set_error_unlocked(ttl_key.get_error());
                return (-1);
            }
            ttl_key += entry_cursor->key;
            if (ttl_key.get_error() != ER_SUCCESS)
            {
                json_free_groups(store_group);
                mutable_this->set_error_unlocked(ttl_key.get_error());
                return (-1);
            }
            written_length = std::snprintf(expiration_buffer, sizeof(expiration_buffer), "%lld", entry_cursor->expiration_timestamp);
            if (written_length < 0 || static_cast<size_t>(written_length) >= sizeof(expiration_buffer))
            {
                json_free_groups(store_group);
                mutable_this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
                return (-1);
            }
            value_item = json_create_item(ttl_key.c_str(), expiration_buffer);
            if (value_item == ft_nullptr)
            {
                json_free_groups(store_group);
                mutable_this->set_error_unlocked(FT_ERR_NO_MEMORY);
                return (-1);
            }
            json_add_item_to_group(store_group, value_item);
        }
        entry_cursor++;
    }
    head_group = ft_nullptr;
    json_append_group(&head_group, store_group);
    if (json_write_to_file(this->_file_path.c_str(), head_group) != 0)
    {
        int stored_error;

        stored_error = ft_errno;
        json_free_groups(head_group);
        if (stored_error != ER_SUCCESS)
            mutable_this->set_error_unlocked(stored_error);
        else
            mutable_this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    json_free_groups(head_group);
    mutable_this->set_error_unlocked(ER_SUCCESS);
    return (0);
}

int kv_store::flush_json_lines_entries(const ft_vector<kv_store_snapshot_entry> &entries) const
{
    FILE *file_handle;
    kv_store *mutable_this;
    const kv_store_snapshot_entry *entry_begin;
    const kv_store_snapshot_entry *entry_end;
    const kv_store_snapshot_entry *entry_cursor;

    mutable_this = const_cast<kv_store *>(this);
    file_handle = std::fopen(this->_file_path.c_str(), "wb");
    if (!file_handle)
    {
        mutable_this->set_error_unlocked(ft_map_system_error(errno));
        return (-1);
    }
    if (this->_encryption_enabled)
    {
        json_group *metadata_group;
        json_group *head_group;
        json_item *encryption_item;
        char *serialized_line;
        size_t written_length;

        metadata_group = json_create_json_group("metadata");
        if (metadata_group == ft_nullptr)
        {
            std::fclose(file_handle);
            mutable_this->set_error_unlocked(FT_ERR_NO_MEMORY);
            return (-1);
        }
        encryption_item = json_create_item("encryption", "aes-128-ecb-base64");
        if (encryption_item == ft_nullptr)
        {
            json_free_groups(metadata_group);
            std::fclose(file_handle);
            mutable_this->set_error_unlocked(FT_ERR_NO_MEMORY);
            return (-1);
        }
        json_add_item_to_group(metadata_group, encryption_item);
        head_group = ft_nullptr;
        json_append_group(&head_group, metadata_group);
        serialized_line = json_write_to_string(head_group);
        if (!serialized_line)
        {
            json_free_groups(head_group);
            std::fclose(file_handle);
            mutable_this->set_error_unlocked(FT_ERR_NO_MEMORY);
            return (-1);
        }
        written_length = std::fwrite(serialized_line, 1, std::strlen(serialized_line), file_handle);
        if (written_length != std::strlen(serialized_line) || std::fputc('\n', file_handle) == EOF)
        {
            json_free_groups(head_group);
            cma_free(serialized_line);
            std::fclose(file_handle);
            mutable_this->set_error_unlocked(ft_map_system_error(errno));
            return (-1);
        }
        json_free_groups(head_group);
        cma_free(serialized_line);
    }
    entry_begin = entries.begin();
    if (entries.get_error() != ER_SUCCESS)
    {
        std::fclose(file_handle);
        mutable_this->set_error_unlocked(entries.get_error());
        return (-1);
    }
    entry_end = entries.end();
    if (entries.get_error() != ER_SUCCESS)
    {
        std::fclose(file_handle);
        mutable_this->set_error_unlocked(entries.get_error());
        return (-1);
    }
    entry_cursor = entry_begin;
    while (entry_cursor != entry_end)
    {
        json_group *entry_group;
        json_group *head_group;
        json_item *item_pointer;
        ft_string stored_value;
        const char *value_pointer;
        char ttl_buffer[32];
        char *serialized_line;
        size_t line_length;

        entry_group = json_create_json_group("entry");
        if (entry_group == ft_nullptr)
        {
            std::fclose(file_handle);
            mutable_this->set_error_unlocked(FT_ERR_NO_MEMORY);
            return (-1);
        }
        value_pointer = entry_cursor->value.c_str();
        if (this->_encryption_enabled)
        {
            ft_string encoded_value;

            encoded_value = ft_string();
            if (encoded_value.get_error() != ER_SUCCESS)
            {
                json_free_groups(entry_group);
                std::fclose(file_handle);
                mutable_this->set_error_unlocked(encoded_value.get_error());
                return (-1);
            }
            if (mutable_this->encrypt_value(entry_cursor->value, encoded_value) != 0)
            {
                json_free_groups(entry_group);
                std::fclose(file_handle);
                return (-1);
            }
            stored_value = encoded_value;
            if (stored_value.get_error() != ER_SUCCESS)
            {
                json_free_groups(entry_group);
                std::fclose(file_handle);
                mutable_this->set_error_unlocked(stored_value.get_error());
                return (-1);
            }
            value_pointer = stored_value.c_str();
        }
        item_pointer = json_create_item("key", entry_cursor->key.c_str());
        if (item_pointer == ft_nullptr)
        {
            json_free_groups(entry_group);
            std::fclose(file_handle);
            mutable_this->set_error_unlocked(FT_ERR_NO_MEMORY);
            return (-1);
        }
        json_add_item_to_group(entry_group, item_pointer);
        item_pointer = json_create_item("value", value_pointer);
        if (item_pointer == ft_nullptr)
        {
            json_free_groups(entry_group);
            std::fclose(file_handle);
            mutable_this->set_error_unlocked(FT_ERR_NO_MEMORY);
            return (-1);
        }
        json_add_item_to_group(entry_group, item_pointer);
        item_pointer = json_create_item("has_expiration", entry_cursor->has_expiration ? "true" : "false");
        if (item_pointer == ft_nullptr)
        {
            json_free_groups(entry_group);
            std::fclose(file_handle);
            mutable_this->set_error_unlocked(FT_ERR_NO_MEMORY);
            return (-1);
        }
        json_add_item_to_group(entry_group, item_pointer);
        if (entry_cursor->has_expiration)
        {
            int written_length;

            written_length = std::snprintf(ttl_buffer, sizeof(ttl_buffer), "%lld", entry_cursor->expiration_timestamp);
            if (written_length < 0 || static_cast<size_t>(written_length) >= sizeof(ttl_buffer))
            {
                json_free_groups(entry_group);
                std::fclose(file_handle);
                mutable_this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
                return (-1);
            }
        }
        else
            ttl_buffer[0] = '\0';
        item_pointer = json_create_item("expiration", ttl_buffer);
        if (item_pointer == ft_nullptr)
        {
            json_free_groups(entry_group);
            std::fclose(file_handle);
            mutable_this->set_error_unlocked(FT_ERR_NO_MEMORY);
            return (-1);
        }
        json_add_item_to_group(entry_group, item_pointer);
        head_group = ft_nullptr;
        json_append_group(&head_group, entry_group);
        serialized_line = json_write_to_string(head_group);
        if (!serialized_line)
        {
            json_free_groups(head_group);
            std::fclose(file_handle);
            mutable_this->set_error_unlocked(FT_ERR_NO_MEMORY);
            return (-1);
        }
        line_length = std::strlen(serialized_line);
        if (std::fwrite(serialized_line, 1, line_length, file_handle) != line_length
            || std::fputc('\n', file_handle) == EOF)
        {
            json_free_groups(head_group);
            cma_free(serialized_line);
            std::fclose(file_handle);
            mutable_this->set_error_unlocked(ft_map_system_error(errno));
            return (-1);
        }
        json_free_groups(head_group);
        cma_free(serialized_line);
        entry_cursor++;
    }
    std::fclose(file_handle);
    mutable_this->set_error_unlocked(ER_SUCCESS);
    return (0);
}

int kv_store::load_json_lines_entries(const char *location, ft_vector<kv_store_snapshot_entry> &out_entries)
{
    ft_vector<ft_string> lines;
    size_t line_count;
    size_t line_index;
    int read_result;

    if (location == ft_nullptr)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    lines = ft_vector<ft_string>();
    if (lines.get_error() != ER_SUCCESS)
    {
        this->set_error(lines.get_error());
        return (-1);
    }
    read_result = ft_open_and_read_file(location, lines, 8192);
    if (read_result != 0)
    {
        int stored_error;

        stored_error = ft_errno;
        if (stored_error == ft_map_system_error(ENOENT))
        {
            this->set_error(ER_SUCCESS);
            return (1);
        }
        this->set_error(stored_error);
        return (-1);
    }
    out_entries.clear();
    if (out_entries.get_error() != ER_SUCCESS)
    {
        this->set_error(out_entries.get_error());
        return (-1);
    }
    line_count = lines.size();
    if (lines.get_error() != ER_SUCCESS)
    {
        this->set_error(lines.get_error());
        return (-1);
    }
    line_index = 0;
    while (line_index < line_count)
    {
        const ft_string &line_value = lines[line_index];
        json_group *line_groups;
        json_group *metadata_group;
        json_group *entry_group;

        line_groups = json_read_from_string(line_value.c_str());
        if (!line_groups)
        {
            this->set_error(ft_errno == ER_SUCCESS ? FT_ERR_INVALID_ARGUMENT : ft_errno);
            return (-1);
        }
        metadata_group = json_find_group(line_groups, "metadata");
        if (metadata_group != ft_nullptr)
        {
            json_item *encryption_item;

            encryption_item = metadata_group->items;
            while (encryption_item != ft_nullptr)
            {
                if (std::strcmp(encryption_item->key, "encryption") == 0)
                {
                    if (std::strcmp(encryption_item->value, "aes-128-ecb-base64") != 0)
                    {
                        json_free_groups(line_groups);
                        this->set_error(FT_ERR_INVALID_ARGUMENT);
                        return (-1);
                    }
                    if (this->_encryption_enabled == false)
                    {
                        json_free_groups(line_groups);
                        this->set_error(FT_ERR_INVALID_ARGUMENT);
                        return (-1);
                    }
                }
                encryption_item = encryption_item->next;
            }
            json_free_groups(line_groups);
            line_index++;
            continue ;
        }
        entry_group = json_find_group(line_groups, "entry");
        if (entry_group == ft_nullptr)
        {
            json_free_groups(line_groups);
            this->set_error(FT_ERR_INVALID_ARGUMENT);
            return (-1);
        }
        kv_store_snapshot_entry snapshot_entry;
        json_item *item_cursor;
        bool has_expiration_flag;
        long long expiration_value;

        snapshot_entry.has_expiration = false;
        snapshot_entry.expiration_timestamp = 0;
        has_expiration_flag = false;
        expiration_value = 0;
        item_cursor = entry_group->items;
        while (item_cursor != ft_nullptr)
        {
            if (std::strcmp(item_cursor->key, "key") == 0)
            {
                snapshot_entry.key = item_cursor->value;
                if (snapshot_entry.key.get_error() != ER_SUCCESS)
                {
                    json_free_groups(line_groups);
                    this->set_error(snapshot_entry.key.get_error());
                    return (-1);
                }
            }
            else if (std::strcmp(item_cursor->key, "value") == 0)
            {
                if (this->_encryption_enabled)
                {
                    ft_string encoded_value(item_cursor->value);
                    ft_string decrypted_value;

                    if (encoded_value.get_error() != ER_SUCCESS)
                    {
                        json_free_groups(line_groups);
                        this->set_error(encoded_value.get_error());
                        return (-1);
                    }
                    if (this->decrypt_value(encoded_value, decrypted_value) != 0)
                    {
                        json_free_groups(line_groups);
                        return (-1);
                    }
                    snapshot_entry.value = decrypted_value;
                }
                else
                    snapshot_entry.value = item_cursor->value;
                if (snapshot_entry.value.get_error() != ER_SUCCESS)
                {
                    json_free_groups(line_groups);
                    this->set_error(snapshot_entry.value.get_error());
                    return (-1);
                }
            }
            else if (std::strcmp(item_cursor->key, "has_expiration") == 0)
            {
                if (std::strcmp(item_cursor->value, "true") == 0)
                    has_expiration_flag = true;
                else
                    has_expiration_flag = false;
            }
            else if (std::strcmp(item_cursor->key, "expiration") == 0)
            {
                if (std::strlen(item_cursor->value) > 0)
                    expiration_value = std::strtoll(item_cursor->value, ft_nullptr, 10);
                else
                    expiration_value = 0;
            }
            item_cursor = item_cursor->next;
        }
        if (snapshot_entry.key.c_str() == ft_nullptr)
        {
            json_free_groups(line_groups);
            this->set_error(FT_ERR_INVALID_ARGUMENT);
            return (-1);
        }
        snapshot_entry.has_expiration = has_expiration_flag;
        snapshot_entry.expiration_timestamp = expiration_value;
        out_entries.push_back(ft_move(snapshot_entry));
        if (out_entries.get_error() != ER_SUCCESS)
        {
            json_free_groups(line_groups);
            this->set_error(out_entries.get_error());
            return (-1);
        }
        json_free_groups(line_groups);
        line_index++;
    }
    this->set_error(ER_SUCCESS);
    return (0);
}

int kv_store::flush_sqlite_entries(const ft_vector<kv_store_snapshot_entry> &entries) const
{
    sqlite3 *database_handle;
    kv_store *mutable_this;
    char *error_message;
    sqlite3_stmt *insert_statement;
    int statement_result;
    const kv_store_snapshot_entry *entry_begin;
    const kv_store_snapshot_entry *entry_end;
    const kv_store_snapshot_entry *entry_cursor;

    mutable_this = const_cast<kv_store *>(this);
    database_handle = ft_nullptr;
    error_message = ft_nullptr;
    insert_statement = ft_nullptr;
    if (sqlite3_open(this->_file_path.c_str(), &database_handle) != SQLITE_OK)
    {
        mutable_this->set_error_unlocked(FT_ERR_IO);
        if (database_handle)
            sqlite3_close(database_handle);
        return (-1);
    }
    if (sqlite3_exec(database_handle, "BEGIN IMMEDIATE TRANSACTION;", ft_nullptr, ft_nullptr, &error_message) != SQLITE_OK)
    {
        if (error_message)
            sqlite3_free(error_message);
        sqlite3_close(database_handle);
        mutable_this->set_error_unlocked(FT_ERR_IO);
        return (-1);
    }
    if (sqlite3_exec(database_handle, "CREATE TABLE IF NOT EXISTS kv_store_entries (key TEXT PRIMARY KEY, value TEXT NOT NULL, has_expiration INTEGER NOT NULL, expiration INTEGER NOT NULL);", ft_nullptr, ft_nullptr, &error_message) != SQLITE_OK)
    {
        if (error_message)
            sqlite3_free(error_message);
        sqlite3_exec(database_handle, "ROLLBACK;", ft_nullptr, ft_nullptr, ft_nullptr);
        sqlite3_close(database_handle);
        mutable_this->set_error_unlocked(FT_ERR_IO);
        return (-1);
    }
    if (sqlite3_exec(database_handle, "CREATE TABLE IF NOT EXISTS kv_store_metadata (id INTEGER PRIMARY KEY CHECK(id=1), encryption INTEGER NOT NULL);", ft_nullptr, ft_nullptr, &error_message) != SQLITE_OK)
    {
        if (error_message)
            sqlite3_free(error_message);
        sqlite3_exec(database_handle, "ROLLBACK;", ft_nullptr, ft_nullptr, ft_nullptr);
        sqlite3_close(database_handle);
        mutable_this->set_error_unlocked(FT_ERR_IO);
        return (-1);
    }
    if (sqlite3_exec(database_handle, "DELETE FROM kv_store_entries;", ft_nullptr, ft_nullptr, &error_message) != SQLITE_OK)
    {
        if (error_message)
            sqlite3_free(error_message);
        sqlite3_exec(database_handle, "ROLLBACK;", ft_nullptr, ft_nullptr, ft_nullptr);
        sqlite3_close(database_handle);
        mutable_this->set_error_unlocked(FT_ERR_IO);
        return (-1);
    }
    sqlite3_stmt *metadata_statement;
    metadata_statement = ft_nullptr;
    if (sqlite3_prepare_v2(database_handle, "INSERT INTO kv_store_metadata (id, encryption) VALUES (1, ?) ON CONFLICT(id) DO UPDATE SET encryption=excluded.encryption;", -1, &metadata_statement, ft_nullptr) != SQLITE_OK)
    {
        sqlite3_exec(database_handle, "ROLLBACK;", ft_nullptr, ft_nullptr, ft_nullptr);
        sqlite3_close(database_handle);
        mutable_this->set_error_unlocked(FT_ERR_IO);
        return (-1);
    }
    if (sqlite3_bind_int(metadata_statement, 1, this->_encryption_enabled ? 1 : 0) != SQLITE_OK)
    {
        sqlite3_finalize(metadata_statement);
        sqlite3_exec(database_handle, "ROLLBACK;", ft_nullptr, ft_nullptr, ft_nullptr);
        sqlite3_close(database_handle);
        mutable_this->set_error_unlocked(FT_ERR_IO);
        return (-1);
    }
    if (sqlite3_step(metadata_statement) != SQLITE_DONE)
    {
        sqlite3_finalize(metadata_statement);
        sqlite3_exec(database_handle, "ROLLBACK;", ft_nullptr, ft_nullptr, ft_nullptr);
        sqlite3_close(database_handle);
        mutable_this->set_error_unlocked(FT_ERR_IO);
        return (-1);
    }
    sqlite3_finalize(metadata_statement);
    metadata_statement = ft_nullptr;
    if (sqlite3_prepare_v2(database_handle, "INSERT INTO kv_store_entries (key, value, has_expiration, expiration) VALUES (?, ?, ?, ?);", -1, &insert_statement, ft_nullptr) != SQLITE_OK)
    {
        sqlite3_exec(database_handle, "ROLLBACK;", ft_nullptr, ft_nullptr, ft_nullptr);
        sqlite3_close(database_handle);
        mutable_this->set_error_unlocked(FT_ERR_IO);
        return (-1);
    }
    entry_begin = entries.begin();
    if (entries.get_error() != ER_SUCCESS)
    {
        sqlite3_finalize(insert_statement);
        sqlite3_exec(database_handle, "ROLLBACK;", ft_nullptr, ft_nullptr, ft_nullptr);
        sqlite3_close(database_handle);
        mutable_this->set_error_unlocked(entries.get_error());
        return (-1);
    }
    entry_end = entries.end();
    if (entries.get_error() != ER_SUCCESS)
    {
        sqlite3_finalize(insert_statement);
        sqlite3_exec(database_handle, "ROLLBACK;", ft_nullptr, ft_nullptr, ft_nullptr);
        sqlite3_close(database_handle);
        mutable_this->set_error_unlocked(entries.get_error());
        return (-1);
    }
    entry_cursor = entry_begin;
    while (entry_cursor != entry_end)
    {
        ft_string stored_value;
        const char *value_pointer;

        value_pointer = entry_cursor->value.c_str();
        if (this->_encryption_enabled)
        {
            ft_string encoded_value;

            encoded_value = ft_string();
            if (encoded_value.get_error() != ER_SUCCESS)
            {
                sqlite3_finalize(insert_statement);
                sqlite3_exec(database_handle, "ROLLBACK;", ft_nullptr, ft_nullptr, ft_nullptr);
                sqlite3_close(database_handle);
                mutable_this->set_error_unlocked(encoded_value.get_error());
                return (-1);
            }
            if (mutable_this->encrypt_value(entry_cursor->value, encoded_value) != 0)
            {
                sqlite3_finalize(insert_statement);
                sqlite3_exec(database_handle, "ROLLBACK;", ft_nullptr, ft_nullptr, ft_nullptr);
                sqlite3_close(database_handle);
                return (-1);
            }
            stored_value = encoded_value;
            if (stored_value.get_error() != ER_SUCCESS)
            {
                sqlite3_finalize(insert_statement);
                sqlite3_exec(database_handle, "ROLLBACK;", ft_nullptr, ft_nullptr, ft_nullptr);
                sqlite3_close(database_handle);
                mutable_this->set_error_unlocked(stored_value.get_error());
                return (-1);
            }
            value_pointer = stored_value.c_str();
        }
        sqlite3_reset(insert_statement);
        sqlite3_clear_bindings(insert_statement);
        if (sqlite3_bind_text(insert_statement, 1, entry_cursor->key.c_str(), -1, SQLITE_STATIC) != SQLITE_OK)
        {
            sqlite3_finalize(insert_statement);
            sqlite3_exec(database_handle, "ROLLBACK;", ft_nullptr, ft_nullptr, ft_nullptr);
            sqlite3_close(database_handle);
            mutable_this->set_error_unlocked(FT_ERR_IO);
            return (-1);
        }
        if (sqlite3_bind_text(insert_statement, 2, value_pointer, -1, SQLITE_STATIC) != SQLITE_OK)
        {
            sqlite3_finalize(insert_statement);
            sqlite3_exec(database_handle, "ROLLBACK;", ft_nullptr, ft_nullptr, ft_nullptr);
            sqlite3_close(database_handle);
            mutable_this->set_error_unlocked(FT_ERR_IO);
            return (-1);
        }
        if (sqlite3_bind_int(insert_statement, 3, entry_cursor->has_expiration ? 1 : 0) != SQLITE_OK)
        {
            sqlite3_finalize(insert_statement);
            sqlite3_exec(database_handle, "ROLLBACK;", ft_nullptr, ft_nullptr, ft_nullptr);
            sqlite3_close(database_handle);
            mutable_this->set_error_unlocked(FT_ERR_IO);
            return (-1);
        }
        if (sqlite3_bind_int64(insert_statement, 4, static_cast<sqlite3_int64>(entry_cursor->expiration_timestamp)) != SQLITE_OK)
        {
            sqlite3_finalize(insert_statement);
            sqlite3_exec(database_handle, "ROLLBACK;", ft_nullptr, ft_nullptr, ft_nullptr);
            sqlite3_close(database_handle);
            mutable_this->set_error_unlocked(FT_ERR_IO);
            return (-1);
        }
        statement_result = sqlite3_step(insert_statement);
        if (statement_result != SQLITE_DONE)
        {
            sqlite3_finalize(insert_statement);
            sqlite3_exec(database_handle, "ROLLBACK;", ft_nullptr, ft_nullptr, ft_nullptr);
            sqlite3_close(database_handle);
            mutable_this->set_error_unlocked(FT_ERR_IO);
            return (-1);
        }
        entry_cursor++;
    }
    sqlite3_finalize(insert_statement);
    insert_statement = ft_nullptr;
    if (sqlite3_exec(database_handle, "COMMIT;", ft_nullptr, ft_nullptr, &error_message) != SQLITE_OK)
    {
        if (error_message)
            sqlite3_free(error_message);
        sqlite3_exec(database_handle, "ROLLBACK;", ft_nullptr, ft_nullptr, ft_nullptr);
        sqlite3_close(database_handle);
        mutable_this->set_error_unlocked(FT_ERR_IO);
        return (-1);
    }
    sqlite3_close(database_handle);
    mutable_this->set_error_unlocked(ER_SUCCESS);
    return (0);
}

int kv_store::load_sqlite_entries(const char *location, ft_vector<kv_store_snapshot_entry> &out_entries)
{
    sqlite3 *database_handle;
    sqlite3_stmt *metadata_statement;
    sqlite3_stmt *select_statement;
    int prepare_result;

    database_handle = ft_nullptr;
    metadata_statement = ft_nullptr;
    select_statement = ft_nullptr;
    if (sqlite3_open(location, &database_handle) != SQLITE_OK)
    {
        if (database_handle)
            sqlite3_close(database_handle);
        this->set_error(FT_ERR_IO);
        return (-1);
    }
    if (sqlite3_exec(database_handle, "CREATE TABLE IF NOT EXISTS kv_store_entries (key TEXT PRIMARY KEY, value TEXT NOT NULL, has_expiration INTEGER NOT NULL, expiration INTEGER NOT NULL);", ft_nullptr, ft_nullptr, ft_nullptr) != SQLITE_OK)
    {
        sqlite3_close(database_handle);
        this->set_error(FT_ERR_IO);
        return (-1);
    }
    if (sqlite3_exec(database_handle, "CREATE TABLE IF NOT EXISTS kv_store_metadata (id INTEGER PRIMARY KEY CHECK(id=1), encryption INTEGER NOT NULL);", ft_nullptr, ft_nullptr, ft_nullptr) != SQLITE_OK)
    {
        sqlite3_close(database_handle);
        this->set_error(FT_ERR_IO);
        return (-1);
    }
    prepare_result = sqlite3_prepare_v2(database_handle, "SELECT encryption FROM kv_store_metadata WHERE id = 1;", -1, &metadata_statement, ft_nullptr);
    if (prepare_result == SQLITE_OK)
    {
        if (sqlite3_step(metadata_statement) == SQLITE_ROW)
        {
            int encryption_flag;

            encryption_flag = sqlite3_column_int(metadata_statement, 0);
            if ((encryption_flag != 0 && this->_encryption_enabled == false)
                || (encryption_flag == 0 && this->_encryption_enabled == true))
            {
                sqlite3_finalize(metadata_statement);
                sqlite3_close(database_handle);
                this->set_error(FT_ERR_INVALID_ARGUMENT);
                return (-1);
            }
        }
        sqlite3_finalize(metadata_statement);
        metadata_statement = ft_nullptr;
    }
    out_entries.clear();
    if (out_entries.get_error() != ER_SUCCESS)
    {
        sqlite3_close(database_handle);
        this->set_error(out_entries.get_error());
        return (-1);
    }
    if (sqlite3_prepare_v2(database_handle, "SELECT key, value, has_expiration, expiration FROM kv_store_entries;", -1, &select_statement, ft_nullptr) != SQLITE_OK)
    {
        sqlite3_close(database_handle);
        this->set_error(FT_ERR_IO);
        return (-1);
    }
    while (true)
    {
        int step_result;

        step_result = sqlite3_step(select_statement);
        if (step_result == SQLITE_DONE)
            break ;
        if (step_result != SQLITE_ROW)
        {
            sqlite3_finalize(select_statement);
            sqlite3_close(database_handle);
            this->set_error(FT_ERR_IO);
            return (-1);
        }
        const unsigned char *key_text;
        const unsigned char *value_text;
        int has_expiration_flag;
        sqlite3_int64 expiration_value;
        kv_store_snapshot_entry snapshot_entry;

        key_text = sqlite3_column_text(select_statement, 0);
        value_text = sqlite3_column_text(select_statement, 1);
        has_expiration_flag = sqlite3_column_int(select_statement, 2);
        expiration_value = sqlite3_column_int64(select_statement, 3);
        if (!key_text || !value_text)
        {
            sqlite3_finalize(select_statement);
            sqlite3_close(database_handle);
            this->set_error(FT_ERR_INVALID_ARGUMENT);
            return (-1);
        }
        snapshot_entry.key = reinterpret_cast<const char *>(key_text);
        if (snapshot_entry.key.get_error() != ER_SUCCESS)
        {
            sqlite3_finalize(select_statement);
            sqlite3_close(database_handle);
            this->set_error(snapshot_entry.key.get_error());
            return (-1);
        }
        if (this->_encryption_enabled)
        {
            ft_string encoded_value(reinterpret_cast<const char *>(value_text));
            ft_string decrypted_value;

            if (encoded_value.get_error() != ER_SUCCESS)
            {
                sqlite3_finalize(select_statement);
                sqlite3_close(database_handle);
                this->set_error(encoded_value.get_error());
                return (-1);
            }
            if (this->decrypt_value(encoded_value, decrypted_value) != 0)
            {
                sqlite3_finalize(select_statement);
                sqlite3_close(database_handle);
                return (-1);
            }
            snapshot_entry.value = decrypted_value;
        }
        else
            snapshot_entry.value = reinterpret_cast<const char *>(value_text);
        if (snapshot_entry.value.get_error() != ER_SUCCESS)
        {
            sqlite3_finalize(select_statement);
            sqlite3_close(database_handle);
            this->set_error(snapshot_entry.value.get_error());
            return (-1);
        }
        snapshot_entry.has_expiration = has_expiration_flag ? true : false;
        snapshot_entry.expiration_timestamp = static_cast<long long>(expiration_value);
        out_entries.push_back(ft_move(snapshot_entry));
        if (out_entries.get_error() != ER_SUCCESS)
        {
            sqlite3_finalize(select_statement);
            sqlite3_close(database_handle);
            this->set_error(out_entries.get_error());
            return (-1);
        }
    }
    sqlite3_finalize(select_statement);
    sqlite3_close(database_handle);
    this->set_error(ER_SUCCESS);
    return (0);
}

int kv_store::flush_memory_mapped_entries(const ft_vector<kv_store_snapshot_entry> &entries) const
{
    kv_store *mutable_this;
    json_group *store_group;
    json_group *head_group;
    const kv_store_snapshot_entry *entry_begin;
    const kv_store_snapshot_entry *entry_end;
    const kv_store_snapshot_entry *entry_cursor;
    char *serialized_buffer;
    size_t serialized_length;

    mutable_this = const_cast<kv_store *>(this);
    store_group = json_create_json_group("kv_store");
    if (store_group == ft_nullptr)
    {
        mutable_this->set_error_unlocked(FT_ERR_NO_MEMORY);
        return (-1);
    }
    if (this->_encryption_enabled)
    {
        json_item *metadata_item;

        metadata_item = json_create_item("__encryption__", "aes-128-ecb-base64");
        if (metadata_item == ft_nullptr)
        {
            json_free_groups(store_group);
            mutable_this->set_error_unlocked(FT_ERR_NO_MEMORY);
            return (-1);
        }
        json_add_item_to_group(store_group, metadata_item);
    }
    entry_begin = entries.begin();
    if (entries.get_error() != ER_SUCCESS)
    {
        json_free_groups(store_group);
        mutable_this->set_error_unlocked(entries.get_error());
        return (-1);
    }
    entry_end = entries.end();
    if (entries.get_error() != ER_SUCCESS)
    {
        json_free_groups(store_group);
        mutable_this->set_error_unlocked(entries.get_error());
        return (-1);
    }
    entry_cursor = entry_begin;
    while (entry_cursor != entry_end)
    {
        json_item *item_pointer;
        ft_string stored_value;
        const char *value_pointer;
        char ttl_buffer[32];

        value_pointer = entry_cursor->value.c_str();
        if (this->_encryption_enabled)
        {
            ft_string encoded_value;

            encoded_value = ft_string();
            if (encoded_value.get_error() != ER_SUCCESS)
            {
                json_free_groups(store_group);
                mutable_this->set_error_unlocked(encoded_value.get_error());
                return (-1);
            }
            if (mutable_this->encrypt_value(entry_cursor->value, encoded_value) != 0)
            {
                json_free_groups(store_group);
                return (-1);
            }
            stored_value = encoded_value;
            if (stored_value.get_error() != ER_SUCCESS)
            {
                json_free_groups(store_group);
                mutable_this->set_error_unlocked(stored_value.get_error());
                return (-1);
            }
            value_pointer = stored_value.c_str();
        }
        item_pointer = json_create_item(entry_cursor->key.c_str(), value_pointer);
        if (item_pointer == ft_nullptr)
        {
            json_free_groups(store_group);
            mutable_this->set_error_unlocked(FT_ERR_NO_MEMORY);
            return (-1);
        }
        json_add_item_to_group(store_group, item_pointer);
        if (entry_cursor->has_expiration)
        {
            int written_length;
            ft_string ttl_key;

            written_length = std::snprintf(ttl_buffer, sizeof(ttl_buffer), "%lld", entry_cursor->expiration_timestamp);
            if (written_length < 0 || static_cast<size_t>(written_length) >= sizeof(ttl_buffer))
            {
                json_free_groups(store_group);
                mutable_this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
                return (-1);
            }
            ttl_key = g_kv_store_ttl_prefix;
            if (ttl_key.get_error() != ER_SUCCESS)
            {
                json_free_groups(store_group);
                mutable_this->set_error_unlocked(ttl_key.get_error());
                return (-1);
            }
            ttl_key += entry_cursor->key;
            if (ttl_key.get_error() != ER_SUCCESS)
            {
                json_free_groups(store_group);
                mutable_this->set_error_unlocked(ttl_key.get_error());
                return (-1);
            }
            item_pointer = json_create_item(ttl_key.c_str(), ttl_buffer);
            if (item_pointer == ft_nullptr)
            {
                json_free_groups(store_group);
                mutable_this->set_error_unlocked(FT_ERR_NO_MEMORY);
                return (-1);
            }
            json_add_item_to_group(store_group, item_pointer);
        }
        entry_cursor++;
    }
    head_group = ft_nullptr;
    json_append_group(&head_group, store_group);
    serialized_buffer = json_write_to_string(head_group);
    if (!serialized_buffer)
    {
        json_free_groups(head_group);
        mutable_this->set_error_unlocked(FT_ERR_NO_MEMORY);
        return (-1);
    }
    serialized_length = std::strlen(serialized_buffer);
#ifdef _WIN32
    HANDLE file_handle;
    LARGE_INTEGER size_value;
    HANDLE mapping_handle;
    void *mapping_view;

    file_handle = CreateFileA(this->_file_path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, ft_nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, ft_nullptr);
    if (file_handle == INVALID_HANDLE_VALUE)
    {
        json_free_groups(head_group);
        cma_free(serialized_buffer);
        mutable_this->set_error_unlocked(ft_map_system_error(GetLastError()));
        return (-1);
    }
    size_value.QuadPart = static_cast<LONGLONG>(serialized_length);
    if (SetFilePointerEx(file_handle, size_value, ft_nullptr, FILE_BEGIN) == FALSE || SetEndOfFile(file_handle) == FALSE)
    {
        CloseHandle(file_handle);
        json_free_groups(head_group);
        cma_free(serialized_buffer);
        mutable_this->set_error_unlocked(ft_map_system_error(GetLastError()));
        return (-1);
    }
    if (serialized_length == 0)
    {
        CloseHandle(file_handle);
        json_free_groups(head_group);
        cma_free(serialized_buffer);
        mutable_this->set_error_unlocked(ER_SUCCESS);
        return (0);
    }
    mapping_handle = CreateFileMappingA(file_handle, ft_nullptr, PAGE_READWRITE, 0, 0, ft_nullptr);
    if (mapping_handle == ft_nullptr)
    {
        CloseHandle(file_handle);
        json_free_groups(head_group);
        cma_free(serialized_buffer);
        mutable_this->set_error_unlocked(ft_map_system_error(GetLastError()));
        return (-1);
    }
    mapping_view = MapViewOfFile(mapping_handle, FILE_MAP_WRITE, 0, 0, 0);
    if (!mapping_view)
    {
        CloseHandle(mapping_handle);
        CloseHandle(file_handle);
        json_free_groups(head_group);
        cma_free(serialized_buffer);
        mutable_this->set_error_unlocked(ft_map_system_error(GetLastError()));
        return (-1);
    }
    if (serialized_length > 0)
        std::memcpy(mapping_view, serialized_buffer, serialized_length);
    UnmapViewOfFile(mapping_view);
    CloseHandle(mapping_handle);
    CloseHandle(file_handle);
#else
    int file_descriptor;
    void *mapping_pointer;

    file_descriptor = open(this->_file_path.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (file_descriptor < 0)
    {
        json_free_groups(head_group);
        cma_free(serialized_buffer);
        mutable_this->set_error_unlocked(ft_map_system_error(errno));
        return (-1);
    }
    if (ftruncate(file_descriptor, static_cast<off_t>(serialized_length)) != 0)
    {
        close(file_descriptor);
        json_free_groups(head_group);
        cma_free(serialized_buffer);
        mutable_this->set_error_unlocked(ft_map_system_error(errno));
        return (-1);
    }
    if (serialized_length == 0)
    {
        close(file_descriptor);
        json_free_groups(head_group);
        cma_free(serialized_buffer);
        mutable_this->set_error_unlocked(ER_SUCCESS);
        return (0);
    }
    mapping_pointer = mmap(ft_nullptr, serialized_length, PROT_READ | PROT_WRITE, MAP_SHARED, file_descriptor, 0);
    if (mapping_pointer == MAP_FAILED)
    {
        close(file_descriptor);
        json_free_groups(head_group);
        cma_free(serialized_buffer);
        mutable_this->set_error_unlocked(ft_map_system_error(errno));
        return (-1);
    }
    std::memcpy(mapping_pointer, serialized_buffer, serialized_length);
    msync(mapping_pointer, serialized_length, MS_SYNC);
    munmap(mapping_pointer, serialized_length);
    close(file_descriptor);
#endif
    json_free_groups(head_group);
    cma_free(serialized_buffer);
    mutable_this->set_error_unlocked(ER_SUCCESS);
    return (0);
}

int kv_store::load_memory_mapped_entries(const char *location, ft_vector<kv_store_snapshot_entry> &out_entries)
{
#ifdef _WIN32
    HANDLE file_handle;
    LARGE_INTEGER size_value;
    HANDLE mapping_handle;
    const char *content_pointer;
    char *buffer_pointer;
    json_group *group_head;

    file_handle = CreateFileA(location, GENERIC_READ, FILE_SHARE_READ, ft_nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, ft_nullptr);
    if (file_handle == INVALID_HANDLE_VALUE)
    {
        DWORD last_error;

        last_error = GetLastError();
        if (last_error == ERROR_FILE_NOT_FOUND)
        {
            this->set_error(ER_SUCCESS);
            return (1);
        }
        this->set_error(ft_map_system_error(static_cast<int>(last_error)));
        return (-1);
    }
    if (GetFileSizeEx(file_handle, &size_value) == FALSE)
    {
        CloseHandle(file_handle);
        this->set_error(ft_map_system_error(GetLastError()));
        return (-1);
    }
    if (size_value.QuadPart == 0)
    {
        CloseHandle(file_handle);
        this->set_error(ER_SUCCESS);
        return (1);
    }
    mapping_handle = CreateFileMappingA(file_handle, ft_nullptr, PAGE_READONLY, 0, 0, ft_nullptr);
    if (mapping_handle == ft_nullptr)
    {
        CloseHandle(file_handle);
        this->set_error(ft_map_system_error(GetLastError()));
        return (-1);
    }
    content_pointer = static_cast<const char *>(MapViewOfFile(mapping_handle, FILE_MAP_READ, 0, 0, static_cast<SIZE_T>(size_value.QuadPart)));
    if (!content_pointer)
    {
        CloseHandle(mapping_handle);
        CloseHandle(file_handle);
        this->set_error(ft_map_system_error(GetLastError()));
        return (-1);
    }
    buffer_pointer = static_cast<char *>(cma_malloc(static_cast<size_t>(size_value.QuadPart) + 1));
    if (!buffer_pointer)
    {
        UnmapViewOfFile(content_pointer);
        CloseHandle(mapping_handle);
        CloseHandle(file_handle);
        this->set_error(FT_ERR_NO_MEMORY);
        return (-1);
    }
    std::memcpy(buffer_pointer, content_pointer, static_cast<size_t>(size_value.QuadPart));
    buffer_pointer[size_value.QuadPart] = '\0';
    UnmapViewOfFile(content_pointer);
    CloseHandle(mapping_handle);
    CloseHandle(file_handle);
    group_head = json_read_from_string(buffer_pointer);
    cma_free(buffer_pointer);
    if (!group_head)
    {
        this->set_error(ft_errno == ER_SUCCESS ? FT_ERR_INVALID_ARGUMENT : ft_errno);
        return (-1);
    }
#else
    int file_descriptor;
    struct stat file_stat;
    void *mapping_pointer;
    char *buffer_pointer;
    json_group *group_head;

    file_descriptor = open(location, O_RDONLY);
    if (file_descriptor < 0)
    {
        if (errno == ENOENT)
        {
            this->set_error(ER_SUCCESS);
            return (1);
        }
        this->set_error(ft_map_system_error(errno));
        return (-1);
    }
    if (fstat(file_descriptor, &file_stat) != 0)
    {
        close(file_descriptor);
        this->set_error(ft_map_system_error(errno));
        return (-1);
    }
    if (file_stat.st_size == 0)
    {
        close(file_descriptor);
        this->set_error(ER_SUCCESS);
        return (1);
    }
    mapping_pointer = mmap(ft_nullptr, static_cast<size_t>(file_stat.st_size), PROT_READ, MAP_SHARED, file_descriptor, 0);
    if (mapping_pointer == MAP_FAILED)
    {
        close(file_descriptor);
        this->set_error(ft_map_system_error(errno));
        return (-1);
    }
    buffer_pointer = static_cast<char *>(cma_malloc(static_cast<size_t>(file_stat.st_size) + 1));
    if (!buffer_pointer)
    {
        munmap(mapping_pointer, static_cast<size_t>(file_stat.st_size));
        close(file_descriptor);
        this->set_error(FT_ERR_NO_MEMORY);
        return (-1);
    }
    std::memcpy(buffer_pointer, mapping_pointer, static_cast<size_t>(file_stat.st_size));
    buffer_pointer[file_stat.st_size] = '\0';
    munmap(mapping_pointer, static_cast<size_t>(file_stat.st_size));
    close(file_descriptor);
    group_head = json_read_from_string(buffer_pointer);
    cma_free(buffer_pointer);
    if (!group_head)
    {
        this->set_error(ft_errno == ER_SUCCESS ? FT_ERR_INVALID_ARGUMENT : ft_errno);
        return (-1);
    }
#endif
    if (this->parse_json_groups(group_head, out_entries) != 0)
    {
        json_free_groups(group_head);
        return (-1);
    }
    json_free_groups(group_head);
    this->set_error(ER_SUCCESS);
    return (0);
}

int kv_store::flush_backend_entries(const ft_vector<kv_store_snapshot_entry> &entries) const
{
    if (this->_backend_type == KV_STORE_BACKEND_JSON)
        return (this->flush_json_entries(entries));
    if (this->_backend_type == KV_STORE_BACKEND_JSON_LINES)
        return (this->flush_json_lines_entries(entries));
    if (this->_backend_type == KV_STORE_BACKEND_SQLITE)
        return (this->flush_sqlite_entries(entries));
    if (this->_backend_type == KV_STORE_BACKEND_MEMORY_MAPPED)
        return (this->flush_memory_mapped_entries(entries));
    const_cast<kv_store *>(this)->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
    return (-1);
}

int kv_store::load_backend_entries(kv_store_backend_type backend_type, const char *location, ft_vector<kv_store_snapshot_entry> &out_entries)
{
    if (backend_type == KV_STORE_BACKEND_JSON)
        return (this->load_json_entries(location, out_entries));
    if (backend_type == KV_STORE_BACKEND_JSON_LINES)
        return (this->load_json_lines_entries(location, out_entries));
    if (backend_type == KV_STORE_BACKEND_SQLITE)
        return (this->load_sqlite_entries(location, out_entries));
    if (backend_type == KV_STORE_BACKEND_MEMORY_MAPPED)
        return (this->load_memory_mapped_entries(location, out_entries));
    this->set_error(FT_ERR_INVALID_ARGUMENT);
    return (-1);
}

int kv_store::set_backend(kv_store_backend_type backend_type, const char *location)
{
    ft_vector<kv_store_snapshot_entry> loaded_entries;
    ft_string location_copy;
    int load_result;
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;

    if (location == ft_nullptr)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    location_copy = location;
    if (location_copy.get_error() != ER_SUCCESS)
    {
        this->set_error(location_copy.get_error());
        return (-1);
    }
    loaded_entries = ft_vector<kv_store_snapshot_entry>();
    if (loaded_entries.get_error() != ER_SUCCESS)
    {
        this->set_error(loaded_entries.get_error());
        return (-1);
    }
    load_result = this->load_backend_entries(backend_type, location, loaded_entries);
    if (load_result < 0)
        return (-1);
    entry_errno = ft_errno;
    lock_error = this->lock_store(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error_unlocked(lock_error);
        kv_store::restore_errno(guard, entry_errno);
        return (-1);
    }
    ft_string previous_path(this->_file_path);
    if (previous_path.get_error() != ER_SUCCESS)
    {
        this->set_error_unlocked(previous_path.get_error());
        kv_store::restore_errno(guard, entry_errno);
        return (-1);
    }
    kv_store_backend_type previous_backend;

    previous_backend = this->_backend_type;
    this->_file_path = location_copy;
    if (this->_file_path.get_error() != ER_SUCCESS)
    {
        int path_error;

        path_error = this->_file_path.get_error();
        this->_file_path = previous_path;
        if (this->_file_path.get_error() != ER_SUCCESS)
            this->_file_path.clear();
        this->_backend_type = previous_backend;
        this->set_error_unlocked(path_error);
        kv_store::restore_errno(guard, entry_errno);
        return (-1);
    }
    this->_backend_type = backend_type;
    if (load_result == 0)
    {
        if (this->apply_snapshot_locked(loaded_entries) != 0)
        {
            this->_file_path = previous_path;
            if (this->_file_path.get_error() != ER_SUCCESS)
                this->_file_path.clear();
            this->_backend_type = previous_backend;
            kv_store::restore_errno(guard, entry_errno);
            return (-1);
        }
    }
    this->set_error_unlocked(ER_SUCCESS);
    kv_store::restore_errno(guard, entry_errno);
    return (0);
}

kv_store_backend_type kv_store::get_backend() const
{
    return (this->_backend_type);
}

int kv_store::write_snapshot(ft_document_sink &sink) const
{
    kv_store *mutable_this;
    ft_vector<kv_store_snapshot_entry> entries;
    json_group *store_group;
    json_group *head_group;
    char *serialized_buffer;
    int write_result;

    mutable_this = const_cast<kv_store *>(this);
    entries = ft_vector<kv_store_snapshot_entry>();
    if (entries.get_error() != ER_SUCCESS)
    {
        mutable_this->set_error(entries.get_error());
        return (-1);
    }
    if (mutable_this->export_snapshot(entries) != 0)
        return (-1);
    store_group = json_create_json_group("kv_store");
    if (store_group == ft_nullptr)
    {
        mutable_this->set_error(FT_ERR_NO_MEMORY);
        return (-1);
    }
    if (this->_encryption_enabled)
    {
        json_item *metadata_item;

        metadata_item = json_create_item("__encryption__", "aes-128-ecb-base64");
        if (metadata_item == ft_nullptr)
        {
            json_free_groups(store_group);
            mutable_this->set_error(FT_ERR_NO_MEMORY);
            return (-1);
        }
        json_add_item_to_group(store_group, metadata_item);
    }
    const kv_store_snapshot_entry *entry_begin;
    const kv_store_snapshot_entry *entry_end;
    const kv_store_snapshot_entry *entry_cursor;

    entry_begin = entries.begin();
    if (entries.get_error() != ER_SUCCESS)
    {
        json_free_groups(store_group);
        mutable_this->set_error(entries.get_error());
        return (-1);
    }
    entry_end = entries.end();
    if (entries.get_error() != ER_SUCCESS)
    {
        json_free_groups(store_group);
        mutable_this->set_error(entries.get_error());
        return (-1);
    }
    entry_cursor = entry_begin;
    while (entry_cursor != entry_end)
    {
        json_item *item_pointer;
        ft_string stored_value;
        const char *value_pointer;

        value_pointer = entry_cursor->value.c_str();
        if (this->_encryption_enabled)
        {
            ft_string encoded_value;

            encoded_value = ft_string();
            if (encoded_value.get_error() != ER_SUCCESS)
            {
                json_free_groups(store_group);
                mutable_this->set_error(encoded_value.get_error());
                return (-1);
            }
            if (mutable_this->encrypt_value(entry_cursor->value, encoded_value) != 0)
            {
                json_free_groups(store_group);
                return (-1);
            }
            stored_value = encoded_value;
            if (stored_value.get_error() != ER_SUCCESS)
            {
                json_free_groups(store_group);
                mutable_this->set_error(stored_value.get_error());
                return (-1);
            }
            value_pointer = stored_value.c_str();
        }
        item_pointer = json_create_item(entry_cursor->key.c_str(), value_pointer);
        if (item_pointer == ft_nullptr)
        {
            json_free_groups(store_group);
            mutable_this->set_error(FT_ERR_NO_MEMORY);
            return (-1);
        }
        json_add_item_to_group(store_group, item_pointer);
        if (entry_cursor->has_expiration)
        {
            ft_string ttl_key;
            char expiration_buffer[32];
            int written_length;

            ttl_key = g_kv_store_ttl_prefix;
            if (ttl_key.get_error() != ER_SUCCESS)
            {
                json_free_groups(store_group);
                mutable_this->set_error(ttl_key.get_error());
                return (-1);
            }
            ttl_key += entry_cursor->key;
            if (ttl_key.get_error() != ER_SUCCESS)
            {
                json_free_groups(store_group);
                mutable_this->set_error(ttl_key.get_error());
                return (-1);
            }
            written_length = std::snprintf(expiration_buffer, sizeof(expiration_buffer), "%lld", entry_cursor->expiration_timestamp);
            if (written_length < 0 || static_cast<size_t>(written_length) >= sizeof(expiration_buffer))
            {
                json_free_groups(store_group);
                mutable_this->set_error(FT_ERR_INVALID_ARGUMENT);
                return (-1);
            }
            item_pointer = json_create_item(ttl_key.c_str(), expiration_buffer);
            if (item_pointer == ft_nullptr)
            {
                json_free_groups(store_group);
                mutable_this->set_error(FT_ERR_NO_MEMORY);
                return (-1);
            }
            json_add_item_to_group(store_group, item_pointer);
        }
        entry_cursor++;
    }
    head_group = ft_nullptr;
    json_append_group(&head_group, store_group);
    serialized_buffer = json_write_to_string(head_group);
    if (!serialized_buffer)
    {
        json_free_groups(head_group);
        mutable_this->set_error(FT_ERR_NO_MEMORY);
        return (-1);
    }
    write_result = sink.write_all(serialized_buffer, std::strlen(serialized_buffer));
    if (write_result != 0)
    {
        json_free_groups(head_group);
        cma_free(serialized_buffer);
        mutable_this->set_error(sink.get_error() != ER_SUCCESS ? sink.get_error() : FT_ERR_IO);
        return (-1);
    }
    json_free_groups(head_group);
    cma_free(serialized_buffer);
    mutable_this->set_error(ER_SUCCESS);
    return (0);
}

int kv_store::read_snapshot(ft_document_source &source)
{
    ft_string content;
    int read_result;
    json_group *group_head;
    ft_vector<kv_store_snapshot_entry> entries;
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;

    content = ft_string();
    if (content.get_error() != ER_SUCCESS)
    {
        this->set_error(content.get_error());
        return (-1);
    }
    read_result = source.read_all(content);
    if (read_result != 0)
    {
        if (source.get_error() != ER_SUCCESS)
            this->set_error(source.get_error());
        else
            this->set_error(FT_ERR_IO);
        return (-1);
    }
    group_head = json_read_from_string(content.c_str());
    if (!group_head)
    {
        this->set_error(ft_errno == ER_SUCCESS ? FT_ERR_INVALID_ARGUMENT : ft_errno);
        return (-1);
    }
    entries = ft_vector<kv_store_snapshot_entry>();
    if (entries.get_error() != ER_SUCCESS)
    {
        json_free_groups(group_head);
        this->set_error(entries.get_error());
        return (-1);
    }
    if (this->parse_json_groups(group_head, entries) != 0)
    {
        json_free_groups(group_head);
        return (-1);
    }
    json_free_groups(group_head);
    entry_errno = ft_errno;
    lock_error = this->lock_store(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error_unlocked(lock_error);
        kv_store::restore_errno(guard, entry_errno);
        return (-1);
    }
    if (this->apply_snapshot_locked(entries) != 0)
    {
        kv_store::restore_errno(guard, entry_errno);
        return (-1);
    }
    kv_store::restore_errno(guard, entry_errno);
    this->set_error(ER_SUCCESS);
    return (0);
}

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

void kv_store::record_set_operation() const noexcept
{
    this->_metrics_set_operations = this->_metrics_set_operations + 1;
    return ;
}

void kv_store::record_delete_operation() const noexcept
{
    this->_metrics_delete_operations = this->_metrics_delete_operations + 1;
    return ;
}

void kv_store::record_get_hit() const noexcept
{
    this->_metrics_get_hits = this->_metrics_get_hits + 1;
    return ;
}

void kv_store::record_get_miss() const noexcept
{
    this->_metrics_get_misses = this->_metrics_get_misses + 1;
    return ;
}

void kv_store::record_prune_metrics(long long removed_entries, long long duration_ms) const noexcept
{
    long long normalized_duration;

    normalized_duration = duration_ms;
    if (normalized_duration < 0)
        normalized_duration = 0;
    this->_metrics_prune_operations = this->_metrics_prune_operations + 1;
    this->_metrics_pruned_entries = this->_metrics_pruned_entries + removed_entries;
    this->_metrics_last_prune_duration_ms = normalized_duration;
    this->_metrics_total_prune_duration_ms = this->_metrics_total_prune_duration_ms + normalized_duration;
    return ;
}

int kv_store::lock_background(ft_unique_lock<pt_mutex> &guard) const noexcept
{
    int entry_errno;
    ft_unique_lock<pt_mutex> local_guard(this->_background_mutex);

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

int kv_store::start_background_thread_locked(long long interval_seconds) noexcept
{
    ft_thread background_instance;

    if (interval_seconds <= 0)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    if (this->_background_thread_active)
    {
        this->_background_interval_seconds = interval_seconds;
        this->set_error_unlocked(ER_SUCCESS);
        return (0);
    }
    this->_background_stop_requested = false;
    this->_background_interval_seconds = interval_seconds;
    background_instance = ft_thread(&kv_store::background_compaction_worker, this);
    if (background_instance.get_error() != ER_SUCCESS)
    {
        this->set_error_unlocked(background_instance.get_error());
        return (-1);
    }
    this->_background_thread = ft_move(background_instance);
    this->_background_thread_active = true;
    this->set_error_unlocked(ER_SUCCESS);
    return (0);
}

void kv_store::stop_background_thread_locked(ft_thread &thread_holder) noexcept
{
    if (this->_background_thread_active == false)
    {
        this->set_error_unlocked(ER_SUCCESS);
        return ;
    }
    this->_background_stop_requested = true;
    this->_background_interval_seconds = 0;
    thread_holder = ft_move(this->_background_thread);
    this->_background_thread_active = false;
    this->set_error_unlocked(ER_SUCCESS);
    return ;
}

void kv_store::background_compaction_worker(kv_store *store) noexcept
{
    if (store == ft_nullptr)
        return ;
    while (true)
    {
        ft_unique_lock<pt_mutex> guard;
        int entry_errno;
        bool stop_requested;
        long long interval_seconds;
        long long sleep_milliseconds;

        entry_errno = ft_errno;
        if (store->lock_background(guard) != ER_SUCCESS)
        {
            ft_errno = entry_errno;
            return ;
        }
        stop_requested = store->_background_stop_requested;
        interval_seconds = store->_background_interval_seconds;
        kv_store::restore_errno(guard, entry_errno);
        if (stop_requested)
            break;
        if (interval_seconds <= 0)
            interval_seconds = 1;
        if (interval_seconds > std::numeric_limits<long long>::max() / 1000)
            interval_seconds = std::numeric_limits<long long>::max() / 1000;
        sleep_milliseconds = interval_seconds * 1000;
        if (sleep_milliseconds <= 0)
            sleep_milliseconds = 1000;
        if (sleep_milliseconds > static_cast<long long>(std::numeric_limits<unsigned int>::max()))
            sleep_milliseconds = static_cast<long long>(std::numeric_limits<unsigned int>::max());
        pt_thread_sleep(static_cast<unsigned int>(sleep_milliseconds));
        entry_errno = ft_errno;
        store->prune_expired();
        ft_errno = entry_errno;
    }
    return ;
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
    t_monotonic_time_point start_time;
    long long removed_entries;

    if (guard.owns_lock() == false)
    {
        this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    start_time = time_monotonic_point_now();
    removed_entries = 0;
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
                removed_entries = removed_entries + 1;
                continue ;
            }
        }
        map_index++;
    }
    t_monotonic_time_point finish_time;
    long long duration_ms;

    finish_time = time_monotonic_point_now();
    duration_ms = time_monotonic_point_diff_ms(start_time, finish_time);
    this->record_prune_metrics(removed_entries, duration_ms);
    this->set_error_unlocked(ER_SUCCESS);
    return (0);
}

int kv_store::kv_set(const char *key_string, const char *value_string, long long ttl_seconds)
{
    ft_vector<kv_store_operation> operations;
    kv_store_operation operation;

    if (key_string == ft_nullptr || value_string == ft_nullptr)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    operations = ft_vector<kv_store_operation>();
    if (operations.get_error() != ER_SUCCESS)
    {
        this->set_error(operations.get_error());
        return (-1);
    }
    if (kv_store_init_set_operation(operation, key_string, value_string, ttl_seconds) != 0)
    {
        this->set_error(ft_errno);
        return (-1);
    }
    operations.push_back(operation);
    if (operations.get_error() != ER_SUCCESS)
    {
        this->set_error(operations.get_error());
        return (-1);
    }
    return (this->kv_apply(operations));
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
        mutable_this->record_get_miss();
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
    mutable_this->record_get_hit();
    kv_store::restore_errno(guard, entry_errno);
    return (value_pointer);
}

int kv_store::kv_delete(const char *key_string)
{
    ft_vector<kv_store_operation> operations;
    kv_store_operation operation;

    if (key_string == ft_nullptr)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    operations = ft_vector<kv_store_operation>();
    if (operations.get_error() != ER_SUCCESS)
    {
        this->set_error(operations.get_error());
        return (-1);
    }
    if (kv_store_init_delete_operation(operation, key_string) != 0)
    {
        this->set_error(ft_errno);
        return (-1);
    }
    operations.push_back(operation);
    if (operations.get_error() != ER_SUCCESS)
    {
        this->set_error(operations.get_error());
        return (-1);
    }
    return (this->kv_apply(operations));
}

int kv_store::kv_flush() const
{
    kv_store *mutable_this;
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;
    int flush_result;
    int flush_error;

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
    kv_store::restore_errno(guard, entry_errno);
    flush_result = 0;
    flush_error = ER_SUCCESS;
    {
        ft_vector<kv_store_snapshot_entry> entries;

        entries = ft_vector<kv_store_snapshot_entry>();
        if (entries.get_error() != ER_SUCCESS)
        {
            mutable_this->set_error(entries.get_error());
            return (-1);
        }
        if (mutable_this->export_snapshot(entries) != 0)
            return (-1);
        flush_result = this->flush_backend_entries(entries);
        if (flush_result != 0)
            flush_error = ft_errno;
    }
    if (flush_result != 0)
    {
        ft_errno = flush_error;
        return (flush_result);
    }
    return (flush_result);
}
int kv_store::kv_apply(const ft_vector<kv_store_operation> &operations)
{
    ft_unique_lock<pt_mutex> guard;
    ft_map<ft_string, kv_store_entry> staged_map;
    size_t operation_count;
    size_t operation_index;
    size_t metrics_set_count;
    size_t metrics_delete_count;
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
    metrics_set_count = 0;
    metrics_delete_count = 0;
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
            metrics_delete_count = metrics_delete_count + 1;
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
                        metrics_set_count = metrics_set_count + 1;
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
                    metrics_set_count = metrics_set_count + 1;
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
            metrics_set_count = metrics_set_count + 1;
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
    size_t metrics_index;

    metrics_index = 0;
    while (metrics_index < metrics_set_count)
    {
        this->record_set_operation();
        metrics_index++;
    }
    metrics_index = 0;
    while (metrics_index < metrics_delete_count)
    {
        this->record_delete_operation();
        metrics_index++;
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
        this->record_delete_operation();
        kv_store::restore_errno(guard, entry_errno);
        if (existing_pair != ft_nullptr)
        {
            ft_vector<kv_store_operation> replication_operations;
            kv_store_operation replication_operation;

            replication_operations = ft_vector<kv_store_operation>();
            if (replication_operations.get_error() != ER_SUCCESS)
            {
                this->set_error(replication_operations.get_error());
                return (-1);
            }
            if (kv_store_init_delete_operation(replication_operation, key_string) != 0)
            {
                this->set_error(ft_errno);
                return (-1);
            }
            replication_operations.push_back(replication_operation);
            if (replication_operations.get_error() != ER_SUCCESS)
            {
                this->set_error(replication_operations.get_error());
                return (-1);
            }
            if (this->notify_replication_listeners(replication_operations) != 0)
                return (-1);
        }
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
            this->record_set_operation();
            kv_store::restore_errno(guard, entry_errno);
            if (existing_pair != ft_nullptr)
            {
                ft_vector<kv_store_operation> replication_operations;
                kv_store_operation replication_operation;

                replication_operations = ft_vector<kv_store_operation>();
                if (replication_operations.get_error() != ER_SUCCESS)
                {
                    this->set_error(replication_operations.get_error());
                    return (-1);
                }
                if (kv_store_init_delete_operation(replication_operation, key_string) != 0)
                {
                    this->set_error(ft_errno);
                    return (-1);
                }
                replication_operations.push_back(replication_operation);
                if (replication_operations.get_error() != ER_SUCCESS)
                {
                    this->set_error(replication_operations.get_error());
                    return (-1);
                }
                if (this->notify_replication_listeners(replication_operations) != 0)
                    return (-1);
            }
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
    this->record_set_operation();
    kv_store::restore_errno(guard, entry_errno);
    {
        ft_vector<kv_store_operation> replication_operations;
        kv_store_operation replication_operation;
        long long replication_ttl_seconds;

        replication_operations = ft_vector<kv_store_operation>();
        if (replication_operations.get_error() != ER_SUCCESS)
        {
            this->set_error(replication_operations.get_error());
            return (-1);
        }
        replication_ttl_seconds = -1;
        if (ttl_seconds >= 0)
            replication_ttl_seconds = ttl_seconds;
        else if (has_expiration)
        {
            long long current_time;

            current_time = this->current_time_seconds();
            if (current_time < 0)
                replication_ttl_seconds = 0;
            else
            {
                replication_ttl_seconds = expiration_timestamp - current_time;
                if (replication_ttl_seconds < 0)
                    replication_ttl_seconds = 0;
            }
        }
        if (kv_store_init_set_operation(replication_operation, key_string, new_value, replication_ttl_seconds) != 0)
        {
            this->set_error(ft_errno);
            return (-1);
        }
        replication_operations.push_back(replication_operation);
        if (replication_operations.get_error() != ER_SUCCESS)
        {
            this->set_error(replication_operations.get_error());
            return (-1);
        }
        if (this->notify_replication_listeners(replication_operations) != 0)
            return (-1);
    }
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

int kv_store::get_metrics(kv_store_metrics &out_metrics) const
{
    ft_unique_lock<pt_mutex> guard;
    kv_store *mutable_this;
    int entry_errno;
    int lock_error;

    entry_errno = ft_errno;
    lock_error = this->lock_store(guard);
    if (lock_error != ER_SUCCESS)
    {
        mutable_this = const_cast<kv_store *>(this);
        mutable_this->set_error_unlocked(lock_error);
        kv_store::restore_errno(guard, entry_errno);
        return (-1);
    }
    out_metrics.set_operations = this->_metrics_set_operations;
    out_metrics.delete_operations = this->_metrics_delete_operations;
    out_metrics.get_hits = this->_metrics_get_hits;
    out_metrics.get_misses = this->_metrics_get_misses;
    out_metrics.prune_operations = this->_metrics_prune_operations;
    out_metrics.pruned_entries = this->_metrics_pruned_entries;
    out_metrics.total_prune_duration_ms = this->_metrics_total_prune_duration_ms;
    out_metrics.last_prune_duration_ms = this->_metrics_last_prune_duration_ms;
    mutable_this = const_cast<kv_store *>(this);
    mutable_this->set_error_unlocked(ER_SUCCESS);
    kv_store::restore_errno(guard, entry_errno);
    return (0);
}

int kv_store::export_snapshot(ft_vector<kv_store_snapshot_entry> &out_entries) const
{
    ft_unique_lock<pt_mutex> guard;
    kv_store *mutable_this;
    int entry_errno;
    int lock_error;
    size_t map_size;
    const Pair<ft_string, kv_store_entry> *map_begin;
    const Pair<ft_string, kv_store_entry> *map_end;
    size_t map_index;

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
    out_entries.clear();
    if (out_entries.get_error() != ER_SUCCESS)
    {
        mutable_this->set_error_unlocked(out_entries.get_error());
        kv_store::restore_errno(guard, entry_errno);
        return (-1);
    }
    map_size = this->_data.size();
    if (this->_data.get_error() != ER_SUCCESS)
    {
        mutable_this->set_error_unlocked(this->_data.get_error());
        kv_store::restore_errno(guard, entry_errno);
        return (-1);
    }
    map_end = this->_data.end();
    if (this->_data.get_error() != ER_SUCCESS)
    {
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
        kv_store_snapshot_entry snapshot_entry;
        ft_string plain_value;
        bool has_expiration;
        long long expiration_timestamp;

        plain_value = ft_string();
        if (plain_value.get_error() != ER_SUCCESS)
        {
            mutable_this->set_error_unlocked(plain_value.get_error());
            kv_store::restore_errno(guard, entry_errno);
            return (-1);
        }
        if (entry_value.copy_value(plain_value) != 0)
        {
            mutable_this->set_error_unlocked(entry_value.get_error());
            kv_store::restore_errno(guard, entry_errno);
            return (-1);
        }
        snapshot_entry.key = entry.key;
        if (snapshot_entry.key.get_error() != ER_SUCCESS)
        {
            mutable_this->set_error_unlocked(snapshot_entry.key.get_error());
            kv_store::restore_errno(guard, entry_errno);
            return (-1);
        }
        snapshot_entry.value = plain_value;
        if (snapshot_entry.value.get_error() != ER_SUCCESS)
        {
            mutable_this->set_error_unlocked(snapshot_entry.value.get_error());
            kv_store::restore_errno(guard, entry_errno);
            return (-1);
        }
        has_expiration = false;
        expiration_timestamp = 0;
        if (entry_value.has_expiration(has_expiration) != 0)
        {
            mutable_this->set_error_unlocked(entry_value.get_error());
            kv_store::restore_errno(guard, entry_errno);
            return (-1);
        }
        if (has_expiration)
        {
            if (entry_value.get_expiration(expiration_timestamp) != 0)
            {
                mutable_this->set_error_unlocked(entry_value.get_error());
                kv_store::restore_errno(guard, entry_errno);
                return (-1);
            }
        }
        snapshot_entry.has_expiration = has_expiration;
        snapshot_entry.expiration_timestamp = expiration_timestamp;
        out_entries.push_back(ft_move(snapshot_entry));
        if (out_entries.get_error() != ER_SUCCESS)
        {
            mutable_this->set_error_unlocked(out_entries.get_error());
            kv_store::restore_errno(guard, entry_errno);
            return (-1);
        }
        map_index++;
    }
    mutable_this->set_error_unlocked(ER_SUCCESS);
    kv_store::restore_errno(guard, entry_errno);
    return (0);
}

int kv_store::export_snapshot_to_file(const char *file_path) const
{
    ft_vector<kv_store_snapshot_entry> entries;
    kv_store *mutable_this;
    json_group *snapshot_group;
    json_group *head_group;
    kv_store_snapshot_entry *entry_begin;
    kv_store_snapshot_entry *entry_end;
    kv_store_snapshot_entry *entry_cursor;
    json_item *value_item;
    int write_result;
    int error_code;

    mutable_this = const_cast<kv_store *>(this);
    if (file_path == ft_nullptr)
    {
        mutable_this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    entries = ft_vector<kv_store_snapshot_entry>();
    if (entries.get_error() != ER_SUCCESS)
    {
        mutable_this->set_error(entries.get_error());
        return (-1);
    }
    if (mutable_this->export_snapshot(entries) != 0)
        return (-1);
    snapshot_group = json_create_json_group("kv_store_snapshot");
    if (snapshot_group == ft_nullptr)
    {
        mutable_this->set_error(FT_ERR_NO_MEMORY);
        return (-1);
    }
    entry_begin = entries.begin();
    if (entries.get_error() != ER_SUCCESS)
    {
        json_free_groups(snapshot_group);
        mutable_this->set_error(entries.get_error());
        return (-1);
    }
    entry_end = entries.end();
    if (entries.get_error() != ER_SUCCESS)
    {
        json_free_groups(snapshot_group);
        mutable_this->set_error(entries.get_error());
        return (-1);
    }
    entry_cursor = entry_begin;
    while (entry_cursor != entry_end)
    {
        if (entry_cursor->key.c_str() == ft_nullptr || entry_cursor->value.c_str() == ft_nullptr)
        {
            json_free_groups(snapshot_group);
            mutable_this->set_error(FT_ERR_INVALID_ARGUMENT);
            return (-1);
        }
        value_item = json_create_item(entry_cursor->key.c_str(), entry_cursor->value.c_str());
        if (value_item == ft_nullptr)
        {
            json_free_groups(snapshot_group);
            mutable_this->set_error(FT_ERR_NO_MEMORY);
            return (-1);
        }
        json_add_item_to_group(snapshot_group, value_item);
        if (entry_cursor->has_expiration)
        {
            ft_string ttl_key;
            char expiration_buffer[32];
            int written_length;

            ttl_key = g_kv_store_ttl_prefix;
            if (ttl_key.get_error() != ER_SUCCESS)
            {
                json_free_groups(snapshot_group);
                mutable_this->set_error(ttl_key.get_error());
                return (-1);
            }
            ttl_key += entry_cursor->key;
            if (ttl_key.get_error() != ER_SUCCESS)
            {
                json_free_groups(snapshot_group);
                mutable_this->set_error(ttl_key.get_error());
                return (-1);
            }
            written_length = std::snprintf(expiration_buffer, sizeof(expiration_buffer), "%lld", entry_cursor->expiration_timestamp);
            if (written_length < 0 || static_cast<size_t>(written_length) >= sizeof(expiration_buffer))
            {
                json_free_groups(snapshot_group);
                mutable_this->set_error(FT_ERR_INVALID_ARGUMENT);
                return (-1);
            }
            value_item = json_create_item(ttl_key.c_str(), expiration_buffer);
            if (value_item == ft_nullptr)
            {
                json_free_groups(snapshot_group);
                mutable_this->set_error(FT_ERR_NO_MEMORY);
                return (-1);
            }
            json_add_item_to_group(snapshot_group, value_item);
        }
        entry_cursor++;
    }
    head_group = ft_nullptr;
    json_append_group(&head_group, snapshot_group);
    write_result = json_write_to_file(file_path, head_group);
    error_code = ft_errno;
    json_free_groups(head_group);
    if (write_result != 0)
    {
        if (error_code != ER_SUCCESS)
            mutable_this->set_error(error_code);
        else
            mutable_this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    mutable_this->set_error(ER_SUCCESS);
    return (0);
}

int kv_store::apply_snapshot_locked(const ft_vector<kv_store_snapshot_entry> &entries)
{
    ft_map<ft_string, kv_store_entry> staged_map;
    const kv_store_snapshot_entry *entry_begin;
    const kv_store_snapshot_entry *entry_end;
    const kv_store_snapshot_entry *entry_cursor;

    if (entries.get_error() != ER_SUCCESS)
    {
        this->set_error_unlocked(entries.get_error());
        return (-1);
    }
    staged_map = ft_map<ft_string, kv_store_entry>();
    if (staged_map.get_error() != ER_SUCCESS)
    {
        this->set_error_unlocked(staged_map.get_error());
        return (-1);
    }
    entry_begin = entries.begin();
    if (entries.get_error() != ER_SUCCESS)
    {
        this->set_error_unlocked(entries.get_error());
        return (-1);
    }
    entry_end = entries.end();
    if (entries.get_error() != ER_SUCCESS)
    {
        this->set_error_unlocked(entries.get_error());
        return (-1);
    }
    entry_cursor = entry_begin;
    while (entry_cursor != entry_end)
    {
        kv_store_entry entry_value;

        if (entry_cursor->key.c_str() == ft_nullptr)
        {
            this->set_error_unlocked(FT_ERR_INVALID_ARGUMENT);
            return (-1);
        }
        if (entry_value.set_value(entry_cursor->value) != 0)
        {
            this->set_error_unlocked(entry_value.get_error());
            return (-1);
        }
        if (entry_value.configure_expiration(entry_cursor->has_expiration, entry_cursor->expiration_timestamp) != 0)
        {
            this->set_error_unlocked(entry_value.get_error());
            return (-1);
        }
        staged_map.insert(entry_cursor->key, entry_value);
        if (staged_map.get_error() != ER_SUCCESS)
        {
            this->set_error_unlocked(staged_map.get_error());
            return (-1);
        }
        entry_cursor++;
    }
    this->_data = ft_move(staged_map);
    if (this->_data.get_error() != ER_SUCCESS)
    {
        this->set_error_unlocked(this->_data.get_error());
        return (-1);
    }
    this->_metrics_set_operations = 0;
    this->_metrics_delete_operations = 0;
    this->_metrics_get_hits = 0;
    this->_metrics_get_misses = 0;
    this->_metrics_prune_operations = 0;
    this->_metrics_pruned_entries = 0;
    this->_metrics_total_prune_duration_ms = 0;
    this->_metrics_last_prune_duration_ms = 0;
    this->set_error_unlocked(ER_SUCCESS);
    return (0);
}

int kv_store::import_snapshot(const ft_vector<kv_store_snapshot_entry> &entries)
{
    ft_unique_lock<pt_mutex> guard;
    int entry_errno;
    int lock_error;

    if (entries.get_error() != ER_SUCCESS)
    {
        this->set_error(entries.get_error());
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
    if (this->apply_snapshot_locked(entries) != 0)
    {
        kv_store::restore_errno(guard, entry_errno);
        return (-1);
    }
    kv_store::restore_errno(guard, entry_errno);
    return (0);
}

int kv_store::start_background_compaction(long long interval_seconds)
{
    ft_unique_lock<pt_mutex> guard;
    kv_store *mutable_this;
    int entry_errno;
    int lock_error;
    int start_result;

    mutable_this = this;
    entry_errno = ft_errno;
    lock_error = this->lock_background(guard);
    if (lock_error != ER_SUCCESS)
    {
        mutable_this->set_error_unlocked(lock_error);
        kv_store::restore_errno(guard, entry_errno);
        return (-1);
    }
    start_result = mutable_this->start_background_thread_locked(interval_seconds);
    kv_store::restore_errno(guard, entry_errno);
    if (start_result != 0)
        return (-1);
    return (0);
}

int kv_store::stop_background_compaction()
{
    ft_unique_lock<pt_mutex> guard;
    kv_store *mutable_this;
    ft_thread thread_holder;
    int entry_errno;
    int lock_error;

    mutable_this = this;
    entry_errno = ft_errno;
    lock_error = this->lock_background(guard);
    if (lock_error != ER_SUCCESS)
    {
        mutable_this->set_error_unlocked(lock_error);
        kv_store::restore_errno(guard, entry_errno);
        return (-1);
    }
    mutable_this->stop_background_thread_locked(thread_holder);
    kv_store::restore_errno(guard, entry_errno);
    if (thread_holder.joinable())
    {
        thread_holder.join();
        if (thread_holder.get_error() != ER_SUCCESS)
        {
            mutable_this->set_error(thread_holder.get_error());
            return (-1);
        }
    }
    entry_errno = ft_errno;
    lock_error = this->lock_background(guard);
    if (lock_error != ER_SUCCESS)
    {
        mutable_this->set_error_unlocked(lock_error);
        kv_store::restore_errno(guard, entry_errno);
        return (-1);
    }
    this->_background_stop_requested = false;
    mutable_this->set_error_unlocked(ER_SUCCESS);
    kv_store::restore_errno(guard, entry_errno);
    return (0);
}

