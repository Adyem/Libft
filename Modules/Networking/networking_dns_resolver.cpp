#include "networking.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../CPP_class/class_string.hpp"
#include "../Template/map.hpp"
#include "../Template/vector.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../Threading/unique_lock.hpp"
#include "../Basic/basic.hpp"
#include "../CMA/CMA.hpp"
#include "../Time/time.hpp"
#include <climits>
#include <cstdio>
#include <cstring>
#include <utility>
#include <errno.h>

#include "../Basic/limits.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../Template/pair.hpp"
#ifdef _WIN32
# include <winsock2.h>
# include <ws2tcpip.h>
#else
# include <netdb.h>
# include <sys/socket.h>
# include <arpa/inet.h>
#endif

struct networking_dns_cache_entry
{
    ft_vector<networking_resolved_address> addresses;
    ft_bool                                   addresses_initialised;
    int64_t                                   expiration_ms;

    networking_dns_cache_entry() noexcept
        : addresses(), addresses_initialised(FT_FALSE), expiration_ms(0)
    {
        if (this->addresses.initialize() == FT_ERR_SUCCESS)
            this->addresses_initialised = FT_TRUE;
        return ;
    }

    ~networking_dns_cache_entry() noexcept
    {
        if (this->addresses_initialised == FT_TRUE)
        {
            (void)this->addresses.destroy();
            this->addresses_initialised = FT_FALSE;
        }
        return ;
    }

    networking_dns_cache_entry(const networking_dns_cache_entry &other) noexcept
        : addresses(), addresses_initialised(FT_FALSE), expiration_ms(other.expiration_ms)
    {
        ft_size_t index;
        ft_size_t count;

        if (this->addresses.initialize() != FT_ERR_SUCCESS)
            return ;
        this->addresses_initialised = FT_TRUE;
        if (other.addresses_initialised == FT_FALSE)
            return ;
        index = 0;
        count = other.addresses.size();
        while (index < count)
        {
            this->addresses.push_back(other.addresses[index]);
            index++;
        }
        return ;
    }

    networking_dns_cache_entry &operator=(const networking_dns_cache_entry &other) noexcept
    {
        if (this != &other)
        {
            ft_size_t index;
            ft_size_t count;

            if (this->addresses_initialised == FT_FALSE)
            {
                if (this->addresses.initialize() != FT_ERR_SUCCESS)
                    return (*this);
                this->addresses_initialised = FT_TRUE;
            }
            this->addresses.clear();
            if (other.addresses_initialised == FT_FALSE)
            {
                this->expiration_ms = other.expiration_ms;
                return (*this);
            }
            index = 0;
            count = other.addresses.size();
            while (index < count)
            {
                this->addresses.push_back(other.addresses[index]);
                index++;
            }
            this->expiration_ms = other.expiration_ms;
        }
        return (*this);
    }
};

static ft_map<ft_string, networking_dns_cache_entry>   g_networking_dns_cache;
static pt_mutex                                        *g_networking_dns_cache_mutex = ft_nullptr;
static const int64_t                                      g_networking_dns_cache_ttl_ms = 60000;

#ifdef LIBFT_TEST_BUILD
static void networking_dns_untrack_runtime_leaks(void) noexcept
{
    if (g_networking_dns_cache.is_initialised() == FT_CLASS_STATE_INITIALISED
        && g_networking_dns_cache._data != ft_nullptr)
        (void)cma_untrack_leak(g_networking_dns_cache._data);
    if (g_networking_dns_cache_mutex != ft_nullptr)
        (void)cma_untrack_leak(g_networking_dns_cache_mutex);
    return ;
}
#endif

static void networking_push_failure(int32_t error_code) noexcept
{
    (void)(error_code);
}

