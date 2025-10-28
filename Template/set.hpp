#ifndef FT_SET_HPP
#define FT_SET_HPP

#include "constructor.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"
#include <cstddef>
#include <utility>
#include "move.hpp"

template <typename ElementType>
class ft_set
{
    private:
        ElementType*         _data;
        size_t               _capacity;
        size_t               _size;
        mutable int          _error_code;
        mutable pt_mutex*    _mutex;
        bool                 _thread_safe_enabled;

        void    set_error(int error) const;
        bool    ensure_capacity(size_t desired_capacity);
        size_t  find_index(const ElementType& value) const;
        size_t  lower_bound(const ElementType& value) const;
        int     lock_internal(bool *lock_acquired) const;
        void    unlock_internal(bool lock_acquired) const;
        void    teardown_thread_safety();

    public:
        ft_set(size_t initial_capacity = 0);
        ~ft_set();

        ft_set(const ft_set&) = delete;
        ft_set& operator=(const ft_set&) = delete;

        ft_set(ft_set&& other) noexcept;
        ft_set& operator=(ft_set&& other) noexcept;

        int     enable_thread_safety();
        void    disable_thread_safety();
        bool    is_thread_safe() const;
        int     lock(bool *lock_acquired) const;
        void    unlock(bool lock_acquired) const;

        void insert(const ElementType& value);
        void insert(ElementType&& value);
        ElementType* find(const ElementType& value);
        const ElementType* find(const ElementType& value) const;
        void remove(const ElementType& value);
        size_t size() const;
        bool empty() const;
        int get_error() const;
        const char* get_error_str() const;
        void clear();
};

