#include "api_internal.hpp"
#include "../Basic/basic.hpp"
#include "../PThread/mutex.hpp"
#include "../CMA/CMA.hpp"
#include "../Logger/logger.hpp"
#include "../Networking/networking.hpp"
#include <cstddef>
#include <set>
#include <new>
#include <cstdio>
#include <errno.h>

#ifdef _WIN32
# include <winsock2.h>
# include <ws2tcpip.h>
#else
# include <sys/socket.h>
# include <sys/types.h>
#endif

struct api_pooled_connection
{
    ft_socket socket;
#if NETWORKING_HAS_OPENSSL
    SSL *tls_session;
    SSL_CTX *tls_context;
#endif
    bool uses_tls;
    bool negotiated_http2;
    long long idle_timestamp_ms;

    static void *operator new(size_t size) noexcept
    {
        void *pointer;

        pointer = cma_malloc(size);
        return (pointer);
    }

    static void *operator new(size_t size, const std::nothrow_t&) noexcept
    {
        return (api_pooled_connection::operator new(size));
    }

    static void operator delete(void *pointer) noexcept
    {
        if (pointer)
            cma_free(pointer);
        return ;
    }

    static void operator delete(void *pointer, const std::nothrow_t&) noexcept
    {
        api_pooled_connection::operator delete(pointer);
        return ;
    }
};

struct api_connection_pool_storage
{
    api_pooled_connection **entries;
    size_t entry_count;
    size_t entry_capacity;
};

static void api_connection_pool_dispose_entry(api_pooled_connection &entry);
static bool api_connection_pool_socket_is_alive(ft_socket &socket);

static api_connection_pool_storage &api_connection_pool_get_storage(void)
{
    static api_connection_pool_storage storage =
    {
        ft_nullptr,
        0,
        0
    };

    return (storage);
}

#if NETWORKING_HAS_OPENSSL
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
    if (!tls_session)
        return (true);
    if (api_connection_pool_get_tls_mutex().lock() != FT_ERR_SUCCESS)
        return (false);
    insert_result = tls_registry.insert(tls_session);
    (void)api_connection_pool_get_tls_mutex().unlock();
    if (!insert_result.second)
        return (false);
    return (true);
}

static bool api_connection_pool_tls_unregister(SSL *tls_session)
{
    std::set<SSL*> &tls_registry = api_connection_pool_get_tls_registry();
    std::set<SSL*>::iterator iterator;
    if (!tls_session)
        return (false);
    if (api_connection_pool_get_tls_mutex().lock() != FT_ERR_SUCCESS)
        return (false);
    iterator = tls_registry.find(tls_session);
    if (iterator == tls_registry.end())
    {
        (void)api_connection_pool_get_tls_mutex().unlock();
        return (false);
    }
    tls_registry.erase(iterator);
    (void)api_connection_pool_get_tls_mutex().unlock();
    return (true);
}
#endif

static bool api_connection_pool_socket_is_alive(ft_socket &socket)
{
    int poll_descriptor;
    int poll_result;
    char peek_byte;
    ssize_t peek_result;

    poll_descriptor = socket.get_file_descriptor();
    if (poll_descriptor < 0)
        return (false);
    poll_result = nw_poll(&poll_descriptor, 1, ft_nullptr, 0, 50);
    if (poll_result < 0)
        return (false);
    if (poll_result == 0)
        return (true);
    if (poll_descriptor == -1)
        return (false);
    peek_byte = 0;
#ifdef _WIN32
    peek_result = socket.receive_data(&peek_byte, 1, MSG_PEEK);
#else
    peek_result = socket.receive_data(&peek_byte, 1, MSG_PEEK | MSG_DONTWAIT);
#endif
    if (peek_result > 0)
        return (false);
    if (peek_result == 0)
        return (false);
#ifdef _WIN32
    if (WSAGetLastError() == WSAEWOULDBLOCK || WSAGetLastError() == WSAEINTR)
        return (true);
#else
    if (errno == EWOULDBLOCK || errno == EAGAIN || errno == EINTR)
        return (true);
#endif
    return (false);
}

#if NETWORKING_HAS_OPENSSL
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
#endif

