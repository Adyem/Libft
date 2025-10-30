#include "class_string_class.hpp"
#include "../CMA/CMA.hpp"
#include "../Libft/libft.hpp"
#include "../Errno/errno.hpp"
#include "class_nullptr.hpp"

ft_string::ft_string() noexcept
    : _data(ft_nullptr), _length(0), _capacity(0), _error_code(0)
{
    this->set_error_unlocked(ER_SUCCESS);
    return ;
}

ft_string::ft_string(const char* init_str) noexcept
    : _data(ft_nullptr), _length(0), _capacity(0), _error_code(0)
{
    this->set_error_unlocked(ER_SUCCESS);
    if (init_str)
        this->assign(init_str, ft_strlen_size_t(init_str));
    return ;
}

ft_string::ft_string(size_t count, char character) noexcept
    : _data(ft_nullptr), _length(0), _capacity(0), _error_code(0)
{
    this->set_error_unlocked(ER_SUCCESS);
    this->assign(count, character);
    return ;
}

ft_string::ft_string(const ft_string& other) noexcept
    : _data(ft_nullptr), _length(0), _capacity(0), _error_code(0)
{
    ft_string::mutex_guard other_guard;
    int entry_errno;
    int lock_error;

    entry_errno = ft_errno;
    lock_error = other.lock_self(other_guard);
    if (lock_error != ER_SUCCESS)
    {
        this->_length = 0;
        this->_capacity = 0;
        this->set_error_unlocked(lock_error);
        ft_errno = entry_errno;
        return ;
    }
    this->_length = other._length;
    this->_capacity = other._capacity;
    this->_error_code = other._error_code;
    if (other._error_code != ER_SUCCESS)
    {
        this->_length = 0;
        this->_capacity = 0;
        this->set_error_unlocked(other._error_code);
        ft_string::restore_errno(other_guard, entry_errno);
        return ;
    }
    if (other._data)
    {
        this->_data = static_cast<char*>(cma_calloc(this->_capacity + 1, sizeof(char)));
        if (!this->_data)
        {
            this->_length = 0;
            this->_capacity = 0;
            this->set_error_unlocked(FT_ERR_NO_MEMORY);
            ft_string::restore_errno(other_guard, entry_errno);
            return ;
        }
        ft_memcpy(this->_data, other._data, this->_length + 1);
    }
    this->set_error_unlocked(ER_SUCCESS);
    ft_string::restore_errno(other_guard, entry_errno);
    return ;
}

ft_string::ft_string(ft_string&& other) noexcept
    : _data(ft_nullptr),
      _length(0),
      _capacity(0),
      _error_code(0)
{
    ft_string::mutex_guard other_guard;
    int entry_errno;
    int lock_error;

    entry_errno = ft_errno;
    lock_error = other.lock_self(other_guard);
    if (lock_error != ER_SUCCESS)
    {
        this->_data = ft_nullptr;
        this->_length = 0;
        this->_capacity = 0;
        this->_error_code = lock_error;
        ft_errno = entry_errno;
        return ;
    }
    this->_data = ft_nullptr;
    this->_length = 0;
    this->_capacity = 0;
    this->_error_code = ER_SUCCESS;
    this->move_unlocked(other);
    ft_string::restore_errno(other_guard, entry_errno);
    return ;
}

ft_string& ft_string::operator=(const ft_string& other) noexcept
{
    ft_string::mutex_guard self_guard;
    ft_string::mutex_guard other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
    {
        this->set_error(other.get_error());
        return (*this);
    }
    entry_errno = ft_errno;
    lock_error = ft_string::lock_pair(*this, other, self_guard, other_guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error_unlocked(lock_error);
        ft_errno = entry_errno;
        return (*this);
    }
    cma_free(this->_data);
    this->_data = ft_nullptr;
    this->_length = other._length;
    this->_capacity = other._capacity;
    this->set_error_unlocked(other._error_code);
    if (other._error_code != ER_SUCCESS)
    {
        this->_length = 0;
        this->_capacity = 0;
        ft_string::restore_errno(self_guard, entry_errno);
        if (other_guard.owns_lock())
            other_guard.unlock();
        return (*this);
    }
    if (other._data)
    {
        this->_data = static_cast<char*>(cma_calloc(this->_capacity + 1, sizeof(char)));
        if (!this->_data)
        {
            this->_length = 0;
            this->_capacity = 0;
            this->set_error_unlocked(FT_ERR_NO_MEMORY);
            ft_string::restore_errno(self_guard, entry_errno);
            if (other_guard.owns_lock())
                other_guard.unlock();
            return (*this);
        }
        ft_memcpy(this->_data, other._data, this->_length + 1);
    }
    this->set_error_unlocked(ER_SUCCESS);
    ft_string::restore_errno(self_guard, entry_errno);
    if (other_guard.owns_lock())
        other_guard.unlock();
    return (*this);
}

ft_string& ft_string::operator=(const char* other) noexcept
{
    ft_string::mutex_guard guard;
    int entry_errno;
    int lock_error;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error_unlocked(lock_error);
        ft_errno = entry_errno;
        return (*this);
    }
    cma_free(this->_data);
    this->_data = ft_nullptr;
    this->_length = 0;
    this->_capacity = 0;
    this->set_error_unlocked(ER_SUCCESS);
    if (other)
    {
        this->_length = ft_strlen_size_t(other);
        this->_capacity = this->_length;
        this->_data = static_cast<char*>(cma_calloc(this->_capacity + 1, sizeof(char)));
        if (!this->_data)
        {
            this->_length = 0;
            this->_capacity = 0;
            this->set_error_unlocked(FT_ERR_NO_MEMORY);
            ft_string::restore_errno(guard, entry_errno);
            return (*this);
        }
        ft_memcpy(this->_data, other, this->_length + 1);
    }
    ft_string::restore_errno(guard, entry_errno);
    return (*this);
}

ft_string& ft_string::operator=(ft_string&& other) noexcept
{
    ft_string::mutex_guard self_guard;
    ft_string::mutex_guard other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = ft_string::lock_pair(*this, other, self_guard, other_guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error_unlocked(lock_error);
        ft_errno = entry_errno;
        return (*this);
    }
    if (this->_error_code != ER_SUCCESS)
    {
        ft_string::restore_errno(self_guard, entry_errno);
        if (other_guard.owns_lock())
            other_guard.unlock();
        return (*this);
    }
    this->move_unlocked(other);
    ft_string::restore_errno(self_guard, entry_errno);
    if (other_guard.owns_lock())
        other_guard.unlock();
    return (*this);
}

ft_string::~ft_string()
{
    cma_free(this->_data);
    this->_data = ft_nullptr;
    this->_length = 0;
    this->_capacity = 0;
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
{
    this->set_error_unlocked(error_code);
    return ;
}