template <typename ElementType>
ft_set<ElementType>::ft_set(size_t initial_capacity)
    : _data(ft_nullptr), _capacity(0), _size(0), _error_code(ER_SUCCESS),
      _mutex(ft_nullptr), _thread_safe_enabled(false)
{
    if (initial_capacity > 0)
    {
        this->_data = static_cast<ElementType*>(cma_malloc(sizeof(ElementType) * initial_capacity));
        if (this->_data == ft_nullptr)
        {
            this->set_error(FT_ERR_NO_MEMORY);
            return ;
        }
        this->_capacity = initial_capacity;
    }
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ElementType>
ft_set<ElementType>::~ft_set()
{
    this->clear();
    if (this->_data != ft_nullptr)
        cma_free(this->_data);
    this->teardown_thread_safety();
    return ;
}

template <typename ElementType>
ft_set<ElementType>::ft_set(ft_set&& other) noexcept
    : _data(other._data), _capacity(other._capacity), _size(other._size),
      _error_code(other._error_code), _mutex(other._mutex),
      _thread_safe_enabled(other._thread_safe_enabled)
{
    other._data = ft_nullptr;
    other._capacity = 0;
    other._size = 0;
    other._error_code = ER_SUCCESS;
    other._mutex = ft_nullptr;
    other._thread_safe_enabled = false;
    this->set_error(this->_error_code);
    return ;
}

template <typename ElementType>
ft_set<ElementType>& ft_set<ElementType>::operator=(ft_set&& other) noexcept
{
    if (this != &other)
    {
        this->clear();
        if (this->_data != ft_nullptr)
            cma_free(this->_data);
        this->teardown_thread_safety();
        this->_data = other._data;
        this->_capacity = other._capacity;
        this->_size = other._size;
        this->_error_code = other._error_code;
        this->_mutex = other._mutex;
        this->_thread_safe_enabled = other._thread_safe_enabled;
        other._data = ft_nullptr;
        other._capacity = 0;
        other._size = 0;
        other._error_code = ER_SUCCESS;
        other._mutex = ft_nullptr;
        other._thread_safe_enabled = false;
    }
    this->set_error(this->_error_code);
    return (*this);
}

template <typename ElementType>
void ft_set<ElementType>::set_error(int error) const
{
    this->_error_code = error;
    ft_errno = error;
    return ;
}

template <typename ElementType>
int ft_set<ElementType>::enable_thread_safety()
{
    void     *memory;
    pt_mutex *mutex_pointer;

    if (this->_thread_safe_enabled && this->_mutex != ft_nullptr)
    {
        this->set_error(ER_SUCCESS);
        return (0);
    }
    memory = cma_malloc(sizeof(pt_mutex));
    if (memory == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return (-1);
    }
    mutex_pointer = new(memory) pt_mutex();
    if (mutex_pointer->get_error() != ER_SUCCESS)
    {
        int mutex_error;

        mutex_error = mutex_pointer->get_error();
        mutex_pointer->~pt_mutex();
        cma_free(memory);
        this->set_error(mutex_error);
        return (-1);
    }
    this->_mutex = mutex_pointer;
    this->_thread_safe_enabled = true;
    this->set_error(ER_SUCCESS);
    return (0);
}

template <typename ElementType>
void ft_set<ElementType>::disable_thread_safety()
{
    this->teardown_thread_safety();
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ElementType>
bool ft_set<ElementType>::is_thread_safe() const
{
    bool enabled;

    enabled = (this->_thread_safe_enabled && this->_mutex != ft_nullptr);
    const_cast<ft_set<ElementType> *>(this)->set_error(ER_SUCCESS);
    return (enabled);
}

template <typename ElementType>
int ft_set<ElementType>::lock(bool *lock_acquired) const
{
    int result;

    result = this->lock_internal(lock_acquired);
    if (result != 0)
        const_cast<ft_set<ElementType> *>(this)->set_error(ft_errno);
    else
        const_cast<ft_set<ElementType> *>(this)->set_error(ER_SUCCESS);
    return (result);
}

template <typename ElementType>
void ft_set<ElementType>::unlock(bool lock_acquired) const
{
    int entry_errno;

    entry_errno = ft_errno;
    this->unlock_internal(lock_acquired);
    if (this->_mutex != ft_nullptr && this->_mutex->get_error() != ER_SUCCESS)
        const_cast<ft_set<ElementType> *>(this)->set_error(this->_mutex->get_error());
    else
    {
        ft_errno = entry_errno;
        const_cast<ft_set<ElementType> *>(this)->set_error(ft_errno);
    }
    return ;
}

template <typename ElementType>
bool ft_set<ElementType>::ensure_capacity(size_t desired_capacity)
{
    if (desired_capacity <= this->_capacity)
    {
        this->set_error(ER_SUCCESS);
        return (true);
    }
    size_t new_capacity;
    if (this->_capacity == 0)
        new_capacity = 1;
    else
        new_capacity = this->_capacity * 2;
    while (new_capacity < desired_capacity)
        new_capacity *= 2;
    ElementType* new_data = static_cast<ElementType*>(cma_malloc(sizeof(ElementType) * new_capacity));
    if (new_data == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return (false);
    }
    size_t index = 0;
    while (index < this->_size)
    {
        construct_at(&new_data[index], ft_move(this->_data[index]));
        destroy_at(&this->_data[index]);
        ++index;
    }
    if (this->_data != ft_nullptr)
        cma_free(this->_data);
    this->_data = new_data;
    this->_capacity = new_capacity;
    this->set_error(ER_SUCCESS);
    return (true);
}

template <typename ElementType>
size_t ft_set<ElementType>::find_index(const ElementType& value) const
{
    size_t left = 0;
    size_t right = this->_size;
    while (left < right)
    {
        size_t mid = left + (right - left) / 2;
        if (this->_data[mid] < value)
            left = mid + 1;
        else if (value < this->_data[mid])
            right = mid;
        else
            return (mid);
    }
    return (this->_size);
}

template <typename ElementType>
size_t ft_set<ElementType>::lower_bound(const ElementType& value) const
{
    size_t left = 0;
    size_t right = this->_size;
    while (left < right)
    {
        size_t mid = left + (right - left) / 2;
        if (this->_data[mid] < value)
            left = mid + 1;
        else
            right = mid;
    }
    return (left);
}

template <typename ElementType>
int ft_set<ElementType>::lock_internal(bool *lock_acquired) const
{
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (!this->_thread_safe_enabled || this->_mutex == ft_nullptr)
    {
        ft_errno = ER_SUCCESS;
        return (0);
    }
    this->_mutex->lock(THREAD_ID);
    if (this->_mutex->get_error() != ER_SUCCESS)
    {
        ft_errno = this->_mutex->get_error();
        return (-1);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    ft_errno = ER_SUCCESS;
    return (0);
}

template <typename ElementType>
void ft_set<ElementType>::unlock_internal(bool lock_acquired) const
{
    int entry_errno;

    if (!lock_acquired || this->_mutex == ft_nullptr)
        return ;
    entry_errno = ft_errno;
    this->_mutex->unlock(THREAD_ID);
    if (this->_mutex->get_error() != ER_SUCCESS)
    {
        ft_errno = this->_mutex->get_error();
        return ;
    }
    ft_errno = entry_errno;
    return ;
}

template <typename ElementType>
void ft_set<ElementType>::teardown_thread_safety()
{
    if (this->_mutex != ft_nullptr)
    {
        this->_mutex->~pt_mutex();
        cma_free(this->_mutex);
        this->_mutex = ft_nullptr;
    }
    this->_thread_safe_enabled = false;
    return ;
}

template <typename ElementType>
void ft_set<ElementType>::insert(const ElementType& value)
{
    bool   lock_acquired;
    size_t position;
    size_t index;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    position = lower_bound(value);
    if (position < this->_size && !(value < this->_data[position]) && !(this->_data[position] < value))
    {
        this->set_error(ER_SUCCESS);
        this->unlock_internal(lock_acquired);
        return ;
    }
    if (!ensure_capacity(this->_size + 1))
    {
        this->unlock_internal(lock_acquired);
        return ;
    }
    index = this->_size;
    while (index > position)
    {
        construct_at(&this->_data[index], ft_move(this->_data[index - 1]));
        destroy_at(&this->_data[index - 1]);
        --index;
    }
    construct_at(&this->_data[position], value);
    ++this->_size;
    this->set_error(ER_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename ElementType>
void ft_set<ElementType>::insert(ElementType&& value)
{
    bool   lock_acquired;
    size_t position;
    size_t index;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    position = lower_bound(value);
    if (position < this->_size && !(value < this->_data[position]) && !(this->_data[position] < value))
    {
        this->set_error(ER_SUCCESS);
        this->unlock_internal(lock_acquired);
        return ;
    }
    if (!ensure_capacity(this->_size + 1))
    {
        this->unlock_internal(lock_acquired);
        return ;
    }
    index = this->_size;
    while (index > position)
    {
        construct_at(&this->_data[index], ft_move(this->_data[index - 1]));
        destroy_at(&this->_data[index - 1]);
        --index;
    }
    construct_at(&this->_data[position], ft_move(value));
    ++this->_size;
    this->set_error(ER_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename ElementType>
ElementType* ft_set<ElementType>::find(const ElementType& value)
{
    bool        lock_acquired;
    size_t      index;
    ElementType *result;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return (ft_nullptr);
    }
    index = find_index(value);
    if (index == this->_size)
    {
        this->set_error(FT_ERR_NOT_FOUND);
        this->unlock_internal(lock_acquired);
        return (ft_nullptr);
    }
    result = &this->_data[index];
    this->set_error(ER_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (result);
}

template <typename ElementType>
const ElementType* ft_set<ElementType>::find(const ElementType& value) const
{
    bool               lock_acquired;
    size_t             index;
    const ElementType *result;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        const_cast<ft_set<ElementType> *>(this)->set_error(ft_errno);
        return (ft_nullptr);
    }
    index = find_index(value);
    if (index == this->_size)
    {
        const_cast<ft_set<ElementType> *>(this)->set_error(FT_ERR_NOT_FOUND);
        this->unlock_internal(lock_acquired);
        return (ft_nullptr);
    }
    result = &this->_data[index];
    const_cast<ft_set<ElementType> *>(this)->set_error(ER_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (result);
}

template <typename ElementType>
void ft_set<ElementType>::remove(const ElementType& value)
{
    bool   lock_acquired;
    size_t index;
    size_t current_index;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    index = find_index(value);
    if (index == this->_size)
    {
        this->set_error(FT_ERR_NOT_FOUND);
        this->unlock_internal(lock_acquired);
        return ;
    }
    destroy_at(&this->_data[index]);
    current_index = index;
    while (current_index + 1 < this->_size)
    {
        construct_at(&this->_data[current_index], ft_move(this->_data[current_index + 1]));
        destroy_at(&this->_data[current_index + 1]);
        ++current_index;
    }
    --this->_size;
    this->set_error(ER_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename ElementType>
size_t ft_set<ElementType>::size() const
{
    bool   lock_acquired;
    size_t current_size;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        const_cast<ft_set<ElementType> *>(this)->set_error(ft_errno);
        return (0);
    }
    current_size = this->_size;
    const_cast<ft_set<ElementType> *>(this)->set_error(ER_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (current_size);
}

template <typename ElementType>
bool ft_set<ElementType>::empty() const
{
    bool lock_acquired;
    bool result;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        const_cast<ft_set<ElementType> *>(this)->set_error(ft_errno);
        return (true);
    }
    result = (this->_size == 0);
    const_cast<ft_set<ElementType> *>(this)->set_error(ER_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (result);
}

template <typename ElementType>
int ft_set<ElementType>::get_error() const
{
    return (this->_error_code);
}

template <typename ElementType>
const char* ft_set<ElementType>::get_error_str() const
{
    return (ft_strerror(this->get_error()));
}

template <typename ElementType>
void ft_set<ElementType>::clear()
{
    bool   lock_acquired;
    size_t index;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    index = 0;
    while (index < this->_size)
    {
        destroy_at(&this->_data[index]);
        ++index;
    }
    this->_size = 0;
    this->set_error(ER_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

#endif
