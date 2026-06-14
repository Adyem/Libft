#include "api_internal.hpp"
#include "../Basic/basic.hpp"
#include "../PThread/mutex.hpp"
#include "../CMA/CMA.hpp"
#include "../Logger/logger.hpp"
#include "../Networking/networking.hpp"
#include "../System_utils/system_utils.hpp"
#include <cstddef>
#include <set>
#include <new>
#include <cstdio>
#include <errno.h>

#include "../Basic/limits.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../Errno/errno.hpp"
#ifdef _WIN32
# include <winsock2.h>
# include <ws2tcpip.h>
#else
# include <sys/socket.h>
# include <sys/types.h>
#endif

struct api_pooled_connection
{
    ft_string key;
    ft_socket socket;
#if NETWORKING_HAS_OPENSSL
    SSL *tls_session;
    SSL_CTX *tls_context;
#endif
    ft_bool uses_tls;
    ft_bool negotiated_http2;
    int64_t idle_timestamp_ms;

    static void *operator new(std::size_t size) noexcept
    {
        void *pointer;

        pointer = cma_malloc(size);
        return (pointer);
    }

    static void *operator new(std::size_t size, const std::nothrow_t&) noexcept
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
    ft_size_t entry_count;
    ft_size_t entry_capacity;
};

static void api_connection_pool_dispose_entry(api_pooled_connection &entry);
static ft_bool api_connection_pool_socket_is_alive(ft_socket &socket);

static void api_connection_pool_remove_entry_at(
    api_connection_pool_storage &storage,
    ft_size_t entry_index)
{
    ft_size_t shift_index;

    if (entry_index >= storage.entry_count)
        return ;
    delete storage.entries[entry_index];
    shift_index = entry_index;
    while (shift_index + 1 < storage.entry_count)
    {
        storage.entries[shift_index] = storage.entries[shift_index + 1];
        shift_index += 1;
    }
    storage.entry_count -= 1;
    storage.entries[storage.entry_count] = ft_nullptr;
    return ;
}

static ft_bool api_connection_pool_ensure_capacity(
    api_connection_pool_storage &storage,
    ft_size_t required_capacity)
{
    api_pooled_connection **new_entries;
    ft_size_t new_capacity;
    ft_size_t entry_index;

    if (storage.entry_capacity >= required_capacity)
        return (FT_TRUE);
    if (storage.entry_capacity == 0)
        new_capacity = 4;
    else
        new_capacity = storage.entry_capacity * 2;
    while (new_capacity < required_capacity)
        new_capacity = new_capacity * 2;
    new_entries = static_cast<api_pooled_connection **>(
        cma_malloc(sizeof(api_pooled_connection *) * new_capacity));
    if (new_entries == ft_nullptr)
        return (FT_FALSE);
    entry_index = 0;
    while (entry_index < new_capacity)
    {
        new_entries[entry_index] = ft_nullptr;
        entry_index += 1;
    }
    entry_index = 0;
    while (entry_index < storage.entry_count)
    {
        new_entries[entry_index] = storage.entries[entry_index];
        entry_index += 1;
    }
    if (storage.entries != ft_nullptr)
        cma_free(storage.entries);
    storage.entries = new_entries;
    storage.entry_capacity = new_capacity;
#ifdef LIBFT_TEST_BUILD
    (void)cma_untrack_leak(storage.entries);
#endif
    return (FT_TRUE);
}

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
    static pt_mutex *tls_mutex = ft_nullptr;
    int32_t initialize_error;

    if (tls_mutex == ft_nullptr)
    {
        tls_mutex = new (std::nothrow) pt_mutex();
        if (tls_mutex == ft_nullptr)
            su_abort();
        initialize_error = tls_mutex->initialize();
        if (initialize_error != FT_ERR_SUCCESS)
            su_abort();
#ifdef LIBFT_TEST_BUILD
        (void)cma_untrack_leak(tls_mutex);
        if (tls_mutex->_native_mutex != ft_nullptr)
            (void)cma_untrack_leak(tls_mutex->_native_mutex);
#endif
    }

    return (*tls_mutex);
}

