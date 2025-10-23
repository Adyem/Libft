#include "pthread.hpp"
#include "mutex.hpp"
#include "../Errno/errno.hpp"

pt_mutex::pt_mutex()
    : _owner(0), _lock(false), _error(ER_SUCCESS), _native_initialized(false)
{
    if (pthread_mutex_init(&this->_native_mutex, ft_nullptr) != 0)
    {
        this->_native_initialized = false;
        this->set_error(FT_ERR_INVALID_STATE);
        return ;
    }
    this->_native_initialized = true;
    this->set_error(ER_SUCCESS);
    return ;
}

pt_mutex::~pt_mutex()
{
    if (this->_native_initialized)
    {
        pthread_mutex_destroy(&this->_native_mutex);
        this->_native_initialized = false;
    }
    return ;
}

void    pt_mutex::set_error(int error) const
{
    this->_error = error;
    ft_errno = error;
    return ;
}

bool pt_mutex::ensure_native_mutex() const
{
    if (this->_native_initialized)
        return (true);
    if (pthread_mutex_init(&this->_native_mutex, ft_nullptr) != 0)
    {
        this->_native_initialized = false;
        this->set_error(FT_ERR_INVALID_STATE);
        return (false);
    }
    this->_native_initialized = true;
    return (true);
}

bool pt_mutex::lockState() const
{
    return (this->_lock.load(std::memory_order_acquire));
}

int pt_mutex::get_error() const
{
    return (this->_error);
}

const char *pt_mutex::get_error_str() const
{
    return (ft_strerror(this->_error));
}

