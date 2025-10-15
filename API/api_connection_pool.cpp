#include "api_internal.hpp"
#include "../Libft/libft.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"
#include <vector>
#include <memory>
#include <chrono>
#include <utility>
#include <cstddef>
#include <string>
#include <set>
#include <cstdlib>
#include <new>

struct api_pooled_connection
{
    ft_socket socket;
    SSL *tls_session;
    SSL_CTX *tls_context;
    bool uses_tls;
    long long idle_timestamp_ms;

    static void *operator new(size_t size)
    {
        void *pointer;

        pointer = std::malloc(size);
        if (!pointer)
            throw std::bad_alloc();
        return (pointer);
    }

    static void operator delete(void *pointer) noexcept
    {
        std::free(pointer);
        return ;
    }
};

typedef std::unique_ptr<api_pooled_connection> t_api_connection_entry;
typedef std::vector<t_api_connection_entry> t_api_connection_bucket;

struct api_connection_bucket_entry
{
    std::string key;
    t_api_connection_bucket bucket;
};

typedef std::vector<api_connection_bucket_entry> t_api_connection_map;
typedef t_api_connection_bucket::difference_type t_api_bucket_difference;
typedef t_api_connection_map::difference_type t_api_map_difference;

static void api_connection_pool_dispose_entry(api_pooled_connection &entry);

static t_api_connection_map &api_connection_pool_get_buckets(void)
{
    static t_api_connection_map buckets;

    return (buckets);
}

static std::set<SSL*> &api_connection_pool_get_tls_registry(void)
{
    static std::set<SSL*> tls_registry;

    return (tls_registry);
}

static pt_mutex &api_connection_pool_get_tls_mutex(void)
{
    static pt_mutex tls_mutex;

    return (tls_mutex);
}

static bool api_connection_pool_tls_register(SSL *tls_session)
{
    std::set<SSL*> &tls_registry = api_connection_pool_get_tls_registry();
    std::pair<std::set<SSL*>::iterator, bool> insert_result;
    ft_unique_lock<pt_mutex> guard(api_connection_pool_get_tls_mutex());

    if (!tls_session)
        return (true);
    insert_result = tls_registry.insert(tls_session);
    if (!insert_result.second)
        return (false);
    return (true);
}

static bool api_connection_pool_tls_unregister(SSL *tls_session)
{
    std::set<SSL*> &tls_registry = api_connection_pool_get_tls_registry();
    std::set<SSL*>::iterator iterator;
    ft_unique_lock<pt_mutex> guard(api_connection_pool_get_tls_mutex());

    if (!tls_session)
        return (false);
    iterator = tls_registry.find(tls_session);
    if (iterator == tls_registry.end())
        return (false);
    tls_registry.erase(iterator);
    return (true);
}

bool api_connection_pool_track_tls_session(SSL *tls_session)
{
    bool registered;

    registered = api_connection_pool_tls_register(tls_session);
    if (!registered)
    {
        api_connection_pool_tls_unregister(tls_session);
        registered = api_connection_pool_tls_register(tls_session);
    }
    return (registered);
}

bool api_connection_pool_untrack_tls_session(SSL *tls_session)
{
    bool removed;

    removed = api_connection_pool_tls_unregister(tls_session);
    return (removed);
}

static void api_connection_pool_clear_buckets(t_api_connection_map &buckets)
{
    size_t index;

    index = 0;
    while (index < buckets.size())
    {
        t_api_connection_bucket &bucket = buckets[index].bucket;
        size_t entry_index;

        entry_index = 0;
        while (entry_index < bucket.size())
        {
            t_api_connection_entry &entry_pointer = bucket[entry_index];

            if (entry_pointer)
                api_connection_pool_dispose_entry(*entry_pointer);
            entry_index++;
        }
        bucket.clear();
        index++;
    }
    buckets.clear();
    return ;
}

static bool api_connection_pool_key_is_valid(const std::string &key)
{
    size_t index;
    size_t length;
    size_t colon_count;

    length = key.size();
    if (length == 0)
        return (false);
    index = 0;
    colon_count = 0;
    while (index < length)
    {
        if (key[index] == ':')
            colon_count++;
        index++;
    }
    if (colon_count < 3)
        return (false);
    return (true);
}

static pt_mutex &api_connection_pool_get_mutex(void)
{
    static pt_mutex connection_pool_mutex;

    return (connection_pool_mutex);
}
static size_t g_api_connection_max_idle = 8;
static long long g_api_connection_idle_timeout_ms = 60000;
static size_t g_api_connection_pool_acquire_calls = 0;
static size_t g_api_connection_pool_reuse_hits = 0;
static size_t g_api_connection_pool_acquire_misses = 0;
static bool g_api_connection_pool_enabled = true;

