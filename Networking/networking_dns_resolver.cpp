#include "networking.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../CPP_class/class_string.hpp"
#include "../Template/map.hpp"
#include "../Template/vector.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"
#include "../Basic/basic.hpp"
#include "../Time/time.hpp"
#include <climits>
#include <cstdio>
#include <cstring>
#include <utility>
#include <errno.h>

#ifdef _WIN32
# include <winsock2.h>
# include <ws2tcpip.h>
#else
# include <netdb.h>
# include <sys/socket.h>
#endif

struct networking_dns_cache_entry
{
    ft_vector<networking_resolved_address> addresses;
    long                                   expiration_ms;

    networking_dns_cache_entry() noexcept
        : addresses(), expiration_ms(0)
    {
        return ;
    }

    ~networking_dns_cache_entry() noexcept
    {
        return ;
    }

    networking_dns_cache_entry(const networking_dns_cache_entry &other) noexcept
        : addresses(), expiration_ms(other.expiration_ms)
    {
        size_t index;
        size_t count;

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
            size_t index;
            size_t count;

            this->addresses.clear();
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
static pt_mutex                                        g_networking_dns_cache_mutex;
static const long                                      g_networking_dns_cache_ttl_ms = 60000;

static void networking_push_failure(int error_code) noexcept
{
    (void)(error_code);
}

static int networking_dns_cache_lock(bool *lock_acquired) noexcept
{
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (g_networking_dns_cache_mutex.lock() != FT_ERR_SUCCESS)
        return (FT_ERR_INTERNAL);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

static int networking_dns_cache_unlock(bool lock_acquired) noexcept
{
    if (lock_acquired == false)
        return (FT_ERR_SUCCESS);
    if (g_networking_dns_cache_mutex.unlock() != FT_ERR_SUCCESS)
        return (FT_ERR_INTERNAL);
    return (FT_ERR_SUCCESS);
}

static bool networking_dns_append_literal(ft_string &target, const char *literal) noexcept
{
    const char  *value;

    value = literal;
    if (value == ft_nullptr)
        value = "";
    target.append(value);
    return (true);
}

static bool networking_dns_append_separator(ft_string &target) noexcept
{
    target.append('|');
    return (true);
}

static bool networking_dns_copy_addresses(const ft_vector<networking_resolved_address> &source,
    ft_vector<networking_resolved_address> &destination) noexcept
{
    size_t  index;
    size_t  count;

    destination.clear();
    count = source.size();
    index = 0;
    while (index < count)
    {
        destination.push_back(source[index]);
        index++;
    }
    return (true);
}

static bool networking_dns_append_number(ft_string &target, int value) noexcept
{
    char number_buffer[32];

    snprintf(number_buffer, sizeof(number_buffer), "%d", value);
    target.append(number_buffer);
    return (true);
}

void networking_dns_set_error(int resolver_status) noexcept
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

bool networking_dns_resolve(const char *host, const char *service,
    int family, int socktype, int protocol, int flags,
    ft_vector<networking_resolved_address> &out_addresses) noexcept
{
    ft_string   cache_key;
    const char  *service_string;
    const char  *service_parameter;
    long        lookup_start_ms;
    bool        lookup_start_valid;
    bool        cache_lock_acquired;
    Pair<ft_string, networking_dns_cache_entry> *cache_entry;
    bool        entry_valid;
    int         cache_lock_error;

    out_addresses.clear();
    if (host == ft_nullptr || host[0] == '\0')
    {
        networking_push_failure(FT_ERR_INVALID_ARGUMENT);
        return (false);
    }
    cache_key = host;
    if (!networking_dns_append_separator(cache_key))
        return (false);
    service_string = service;
    if (service_string == ft_nullptr)
        service_string = "";
    if (!networking_dns_append_literal(cache_key, service_string))
        return (false);
    if (!networking_dns_append_separator(cache_key))
        return (false);
    if (!networking_dns_append_number(cache_key, family))
        return (false);
    if (!networking_dns_append_separator(cache_key))
        return (false);
    if (!networking_dns_append_number(cache_key, socktype))
        return (false);
    if (!networking_dns_append_separator(cache_key))
        return (false);
    if (!networking_dns_append_number(cache_key, protocol))
        return (false);
    if (!networking_dns_append_separator(cache_key))
        return (false);
    if (!networking_dns_append_number(cache_key, flags))
        return (false);
    lookup_start_ms = time_now_ms();
    lookup_start_valid = true;
    cache_lock_acquired = false;
    cache_lock_error = networking_dns_cache_lock(&cache_lock_acquired);
    if (cache_lock_error != FT_ERR_SUCCESS)
    {
        networking_push_failure(cache_lock_error);
        return (false);
    }
    cache_entry = g_networking_dns_cache.find(cache_key);
    if (cache_entry != g_networking_dns_cache.end())
    {
        entry_valid = true;
        if (lookup_start_valid)
        {
            if (cache_entry->value.expiration_ms < lookup_start_ms)
                entry_valid = false;
        }
        if (entry_valid)
        {
            if (!networking_dns_copy_addresses(cache_entry->value.addresses, out_addresses))
            {
                int cache_unlock_error = networking_dns_cache_unlock(cache_lock_acquired);

                if (cache_unlock_error != FT_ERR_SUCCESS)
                {
                    networking_push_failure(cache_unlock_error);
                }
                return (false);
            }
            {
                int cache_unlock_error = networking_dns_cache_unlock(cache_lock_acquired);

                if (cache_unlock_error != FT_ERR_SUCCESS)
                {
                    networking_push_failure(cache_unlock_error);
                    return (false);
                }
            }
            (void)(FT_ERR_SUCCESS);
            return (true);
        }
        g_networking_dns_cache.remove(cache_key);
    }
        {
            int cache_unlock_error = networking_dns_cache_unlock(cache_lock_acquired);

        if (cache_unlock_error != FT_ERR_SUCCESS)
        {
            networking_push_failure(cache_unlock_error);
            return (false);
        }
    }
    service_parameter = ft_nullptr;
    if (service_string[0] != '\0')
        service_parameter = service_string;
    struct addrinfo hints;
    struct addrinfo *results;
    struct addrinfo *current;
    int resolver_status;

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
        return (false);
    }
    current = results;
    while (current != ft_nullptr)
    {
        networking_resolved_address resolved;

        ft_memset(&resolved, 0, sizeof(resolved));
        if (current->ai_addrlen <= sizeof(resolved.address))
        {
            ft_memcpy(&resolved.address, current->ai_addr, current->ai_addrlen);
            resolved.length = current->ai_addrlen;
            out_addresses.push_back(resolved);
        }
        current = current->ai_next;
    }
    freeaddrinfo(results);
    size_t result_count;

    result_count = out_addresses.size();
    if (result_count == 0)
    {
        networking_push_failure(FT_ERR_SOCKET_RESOLVE_FAILED);
        return (false);
    }
    long cache_record_ms;
    bool cache_record_valid;

    cache_record_ms = time_now_ms();
    cache_record_valid = true;
    if (cache_record_valid)
    {
        networking_dns_cache_entry cache_value;
        long expiration_ms;

        if (cache_record_ms > LONG_MAX - g_networking_dns_cache_ttl_ms)
            expiration_ms = LONG_MAX;
        else
            expiration_ms = cache_record_ms + g_networking_dns_cache_ttl_ms;
        cache_value.expiration_ms = expiration_ms;
        if (networking_dns_copy_addresses(out_addresses, cache_value.addresses))
        {
            bool update_lock_acquired;
            int update_lock_error;

            update_lock_acquired = false;
            update_lock_error = networking_dns_cache_lock(&update_lock_acquired);

            if (update_lock_error == FT_ERR_SUCCESS)
            {
                g_networking_dns_cache.remove(cache_key);
                g_networking_dns_cache.insert(cache_key, cache_value);
                int update_unlock_error = networking_dns_cache_unlock(update_lock_acquired);

                if (update_unlock_error != FT_ERR_SUCCESS)
                {
                    networking_push_failure(update_unlock_error);
                    return (false);
                }
            }
        }
    }
    (void)(FT_ERR_SUCCESS);
    return (true);
}

bool networking_dns_resolve_first(const char *host, const char *service,
    int family, int socktype, int protocol, int flags,
    networking_resolved_address &out_address) noexcept
{
    ft_vector<networking_resolved_address> results;
    size_t count;

    if (!networking_dns_resolve(host, service, family, socktype, protocol, flags, results))
        return (false);
    count = results.size();
    if (count == 0)
    {
        networking_push_failure(FT_ERR_SOCKET_RESOLVE_FAILED);
        return (false);
    }
    out_address = results[0];
    (void)(FT_ERR_SUCCESS);
    return (true);
}

void networking_dns_clear_cache(void) noexcept
{
    bool cache_lock_acquired;
    int cache_lock_error;

    cache_lock_acquired = false;
    cache_lock_error = networking_dns_cache_lock(&cache_lock_acquired);

    if (cache_lock_error != FT_ERR_SUCCESS)
    {
        networking_push_failure(cache_lock_error);
        return ;
    }
    g_networking_dns_cache.clear();
    {
        int cache_unlock_error = networking_dns_cache_unlock(cache_lock_acquired);

        if (cache_unlock_error != FT_ERR_SUCCESS)
        {
            networking_push_failure(cache_unlock_error);
            return ;
        }
    }
    (void)(FT_ERR_SUCCESS);
    return ;
}
