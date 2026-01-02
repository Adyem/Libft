#include "class_string.hpp"
#include "../CMA/CMA.hpp"
#include "../Libft/libft.hpp"
#include "../Errno/errno.hpp"
#include "../Errno/errno_internal.hpp"
#include "class_nullptr.hpp"

ft_string::ft_string() noexcept
    : _data(ft_nullptr)
    , _length(0)
    , _capacity(0)
    , _error_code(0)
    , _system_error_code(FT_SYS_ERR_SUCCESS)
    , _mutex()
{
    int previous_errno;

    previous_errno = ft_errno;
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    this->set_system_error_unlocked(FT_SYS_ERR_SUCCESS);
    ft_set_errno_locked(previous_errno);
    return ;
}

ft_string::ft_string(const char* init_str) noexcept
    : _data(ft_nullptr)
    , _length(0)
    , _capacity(0)
    , _error_code(0)
    , _system_error_code(FT_SYS_ERR_SUCCESS)
    , _mutex()
{
    int previous_errno;

    previous_errno = ft_errno;
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    this->set_system_error_unlocked(FT_SYS_ERR_SUCCESS);
    if (init_str)
        this->assign(init_str, ft_strlen_size_t(init_str));
    ft_set_errno_locked(previous_errno);
    return ;
}

ft_string::ft_string(size_t count, char character) noexcept
    : _data(ft_nullptr)
    , _length(0)
    , _capacity(0)
    , _error_code(0)
    , _system_error_code(FT_SYS_ERR_SUCCESS)
    , _mutex()
{
    int previous_errno;

    previous_errno = ft_errno;
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    this->set_system_error_unlocked(FT_SYS_ERR_SUCCESS);
    this->assign(count, character);
    ft_set_errno_locked(previous_errno);
    return ;
}

ft_string::ft_string(const ft_string& other) noexcept
    : _data(ft_nullptr)
    , _length(0)
    , _capacity(0)
    , _error_code(0)
    , _system_error_code(FT_SYS_ERR_SUCCESS)
    , _mutex()
{
    ft_string::mutex_guard other_guard;
    int lock_error;
    int other_error;
    int previous_errno;

    previous_errno = ft_errno;
    lock_error = other.lock_self(other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->_length = 0;
        this->_capacity = 0;
        this->set_system_error(lock_error);
        ft_set_errno_locked(previous_errno);
        return ;
    }
    this->_length = other._length;
    this->_capacity = other._capacity;
    other_error = other.get_error();
    this->_system_error_code = other._system_error_code;
    if (other_error != FT_ERR_SUCCESSS)
    {
        this->_length = 0;
        this->_capacity = 0;
        this->set_error_unlocked(other_error);
        this->set_system_error_unlocked(FT_SYS_ERR_SUCCESS);
        ft_set_errno_locked(previous_errno);
        return ;
    }
    if (other._data)
    {
        this->_data = static_cast<char*>(cma_calloc(this->_capacity + 1, sizeof(char)));
        if (!this->_data)
        {
            this->_length = 0;
            this->_capacity = 0;
            this->set_system_error(FT_SYS_ERR_NO_MEMORY);
            ft_set_errno_locked(previous_errno);
            return ;
        }
        ft_memcpy(this->_data, other._data, this->_length + 1);
    }
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    this->set_system_error_unlocked(FT_SYS_ERR_SUCCESS);
    ft_set_errno_locked(previous_errno);
    return ;
}

ft_string::ft_string(ft_string&& other) noexcept
    : _data(ft_nullptr)
    , _length(0)
    , _capacity(0)
    , _error_code(0)
    , _mutex()
{
    ft_string::mutex_guard other_guard;
    int lock_error;
    int previous_errno;

    previous_errno = ft_errno;
    lock_error = other.lock_self(other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->_data = ft_nullptr;
        this->_length = 0;
        this->_capacity = 0;
        this->set_error_unlocked(lock_error);
        this->set_system_error(lock_error);
        ft_set_errno_locked(previous_errno);
        return ;
    }
    this->_data = ft_nullptr;
    this->_length = 0;
    this->_capacity = 0;
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    this->_system_error_code = FT_SYS_ERR_SUCCESS;
    this->move_unlocked(other);
    this->set_error(FT_ERR_SUCCESSS);
    this->set_system_error_unlocked(FT_SYS_ERR_SUCCESS);
    ft_set_errno_locked(previous_errno);
    return ;
}

