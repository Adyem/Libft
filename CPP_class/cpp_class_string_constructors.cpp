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
    , _mutex()
{
    this->push_error_unlocked(FT_ERR_SUCCESSS);
    return ;
}

ft_string::ft_string(const char* initial_string) noexcept
    : _data(ft_nullptr)
    , _length(0)
    , _capacity(0)
    , _mutex()
{
    this->push_error_unlocked(FT_ERR_SUCCESSS);
    if (initial_string)
        this->assign(initial_string, ft_strlen_size_t(initial_string));
    return ;
}

ft_string::ft_string(size_t count, char character) noexcept
    : _data(ft_nullptr)
    , _length(0)
    , _capacity(0)
    , _mutex()
{
    this->push_error_unlocked(FT_ERR_SUCCESSS);
    this->assign(count, character);
    return ;
}

ft_string::ft_string(const ft_string& other) noexcept
    : _data(ft_nullptr)
    , _length(0)
    , _capacity(0)
    , _mutex()
{
    ft_string::mutex_guard other_guard;
    int lock_error;
    int other_error;

    lock_error = other.lock_self(other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->_length = 0;
        this->_capacity = 0;
        this->push_error_unlocked(lock_error);
        return ;
    }
    this->_length = other._length;
    this->_capacity = other._capacity;
    other_error = ft_string::last_operation_error();
    if (other_error != FT_ERR_SUCCESSS)
    {
        this->_length = 0;
        this->_capacity = 0;
        this->push_error_unlocked(other_error);
        return ;
    }
    if (other._data)
    {
        this->_data = static_cast<char*>(cma_calloc(this->_capacity + 1, sizeof(char)));
        if (!this->_data)
        {
            this->_length = 0;
            this->_capacity = 0;
            this->push_error_unlocked(FT_ERR_SYSTEM);
            return ;
        }
        ft_memcpy(this->_data, other._data, this->_length + 1);
    }
    if (!this->_data)
        this->_capacity = 0;
    this->push_error_unlocked(FT_ERR_SUCCESSS);
    return ;
}

ft_string::ft_string(ft_string&& other) noexcept
    : _data(ft_nullptr)
    , _length(0)
    , _capacity(0)
    , _mutex()
{
    ft_string::mutex_guard other_guard;
    int lock_error;

    lock_error = other.lock_self(other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->_data = ft_nullptr;
        this->_length = 0;
        this->_capacity = 0;
        this->push_error_unlocked(lock_error);
        return ;
    }
    this->_data = ft_nullptr;
    this->_length = 0;
    this->_capacity = 0;
    this->push_error_unlocked(FT_ERR_SUCCESSS);
    this->move_unlocked(other);
    this->push_error(FT_ERR_SUCCESSS);
    return ;
}

ft_string& ft_string::operator=(const ft_string& other) noexcept
{
    ft_string::mutex_guard self_guard;
    ft_string::mutex_guard other_guard;
    int lock_error;
    int other_error;

    if (this == &other)
    {
        this->push_error(ft_string::last_operation_error());
        return (*this);
    }
    lock_error = ft_string::lock_pair(*this, other, self_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->push_error_unlocked(lock_error);
        return (*this);
    }
    cma_free(this->_data);
    this->_data = ft_nullptr;
    this->_length = other._length;
    this->_capacity = other._capacity;
    other_error = ft_string::last_operation_error();
    this->push_error_unlocked(other_error);
    if (other_error != FT_ERR_SUCCESSS)
    {
        this->_length = 0;
        this->_capacity = 0;
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
            this->push_error_unlocked(FT_ERR_SYSTEM);
            if (other_guard.owns_lock())
                other_guard.unlock();
            return (*this);
        }
        ft_memcpy(this->_data, other._data, this->_length + 1);
    }
    if (!this->_data)
        this->_capacity = 0;
    this->push_error_unlocked(FT_ERR_SUCCESSS);
    if (other_guard.owns_lock())
        other_guard.unlock();
    this->push_error(FT_ERR_SUCCESSS);
    return (*this);
}

ft_string& ft_string::operator=(const char* other) noexcept
{
    ft_string::mutex_guard guard;
    int lock_error;

    lock_error = this->lock_self(guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->push_error_unlocked(lock_error);
        return (*this);
    }
    cma_free(this->_data);
    this->_data = ft_nullptr;
    this->_length = 0;
    this->_capacity = 0;
    this->push_error_unlocked(FT_ERR_SUCCESSS);
    if (other)
    {
        this->_length = ft_strlen_size_t(other);
        this->_capacity = this->_length;
        this->_data = static_cast<char*>(cma_calloc(this->_capacity + 1, sizeof(char)));
        if (!this->_data)
        {
            this->_length = 0;
            this->_capacity = 0;
            this->push_error_unlocked(FT_ERR_SYSTEM);
            return (*this);
        }
        ft_memcpy(this->_data, other, this->_length + 1);
    }
    this->push_error(FT_ERR_SUCCESSS);
    return (*this);
}

ft_string& ft_string::operator=(ft_string&& other) noexcept
{
    ft_string::mutex_guard self_guard;
    ft_string::mutex_guard other_guard;
    int lock_error;

    this->push_error(FT_ERR_SUCCESSS);
    if (this == &other)
        return (*this);
    lock_error = ft_string::lock_pair(*this, other, self_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->push_error_unlocked(lock_error);
        return (*this);
    }
    if (ft_string::last_operation_error() != FT_ERR_SUCCESSS)
    {
        if (other_guard.owns_lock())
            other_guard.unlock();
        return (*this);
    }
    this->move_unlocked(other);
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
    void* pointer = cma_malloc(size);
    if (!pointer)
        return (ft_nullptr);
    return (pointer);
}

void ft_string::operator delete(void* pointer) noexcept
{
    cma_free(pointer);
    return ;
}

void *ft_string::operator new[](size_t size) noexcept
{
    void* pointer = cma_malloc(size);
    if (!pointer)
        return (ft_nullptr);
    return (pointer);
}

void ft_string::operator delete[](void* pointer) noexcept
{
    cma_free(pointer);
    return ;
}

ft_string::ft_string(int error_code) noexcept
    : _data(ft_nullptr)
    , _length(0)
    , _capacity(0)
    , _mutex()
{
    this->push_error_unlocked(error_code);
    return ;
}