static ft_bool api_connection_pool_tls_register(SSL *tls_session)
{
    std::set<SSL*> &tls_registry = api_connection_pool_get_tls_registry();
    std::pair<std::set<SSL*>::iterator, ft_bool> insert_result;
    if (!tls_session)
        return (FT_TRUE);
    if (api_connection_pool_get_tls_mutex().lock() != FT_ERR_SUCCESS)
        return (FT_FALSE);
    insert_result = tls_registry.insert(tls_session);
    (void)api_connection_pool_get_tls_mutex().unlock();
    if (!insert_result.second)
        return (FT_FALSE);
    return (FT_TRUE);
}

static ft_bool api_connection_pool_tls_unregister(SSL *tls_session)
{
    std::set<SSL*> &tls_registry = api_connection_pool_get_tls_registry();
    std::set<SSL*>::iterator iterator;
    if (!tls_session)
        return (FT_FALSE);
    if (api_connection_pool_get_tls_mutex().lock() != FT_ERR_SUCCESS)
        return (FT_FALSE);
    iterator = tls_registry.find(tls_session);
    if (iterator == tls_registry.end())
    {
        (void)api_connection_pool_get_tls_mutex().unlock();
        return (FT_FALSE);
    }
    tls_registry.erase(iterator);
    (void)api_connection_pool_get_tls_mutex().unlock();
    return (FT_TRUE);
}
#endif

static ft_bool api_connection_pool_socket_is_alive(ft_socket &socket)
{
    int32_t poll_descriptor;
    int32_t poll_result;
    char peek_byte;
    ssize_t peek_result;

    poll_descriptor = socket.get_file_descriptor();
    if (poll_descriptor < 0)
        return (FT_FALSE);
    poll_result = nw_poll(&poll_descriptor, 1, ft_nullptr, 0, 50);
    if (poll_result < 0)
        return (FT_FALSE);
    if (poll_result == 0)
        return (FT_TRUE);
    if (poll_descriptor == -1)
        return (FT_FALSE);
    peek_byte = 0;
#ifdef _WIN32
    peek_result = socket.receive_data(&peek_byte, 1, MSG_PEEK);
    if (peek_result < 0)
    {
        int32_t last_error;

        last_error = WSAGetLastError();
        if (last_error == WSAEWOULDBLOCK || last_error == WSAEINTR)
            return (FT_TRUE);
        return (FT_FALSE);
    }
#else
    peek_result = socket.receive_data(&peek_byte, 1, MSG_PEEK | MSG_DONTWAIT);
    if (peek_result > 0)
        return (FT_FALSE);
    if (peek_result == 0)
        return (FT_FALSE);
    if (errno == EWOULDBLOCK || errno == EAGAIN || errno == EINTR)
        return (FT_TRUE);
#endif
    return (FT_FALSE);
}

#if NETWORKING_HAS_OPENSSL
ft_bool api_connection_pool_track_tls_session(SSL *tls_session)
{
    ft_bool registered;

    registered = api_connection_pool_tls_register(tls_session);
    if (!registered)
    {
        api_connection_pool_tls_unregister(tls_session);
        registered = api_connection_pool_tls_register(tls_session);
    }
    return (registered);
}

ft_bool api_connection_pool_untrack_tls_session(SSL *tls_session)
{
    ft_bool removed;

    removed = api_connection_pool_tls_unregister(tls_session);
    return (removed);
}
#endif

static void api_connection_pool_clear_storage(api_connection_pool_storage &storage)
{
    ft_size_t entry_index;

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
    static pt_mutex *connection_pool_mutex = ft_nullptr;
    int32_t initialize_error;

    if (connection_pool_mutex == ft_nullptr)
    {
        connection_pool_mutex = new (std::nothrow) pt_mutex();
        if (connection_pool_mutex == ft_nullptr)
            su_abort();
        initialize_error = connection_pool_mutex->initialize();
        if (initialize_error != FT_ERR_SUCCESS)
            su_abort();
#ifdef LIBFT_TEST_BUILD
        (void)cma_untrack_leak(connection_pool_mutex);
        if (connection_pool_mutex->_native_mutex != ft_nullptr)
            (void)cma_untrack_leak(connection_pool_mutex->_native_mutex);
#endif
    }

    return (*connection_pool_mutex);
}
static ft_size_t g_api_connection_max_idle = 8;
static int64_t g_api_connection_idle_timeout_ms = 60000;
static ft_size_t g_api_connection_pool_acquire_calls = 0;
static ft_size_t g_api_connection_pool_reuse_hits = 0;
static ft_size_t g_api_connection_pool_acquire_misses = 0;
static ft_bool g_api_connection_pool_enabled = FT_TRUE;

