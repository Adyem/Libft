#include "condition.hpp"
#include "mutex.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Libft/libft.hpp"
#include <cerrno>

pt_condition_variable::pt_condition_variable()
    : _condition(), _mutex(), _condition_initialized(false), _mutex_initialized(false), _error_code(ER_SUCCESS)
{
    if (pthread_mutex_init(&this->_mutex, ft_nullptr) != 0)
    {
        this->set_error(errno + ERRNO_OFFSET);
        return ;
    }
    this->_mutex_initialized = true;
    if (pt_cond_init(&this->_condition, ft_nullptr) != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    this->_condition_initialized = true;
    this->set_error(ER_SUCCESS);
    return ;
}

pt_condition_variable::~pt_condition_variable()
{
    if (this->_condition_initialized)
        pt_cond_destroy(&this->_condition);
    if (this->_mutex_initialized)
        pthread_mutex_destroy(&this->_mutex);
    this->set_error(ER_SUCCESS);
    return ;
}

void pt_condition_variable::set_error(int error) const
{
    this->_error_code = error;
    ft_errno = error;
    return ;
}

int pt_condition_variable::wait(pt_mutex &mutex)
{
    if (!this->_condition_initialized || !this->_mutex_initialized)
    {
        this->set_error(FT_EINVAL);
        return (-1);
    }
    if (!mutex.lockState())
    {
        this->set_error(PT_ERR_MUTEX_OWNER);
        return (-1);
    }
    if (pthread_mutex_lock(&this->_mutex) != 0)
    {
        this->set_error(errno + ERRNO_OFFSET);
        return (-1);
    }
    if (mutex.unlock(THREAD_ID) != SUCCES)
    {
        int unlock_error;

        unlock_error = mutex.get_error();
        pthread_mutex_unlock(&this->_mutex);
        this->set_error(unlock_error);
        return (-1);
    }
    if (pt_cond_wait(&this->_condition, &this->_mutex) != 0)
    {
        int wait_error;

        wait_error = ft_errno;
        pthread_mutex_unlock(&this->_mutex);
        if (mutex.lock(THREAD_ID) != SUCCES)
        {
            int relock_error;

            relock_error = mutex.get_error();
            this->set_error(relock_error);
            return (-1);
        }
        this->set_error(wait_error);
        return (-1);
    }
    if (pthread_mutex_unlock(&this->_mutex) != 0)
    {
        int unlock_error;

        unlock_error = errno + ERRNO_OFFSET;
        if (mutex.lock(THREAD_ID) != SUCCES)
        {
            int relock_error;

            relock_error = mutex.get_error();
            this->set_error(relock_error);
            return (-1);
        }
        this->set_error(unlock_error);
        return (-1);
    }
    if (mutex.lock(THREAD_ID) != SUCCES)
    {
        int relock_error;

        relock_error = mutex.get_error();
        this->set_error(relock_error);
        return (-1);
    }
    this->set_error(ER_SUCCESS);
    return (0);
}

int pt_condition_variable::wait_until(pt_mutex &mutex, const struct timespec &absolute_time)
{
    int wait_result;

    if (!this->_condition_initialized || !this->_mutex_initialized)
    {
        this->set_error(FT_EINVAL);
        return (-1);
    }
    if (!mutex.lockState())
    {
        this->set_error(PT_ERR_MUTEX_OWNER);
        return (-1);
    }
    if (pthread_mutex_lock(&this->_mutex) != 0)
    {
        this->set_error(errno + ERRNO_OFFSET);
        return (-1);
    }
    if (mutex.unlock(THREAD_ID) != SUCCES)
    {
        int unlock_error;

        unlock_error = mutex.get_error();
        pthread_mutex_unlock(&this->_mutex);
        this->set_error(unlock_error);
        return (-1);
    }
    wait_result = pthread_cond_timedwait(&this->_condition, &this->_mutex, &absolute_time);
    if (pthread_mutex_unlock(&this->_mutex) != 0)
    {
        int unlock_error;

        unlock_error = errno + ERRNO_OFFSET;
        if (mutex.lock(THREAD_ID) != SUCCES)
        {
            int relock_error;

            relock_error = mutex.get_error();
            this->set_error(relock_error);
            return (-1);
        }
        this->set_error(unlock_error);
        return (-1);
    }
    if (mutex.lock(THREAD_ID) != SUCCES)
    {
        int relock_error;

        relock_error = mutex.get_error();
        this->set_error(relock_error);
        return (-1);
    }
    if (wait_result == 0)
    {
        this->set_error(ER_SUCCESS);
        return (0);
    }
    if (wait_result == ETIMEDOUT)
    {
        this->set_error(ER_SUCCESS);
        return (ETIMEDOUT);
    }
    this->set_error(wait_result + ERRNO_OFFSET);
    return (-1);
}

int pt_condition_variable::signal()
{
    if (!this->_condition_initialized || !this->_mutex_initialized)
    {
        this->set_error(FT_EINVAL);
        return (-1);
    }
    if (pthread_mutex_lock(&this->_mutex) != 0)
    {
        this->set_error(errno + ERRNO_OFFSET);
        return (-1);
    }
    if (pt_cond_signal(&this->_condition) != 0)
    {
        int signal_error;

        signal_error = ft_errno;
        pthread_mutex_unlock(&this->_mutex);
        this->set_error(signal_error);
        return (-1);
    }
    if (pthread_mutex_unlock(&this->_mutex) != 0)
    {
        this->set_error(errno + ERRNO_OFFSET);
        return (-1);
    }
    this->set_error(ER_SUCCESS);
    return (0);
}

int pt_condition_variable::broadcast()
{
    if (!this->_condition_initialized || !this->_mutex_initialized)
    {
        this->set_error(FT_EINVAL);
        return (-1);
    }
    if (pthread_mutex_lock(&this->_mutex) != 0)
    {
        this->set_error(errno + ERRNO_OFFSET);
        return (-1);
    }
    if (pt_cond_broadcast(&this->_condition) != 0)
    {
        int broadcast_error;

        broadcast_error = ft_errno;
        pthread_mutex_unlock(&this->_mutex);
        this->set_error(broadcast_error);
        return (-1);
    }
    if (pthread_mutex_unlock(&this->_mutex) != 0)
    {
        this->set_error(errno + ERRNO_OFFSET);
        return (-1);
    }
    this->set_error(ER_SUCCESS);
    return (0);
}

int pt_condition_variable::get_error() const
{
    return (this->_error_code);
}

const char *pt_condition_variable::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}