static int32_t networking_dns_cache_lock(ft_bool *lock_acquired) noexcept
{
    uint32_t    lock_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = FT_FALSE;
    lock_error = pt_mutex_lock_if_not_null(g_networking_dns_cache_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (FT_ERR_INTERNAL);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = FT_TRUE;
    return (FT_ERR_SUCCESS);
}

static int32_t networking_dns_cache_unlock(ft_bool lock_acquired) noexcept
{
    if (lock_acquired == FT_FALSE)
        return (FT_ERR_SUCCESS);
    (void)pt_mutex_unlock_if_not_null(g_networking_dns_cache_mutex);
    return (FT_ERR_SUCCESS);
}

static int32_t networking_dns_cache_ensure_initialised(void) noexcept
{
    int32_t initialise_error;

    if (g_networking_dns_cache.is_initialised() == FT_CLASS_STATE_INITIALISED)
        return (FT_ERR_SUCCESS);
    initialise_error = g_networking_dns_cache.initialize();
    if (initialise_error != FT_ERR_SUCCESS)
        return (initialise_error);
#ifdef LIBFT_TEST_BUILD
    networking_dns_untrack_runtime_leaks();
#endif
    return (FT_ERR_SUCCESS);
}

int32_t networking_dns_enable_thread_safety(void) noexcept
{
    pt_mutex    *new_mutex;
    int32_t     initialise_error;

    if (g_networking_dns_cache_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    new_mutex = new (std::nothrow) pt_mutex();
    if (new_mutex == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    initialise_error = new_mutex->initialize();
    if (initialise_error != FT_ERR_SUCCESS)
    {
        delete new_mutex;
        return (initialise_error);
    }
    g_networking_dns_cache_mutex = new_mutex;
#ifdef LIBFT_TEST_BUILD
    networking_dns_untrack_runtime_leaks();
#endif
    return (FT_ERR_SUCCESS);
}

int32_t networking_dns_disable_thread_safety(void) noexcept
{
    pt_mutex    *mutex_pointer;
    int32_t     first_error;
    int32_t     destroy_error;

    mutex_pointer = g_networking_dns_cache_mutex;
    g_networking_dns_cache_mutex = ft_nullptr;
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_SUCCESS);
    first_error = FT_ERR_SUCCESS;
    destroy_error = mutex_pointer->destroy();
    if (destroy_error != FT_ERR_SUCCESS)
        first_error = destroy_error;
    delete mutex_pointer;
    return (first_error);
}

ft_bool networking_dns_is_thread_safe(void) noexcept
{
    if (g_networking_dns_cache_mutex != ft_nullptr)
        return (FT_TRUE);
    return (FT_FALSE);
}

static ft_bool networking_dns_append_literal(ft_string &target, const char *literal) noexcept
{
    const char  *value;

    value = literal;
    if (value == ft_nullptr)
        value = "";
    target.append(value);
    return (FT_TRUE);
}

static ft_bool networking_dns_append_separator(ft_string &target) noexcept
{
    target.append('|');
    return (FT_TRUE);
}

static ft_bool networking_dns_copy_addresses(const ft_vector<networking_resolved_address> &source,
    ft_vector<networking_resolved_address> &destination) noexcept
{
    ft_size_t  index;
    ft_size_t  count;

    destination.clear();
    count = source.size();
    index = 0;
    while (index < count)
    {
        destination.push_back(source[index]);
        index++;
    }
    return (FT_TRUE);
}

static ft_bool networking_dns_append_number(ft_string &target, int32_t value) noexcept
{
    char number_buffer[32];

    snprintf(number_buffer, sizeof(number_buffer), "%d", value);
    target.append(number_buffer);
    return (FT_TRUE);
}

void networking_dns_set_error(int32_t resolver_status) noexcept
{
#ifdef EAI_BADFLAGS
    if (resolver_status == EAI_BADFLAGS)
    {
        networking_push_failure(FT_ERR_SOCKET_RESOLVE_BAD_FLAGS);
        return ;
    }
#endif
#ifdef EAI_AGAIN
    if (resolver_status == EAI_AGAIN)
    {
        networking_push_failure(FT_ERR_SOCKET_RESOLVE_AGAIN);
        return ;
    }
#endif
#ifdef EAI_FAIL
    if (resolver_status == EAI_FAIL)
    {
        networking_push_failure(FT_ERR_SOCKET_RESOLVE_FAIL);
        return ;
    }
#endif
#ifdef EAI_FAMILY
    if (resolver_status == EAI_FAMILY)
    {
        networking_push_failure(FT_ERR_SOCKET_RESOLVE_FAMILY);
        return ;
    }
#endif
#ifdef EAI_ADDRFAMILY
    if (resolver_status == EAI_ADDRFAMILY)
    {
        networking_push_failure(FT_ERR_SOCKET_RESOLVE_FAMILY);
        return ;
    }
#endif
#ifdef EAI_SOCKTYPE
    if (resolver_status == EAI_SOCKTYPE)
    {
        networking_push_failure(FT_ERR_SOCKET_RESOLVE_SOCKTYPE);
        return ;
    }
#endif
#ifdef EAI_SERVICE
    if (resolver_status == EAI_SERVICE)
    {
        networking_push_failure(FT_ERR_SOCKET_RESOLVE_SERVICE);
        return ;
    }
#endif
#ifdef EAI_MEMORY
    if (resolver_status == EAI_MEMORY)
    {
        networking_push_failure(FT_ERR_SOCKET_RESOLVE_MEMORY);
        return ;
    }
#endif
#ifdef EAI_NONAME
    if (resolver_status == EAI_NONAME)
    {
        networking_push_failure(FT_ERR_SOCKET_RESOLVE_NO_NAME);
        return ;
    }
#endif
#ifdef EAI_NODATA
    if (resolver_status == EAI_NODATA)
    {
        networking_push_failure(FT_ERR_SOCKET_RESOLVE_NO_NAME);
        return ;
    }
#endif
#ifdef EAI_OVERFLOW
    if (resolver_status == EAI_OVERFLOW)
    {
        networking_push_failure(FT_ERR_SOCKET_RESOLVE_OVERFLOW);
        return ;
    }
#endif
#ifdef EAI_SYSTEM
    if (resolver_status == EAI_SYSTEM)
    {
#ifdef _WIN32
        networking_push_failure(FT_ERR_SOCKET_RESOLVE_FAIL);
#else
        networking_push_failure(FT_ERR_SOCKET_RESOLVE_FAIL);
#endif
        return ;
    }
#endif
    networking_push_failure(FT_ERR_SOCKET_RESOLVE_FAILED);
    return ;
}

ft_bool networking_dns_resolve(const char *host, const char *service,
    int32_t family, int32_t socktype, int32_t protocol, int32_t flags,
    ft_vector<networking_resolved_address> &out_addresses) noexcept
{
    ft_string   cache_key;
    const char  *service_string;
    const char  *service_parameter;
    int64_t        lookup_start_ms;
    ft_bool        lookup_start_valid;
    ft_bool        cache_lock_acquired;
    Pair<ft_string, networking_dns_cache_entry> *cache_entry;
    ft_bool        entry_valid;
    int32_t         cache_lock_error;

    out_addresses.clear();
    if (host == ft_nullptr || host[0] == '\0')
    {
        networking_push_failure(FT_ERR_INVALID_ARGUMENT);
        return (FT_FALSE);
    }
    cache_key = host;
    if (!networking_dns_append_separator(cache_key))
        return (FT_FALSE);
    service_string = service;
    if (service_string == ft_nullptr)
        service_string = "";
    if (!networking_dns_append_literal(cache_key, service_string))
        return (FT_FALSE);
    if (!networking_dns_append_separator(cache_key))
        return (FT_FALSE);
    if (!networking_dns_append_number(cache_key, family))
        return (FT_FALSE);
    if (!networking_dns_append_separator(cache_key))
        return (FT_FALSE);
    if (!networking_dns_append_number(cache_key, socktype))
        return (FT_FALSE);
    if (!networking_dns_append_separator(cache_key))
        return (FT_FALSE);
    if (!networking_dns_append_number(cache_key, protocol))
        return (FT_FALSE);
    if (!networking_dns_append_separator(cache_key))
        return (FT_FALSE);
    if (!networking_dns_append_number(cache_key, flags))
        return (FT_FALSE);
    lookup_start_ms = time_now_ms();
    lookup_start_valid = FT_TRUE;
    cache_lock_error = networking_dns_cache_ensure_initialised();
    if (cache_lock_error != FT_ERR_SUCCESS)
    {
        networking_push_failure(cache_lock_error);
        return (FT_FALSE);
    }
    cache_lock_acquired = FT_FALSE;
    cache_lock_error = networking_dns_cache_lock(&cache_lock_acquired);
    if (cache_lock_error != FT_ERR_SUCCESS)
    {
        networking_push_failure(cache_lock_error);
        return (FT_FALSE);
    }
    cache_entry = g_networking_dns_cache.find(cache_key);
    if (cache_entry != g_networking_dns_cache.end())
    {
        entry_valid = FT_TRUE;
        if (lookup_start_valid)
        {
            if (cache_entry->value.expiration_ms < lookup_start_ms)
                entry_valid = FT_FALSE;
        }
        if (entry_valid)
        {
            if (!networking_dns_copy_addresses(cache_entry->value.addresses, out_addresses))
            {
                int32_t cache_unlock_error = networking_dns_cache_unlock(cache_lock_acquired);

                if (cache_unlock_error != FT_ERR_SUCCESS)
                {
                    networking_push_failure(cache_unlock_error);
                }
                return (FT_FALSE);
            }
            {
                int32_t cache_unlock_error = networking_dns_cache_unlock(cache_lock_acquired);

                if (cache_unlock_error != FT_ERR_SUCCESS)
                {
                    networking_push_failure(cache_unlock_error);
                    return (FT_FALSE);
                }
            }
            (void)(FT_ERR_SUCCESS);
            return (FT_TRUE);
        }
        g_networking_dns_cache.remove(cache_key);
    }
        {
            int32_t cache_unlock_error = networking_dns_cache_unlock(cache_lock_acquired);

        if (cache_unlock_error != FT_ERR_SUCCESS)
        {
            networking_push_failure(cache_unlock_error);
            return (FT_FALSE);
        }
    }
    service_parameter = ft_nullptr;
    if (service_string[0] != '\0')
        service_parameter = service_string;
    struct addrinfo hints;
    struct addrinfo *results;
    struct addrinfo *current;
    int32_t resolver_status;

    ft_memset(&hints, 0, sizeof(hints));
    hints.ai_family = family;
    hints.ai_socktype = socktype;
    hints.ai_protocol = protocol;
    hints.ai_flags = flags;
    results = ft_nullptr;
    resolver_status = getaddrinfo(host, service_parameter, &hints, &results);
    if (resolver_status != 0)
    {
        networking_dns_set_error(resolver_status);
        if (results != ft_nullptr)
            freeaddrinfo(results);
        return (FT_FALSE);
    }
    current = results;
    while (current != ft_nullptr)
    {
        networking_resolved_address resolved;

        ft_memset(&resolved, 0, sizeof(resolved));
        if (current->ai_addrlen <= sizeof(resolved.address))
        {
            ft_memcpy(&resolved.address, current->ai_addr, current->ai_addrlen);
            resolved.length = static_cast<socklen_t>(current->ai_addrlen);
            out_addresses.push_back(resolved);
        }
        current = current->ai_next;
    }
    freeaddrinfo(results);
    ft_size_t result_count;

    result_count = out_addresses.size();
    if (result_count == 0)
    {
        networking_push_failure(FT_ERR_SOCKET_RESOLVE_FAILED);
        return (FT_FALSE);
    }
    int64_t cache_record_ms;
    ft_bool cache_record_valid;

    cache_record_ms = time_now_ms();
    cache_record_valid = FT_TRUE;
    if (cache_record_valid)
    {
        networking_dns_cache_entry cache_value;
        int64_t expiration_ms;

        if (cache_record_ms > LONG_MAX - g_networking_dns_cache_ttl_ms)
            expiration_ms = LONG_MAX;
        else
            expiration_ms = cache_record_ms + g_networking_dns_cache_ttl_ms;
        cache_value.expiration_ms = expiration_ms;
        if (cache_value.addresses_initialised == FT_FALSE)
            return (FT_TRUE);
        if (networking_dns_copy_addresses(out_addresses, cache_value.addresses))
        {
            ft_bool update_lock_acquired;
            int32_t update_lock_error;

            update_lock_acquired = FT_FALSE;
            update_lock_error = networking_dns_cache_lock(&update_lock_acquired);

            if (update_lock_error == FT_ERR_SUCCESS)
            {
                g_networking_dns_cache.remove(cache_key);
                g_networking_dns_cache.insert(cache_key, cache_value);
#ifdef LIBFT_TEST_BUILD
                networking_dns_untrack_runtime_leaks();
#endif
                int32_t update_unlock_error = networking_dns_cache_unlock(update_lock_acquired);

                if (update_unlock_error != FT_ERR_SUCCESS)
                {
                    networking_push_failure(update_unlock_error);
                    return (FT_FALSE);
                }
            }
        }
    }
    (void)(FT_ERR_SUCCESS);
    return (FT_TRUE);
}

ft_bool networking_dns_resolve_first(const char *host, const char *service,
    int32_t family, int32_t socktype, int32_t protocol, int32_t flags,
    networking_resolved_address &out_address) noexcept
{
    ft_vector<networking_resolved_address> results;
    int32_t     initialise_error;
    ft_size_t count;

    initialise_error = results.initialize();
    if (initialise_error != FT_ERR_SUCCESS)
    {
        networking_push_failure(initialise_error);
        return (FT_FALSE);
    }
    if (!networking_dns_resolve(host, service, family, socktype, protocol, flags, results))
    {
        (void)results.destroy();
        return (FT_FALSE);
    }
    count = results.size();
    if (count == 0)
    {
        networking_push_failure(FT_ERR_SOCKET_RESOLVE_FAILED);
        (void)results.destroy();
        return (FT_FALSE);
    }
    out_address = results[0];
    (void)results.destroy();
    return (FT_TRUE);
}

ft_bool networking_resolved_address_to_string(
    const networking_resolved_address &address, char *buffer,
    ft_size_t buffer_size) noexcept
{
    const void *source_address;
    int32_t family;

    if (buffer == ft_nullptr || buffer_size == 0)
    {
        networking_push_failure(FT_ERR_INVALID_POINTER);
        return (FT_FALSE);
    }
    buffer[0] = '\0';
    family = address.address.ss_family;
    if (family == AF_INET)
    {
        source_address = &reinterpret_cast<const sockaddr_in *>(&address.address)->sin_addr;
    }
    else if (family == AF_INET6)
    {
        source_address = &reinterpret_cast<const sockaddr_in6 *>(&address.address)->sin6_addr;
    }
    else
    {
        networking_push_failure(FT_ERR_SOCKET_RESOLVE_FAMILY);
        return (FT_FALSE);
    }
    if (!inet_ntop(family, source_address, buffer,
            static_cast<socklen_t>(buffer_size)))
    {
        networking_push_failure(FT_ERR_SOCKET_RESOLVE_FAILED);
        return (FT_FALSE);
    }
    return (FT_TRUE);
}

void networking_dns_clear_cache(void) noexcept
{
    ft_bool cache_lock_acquired;
    int32_t cache_lock_error;
    int32_t cache_init_error;

    cache_init_error = networking_dns_cache_ensure_initialised();
    if (cache_init_error != FT_ERR_SUCCESS)
    {
        networking_push_failure(cache_init_error);
        return ;
    }
    cache_lock_acquired = FT_FALSE;
    cache_lock_error = networking_dns_cache_lock(&cache_lock_acquired);

    if (cache_lock_error != FT_ERR_SUCCESS)
    {
        networking_push_failure(cache_lock_error);
        return ;
    }
    g_networking_dns_cache.clear();
    cache_lock_error = networking_dns_cache_unlock(cache_lock_acquired);
    if (cache_lock_error != FT_ERR_SUCCESS)
    {
        networking_push_failure(cache_lock_error);
        return ;
    }
    return ;
}

#ifdef LIBFT_TEST_BUILD
void networking_dns_destroy_cache_for_tests(void) noexcept
{
    (void)g_networking_dns_cache.destroy();
    return ;
}
#endif
