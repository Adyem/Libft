#ifndef FT_CIRCULAR_BUFFER_HPP
#define FT_CIRCULAR_BUFFER_HPP

#include "constructor.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread.hpp"
#include <cstddef>
#include "move.hpp"

template <typename ElementType>
class ft_circular_buffer
{
    private:
        ElementType* _buffer;
        size_t       _capacity;
        size_t       _head;
        size_t       _tail;
        size_t       _size;
        mutable pt_recursive_mutex* _mutex;

        void destroy_elements_locked();
        void release_buffer_locked();
        int  lock_internal(bool *lock_acquired) const;
        int  unlock_internal(bool lock_acquired) const;
        int  prepare_thread_safety();
        void teardown_thread_safety();

    public:
        explicit ft_circular_buffer(size_t capacity);
        ~ft_circular_buffer();

        ft_circular_buffer(const ft_circular_buffer&) = delete;
        ft_circular_buffer& operator=(const ft_circular_buffer&) = delete;

        ft_circular_buffer(ft_circular_buffer&& other) noexcept;
        ft_circular_buffer& operator=(ft_circular_buffer&& other) noexcept;

        void push(const ElementType& value);
        void push(ElementType&& value);
        ElementType pop();

        bool is_full() const;
        bool is_empty() const;
        size_t size() const;
        size_t capacity() const;

        int  enable_thread_safety();
        void disable_thread_safety();
        bool is_thread_safe() const;
        int  lock(bool *lock_acquired) const;
        void unlock(bool lock_acquired) const;

        void clear();

    #ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex* get_mutex_for_validation() const noexcept;
    #endif
};

template <typename ElementType>
ft_circular_buffer<ElementType>::ft_circular_buffer(size_t capacity)
    : _buffer(ft_nullptr), _capacity(capacity), _head(0), _tail(0), _size(0), _mutex(ft_nullptr)
{
    if (capacity == 0)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESS);
        return ;
    }
    _buffer = static_cast<ElementType*>(cma_malloc(sizeof(ElementType) * capacity));
    if (_buffer == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        _capacity = 0;
        return ;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

template <typename ElementType>
ft_circular_buffer<ElementType>::~ft_circular_buffer()
{
    bool lock_acquired;
    int  lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error == FT_ERR_SUCCESS)
    {
        this->destroy_elements_locked();
        if (this->_buffer != ft_nullptr)
        {
            cma_free(this->_buffer);
            this->_buffer = ft_nullptr;
        }
        this->_capacity = 0;
        this->_head = 0;
        this->_tail = 0;
        this->_size = 0;
        int unlock_error = this->unlock_internal(lock_acquired);
        if (unlock_error != FT_ERR_SUCCESS)
            ft_global_error_stack_push(unlock_error);
        else
            ft_global_error_stack_push(FT_ERR_SUCCESS);
    }
    else
    {
        ft_global_error_stack_push(lock_error);
    }
    this->teardown_thread_safety();
    return ;
}

template <typename ElementType>
ft_circular_buffer<ElementType>::ft_circular_buffer(ft_circular_buffer<ElementType>&& other) noexcept
    : _buffer(ft_nullptr), _capacity(0), _head(0), _tail(0), _size(0), _mutex(ft_nullptr)
{
    bool other_thread_safe;
    bool lock_acquired;
    int  lock_error;

    other_thread_safe = (other._mutex != ft_nullptr);
    lock_acquired = false;
    lock_error = other.lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    this->_buffer = other._buffer;
    this->_capacity = other._capacity;
    this->_head = other._head;
    this->_tail = other._tail;
    this->_size = other._size;
    other._buffer = ft_nullptr;
    other._capacity = 0;
    other._head = 0;
    other._tail = 0;
    other._size = 0;
    other.unlock_internal(lock_acquired);
    other.teardown_thread_safety();
    if (other_thread_safe)
    {
        int enable_error;

        enable_error = this->enable_thread_safety();
        if (enable_error != FT_ERR_SUCCESS)
        {
            ft_global_error_stack_push(enable_error);
            return ;
        }
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

template <typename ElementType>
ft_circular_buffer<ElementType>& ft_circular_buffer<ElementType>::operator=(ft_circular_buffer<ElementType>&& other) noexcept
{
    if (this == &other)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESS);
        return (*this);
    }
    bool other_thread_safe;
    bool lock_acquired;
    int  lock_error;
    bool other_lock_acquired;
    int  other_lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return (*this);
    }
    this->destroy_elements_locked();
    this->release_buffer_locked();
    this->_buffer = ft_nullptr;
    this->_capacity = 0;
    this->_head = 0;
    this->_tail = 0;
    this->_size = 0;
    this->unlock_internal(lock_acquired);
    this->teardown_thread_safety();
    other_lock_acquired = false;
    other_lock_error = other.lock_internal(&other_lock_acquired);
    if (other_lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(other_lock_error);
        return (*this);
    }
    this->_buffer = other._buffer;
    this->_capacity = other._capacity;
    this->_head = other._head;
    this->_tail = other._tail;
    this->_size = other._size;
    other._buffer = ft_nullptr;
    other._capacity = 0;
    other._head = 0;
    other._tail = 0;
    other._size = 0;
    other.unlock_internal(other_lock_acquired);
    other_thread_safe = (other._mutex != ft_nullptr);
    other.teardown_thread_safety();
    if (other_thread_safe)
    {
        int enable_error;

        enable_error = this->enable_thread_safety();
        if (enable_error != FT_ERR_SUCCESS)
        {
            ft_global_error_stack_push(enable_error);
            return (*this);
        }
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (*this);
}