static long long api_connection_pool_now_ms(void)
{
    std::chrono::steady_clock::time_point now_point;
    std::chrono::steady_clock::duration duration_since_epoch;
    long long milliseconds;

    now_point = std::chrono::steady_clock::now();
    duration_since_epoch = now_point.time_since_epoch();
    milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
            duration_since_epoch).count();
    return (milliseconds);
}

static ft_string api_connection_pool_build_key(const char *host, uint16_t port,
        api_connection_security_mode security_mode,
        const char *security_identity)
{
    ft_string key;
    ft_string port_string;

    if (host)
        key = host;
    key += ":";
    port_string = ft_to_string(static_cast<long>(port));
    key += port_string;
    key += ":";
    if (security_mode == api_connection_security_mode::TLS)
        key += "tls";
    else
        key += "plain";
    key += ":";
    if (security_identity && security_identity[0] != '\0')
        key += security_identity;
    return (key);
}

static void api_connection_pool_free_tls(SSL *tls_session, SSL_CTX *tls_context)
{
    if (tls_session)
        SSL_free(tls_session);
    if (tls_context)
        SSL_CTX_free(tls_context);
    return ;
}

static void api_connection_pool_dispose_entry(api_pooled_connection &entry)
{
    entry.socket.disconnect_all_clients();
    if (entry.uses_tls)
    {
        if (api_connection_pool_tls_unregister(entry.tls_session))
            api_connection_pool_free_tls(entry.tls_session, entry.tls_context);
    }
    entry.tls_session = ft_nullptr;
    entry.tls_context = ft_nullptr;
    entry.uses_tls = false;
    entry.idle_timestamp_ms = 0;
    entry.socket.close_socket();
    return ;
}

static void api_connection_pool_bucket_erase(t_api_connection_bucket &entries, size_t index)
{
    t_api_bucket_difference offset;

    offset = static_cast<t_api_bucket_difference>(index);
    entries.erase(entries.begin() + offset);
    return ;
}

static void api_connection_pool_drop_tls_duplicates(SSL *tls_session)
{
    t_api_connection_map &buckets = api_connection_pool_get_buckets();
    size_t bucket_index;

    if (!tls_session)
        return ;
    api_connection_pool_tls_unregister(tls_session);
    bucket_index = 0;
    while (bucket_index < buckets.size())
    {
        t_api_connection_bucket &bucket = buckets[bucket_index].bucket;
        size_t entry_index;

        entry_index = 0;
        while (entry_index < bucket.size())
        {
            t_api_connection_entry &entry_pointer = bucket[entry_index];

            if (!entry_pointer)
            {
                entry_index++;
                continue ;
            }
            api_pooled_connection *entry = entry_pointer.get();

            if (entry->tls_session != tls_session)
            {
                entry_index++;
                continue ;
            }
            entry->socket.close_socket();
            entry->tls_session = ft_nullptr;
            entry->tls_context = ft_nullptr;
            entry->uses_tls = false;
            api_connection_pool_bucket_erase(bucket, entry_index);
            continue ;
        }
        bucket_index++;
    }
    return ;
}

static void api_connection_pool_prune_expired(t_api_connection_bucket &entries,
        long long now_ms)
{
    size_t index;

    index = 0;
    while (index < entries.size())
    {
        bool remove_entry;
        t_api_connection_entry &entry_pointer = entries[index];
        api_pooled_connection *entry;

        if (!entry_pointer)
        {
            api_connection_pool_bucket_erase(entries, index);
            continue;
        }
        entry = entry_pointer.get();
        remove_entry = false;
        if (g_api_connection_idle_timeout_ms >= 0)
        {
            long long age;

            age = now_ms - entry->idle_timestamp_ms;
            if (age >= g_api_connection_idle_timeout_ms)
                remove_entry = true;
        }
        if (entry->socket.get_error() != ER_SUCCESS)
            remove_entry = true;
        if (remove_entry)
        {
            api_connection_pool_dispose_entry(*entry);
            api_connection_pool_bucket_erase(entries, index);
        }
        else
            index++;
    }
    return ;
}

static void api_connection_pool_remove_oldest(t_api_connection_bucket &entries)
{
    if (entries.empty())
        return ;
    t_api_connection_entry entry_pointer;

    entry_pointer = std::move(entries.front());
    entries.erase(entries.begin());
    if (!entry_pointer)
        return ;
    api_connection_pool_dispose_entry(*entry_pointer);
    return ;
}