struct api_connection_pool_dispose_snapshot
{
    ft_bool socket_cleanup_allowed;
    ft_bool socket_is_open;
    int32_t socket_error;
    ft_size_t client_count;
};

static api_connection_pool_dispose_snapshot g_api_connection_pool_last_dispose_snapshot =
{
    FT_FALSE,
    FT_FALSE,
    FT_ERR_SUCCESS,
    0
};

static void api_connection_pool_record_dispose_snapshot(
    ft_bool socket_cleanup_allowed,
    ft_bool socket_is_open,
    int32_t socket_error,
    ft_size_t client_count)
{
    int32_t cleanup_allowed_flag;
    int32_t socket_open_flag;

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
        cleanup_allowed_flag, socket_open_flag, socket_error, client_count);
    return ;
}

static int32_t api_connection_pool_build_key(const char *host, uint16_t port,
        api_connection_security_mode security_mode,
        const char *security_identity, ft_string &key)
{
    char port_buffer[16];
    int32_t key_initialization_error;

    if (key.is_initialised() == FT_FALSE)
        key_initialization_error = key.initialize();
    else
        key_initialization_error = key.clear();
    if (key_initialization_error != FT_ERR_SUCCESS)
        return (key_initialization_error);

    if (host)
        key = host;
    key += ":";
    std::snprintf(port_buffer, sizeof(port_buffer), "%u", static_cast<uint32_t>(port));
    key += port_buffer;
    key += ":";
    if (security_mode == api_connection_security_mode::TLS)
        key += "tls";
    else
        key += "plain";
    key += ":";
    if (security_identity && security_identity[0] != '\0')
        key += security_identity;
    return (key.get_error());
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
    ft_bool socket_is_open;
    ft_size_t client_count;
    int32_t socket_error;

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
    entry.uses_tls = FT_FALSE;
    entry.negotiated_http2 = FT_FALSE;
    entry.idle_timestamp_ms = 0;
    api_connection_pool_record_dispose_snapshot(
        FT_TRUE,
        socket_is_open,
        socket_error,
        client_count);
    if (socket_is_open)
        entry.socket.close_socket();
    return ;
}

