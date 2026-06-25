#include "api_http_internal.hpp"
#include "../Errno/errno.hpp"
#include <errno.h>

#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#ifdef _WIN32
# include <winsock2.h>
# include <ws2tcpip.h>
#endif

int32_t api_request_set_resolve_error(int32_t resolver_status)
{
    int32_t error_code = FT_ERR_SOCKET_RESOLVE_FAILED;

#ifdef EAI_BADFLAGS
    if (resolver_status == EAI_BADFLAGS)
        return (FT_ERR_SOCKET_RESOLVE_BAD_FLAGS);
#endif
#ifdef EAI_AGAIN
    if (resolver_status == EAI_AGAIN)
        return (FT_ERR_SOCKET_RESOLVE_AGAIN);
#endif
#ifdef EAI_FAIL
    if (resolver_status == EAI_FAIL)
        return (FT_ERR_SOCKET_RESOLVE_FAIL);
#endif
#ifdef EAI_FAMILY
    if (resolver_status == EAI_FAMILY)
        return (FT_ERR_SOCKET_RESOLVE_FAMILY);
#endif
#ifdef EAI_ADDRFAMILY
    if (resolver_status == EAI_ADDRFAMILY)
        return (FT_ERR_SOCKET_RESOLVE_FAMILY);
#endif
#ifdef EAI_SOCKTYPE
    if (resolver_status == EAI_SOCKTYPE)
        return (FT_ERR_SOCKET_RESOLVE_SOCKTYPE);
#endif
#ifdef EAI_SERVICE
    if (resolver_status == EAI_SERVICE)
        return (FT_ERR_SOCKET_RESOLVE_SERVICE);
#endif
#ifdef EAI_MEMORY
    if (resolver_status == EAI_MEMORY)
        return (FT_ERR_SOCKET_RESOLVE_MEMORY);
#endif
#ifdef EAI_NONAME
    if (resolver_status == EAI_NONAME)
        return (FT_ERR_SOCKET_RESOLVE_NO_NAME);
#endif
#ifdef EAI_NODATA
    if (resolver_status == EAI_NODATA)
        return (FT_ERR_SOCKET_RESOLVE_NO_NAME);
#endif
#ifdef EAI_OVERFLOW
    if (resolver_status == EAI_OVERFLOW)
        return (FT_ERR_SOCKET_RESOLVE_OVERFLOW);
#endif
#ifdef EAI_SYSTEM
    if (resolver_status == EAI_SYSTEM)
    {
#ifdef _WIN32
        return (cmp_map_system_error_to_ft(WSAGetLastError()));
#else
        int32_t system_error = errno;

        if (system_error != 0)
            return (cmp_map_system_error_to_ft(system_error));
        return (FT_ERR_SOCKET_RESOLVE_FAIL);
#endif
    }
#endif
    (void)resolver_status;
    return (error_code);
}

ft_bool api_is_configuration_socket_error(int32_t error_code)
{
    if (error_code == FT_ERR_CONFIGURATION)
        return (FT_TRUE);
    if (error_code == FT_ERR_SOCKET_RESOLVE_FAILED)
        return (FT_TRUE);
    if (error_code == FT_ERR_SOCKET_RESOLVE_AGAIN)
        return (FT_TRUE);
    if (error_code == FT_ERR_SOCKET_RESOLVE_FAIL)
        return (FT_TRUE);
    if (error_code == FT_ERR_SOCKET_RESOLVE_BAD_FLAGS)
        return (FT_TRUE);
    if (error_code == FT_ERR_SOCKET_RESOLVE_FAMILY)
        return (FT_TRUE);
    if (error_code == FT_ERR_SOCKET_RESOLVE_SOCKTYPE)
        return (FT_TRUE);
    if (error_code == FT_ERR_SOCKET_RESOLVE_SERVICE)
        return (FT_TRUE);
    if (error_code == FT_ERR_SOCKET_RESOLVE_MEMORY)
        return (FT_TRUE);
    if (error_code == FT_ERR_SOCKET_RESOLVE_NO_NAME)
        return (FT_TRUE);
    if (error_code == FT_ERR_SOCKET_RESOLVE_OVERFLOW)
        return (FT_TRUE);
    return (FT_FALSE);
}
