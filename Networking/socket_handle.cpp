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

void ft_socket_handle::set_error(int error_code) const noexcept
{
    ft_errno = error_code;
    this->_error_code = error_code;
    return ;
}

ft_socket_handle::ft_socket_handle() : _socket_fd(-1), _error_code(ER_SUCCESS)
{
    this->set_error(ER_SUCCESS);
    return ;
}

ft_socket_handle::ft_socket_handle(int socket_fd) : _socket_fd(-1), _error_code(ER_SUCCESS)
{
    this->set_error(ER_SUCCESS);
    this->reset(socket_fd);
    return ;
}

ft_socket_handle::~ft_socket_handle()
{
    this->close();
    return ;
}

ft_socket_handle::ft_socket_handle(ft_socket_handle &&other) noexcept
    : _socket_fd(other._socket_fd), _error_code(other._error_code)
{
    other._socket_fd = -1;
    other._error_code = ER_SUCCESS;
    this->set_error(this->_error_code);
    return ;
}

ft_socket_handle &ft_socket_handle::operator=(ft_socket_handle &&other) noexcept
{
    if (this != &other)
    {
        this->close();
        this->_socket_fd = other._socket_fd;
        this->_error_code = other._error_code;
        other._socket_fd = -1;
        other._error_code = ER_SUCCESS;
        this->set_error(this->_error_code);
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
        this->set_error(ER_SUCCESS);
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
    this->set_error(ER_SUCCESS);
    return (true);
}

bool ft_socket_handle::close()
{
    if (this->_socket_fd < 0)
    {
        this->set_error(ER_SUCCESS);
        return (true);
    }
    if (nw_close(this->_socket_fd) != 0)
    {
        int close_error;

        close_error = ft_errno;
        if (close_error == ER_SUCCESS)
        {
            close_error = FT_ERR_SOCKET_CLOSE_FAILED;
        }
        this->set_error(close_error);
        return (false);
    }
    this->_socket_fd = -1;
    this->set_error(ER_SUCCESS);
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

int ft_socket_handle::get_error() const
{
    return (this->_error_code);
}

const char *ft_socket_handle::get_error_str() const
{
    return (ft_strerror(this->_error_code));
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
            ft_errno = ft_map_system_error(WSAGetLastError());
            if (ft_errno == ER_SUCCESS)
            {
                ft_errno = FT_ERR_INITIALIZATION_FAILED;
            }
            return (ft_errno);
        }
        ft_socket_runtime_initialized() = true;
    }
    if (reference_count < 0)
    {
        reference_count = 0;
    }
    reference_count++;
#endif
    ft_errno = ER_SUCCESS;
    return (ER_SUCCESS);
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
                ft_errno = ft_map_system_error(WSAGetLastError());
                if (ft_errno == ER_SUCCESS)
                {
                    ft_errno = FT_ERR_INTERNAL;
                }
            }
            else
            {
                ft_errno = ER_SUCCESS;
            }
            initialized = false;
            return ;
        }
    }
#endif
    ft_errno = ER_SUCCESS;
    return ;
}