ft_bool api_connection_pool_acquire(api_connection_pool_handle &handle,
        const char *host, uint16_t port,
        api_connection_security_mode security_mode,
        const char *security_identity)
{
    api_connection_pool_storage &storage = api_connection_pool_get_storage();
    ft_bool handle_lock_acquired;
    int32_t lock_error;
    ft_size_t entry_index;

    handle_lock_acquired = FT_FALSE;
    if (handle.lock(&handle_lock_acquired) != FT_ERR_SUCCESS)
    {
        g_api_connection_pool_acquire_misses++;
        return (FT_FALSE);
    }

    if (api_connection_pool_build_key(host, port, security_mode,
            security_identity, handle.key) != FT_ERR_SUCCESS)
    {
        handle.unlock(handle_lock_acquired);
        g_api_connection_pool_acquire_misses++;
        return (FT_FALSE);
    }
    handle.security_mode = security_mode;
#if NETWORKING_HAS_OPENSSL
    handle.tls_session = ft_nullptr;
    handle.tls_context = ft_nullptr;
#endif
    handle.has_socket = FT_FALSE;
    handle.from_pool = FT_FALSE;
    handle.should_store = FT_FALSE;
    handle.negotiated_http2 = FT_FALSE;
    handle.plain_socket_timed_out = FT_FALSE;
    handle.plain_socket_validated = FT_FALSE;
    g_api_connection_pool_acquire_calls++;
    if (!g_api_connection_pool_enabled)
    {
        handle.unlock(handle_lock_acquired);
        g_api_connection_pool_acquire_misses++;
        return (FT_FALSE);
    }
    handle.should_store = FT_TRUE;
    lock_error = api_connection_pool_get_mutex().lock();
    if (lock_error != FT_ERR_SUCCESS)
    {
        handle.unlock(handle_lock_acquired);
        g_api_connection_pool_acquire_misses++;
        return (FT_FALSE);
    }
    entry_index = 0;
    while (entry_index < storage.entry_count)
    {
        api_pooled_connection *entry;
        ft_bool entry_matches;

        entry = storage.entries[entry_index];
        entry_matches = FT_FALSE;
        if (entry != ft_nullptr && entry->key.c_str() != ft_nullptr
            && handle.key.c_str() != ft_nullptr
            && ft_strcmp(entry->key.c_str(), handle.key.c_str()) == 0)
            entry_matches = FT_TRUE;
        if (entry_matches == FT_FALSE)
        {
            entry_index += 1;
            continue ;
        }
        if ((g_api_connection_idle_timeout_ms > 0)
            && (time_monotonic() - entry->idle_timestamp_ms
                > g_api_connection_idle_timeout_ms))
        {
            api_connection_pool_remove_entry_at(storage, entry_index);
            continue ;
        }
        if (!api_connection_pool_socket_is_alive(entry->socket))
        {
            api_connection_pool_remove_entry_at(storage, entry_index);
            continue ;
        }
        if (handle.socket.move(entry->socket) != FT_ERR_SUCCESS)
        {
            api_connection_pool_remove_entry_at(storage, entry_index);
            (void)api_connection_pool_get_mutex().unlock();
            handle.unlock(handle_lock_acquired);
            g_api_connection_pool_acquire_misses++;
            return (FT_FALSE);
        }
#if NETWORKING_HAS_OPENSSL
        handle.tls_session = entry->tls_session;
        handle.tls_context = entry->tls_context;
        entry->tls_session = ft_nullptr;
        entry->tls_context = ft_nullptr;
#endif
        handle.has_socket = FT_TRUE;
        handle.from_pool = FT_TRUE;
        handle.should_store = FT_TRUE;
        handle.negotiated_http2 = entry->negotiated_http2;
        handle.plain_socket_timed_out = FT_FALSE;
        handle.plain_socket_validated = FT_FALSE;
        api_connection_pool_remove_entry_at(storage, entry_index);
        (void)api_connection_pool_get_mutex().unlock();
        handle.unlock(handle_lock_acquired);
        g_api_connection_pool_reuse_hits++;
        return (FT_TRUE);
    }
    (void)api_connection_pool_get_mutex().unlock();
    handle.unlock(handle_lock_acquired);
    g_api_connection_pool_acquire_misses++;
    return (FT_FALSE);
}

void api_connection_pool_mark_idle(api_connection_pool_handle &handle)
{
    api_connection_pool_storage &storage = api_connection_pool_get_storage();
    api_pooled_connection *entry;
    ft_bool handle_lock_acquired;
    int32_t lock_error;

    handle_lock_acquired = FT_FALSE;
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
    entry = new (std::nothrow) api_pooled_connection();
    if (entry == ft_nullptr)
    {
        handle.unlock(handle_lock_acquired);
        api_connection_pool_evict(handle);
        return ;
    }
    if (entry->key.initialize(handle.key) != FT_ERR_SUCCESS)
    {
        delete entry;
        handle.unlock(handle_lock_acquired);
        api_connection_pool_evict(handle);
        return ;
    }
    if (entry->socket.move(handle.socket) != FT_ERR_SUCCESS)
    {
        delete entry;
        handle.unlock(handle_lock_acquired);
        api_connection_pool_evict(handle);
        return ;
    }
#if NETWORKING_HAS_OPENSSL
    entry->tls_session = handle.tls_session;
    entry->tls_context = handle.tls_context;
    handle.tls_session = ft_nullptr;
    handle.tls_context = ft_nullptr;
#endif
    entry->uses_tls = (handle.security_mode == api_connection_security_mode::TLS);
    entry->negotiated_http2 = handle.negotiated_http2;
    entry->idle_timestamp_ms = time_monotonic();
    lock_error = api_connection_pool_get_mutex().lock();
    if (lock_error != FT_ERR_SUCCESS)
    {
        handle.unlock(handle_lock_acquired);
        api_connection_pool_dispose_entry(*entry);
        delete entry;
        api_connection_pool_evict(handle);
        return ;
    }
    if (g_api_connection_max_idle > 0
        && storage.entry_count >= g_api_connection_max_idle)
        api_connection_pool_remove_entry_at(storage, 0);
    if (!api_connection_pool_ensure_capacity(storage, storage.entry_count + 1))
    {
        (void)api_connection_pool_get_mutex().unlock();
        handle.unlock(handle_lock_acquired);
        api_connection_pool_dispose_entry(*entry);
        delete entry;
        api_connection_pool_evict(handle);
        return ;
    }
    storage.entries[storage.entry_count] = entry;
    storage.entry_count += 1;
    (void)api_connection_pool_get_mutex().unlock();
#if NETWORKING_HAS_OPENSSL
#endif
    handle.has_socket = FT_FALSE;
    handle.from_pool = FT_FALSE;
    handle.should_store = FT_FALSE;
    handle.negotiated_http2 = FT_FALSE;
    handle.plain_socket_timed_out = FT_FALSE;
    handle.plain_socket_validated = FT_FALSE;
    handle.unlock(handle_lock_acquired);
    return ;
}

