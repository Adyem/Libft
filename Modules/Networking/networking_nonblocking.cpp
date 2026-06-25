#include "networking.hpp"

#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"
#ifdef _WIN32
# include <winsock2.h>
#else
# include <fcntl.h>
# include <unistd.h>
#endif

int32_t nw_set_nonblocking(int32_t socket_fd)
{
#ifdef _WIN32
    u_long mode;

    mode = 1;
    if (ioctlsocket(static_cast<SOCKET>(socket_fd), FIONBIO, &mode) != 0)
        return (-1);
    return (0);
#else
    int32_t flags;

    flags = fcntl(socket_fd, F_GETFL, 0);
    if (flags == -1)
        return (-1);
    if (fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK) == -1)
        return (-1);
    return (0);
#endif
}