static std::string api_connection_pool_to_std_key(const ft_string &key)
{
    const char *c_string;

    c_string = key.c_str();
    if (!c_string)
        return (std::string());
    return (std::string(c_string));
}

static t_api_connection_map::iterator api_connection_pool_find_entry(
        t_api_connection_map &buckets, const std::string &key)
{
    size_t index;

    index = 0;
    while (index < buckets.size())
    {
        t_api_connection_map::iterator iterator;

        iterator = buckets.begin() + static_cast<t_api_map_difference>(index);
        if (iterator->key == key)
            return (iterator);
        index++;
    }
    return (buckets.end());
}

bool api_connection_pool_acquire(api_connection_pool_handle &handle,
        const char *host, uint16_t port,
        api_connection_security_mode security_mode,
        const char *security_identity)
{
    ft_string key;
    std::string map_key;
    t_api_connection_map &buckets = api_connection_pool_get_buckets();
    ft_unique_lock<pt_mutex> guard(api_connection_pool_get_mutex());
    t_api_connection_map::iterator iterator;
    long long now_ms;

    key = api_connection_pool_build_key(host, port, security_mode, security_identity);
    handle.key = key;
    handle.security_mode = security_mode;
    handle.tls_session = ft_nullptr;
    handle.tls_context = ft_nullptr;
    handle.has_socket = false;
    handle.from_pool = false;
    handle.should_store = false;
    g_api_connection_pool_acquire_calls++;
    if (!g_api_connection_pool_enabled)
    {
        g_api_connection_pool_acquire_misses++;
        return (false);
    }
    handle.should_store = true;
    map_key = api_connection_pool_to_std_key(handle.key);
    if (!api_connection_pool_key_is_valid(map_key))
    {
        handle.should_store = false;
        g_api_connection_pool_acquire_misses++;
        return (false);
    }
    iterator = api_connection_pool_find_entry(buckets, map_key);
    if (iterator == buckets.end())
    {
        g_api_connection_pool_acquire_misses++;
        return (false);
    }
    now_ms = api_connection_pool_now_ms();
    api_connection_pool_prune_expired(iterator->bucket, now_ms);
    while (!iterator->bucket.empty())
    {
        size_t last_index;
        t_api_connection_entry entry_pointer;
        api_pooled_connection *entry;

        last_index = iterator->bucket.size() - 1;
        entry_pointer = std::move(iterator->bucket[last_index]);
        iterator->bucket.erase(iterator->bucket.begin()
                + static_cast<t_api_bucket_difference>(last_index));
        if (!entry_pointer)
            continue ;
        entry = entry_pointer.get();
        handle.socket = std::move(entry->socket);
        handle.tls_session = entry->tls_session;
        handle.tls_context = entry->tls_context;
        handle.has_socket = true;
        handle.from_pool = true;
        handle.should_store = true;
        if (entry->uses_tls)
            api_connection_pool_tls_unregister(handle.tls_session);
        entry->tls_session = ft_nullptr;
        entry->tls_context = ft_nullptr;
        entry->uses_tls = false;
        if (iterator->bucket.empty())
        {
            t_api_map_difference bucket_offset;

            bucket_offset = iterator - buckets.begin();
            buckets.erase(buckets.begin() + bucket_offset);
        }
        g_api_connection_pool_reuse_hits++;
        return (true);
    }
    if (iterator->bucket.empty())
    {
        t_api_map_difference bucket_offset;

        bucket_offset = iterator - buckets.begin();
        buckets.erase(buckets.begin() + bucket_offset);
    }
    g_api_connection_pool_acquire_misses++;
    return (false);
}

