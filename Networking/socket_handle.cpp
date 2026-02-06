#include "socket_handle.hpp"
#include "networking.hpp"
#include <mutex>

#ifdef _WIN32
# include <winsock2.h>
# include <ws2tcpip.h>
# include <io.h>
#else
# include <unistd.h>
# include <sys/socket.h>
# include <errno.h>
#endif

#ifdef _WIN32
static std::mutex &ft_socket_runtime_mutex()
{
    static std::mutex runtime_mutex;
    return (runtime_mutex);
}

static int &ft_socket_runtime_reference_count()
{
    static int reference_count = 0;
    return (reference_count);
}

static bool &ft_socket_runtime_initialized()
{
    static bool initialized = false;
    return (initialized);
}
#endif

ft_socket_handle::ft_socket_handle() : _socket_fd(-1)
{
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

ft_socket_handle::ft_socket_handle(int socket_fd) : _socket_fd(-1)
{
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    this->reset(socket_fd);
    return ;
}

ft_socket_handle::~ft_socket_handle()
{
    this->close();
    return ;
}

ft_socket_handle::ft_socket_handle(ft_socket_handle &&other) noexcept
    : _socket_fd(other._socket_fd)
{
    other._socket_fd = -1;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

ft_socket_handle &ft_socket_handle::operator=(ft_socket_handle &&other) noexcept
{
    if (this != &other)
    {
        this->close();
        this->_socket_fd = other._socket_fd;
        other._socket_fd = -1;
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
    }
    return (*this);
}

bool ft_socket_handle::reset(int socket_fd)
{
    if (socket_fd < 0)
    {
        if (!this->close())
        {
            return (false);
        }
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (true);
    }
    if (this->_socket_fd >= 0)
    {
        if (!this->close())
        {
            return (false);
        }
    }
    this->_socket_fd = socket_fd;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (true);
}

bool ft_socket_handle::close()
{
    if (this->_socket_fd < 0)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (true);
    }
    if (nw_close(this->_socket_fd) != 0)
    {
        int close_error;

        close_error = ft_global_error_stack_peek_last_error();
        if (close_error == FT_ERR_SUCCESSS)
        {
            close_error = FT_ERR_SOCKET_CLOSE_FAILED;
        }
        ft_global_error_stack_push(close_error);
        return (false);
    }
    this->_socket_fd = -1;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (true);
}

bool ft_socket_handle::is_valid() const
{
    if (this->_socket_fd >= 0)
    {
        return (true);
    }
    return (false);
}

int ft_socket_handle::get() const
{
    return (this->_socket_fd);
}

int ft_socket_runtime_acquire()
{
#ifdef _WIN32
    std::lock_guard<std::mutex> lock(ft_socket_runtime_mutex());
    int &reference_count = ft_socket_runtime_reference_count();
    if (reference_count == 0)
    {
        WSADATA data;
        if (WSAStartup(MAKEWORD(2, 2), &data) != 0)
        {
            int runtime_error;

            runtime_error = ft_map_system_error(WSAGetLastError());
            if (runtime_error == FT_ERR_SUCCESSS)
            {
                runtime_error = FT_ERR_INITIALIZATION_FAILED;
            }
            ft_global_error_stack_push(runtime_error);
            return (runtime_error);
        }
        ft_socket_runtime_initialized() = true;
    }
    if (reference_count < 0)
    {
        reference_count = 0;
    }
    reference_count++;
#endif
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

void ft_socket_runtime_release()
{
#ifdef _WIN32
    std::lock_guard<std::mutex> lock(ft_socket_runtime_mutex());
    int &reference_count = ft_socket_runtime_reference_count();
    bool &initialized = ft_socket_runtime_initialized();
    if (reference_count > 0)
    {
        reference_count--;
        if (reference_count == 0 && initialized)
        {
            if (WSACleanup() != 0)
            {
                int cleanup_error;

                cleanup_error = ft_map_system_error(WSAGetLastError());
                if (cleanup_error == FT_ERR_SUCCESSS)
                {
                    cleanup_error = FT_ERR_INTERNAL;
                }
                ft_global_error_stack_push(cleanup_error);
            }
            else
            {
                ft_global_error_stack_push(FT_ERR_SUCCESSS);
            }
            initialized = false;
            return ;
        }
    }
#endif
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}