void api_connection_pool_evict(api_connection_pool_handle &handle)
{
    ft_bool handle_lock_acquired;

    handle_lock_acquired = FT_FALSE;
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
        ft_bool tls_removed;

        tls_removed = api_connection_pool_untrack_tls_session(handle.tls_session);
        if (tls_removed)
            api_connection_pool_free_tls(handle.tls_session, handle.tls_context);
        handle.tls_session = ft_nullptr;
        handle.tls_context = ft_nullptr;
#endif
    }
    handle.socket.disconnect_all_clients();
    handle.socket.close_socket();
    handle.has_socket = FT_FALSE;
    handle.from_pool = FT_FALSE;
    handle.should_store = FT_FALSE;
    handle.negotiated_http2 = FT_FALSE;
    handle.plain_socket_timed_out = FT_FALSE;
    handle.plain_socket_validated = FT_FALSE;
    handle.unlock(handle_lock_acquired);
    return ;
}

void api_connection_pool_disable_store(api_connection_pool_handle &handle)
{
    ft_bool handle_lock_acquired;

    handle_lock_acquired = FT_FALSE;
    if (handle.lock(&handle_lock_acquired) != FT_ERR_SUCCESS)
        return ;
    handle.should_store = FT_FALSE;
    handle.negotiated_http2 = FT_FALSE;
    handle.unlock(handle_lock_acquired);
    return ;
}

void api_connection_pool_flush(void)
{
    api_connection_pool_storage &storage = api_connection_pool_get_storage();
    int32_t lock_error;

    lock_error = api_connection_pool_get_mutex().lock();
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    api_connection_pool_clear_storage(storage);
    (void)api_connection_pool_get_mutex().unlock();
    return ;
}

void api_connection_pool_set_enabled(ft_bool enabled)
{
    api_connection_pool_storage &storage = api_connection_pool_get_storage();
    int32_t lock_error;

    lock_error = api_connection_pool_get_mutex().lock();
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    g_api_connection_pool_enabled = enabled;
    if (!enabled)
        api_connection_pool_clear_storage(storage);
    (void)api_connection_pool_get_mutex().unlock();
    return ;
}

ft_bool api_connection_pool_is_enabled(void)
{
    return (g_api_connection_pool_enabled);
}

void api_connection_pool_set_max_idle(ft_size_t max_idle)
{
    if (max_idle == 0)
        g_api_connection_max_idle = 0;
    else
        g_api_connection_max_idle = max_idle;
    return ;
}

ft_size_t api_connection_pool_get_max_idle(void)
{
    return (g_api_connection_max_idle);
}

void api_connection_pool_set_idle_timeout(int64_t idle_timeout_ms)
{
    g_api_connection_idle_timeout_ms = idle_timeout_ms;
    return ;
}

int64_t api_connection_pool_get_idle_timeout(void)
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

ft_size_t api_debug_get_connection_pool_acquires(void)
{
    return (g_api_connection_pool_acquire_calls);
}

ft_size_t api_debug_get_connection_pool_reuses(void)
{
    return (g_api_connection_pool_reuse_hits);
}

ft_size_t api_debug_get_connection_pool_misses(void)
{
    return (g_api_connection_pool_acquire_misses);
}