void api_connection_pool_mark_idle(api_connection_pool_handle &handle)
{
    long long now_ms;
    t_api_connection_map::iterator iterator;
    t_api_connection_bucket *bucket;
    std::string map_key;

    if (!handle.has_socket)
        return ;
    if (!handle.should_store)
    {
        api_connection_pool_evict(handle);
        return ;
    }
    if (!g_api_connection_pool_enabled)
    {
        api_connection_pool_evict(handle);
        return ;
    }
    if (handle.security_mode == api_connection_security_mode::TLS)
    {
        api_connection_pool_evict(handle);
        return ;
    }
    if (g_api_connection_max_idle == 0)
    {
        api_connection_pool_evict(handle);
        return ;
    }
    if (handle.socket.get_error() != ER_SUCCESS)
    {
        api_connection_pool_evict(handle);
        return ;
    }
    now_ms = api_connection_pool_now_ms();
    t_api_connection_map &buckets = api_connection_pool_get_buckets();
    ft_unique_lock<pt_mutex> guard(api_connection_pool_get_mutex());

    map_key = api_connection_pool_to_std_key(handle.key);
    if (!api_connection_pool_key_is_valid(map_key))
    {
        api_connection_pool_evict(handle);
        return ;
    }
    iterator = api_connection_pool_find_entry(buckets, map_key);
    if (iterator == buckets.end())
    {
        api_connection_bucket_entry new_entry;

        new_entry.key = map_key;
        buckets.push_back(std::move(new_entry));
        iterator = buckets.begin() + static_cast<t_api_map_difference>(
                buckets.size() - 1);
    }
    bucket = &iterator->bucket;
    api_connection_pool_prune_expired(*bucket, now_ms);
    while (bucket->size() >= g_api_connection_max_idle)
        api_connection_pool_remove_oldest(*bucket);
    t_api_connection_entry entry_pointer(new api_pooled_connection());
    if (!entry_pointer)
    {
        api_connection_pool_evict(handle);
        return ;
    }
    entry_pointer->socket = std::move(handle.socket);
    entry_pointer->tls_session = handle.tls_session;
    entry_pointer->tls_context = handle.tls_context;
    entry_pointer->uses_tls = handle.security_mode == api_connection_security_mode::TLS;
    entry_pointer->idle_timestamp_ms = now_ms;
    if (!api_connection_pool_tls_register(entry_pointer->tls_session))
    {
        api_connection_pool_drop_tls_duplicates(entry_pointer->tls_session);
        if (!api_connection_pool_tls_register(entry_pointer->tls_session))
        {
            api_connection_pool_evict(handle);
            return ;
        }
    }
    bucket->push_back(std::move(entry_pointer));
    handle.tls_session = ft_nullptr;
    handle.tls_context = ft_nullptr;
    handle.has_socket = false;
    handle.from_pool = false;
    return ;
}

void api_connection_pool_evict(api_connection_pool_handle &handle)
{
    if (!handle.has_socket)
        return ;
    if (handle.security_mode == api_connection_security_mode::TLS)
    {
        bool tls_removed;

        tls_removed = api_connection_pool_untrack_tls_session(handle.tls_session);
        if (tls_removed)
            api_connection_pool_free_tls(handle.tls_session, handle.tls_context);
        handle.tls_session = ft_nullptr;
        handle.tls_context = ft_nullptr;
    }
    handle.socket.disconnect_all_clients();
    handle.socket.close_socket();
    handle.has_socket = false;
    handle.from_pool = false;
    handle.should_store = false;
    return ;
}

void api_connection_pool_disable_store(api_connection_pool_handle &handle)
{
    handle.should_store = false;
    return ;
}

void api_connection_pool_flush(void)
{
    t_api_connection_map &buckets = api_connection_pool_get_buckets();
    ft_unique_lock<pt_mutex> guard(api_connection_pool_get_mutex());

    api_connection_pool_clear_buckets(buckets);
    return ;
}

void api_connection_pool_set_enabled(bool enabled)
{
    t_api_connection_map &buckets = api_connection_pool_get_buckets();
    ft_unique_lock<pt_mutex> guard(api_connection_pool_get_mutex());

    g_api_connection_pool_enabled = enabled;
    if (!enabled)
        api_connection_pool_clear_buckets(buckets);
    return ;
}

bool api_connection_pool_is_enabled(void)
{
    return (g_api_connection_pool_enabled);
}

void api_connection_pool_set_max_idle(size_t max_idle)
{
    if (max_idle == 0)
        g_api_connection_max_idle = 0;
    else
        g_api_connection_max_idle = max_idle;
    return ;
}

size_t api_connection_pool_get_max_idle(void)
{
    return (g_api_connection_max_idle);
}

void api_connection_pool_set_idle_timeout(long long idle_timeout_ms)
{
    g_api_connection_idle_timeout_ms = idle_timeout_ms;
    return ;
}

long long api_connection_pool_get_idle_timeout(void)
{
    return (g_api_connection_idle_timeout_ms);
}

void api_debug_reset_connection_pool_counters(void)
{
    g_api_connection_pool_acquire_calls = 0;
    g_api_connection_pool_reuse_hits = 0;
    g_api_connection_pool_acquire_misses = 0;
    return ;
}

size_t api_debug_get_connection_pool_acquires(void)
{
    return (g_api_connection_pool_acquire_calls);
}

size_t api_debug_get_connection_pool_reuses(void)
{
    return (g_api_connection_pool_reuse_hits);
}

size_t api_debug_get_connection_pool_misses(void)
{
    return (g_api_connection_pool_acquire_misses);
}

