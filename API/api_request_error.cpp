#include "api_http_internal.hpp"
#include "../Errno/errno.hpp"
#include <errno.h>
#ifdef _WIN32
# include <winsock2.h>
# include <ws2tcpip.h>
#endif

void api_request_set_resolve_error(int resolver_status)
{
#ifdef EAI_BADFLAGS
    if (resolver_status == EAI_BADFLAGS)
    {
        ft_errno = FT_ERR_SOCKET_RESOLVE_BAD_FLAGS;
        return ;
    }
#endif
#ifdef EAI_AGAIN
    if (resolver_status == EAI_AGAIN)
    {
        ft_errno = FT_ERR_SOCKET_RESOLVE_AGAIN;
        return ;
    }
#endif
#ifdef EAI_FAIL
    if (resolver_status == EAI_FAIL)
    {
        ft_errno = FT_ERR_SOCKET_RESOLVE_FAIL;
        return ;
    }
#endif
#ifdef EAI_FAMILY
    if (resolver_status == EAI_FAMILY)
    {
        ft_errno = FT_ERR_SOCKET_RESOLVE_FAMILY;
        return ;
    }
#endif
#ifdef EAI_ADDRFAMILY
    if (resolver_status == EAI_ADDRFAMILY)
    {
        ft_errno = FT_ERR_SOCKET_RESOLVE_FAMILY;
        return ;
    }
#endif
#ifdef EAI_SOCKTYPE
    if (resolver_status == EAI_SOCKTYPE)
    {
        ft_errno = FT_ERR_SOCKET_RESOLVE_SOCKTYPE;
        return ;
    }
#endif
#ifdef EAI_SERVICE
    if (resolver_status == EAI_SERVICE)
    {
        ft_errno = FT_ERR_SOCKET_RESOLVE_SERVICE;
        return ;
    }
#endif
#ifdef EAI_MEMORY
    if (resolver_status == EAI_MEMORY)
    {
        ft_errno = FT_ERR_SOCKET_RESOLVE_MEMORY;
        return ;
    }
#endif
#ifdef EAI_NONAME
    if (resolver_status == EAI_NONAME)
    {
        ft_errno = FT_ERR_SOCKET_RESOLVE_NO_NAME;
        return ;
    }
#endif
#ifdef EAI_NODATA
    if (resolver_status == EAI_NODATA)
    {
        ft_errno = FT_ERR_SOCKET_RESOLVE_NO_NAME;
        return ;
    }
#endif
#ifdef EAI_OVERFLOW
    if (resolver_status == EAI_OVERFLOW)
    {
        ft_errno = FT_ERR_SOCKET_RESOLVE_OVERFLOW;
        return ;
    }
#endif
#ifdef EAI_SYSTEM
    if (resolver_status == EAI_SYSTEM)
    {
#ifdef _WIN32
        ft_errno = WSAGetLastError() + ERRNO_OFFSET;
#else
        if (errno != 0)
            ft_errno = errno + ERRNO_OFFSET;
        else
            ft_errno = FT_ERR_SOCKET_RESOLVE_FAIL;
#endif
        return ;
    }
#endif
    (void)resolver_status;
    ft_errno = FT_ERR_SOCKET_RESOLVE_FAILED;
    return ;
}
