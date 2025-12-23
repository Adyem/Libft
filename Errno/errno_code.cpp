#include "errno.hpp"
#include "errno_internal.hpp"

#include <cstdlib>

ft_errno_mutex_wrapper::ft_errno_mutex_wrapper(void)
{
    return ;
}

ft_errno_mutex_wrapper::~ft_errno_mutex_wrapper(void)
{
    return ;
}

void ft_errno_mutex_wrapper::lock(void)
{
    try
    {
        this->_mutex.lock();
    }
    catch (...)
    {
        std::abort();
    }
    return ;
}

void ft_errno_mutex_wrapper::unlock(void)
{
    try
    {
        this->_mutex.unlock();
    }
    catch (...)
    {
        std::abort();
    }
    return ;
}

ft_errno_mutex_wrapper &ft_errno_mutex()
{
    static ft_errno_mutex_wrapper errno_mutex;

    return (errno_mutex);
}

void ft_set_errno_locked(int error_code)
{
    std::lock_guard<ft_errno_mutex_wrapper> lock(ft_errno_mutex());

    ft_errno = error_code;
    return ;
}

void ft_set_sys_errno_locked(int error_code)
{
    std::lock_guard<ft_errno_mutex_wrapper> lock(ft_errno_mutex());

    ft_sys_errno = error_code;
    return ;
}

int &ft_errno_reference(void)
{
    static thread_local int thread_errno = FT_ERR_SUCCESSS;

    return (thread_errno);
}

int &ft_sys_errno_reference(void)
{
    static thread_local int thread_sys_errno = FT_SYS_ERR_SUCCESS;

    return (thread_sys_errno);
}