ft_string& ft_string::operator=(const ft_string& other) noexcept
{
    ft_string::mutex_guard self_guard;
    ft_string::mutex_guard other_guard;
    int lock_error;
    int other_error;
    int previous_errno;

    previous_errno = ft_errno;
    if (this == &other)
    {
        this->set_error(other.get_error());
        ft_set_errno_locked(previous_errno);
        return (*this);
    }
    lock_error = ft_string::lock_pair(*this, other, self_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_system_error(lock_error);
        ft_set_errno_locked(previous_errno);
        return (*this);
    }
    cma_free(this->_data);
    this->_data = ft_nullptr;
    this->_length = other._length;
    this->_capacity = other._capacity;
    other_error = other.get_error();
    this->set_error_unlocked(other_error);
    this->_system_error_code = other._system_error_code;
    if (other_error != FT_ERR_SUCCESSS)
    {
        this->_length = 0;
        this->_capacity = 0;
        if (other_guard.owns_lock())
            other_guard.unlock();
        this->set_system_error_unlocked(FT_SYS_ERR_SUCCESS);
        ft_set_errno_locked(previous_errno);
        return (*this);
    }
    if (other._data)
    {
        this->_data = static_cast<char*>(cma_calloc(this->_capacity + 1, sizeof(char)));
        if (!this->_data)
        {
            this->_length = 0;
            this->_capacity = 0;
            this->set_system_error(FT_SYS_ERR_NO_MEMORY);
            if (other_guard.owns_lock())
                other_guard.unlock();
            ft_set_errno_locked(previous_errno);
            return (*this);
        }
        ft_memcpy(this->_data, other._data, this->_length + 1);
    }
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    this->_system_error_code = other._system_error_code;
    if (other_guard.owns_lock())
        other_guard.unlock();
    this->set_error(FT_ERR_SUCCESSS);
    this->set_system_error(FT_SYS_ERR_SUCCESS);
    ft_set_errno_locked(previous_errno);
    return (*this);
}

ft_string& ft_string::operator=(const char* other) noexcept
{
    ft_string::mutex_guard guard;
    int lock_error;
    int previous_errno;

    previous_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_system_error(lock_error);
        ft_set_errno_locked(previous_errno);
        return (*this);
    }
    cma_free(this->_data);
    this->_data = ft_nullptr;
    this->_length = 0;
    this->_capacity = 0;
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    this->set_system_error_unlocked(FT_SYS_ERR_SUCCESS);
    if (other)
    {
        this->_length = ft_strlen_size_t(other);
        this->_capacity = this->_length;
        this->_data = static_cast<char*>(cma_calloc(this->_capacity + 1, sizeof(char)));
        if (!this->_data)
        {
            this->_length = 0;
            this->_capacity = 0;
            this->set_system_error(FT_SYS_ERR_NO_MEMORY);
            ft_set_errno_locked(previous_errno);
            return (*this);
        }
        ft_memcpy(this->_data, other, this->_length + 1);
    }
    this->set_error(FT_ERR_SUCCESSS);
    this->set_system_error_unlocked(FT_SYS_ERR_SUCCESS);
    ft_set_errno_locked(previous_errno);
    return (*this);
}

ft_string& ft_string::operator=(ft_string&& other) noexcept
{
    ft_string::mutex_guard self_guard;
    ft_string::mutex_guard other_guard;
    int lock_error;
    int previous_errno;

    previous_errno = ft_errno;
    this->set_error(FT_ERR_SUCCESSS);
    if (this == &other)
    {
        ft_set_errno_locked(previous_errno);
        return (*this);
    }
    lock_error = ft_string::lock_pair(*this, other, self_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_system_error(lock_error);
        ft_set_errno_locked(previous_errno);
        return (*this);
    }
    if (this->_error_code != FT_ERR_SUCCESSS)
    {
        if (other_guard.owns_lock())
            other_guard.unlock();
        ft_set_errno_locked(previous_errno);
        return (*this);
    }
    this->move_unlocked(other);
    if (other_guard.owns_lock())
        other_guard.unlock();
    this->set_system_error_unlocked(FT_SYS_ERR_SUCCESS);
    ft_set_errno_locked(previous_errno);
    return (*this);
}

ft_string::~ft_string()
{
    int previous_errno;

    previous_errno = ft_errno;
    cma_free(this->_data);
    this->_data = ft_nullptr;
    this->_length = 0;
    this->_capacity = 0;
    ft_set_errno_locked(previous_errno);
    return ;
}

void *ft_string::operator new(size_t size) noexcept
{
    void* ptr = cma_malloc(size);
    if (!ptr)
        return (ft_nullptr);
    return (ptr);
}

void ft_string::operator delete(void* ptr) noexcept
{
    cma_free(ptr);
    return ;
}

void *ft_string::operator new[](size_t size) noexcept
{
    void* ptr = cma_malloc(size);
    if (!ptr)
        return (ft_nullptr);
    return (ptr);
}

void ft_string::operator delete[](void* ptr) noexcept
{
    cma_free(ptr);
    return ;
}

ft_string::ft_string(int error_code) noexcept
    : _data(ft_nullptr)
    , _length(0)
    , _capacity(0)
    , _error_code(error_code)
    , _system_error_code(FT_SYS_ERR_SUCCESS)
    , _mutex()
{
    int previous_errno;

    previous_errno = ft_errno;
    this->set_error_unlocked(error_code);
    this->set_system_error_unlocked(FT_SYS_ERR_SUCCESS);
    ft_set_errno_locked(previous_errno);
    return ;
}
