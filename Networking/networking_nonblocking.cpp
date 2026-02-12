#include "networking.hpp"

#ifdef _WIN32
# include <winsock2.h>
#else
# include <fcntl.h>
# include <unistd.h>
# include <errno.h>
#endif
#include "../Errno/errno.hpp"

int nw_set_nonblocking(int socket_fd)
{
#ifdef _WIN32
    u_long mode;
    mode = 1;
    if (ioctlsocket(static_cast<SOCKET>(socket_fd), FIONBIO, &mode) != 0)
    {
        ft_global_error_stack_push(ft_map_system_error(WSAGetLastError()));
        return (-1);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (0);
#else
    int flags;
    flags = fcntl(socket_fd, F_GETFL, 0);
    if (flags == -1)
    {
        ft_global_error_stack_push(ft_map_system_error(errno));
        return (-1);
    }
    if (fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        ft_global_error_stack_push(ft_map_system_error(errno));
        return (-1);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (0);
#endif
}
