#include "socket_handle.hpp"
#include "networking.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
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

ft_socket_handle::ft_socket_handle() : _initialized_state(ft_socket_handle::_state_uninitialized), _socket_fd(-1)
{
    return ;
}

ft_socket_handle::ft_socket_handle(int) : _initialized_state(ft_socket_handle::_state_uninitialized), _socket_fd(-1)
{
    return ;
}

ft_socket_handle::~ft_socket_handle()
{
    if (this->_initialized_state == ft_socket_handle::_state_uninitialized)
    {
        pf_printf_fd(2, "ft_socket_handle lifecycle error: %s\n",
            "destructor called on uninitialized instance");
        su_abort();
    }
    if (this->_initialized_state == ft_socket_handle::_state_initialized)
        (void)this->destroy();
    return ;
}

void ft_socket_handle::abort_lifecycle_error(const char *method_name, const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_socket_handle lifecycle error: %s: %s\n",
        method_name, reason);
    su_abort();
    return ;
}

void ft_socket_handle::abort_if_not_initialized(const char *method_name) const
{
    if (this->_initialized_state == ft_socket_handle::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name, "called while object is not initialized");
    return ;
}

int ft_socket_handle::initialize()
{
    if (this->_initialized_state == ft_socket_handle::_state_initialized)
        this->abort_lifecycle_error("ft_socket_handle::initialize",
            "initialize called on initialized instance");
    this->_socket_fd = -1;
    this->_initialized_state = ft_socket_handle::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int ft_socket_handle::destroy()
{
    if (this->_initialized_state != ft_socket_handle::_state_initialized)
        this->abort_lifecycle_error("ft_socket_handle::destroy",
            "destroy called on non-initialized instance");
    (void)this->close();
    this->_initialized_state = ft_socket_handle::_state_destroyed;
    return (FT_ERR_SUCCESS);
}

bool ft_socket_handle::reset(int socket_fd)
{
    this->abort_if_not_initialized("ft_socket_handle::reset");
    if (socket_fd < 0)
    {
        if (!this->close())
            return (false);
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
    return (true);
}

bool ft_socket_handle::close()
{
    this->abort_if_not_initialized("ft_socket_handle::close");
    if (this->_socket_fd < 0)
        return (true);
    if (nw_close(this->_socket_fd) != 0)
        return (false);
    this->_socket_fd = -1;
    return (true);
}

bool ft_socket_handle::is_valid() const
{
    this->abort_if_not_initialized("ft_socket_handle::is_valid");
    if (this->_socket_fd >= 0)
        return (true);
    return (false);
}

int ft_socket_handle::get() const
{
    this->abort_if_not_initialized("ft_socket_handle::get");
    return (this->_socket_fd);
}

int ft_socket_runtime_acquire()
{
#ifdef _WIN32
    std::mutex &runtime_mutex = ft_socket_runtime_mutex();
    int &reference_count = ft_socket_runtime_reference_count();

    runtime_mutex.lock();
    if (reference_count == 0)
    {
        WSADATA data;

        if (WSAStartup(MAKEWORD(2, 2), &data) != 0)
        {
            runtime_mutex.unlock();
            return (FT_ERR_INITIALIZATION_FAILED);
        }
        ft_socket_runtime_initialized() = true;
    }
    if (reference_count < 0)
    {
        reference_count = 0;
    }
    reference_count++;
    runtime_mutex.unlock();
#endif
    (void)(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

void ft_socket_runtime_release()
{
#ifdef _WIN32
    std::mutex &runtime_mutex = ft_socket_runtime_mutex();
    int &reference_count = ft_socket_runtime_reference_count();
    bool &initialized = ft_socket_runtime_initialized();

    runtime_mutex.lock();
    if (reference_count > 0)
    {
        reference_count--;
        if (reference_count == 0 && initialized)
        {
            if (WSACleanup() != 0)
            {
                (void)(FT_ERR_INTERNAL);
            }
            else
            {
                (void)(FT_ERR_SUCCESS);
            }
            initialized = false;
            runtime_mutex.unlock();
            return ;
        }
    }
    runtime_mutex.unlock();
#endif
    (void)(FT_ERR_SUCCESS);
    return ;
}
