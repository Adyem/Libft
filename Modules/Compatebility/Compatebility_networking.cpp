#include "compatebility_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/limits.hpp"
#include <cerrno>
#include <cstdint>

#if defined(_WIN32) || defined(_WIN64)
# include <winsock2.h>
# include <ws2tcpip.h>
#else
# include <sys/socket.h>
# include <unistd.h>
#endif

static int32_t cmp_socket_map_send_error(void)
{
#if defined(_WIN32) || defined(_WIN64)
    return (cmp_map_system_error_to_ft(WSAGetLastError()));
#else
    return (cmp_map_system_error_to_ft(errno));
#endif
}

static void cmp_socket_normalize_send_length(ft_size_t source,
    int32_t *destination_out)
{
    if (source > static_cast<ft_size_t>(FT_INT32_MAX))
        *destination_out = FT_INT32_MAX;
    else
        *destination_out = static_cast<int32_t>(source);
    return ;
}

int32_t cmp_socket_send_all(intptr_t socket_handle, const void *buffer,
                            ft_size_t length, int32_t flags,
                            int64_t *bytes_sent_out)
{
    if (buffer == ft_nullptr)
    {
        if (bytes_sent_out != ft_nullptr)
            *bytes_sent_out = 0;
        return (FT_ERR_INVALID_POINTER);
    }
    if (length == 0)
    {
        if (bytes_sent_out != ft_nullptr)
            *bytes_sent_out = 0;
        return (FT_ERR_SUCCESS);
    }
#if defined(_WIN32) || defined(_WIN64)
    SOCKET socket_descriptor = static_cast<SOCKET>(socket_handle);
    if (socket_descriptor == INVALID_SOCKET)
    {
        if (bytes_sent_out != ft_nullptr)
            *bytes_sent_out = 0;
        return (FT_ERR_INVALID_HANDLE);
    }
#else
    int32_t socket_descriptor = static_cast<int32_t>(socket_handle);
    if (socket_descriptor < 0)
    {
        if (bytes_sent_out != ft_nullptr)
            *bytes_sent_out = 0;
        return (FT_ERR_INVALID_HANDLE);
    }
#endif
    const char *data = static_cast<const char *>(buffer);
    ft_size_t total_sent = 0;
    while (total_sent < length)
    {
        int32_t send_length;
        int64_t chunk;

        cmp_socket_normalize_send_length(length - total_sent, &send_length);
#if defined(_WIN32) || defined(_WIN64)
        chunk = ::send(socket_descriptor, data + total_sent, send_length, flags);
#else
        chunk = ::send(socket_descriptor, data + total_sent, send_length, flags);
#endif
        if (chunk < 0)
        {
            int32_t error_code = cmp_socket_map_send_error();
            if (bytes_sent_out != ft_nullptr)
                *bytes_sent_out = static_cast<int64_t>(total_sent);
            return (error_code);
        }
        if (chunk == 0)
        {
            if (bytes_sent_out != ft_nullptr)
                *bytes_sent_out = static_cast<int64_t>(total_sent);
            return (FT_ERR_IO);
        }
        total_sent += static_cast<ft_size_t>(chunk);
    }
    if (bytes_sent_out != ft_nullptr)
        *bytes_sent_out = static_cast<int64_t>(total_sent);
    return (FT_ERR_SUCCESS);
}
