#include "api_internal.hpp"
#include "../Libft/libft.hpp"
#include "../Errno/errno.hpp"
#include "../Template/map.hpp"
#include "../Template/pair.hpp"
#include <vector>
#include <chrono>
#include <utility>
#include <cstddef>

struct api_pooled_connection
{
    ft_socket socket;
    SSL *tls_session;
    SSL_CTX *tls_context;
    bool uses_tls;
    long long idle_timestamp_ms;
};

typedef std::vector<api_pooled_connection> t_api_connection_bucket;
typedef Pair<ft_string, t_api_connection_bucket> t_api_connection_pair;
typedef ft_map<ft_string, t_api_connection_bucket> t_api_connection_map;
typedef t_api_connection_bucket::difference_type t_api_bucket_difference;

static t_api_connection_map g_api_connection_buckets;
static size_t g_api_connection_max_idle = 8;
static long long g_api_connection_idle_timeout_ms = 60000;
static size_t g_api_connection_pool_acquire_calls = 0;
static size_t g_api_connection_pool_reuse_hits = 0;
static size_t g_api_connection_pool_acquire_misses = 0;

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
    if (entry.uses_tls)
    {
        api_connection_pool_free_tls(entry.tls_session, entry.tls_context);
        entry.tls_session = ft_nullptr;
        entry.tls_context = ft_nullptr;
    }
    return ;
}

static void api_connection_pool_bucket_erase(t_api_connection_bucket &entries, size_t index)
{
    t_api_bucket_difference offset;

    offset = static_cast<t_api_bucket_difference>(index);
    entries.erase(entries.begin() + offset);
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
        api_pooled_connection &entry = entries[index];

        remove_entry = false;
        if (g_api_connection_idle_timeout_ms >= 0)
        {
            long long age;

            age = now_ms - entry.idle_timestamp_ms;
            if (age >= g_api_connection_idle_timeout_ms)
                remove_entry = true;
        }
        if (entry.socket.get_error() != ER_SUCCESS)
            remove_entry = true;
        if (remove_entry)
        {
            api_connection_pool_dispose_entry(entry);
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
    api_pooled_connection entry = std::move(entries.front());

    api_connection_pool_dispose_entry(entry);
    entries.erase(entries.begin());
    return ;
}

static void api_connection_pool_cleanup_if_empty(const ft_string &key)
{
    t_api_connection_pair *pair;

    pair = g_api_connection_buckets.find(key);
    if (pair == ft_nullptr)
        return ;
    if (!pair->value.empty())
        return ;
    g_api_connection_buckets.remove(key);
    return ;
}

bool api_connection_pool_acquire(api_connection_pool_handle &handle,
        const char *host, uint16_t port,
        api_connection_security_mode security_mode,
        const char *security_identity)
{
    ft_string key;
    t_api_connection_pair *pair;
    bool acquired;

    key = api_connection_pool_build_key(host, port, security_mode, security_identity);
    handle.key = key;
    handle.security_mode = security_mode;
    handle.tls_session = ft_nullptr;
    handle.tls_context = ft_nullptr;
    handle.has_socket = false;
    handle.from_pool = false;
    handle.should_store = true;
    acquired = false;
    g_api_connection_pool_acquire_calls++;
    pair = g_api_connection_buckets.find(key);
    if (pair == ft_nullptr)
    {
        g_api_connection_pool_acquire_misses++;
        return (false);
    }
    long long now_ms;

    now_ms = api_connection_pool_now_ms();
    api_connection_pool_prune_expired(pair->value, now_ms);
    while (!pair->value.empty() && !acquired)
    {
        size_t index;
        api_pooled_connection entry;
        bool valid_entry;

        index = pair->value.size() - 1;
        entry = std::move(pair->value[index]);
        pair->value.pop_back();
        valid_entry = true;
        if (entry.uses_tls && security_mode != api_connection_security_mode::TLS)
            valid_entry = false;
        if (!entry.uses_tls && security_mode == api_connection_security_mode::TLS)
            valid_entry = false;
        if (entry.socket.get_error() != ER_SUCCESS)
            valid_entry = false;
        if (!valid_entry)
        {
            api_connection_pool_dispose_entry(entry);
            continue;
        }
        handle.socket = std::move(entry.socket);
        handle.tls_session = entry.tls_session;
        handle.tls_context = entry.tls_context;
        handle.has_socket = true;
        handle.from_pool = true;
        acquired = true;
    }
    api_connection_pool_cleanup_if_empty(key);
    if (acquired)
        g_api_connection_pool_reuse_hits++;
    else
        g_api_connection_pool_acquire_misses++;
    return (acquired);
}

void api_connection_pool_mark_idle(api_connection_pool_handle &handle)
{
    api_pooled_connection entry;
    long long now_ms;
    t_api_connection_pair *pair;

    if (!handle.has_socket)
        return ;
    if (!handle.should_store)
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
    pair = g_api_connection_buckets.find(handle.key);
    if (pair == ft_nullptr)
    {
        t_api_connection_bucket new_bucket;

        g_api_connection_buckets.insert(handle.key, std::move(new_bucket));
        pair = g_api_connection_buckets.find(handle.key);
        if (pair == ft_nullptr)
        {
            api_connection_pool_evict(handle);
            return ;
        }
    }
    api_connection_pool_prune_expired(pair->value, now_ms);
    while (pair->value.size() >= g_api_connection_max_idle)
        api_connection_pool_remove_oldest(pair->value);
    entry.socket = std::move(handle.socket);
    entry.tls_session = handle.tls_session;
    entry.tls_context = handle.tls_context;
    entry.uses_tls = handle.security_mode == api_connection_security_mode::TLS;
    entry.idle_timestamp_ms = now_ms;
    pair->value.push_back(std::move(entry));
    handle.tls_session = ft_nullptr;
    handle.tls_context = ft_nullptr;
    handle.has_socket = false;
    handle.from_pool = false;
    return ;
}

void api_connection_pool_evict(api_connection_pool_handle &handle)
{
    ft_socket reset_socket;

    if (!handle.has_socket)
        return ;
    if (handle.security_mode == api_connection_security_mode::TLS)
    {
        api_connection_pool_free_tls(handle.tls_session, handle.tls_context);
        handle.tls_session = ft_nullptr;
        handle.tls_context = ft_nullptr;
    }
    handle.socket.close_socket();
    handle.socket = std::move(reset_socket);
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
    size_t bucket_count;
    t_api_connection_pair *end_pair;
    t_api_connection_pair *begin_pair;
    size_t bucket_index;

    bucket_count = g_api_connection_buckets.size();
    if (bucket_count == 0)
    {
        g_api_connection_buckets.clear();
        return ;
    }
    end_pair = g_api_connection_buckets.end();
    begin_pair = end_pair - static_cast<std::ptrdiff_t>(bucket_count);
    bucket_index = 0;
    while (bucket_index < bucket_count)
    {
        t_api_connection_bucket &bucket = begin_pair[bucket_index].value;
        size_t index;

        index = 0;
        while (index < bucket.size())
        {
            api_pooled_connection &entry = bucket[index];

            api_connection_pool_dispose_entry(entry);
            index++;
        }
        bucket.clear();
        bucket_index++;
    }
    g_api_connection_buckets.clear();
    return ;
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

