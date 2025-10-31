#include "class_nullptr.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/pthread.hpp"

namespace ft
{
    static const nullptr_t *get_nullptr_address(const nullptr_t &value) noexcept
    {
        return (reinterpret_cast<const nullptr_t*>(
            &const_cast<char&>(reinterpret_cast<const volatile char&>(value))));
    }

    nullptr_t::mutex_guard::mutex_guard() noexcept
        : _mutex(static_cast<pt_mutex*>(nullptr))
        , _owns_lock(false)
        , _error_code(ER_SUCCESS)
    {
        return ;
    }

    nullptr_t::mutex_guard::~mutex_guard() noexcept
    {
        if (this->_owns_lock && this->_mutex != static_cast<pt_mutex*>(nullptr))
            this->unlock();
        return ;
    }

    int nullptr_t::mutex_guard::lock(pt_mutex &mutex) noexcept
    {
        int lock_result;

        if (this->_owns_lock)
            return (FT_ERR_MUTEX_ALREADY_LOCKED);
        lock_result = mutex.lock(THREAD_ID);
        if (lock_result != ER_SUCCESS)
        {
            this->_mutex = static_cast<pt_mutex*>(nullptr);
            this->_owns_lock = false;
            this->_error_code = lock_result;
            return (lock_result);
        }
        this->_mutex = &mutex;
        this->_owns_lock = true;
        this->_error_code = ER_SUCCESS;
        return (ER_SUCCESS);
    }

    void nullptr_t::mutex_guard::unlock() noexcept
    {
        int unlock_result;

        if (!this->_owns_lock || this->_mutex == static_cast<pt_mutex*>(nullptr))
            return ;
        unlock_result = this->_mutex->unlock(THREAD_ID);
        if (unlock_result != ER_SUCCESS)
        {
            this->_error_code = unlock_result;
            return ;
        }
        this->_mutex = static_cast<pt_mutex*>(nullptr);
        this->_owns_lock = false;
        this->_error_code = ER_SUCCESS;
        return ;
    }

    bool nullptr_t::mutex_guard::owns_lock() const noexcept
    {
        return (this->_owns_lock);
    }

    int nullptr_t::mutex_guard::get_error() const noexcept
    {
        return (this->_error_code);
    }

    void nullptr_t::set_error_unlocked(int error_code) const noexcept
    {
        this->_error_code = error_code;
        ft_errno = error_code;
        return ;
    }

    void nullptr_t::set_error(int error_code) const noexcept
    {
        mutex_guard guard;
        int entry_errno;

        entry_errno = ft_errno;
        if (guard.lock(this->_mutex) != ER_SUCCESS)
        {
            ft_errno = entry_errno;
            return ;
        }
        this->set_error_unlocked(error_code);
        nullptr_t::restore_errno(guard, entry_errno);
        return ;
    }

    void nullptr_t::restore_errno(mutex_guard &guard, int entry_errno) noexcept
    {
        int operation_errno;

        operation_errno = ft_errno;
        if (guard.owns_lock())
            guard.unlock();
        if (guard.get_error() != ER_SUCCESS)
        {
            ft_errno = guard.get_error();
            return ;
        }
        if (operation_errno != ER_SUCCESS)
        {
            ft_errno = operation_errno;
            return ;
        }
        ft_errno = entry_errno;
        return ;
    }

    nullptr_t::nullptr_t() noexcept
        : _error_code(ER_SUCCESS)
        , _mutex()
    {
        this->set_error(ER_SUCCESS);
        return ;
    }

    nullptr_t::nullptr_t(const nullptr_t &other) noexcept
        : _error_code(ER_SUCCESS)
        , _mutex()
    {
        int entry_errno;
        int other_error;

        entry_errno = ft_errno;
        other_error = other.get_error();
        this->_error_code = other_error;
        this->set_error(other_error);
        ft_errno = entry_errno;
        return ;
    }

    nullptr_t &nullptr_t::operator=(const nullptr_t &other) noexcept
    {
        mutex_guard guard;
        int entry_errno;
        int lock_error;
        int other_error;
        const nullptr_t *other_address;

        other_address = get_nullptr_address(other);
        if (this == other_address)
            return (*this);
        entry_errno = ft_errno;
        lock_error = guard.lock(this->_mutex);
        if (lock_error != ER_SUCCESS)
        {
            this->set_error_unlocked(lock_error);
            nullptr_t::restore_errno(guard, entry_errno);
            return (*this);
        }
        other_error = other.get_error();
        this->_error_code = other_error;
        ft_errno = other_error;
        nullptr_t::restore_errno(guard, entry_errno);
        return (*this);
    }

    nullptr_t::~nullptr_t() noexcept
    {
        this->set_error(ER_SUCCESS);
        return ;
    }

    int nullptr_t::get_error() const noexcept
    {
        mutex_guard guard;
        int entry_errno;
        int lock_error;
        int error_code;

        entry_errno = ft_errno;
        lock_error = guard.lock(this->_mutex);
        if (lock_error != ER_SUCCESS)
        {
            this->set_error_unlocked(lock_error);
            nullptr_t::restore_errno(guard, entry_errno);
            return (lock_error);
        }
        error_code = this->_error_code;
        nullptr_t::restore_errno(guard, entry_errno);
        return (error_code);
    }

    const char *nullptr_t::get_error_str() const noexcept
    {
        mutex_guard guard;
        int entry_errno;
        int lock_error;
        const char *error_string;

        entry_errno = ft_errno;
        lock_error = guard.lock(this->_mutex);
        if (lock_error != ER_SUCCESS)
        {
            this->set_error_unlocked(lock_error);
            nullptr_t::restore_errno(guard, entry_errno);
            return (ft_strerror(lock_error));
        }
        error_string = ft_strerror(this->_error_code);
        nullptr_t::restore_errno(guard, entry_errno);
        return (error_string);
    }

    void nullptr_t::operator&() const
    {
        return ;
    }

    const nullptr_t ft_nullptr_instance = nullptr_t();
}