static void api_connection_pool_clear_storage(api_connection_pool_storage &storage)
{
    size_t entry_index;

    entry_index = 0;
    while (entry_index < storage.entry_count)
    {
        api_pooled_connection *entry_pointer;

        entry_pointer = storage.entries[entry_index];
        if (entry_pointer)
        {
            api_connection_pool_dispose_entry(*entry_pointer);
            delete entry_pointer;
        }
        entry_index++;
    }
    if (storage.entries)
    {
        cma_free(storage.entries);
        storage.entries = ft_nullptr;
    }
    storage.entry_count = 0;
    storage.entry_capacity = 0;
    return ;
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

struct api_connection_pool_dispose_snapshot
{
    bool socket_cleanup_allowed;
    bool socket_is_open;
    int socket_error;
    size_t client_count;
};

static api_connection_pool_dispose_snapshot g_api_connection_pool_last_dispose_snapshot =
{
    false,
    false,
    FT_ERR_SUCCESS,
    0
};

static void api_connection_pool_record_dispose_snapshot(
    bool socket_cleanup_allowed,
    bool socket_is_open,
    int socket_error,
    size_t client_count)
{
    int cleanup_allowed_flag;
    int socket_open_flag;

    g_api_connection_pool_last_dispose_snapshot.socket_cleanup_allowed = socket_cleanup_allowed;
    g_api_connection_pool_last_dispose_snapshot.socket_is_open = socket_is_open;
    g_api_connection_pool_last_dispose_snapshot.socket_error = socket_error;
    g_api_connection_pool_last_dispose_snapshot.client_count = client_count;
    cleanup_allowed_flag = 0;
    if (socket_cleanup_allowed)
        cleanup_allowed_flag = 1;
    socket_open_flag = 0;
    if (socket_is_open)
        socket_open_flag = 1;
    ft_log_debug(
        "api_connection_pool_dispose_entry snapshot cleanup_allowed=%d socket_open=%d socket_error=%d client_count=%llu",
        cleanup_allowed_flag,
        socket_open_flag,
        socket_error,
        static_cast<unsigned long long>(client_count));
    return ;
}

static ft_string api_connection_pool_build_key(const char *host, uint16_t port,
        api_connection_security_mode security_mode,
        const char *security_identity)
{
    ft_string key;
    char port_buffer[16];

    if (host)
        key = host;
    key += ":";
    std::snprintf(port_buffer, sizeof(port_buffer), "%u", static_cast<unsigned int>(port));
    key += port_buffer;
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

#if NETWORKING_HAS_OPENSSL
static void api_connection_pool_free_tls(SSL *tls_session, SSL_CTX *tls_context)
{
    if (tls_session)
        SSL_free(tls_session);
    if (tls_context)
        SSL_CTX_free(tls_context);
    return ;
}
#endif

static void api_connection_pool_dispose_entry(api_pooled_connection &entry)
{
    bool socket_is_open;
    size_t client_count;
    int socket_error;

    socket_is_open = (entry.socket.get_file_descriptor() >= 0);
    client_count = 0;
    socket_error = FT_ERR_SUCCESS;
    if (socket_is_open)
        client_count = entry.socket.get_client_count();
    {
        entry.socket.disconnect_all_clients();
    }
#if NETWORKING_HAS_OPENSSL
    if (entry.uses_tls && entry.tls_session)
    {
        if (api_connection_pool_tls_unregister(entry.tls_session))
            api_connection_pool_free_tls(entry.tls_session, entry.tls_context);
    }
    entry.tls_session = ft_nullptr;
    entry.tls_context = ft_nullptr;
#endif
    entry.uses_tls = false;
    entry.negotiated_http2 = false;
    entry.idle_timestamp_ms = 0;
    api_connection_pool_record_dispose_snapshot(
        true,
        socket_is_open,
        socket_error,
        client_count);
    if (socket_is_open)
        entry.socket.close_socket();
    return ;
}

bool api_connection_pool_acquire(api_connection_pool_handle &handle,
        const char *host, uint16_t port,
        api_connection_security_mode security_mode,
        const char *security_identity)
{
    ft_string key;
    bool handle_lock_acquired;

    handle_lock_acquired = false;
    if (handle.lock(&handle_lock_acquired) != FT_ERR_SUCCESS)
    {
        g_api_connection_pool_acquire_misses++;
        return (false);
    }

    key = api_connection_pool_build_key(host, port, security_mode, security_identity);
    handle.key = key;
    handle.security_mode = security_mode;
#if NETWORKING_HAS_OPENSSL
    handle.tls_session = ft_nullptr;
    handle.tls_context = ft_nullptr;
#endif
    handle.has_socket = false;
    handle.from_pool = false;
    handle.should_store = false;
    handle.negotiated_http2 = false;
    handle.plain_socket_validated = false;
    g_api_connection_pool_acquire_calls++;
    if (!g_api_connection_pool_enabled)
    {
        handle.unlock(handle_lock_acquired);
        g_api_connection_pool_acquire_misses++;
        return (false);
    }
    handle.should_store = true;
    handle.should_store = false;
    handle.unlock(handle_lock_acquired);
    g_api_connection_pool_acquire_misses++;
    return (false);
}

void api_connection_pool_mark_idle(api_connection_pool_handle &handle)
{
    bool handle_lock_acquired;

    handle_lock_acquired = false;
    if (handle.lock(&handle_lock_acquired) != FT_ERR_SUCCESS)
        return ;
    if (!handle.has_socket)
    {
        handle.unlock(handle_lock_acquired);
        return ;
    }
    if (handle.plain_socket_timed_out)
    {
        handle.unlock(handle_lock_acquired);
        api_connection_pool_evict(handle);
        return ;
    }
    if (!handle.should_store)
    {
        handle.unlock(handle_lock_acquired);
        api_connection_pool_evict(handle);
        return ;
    }
    if (!g_api_connection_pool_enabled)
    {
        handle.unlock(handle_lock_acquired);
        api_connection_pool_evict(handle);
        return ;
    }
    if (handle.security_mode == api_connection_security_mode::TLS)
    {
        handle.unlock(handle_lock_acquired);
        api_connection_pool_evict(handle);
        return ;
    }
    if (g_api_connection_max_idle == 0)
    {
        handle.unlock(handle_lock_acquired);
        api_connection_pool_evict(handle);
        return ;
    }
    if (!api_connection_pool_socket_is_alive(handle.socket))
    {
        handle.unlock(handle_lock_acquired);
        api_connection_pool_evict(handle);
        return ;
    }
    handle.unlock(handle_lock_acquired);
    api_connection_pool_evict(handle);
    handle_lock_acquired = false;
    if (handle.lock(&handle_lock_acquired) != FT_ERR_SUCCESS)
        return ;
#if NETWORKING_HAS_OPENSSL
    handle.tls_session = ft_nullptr;
    handle.tls_context = ft_nullptr;
#endif
    handle.has_socket = false;
    handle.from_pool = false;
    handle.should_store = false;
    handle.negotiated_http2 = false;
    handle.plain_socket_timed_out = false;
    handle.plain_socket_validated = false;
    handle.unlock(handle_lock_acquired);
    return ;
}

void api_connection_pool_evict(api_connection_pool_handle &handle)
{
    bool handle_lock_acquired;

    handle_lock_acquired = false;
    if (handle.lock(&handle_lock_acquired) != FT_ERR_SUCCESS)
        return ;
    if (!handle.has_socket)
    {
        handle.unlock(handle_lock_acquired);
        return ;
    }
    if (handle.security_mode == api_connection_security_mode::TLS)
    {
#if NETWORKING_HAS_OPENSSL
        bool tls_removed;

        tls_removed = api_connection_pool_untrack_tls_session(handle.tls_session);
        if (tls_removed)
            api_connection_pool_free_tls(handle.tls_session, handle.tls_context);
        handle.tls_session = ft_nullptr;
        handle.tls_context = ft_nullptr;
#endif
    }
    handle.socket.disconnect_all_clients();
    handle.socket.close_socket();
    handle.has_socket = false;
    handle.from_pool = false;
    handle.should_store = false;
    handle.negotiated_http2 = false;
    handle.plain_socket_timed_out = false;
    handle.plain_socket_validated = false;
    handle.unlock(handle_lock_acquired);
    return ;
}

void api_connection_pool_disable_store(api_connection_pool_handle &handle)
{
    bool handle_lock_acquired;

    handle_lock_acquired = false;
    if (handle.lock(&handle_lock_acquired) != FT_ERR_SUCCESS)
        return ;
    handle.should_store = false;
    handle.negotiated_http2 = false;
    handle.unlock(handle_lock_acquired);
    return ;
}

void api_connection_pool_flush(void)
{
    api_connection_pool_storage &storage = api_connection_pool_get_storage();
    int lock_error;

    lock_error = api_connection_pool_get_mutex().lock();
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    api_connection_pool_clear_storage(storage);
    (void)api_connection_pool_get_mutex().unlock();
    return ;
}

void api_connection_pool_set_enabled(bool enabled)
{
    api_connection_pool_storage &storage = api_connection_pool_get_storage();
    int lock_error;

    lock_error = api_connection_pool_get_mutex().lock();
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    g_api_connection_pool_enabled = enabled;
    if (!enabled)
        api_connection_pool_clear_storage(storage);
    (void)api_connection_pool_get_mutex().unlock();
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