template <typename ElementType>
int ft_circular_buffer<ElementType>::enable_thread_safety()
{
    int result;

    if (this->_mutex != ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    result = this->prepare_thread_safety();
    ft_global_error_stack_push(result);
    return (result);
}

template <typename ElementType>
void ft_circular_buffer<ElementType>::disable_thread_safety()
{
    this->teardown_thread_safety();
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

template <typename ElementType>
bool ft_circular_buffer<ElementType>::is_thread_safe() const
{
    bool enabled;

    enabled = (this->_mutex != ft_nullptr);
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (enabled);
}

template <typename ElementType>
int ft_circular_buffer<ElementType>::lock(bool *lock_acquired) const
{
    int result;

    result = this->lock_internal(lock_acquired);
    if (result != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(result);
        return (-1);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (0);
}

template <typename ElementType>
void ft_circular_buffer<ElementType>::unlock(bool lock_acquired) const
{
    int unlock_error;

    unlock_error = this->unlock_internal(lock_acquired);
    ft_global_error_stack_push(unlock_error);
    return ;
}


template <typename ElementType>
void ft_circular_buffer<ElementType>::push(const ElementType& value)
{
    bool lock_acquired;
    int  lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    if (this->_size == this->_capacity)
    {
        ft_global_error_stack_push(FT_ERR_FULL);
        this->unlock_internal(lock_acquired);
        return ;
    }
    construct_at(&this->_buffer[this->_tail], value);
    if (this->_capacity != 0)
        this->_tail = (this->_tail + 1) % this->_capacity;
    this->_size += 1;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename ElementType>
void ft_circular_buffer<ElementType>::push(ElementType&& value)
{
    bool lock_acquired;
    int  lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    if (this->_size == this->_capacity)
    {
        ft_global_error_stack_push(FT_ERR_FULL);
        this->unlock_internal(lock_acquired);
        return ;
    }
    construct_at(&this->_buffer[this->_tail], ft_move(value));
    if (this->_capacity != 0)
        this->_tail = (this->_tail + 1) % this->_capacity;
    this->_size += 1;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename ElementType>
ElementType ft_circular_buffer<ElementType>::pop()
{
    bool        lock_acquired;
    ElementType value;
    int         lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return (ElementType());
    }
    if (this->_size == 0)
    {
        ft_global_error_stack_push(FT_ERR_EMPTY);
        this->unlock_internal(lock_acquired);
        return (ElementType());
    }
    value = ft_move(this->_buffer[this->_head]);
    destroy_at(&this->_buffer[this->_head]);
    if (this->_capacity != 0)
        this->_head = (this->_head + 1) % this->_capacity;
    this->_size -= 1;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (value);
}

template <typename ElementType>
bool ft_circular_buffer<ElementType>::is_full() const
{
    bool lock_acquired;
    bool result;
    int  lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return (false);
    }
    result = (this->_size == this->_capacity);
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (result);
}

template <typename ElementType>
bool ft_circular_buffer<ElementType>::is_empty() const
{
    bool lock_acquired;
    bool result;
    int  lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return (true);
    }
    result = (this->_size == 0);
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (result);
}

template <typename ElementType>
size_t ft_circular_buffer<ElementType>::size() const
{
    bool   lock_acquired;
    size_t current_size;
    int    lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return (0);
    }
    current_size = this->_size;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (current_size);
}

template <typename ElementType>
size_t ft_circular_buffer<ElementType>::capacity() const
{
    bool   lock_acquired;
    size_t current_capacity;
    int    lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return (0);
    }
    current_capacity = this->_capacity;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (current_capacity);
}

template <typename ElementType>
void ft_circular_buffer<ElementType>::clear()
{
    bool lock_acquired;
    int  lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    this->destroy_elements_locked();
    this->_head = 0;
    this->_tail = 0;
    this->_size = 0;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename ElementType>
void ft_circular_buffer<ElementType>::destroy_elements_locked()
{
    if (this->_capacity == 0)
        return ;
    size_t index = 0;
    while (index < this->_size)
    {
        destroy_at(&this->_buffer[(this->_head + index) % this->_capacity]);
        ++index;
    }
    return ;
}

template <typename ElementType>
void ft_circular_buffer<ElementType>::release_buffer_locked()
{
    if (this->_buffer != ft_nullptr)
    {
        cma_free(this->_buffer);
        this->_buffer = ft_nullptr;
    }
    return ;
}

template <typename ElementType>
int ft_circular_buffer<ElementType>::lock_internal(bool *lock_acquired) const
{
    if (lock_acquired)
        *lock_acquired = false;
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    int result = pt_recursive_mutex_lock_with_error(*this->_mutex);
    if (result != FT_ERR_SUCCESS)
        return (result);
    if (lock_acquired)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

template <typename ElementType>
int ft_circular_buffer<ElementType>::unlock_internal(bool lock_acquired) const
{
    if (!lock_acquired || this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (pt_recursive_mutex_unlock_with_error(*this->_mutex));
}

template <typename ElementType>
int ft_circular_buffer<ElementType>::prepare_thread_safety()
{
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (pt_recursive_mutex_create_with_error(&this->_mutex));
}

template <typename ElementType>
void ft_circular_buffer<ElementType>::teardown_thread_safety()
{
    pt_recursive_mutex_destroy(&this->_mutex);
    return ;
}

#ifdef LIBFT_TEST_BUILD
template <typename ElementType>
pt_recursive_mutex* ft_circular_buffer<ElementType>::get_mutex_for_validation() const noexcept
{
    return (this->_mutex);
}
#endif

#endif
