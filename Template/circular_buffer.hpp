#ifndef FT_CIRCULAR_BUFFER_HPP
#define FT_CIRCULAR_BUFFER_HPP

#include "constructor.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Libft/libft.hpp"
#include <cstddef>
#include <utility>
#include "move.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"

template <typename ElementType>
class ft_circular_buffer
{
    private:
        ElementType*  _buffer;
        size_t        _capacity;
        size_t        _head;
        size_t        _tail;
        size_t        _size;
        mutable int   _error_code;
        mutable pt_mutex *_mutex;
        bool             _thread_safe_enabled;

        void set_error_unlocked(int error) const;
        void set_error(int error) const;
        void destroy_elements_locked();
        void release_buffer_locked();
        int  lock_internal(bool *lock_acquired) const;
        void unlock_internal(bool lock_acquired) const;
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

        int enable_thread_safety();
        void disable_thread_safety();
        bool is_thread_safe() const;
        int lock(bool *lock_acquired) const;
        void unlock(bool lock_acquired) const;

        int get_error() const;
        const char* get_error_str() const;

        void clear();
};

template <typename ElementType>
ft_circular_buffer<ElementType>::ft_circular_buffer(size_t capacity)
    : _buffer(ft_nullptr),
      _capacity(capacity),
      _head(0),
      _tail(0),
      _size(0),
      _error_code(FT_ER_SUCCESSS),
      _mutex(ft_nullptr),
      _thread_safe_enabled(false)
{
    if (capacity == 0)
    {
        this->set_error_unlocked(FT_ER_SUCCESSS);
        return ;
    }
    _buffer = static_cast<ElementType*>(cma_malloc(sizeof(ElementType) * capacity));
    if (_buffer == ft_nullptr)
    {
        this->set_error_unlocked(FT_ERR_NO_MEMORY);
        _capacity = 0;
        return ;
    }
    this->set_error_unlocked(FT_ER_SUCCESSS);
    return ;
}

template <typename ElementType>
ft_circular_buffer<ElementType>::~ft_circular_buffer()
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) == 0)
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
        this->set_error_unlocked(FT_ER_SUCCESSS);
        this->unlock_internal(lock_acquired);
    }
    else
        this->set_error_unlocked(ft_errno);
    this->teardown_thread_safety();
    return ;
}

template <typename ElementType>
ft_circular_buffer<ElementType>::ft_circular_buffer(ft_circular_buffer&& other) noexcept
    : _buffer(ft_nullptr),
      _capacity(0),
      _head(0),
      _tail(0),
      _size(0),
      _error_code(FT_ER_SUCCESSS),
      _mutex(ft_nullptr),
      _thread_safe_enabled(false)
{
    bool other_lock_acquired;
    pt_mutex *transferred_mutex;
    bool other_thread_safe;

    other_lock_acquired = false;
    transferred_mutex = ft_nullptr;
    other_thread_safe = false;
    if (other.lock_internal(&other_lock_acquired) != 0)
    {
        this->set_error_unlocked(ft_errno);
        return ;
    }
    transferred_mutex = other._mutex;
    other_thread_safe = (other._thread_safe_enabled && other._mutex != ft_nullptr);
    this->_buffer = other._buffer;
    this->_capacity = other._capacity;
    this->_head = other._head;
    this->_tail = other._tail;
    this->_size = other._size;
    this->_error_code = other._error_code;
    this->_mutex = ft_nullptr;
    this->_thread_safe_enabled = false;
    other._buffer = ft_nullptr;
    other._capacity = 0;
    other._head = 0;
    other._tail = 0;
    other._size = 0;
    other._error_code = FT_ER_SUCCESSS;
    other._mutex = ft_nullptr;
    other._thread_safe_enabled = false;
    other.unlock_internal(other_lock_acquired);
    if (transferred_mutex != ft_nullptr)
    {
        transferred_mutex->~pt_mutex();
        cma_free(transferred_mutex);
    }
    if (other_thread_safe)
    {
        if (this->enable_thread_safety() != 0)
            return ;
    }
    this->set_error_unlocked(FT_ER_SUCCESSS);
    return ;
}

template <typename ElementType>
ft_circular_buffer<ElementType>& ft_circular_buffer<ElementType>::operator=(ft_circular_buffer&& other) noexcept
{
    if (this != &other)
    {
        ft_circular_buffer<ElementType>* first;
        ft_circular_buffer<ElementType>* second;
        bool first_lock_acquired;
        bool second_lock_acquired;
        pt_mutex *previous_mutex;
        bool previous_thread_safe;
        pt_mutex *transferred_mutex;
        bool other_thread_safe;

        first = this;
        second = &other;
        if (first > second)
        {
            ft_circular_buffer<ElementType>* temp_pointer;

            temp_pointer = first;
            first = second;
            second = temp_pointer;
        }
        first_lock_acquired = false;
        if (first->lock_internal(&first_lock_acquired) != 0)
        {
            this->set_error_unlocked(ft_errno);
            other.set_error_unlocked(ft_errno);
            return (*this);
        }
        second_lock_acquired = false;
        if (second->lock_internal(&second_lock_acquired) != 0)
        {
            first->unlock_internal(first_lock_acquired);
            this->set_error_unlocked(ft_errno);
            other.set_error_unlocked(ft_errno);
            return (*this);
        }
        previous_mutex = this->_mutex;
        previous_thread_safe = this->_thread_safe_enabled;
        transferred_mutex = other._mutex;
        other_thread_safe = (other._thread_safe_enabled && other._mutex != ft_nullptr);
        this->destroy_elements_locked();
        this->release_buffer_locked();
        this->_buffer = other._buffer;
        this->_capacity = other._capacity;
        this->_head = other._head;
        this->_tail = other._tail;
        this->_size = other._size;
        this->_error_code = other._error_code;
        this->_mutex = ft_nullptr;
        this->_thread_safe_enabled = false;
        other._buffer = ft_nullptr;
        other._capacity = 0;
        other._head = 0;
        other._tail = 0;
        other._size = 0;
        other._error_code = FT_ER_SUCCESSS;
        other._mutex = ft_nullptr;
        other._thread_safe_enabled = false;
        other.unlock_internal(second_lock_acquired);
        this->unlock_internal(first_lock_acquired);
        if (previous_thread_safe && previous_mutex != ft_nullptr)
        {
            previous_mutex->~pt_mutex();
            cma_free(previous_mutex);
        }
        if (transferred_mutex != ft_nullptr)
        {
            transferred_mutex->~pt_mutex();
            cma_free(transferred_mutex);
        }
        if (other_thread_safe)
        {
            if (this->enable_thread_safety() != 0)
                return (*this);
        }
        this->set_error_unlocked(FT_ER_SUCCESSS);
        other.set_error_unlocked(FT_ER_SUCCESSS);
    }
    return (*this);
}

template <typename ElementType>
void ft_circular_buffer<ElementType>::set_error_unlocked(int error) const
{
    this->_error_code = error;
    ft_errno = error;
    return ;
}

template <typename ElementType>
void ft_circular_buffer<ElementType>::set_error(int error) const
{
    this->set_error_unlocked(error);
    return ;
}

template <typename ElementType>
void ft_circular_buffer<ElementType>::push(const ElementType& value)
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error_unlocked(ft_errno);
        return ;
    }
    if (this->_size == this->_capacity)
    {
        this->set_error_unlocked(FT_ERR_FULL);
        this->unlock_internal(lock_acquired);
        return ;
    }
    construct_at(&this->_buffer[this->_tail], value);
    if (this->_capacity != 0)
        this->_tail = (this->_tail + 1) % this->_capacity;
    this->_size += 1;
    this->set_error_unlocked(FT_ER_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename ElementType>
void ft_circular_buffer<ElementType>::push(ElementType&& value)
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error_unlocked(ft_errno);
        return ;
    }
    if (this->_size == this->_capacity)
    {
        this->set_error_unlocked(FT_ERR_FULL);
        this->unlock_internal(lock_acquired);
        return ;
    }
    construct_at(&this->_buffer[this->_tail], ft_move(value));
    if (this->_capacity != 0)
        this->_tail = (this->_tail + 1) % this->_capacity;
    this->_size += 1;
    this->set_error_unlocked(FT_ER_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename ElementType>
ElementType ft_circular_buffer<ElementType>::pop()
{
    bool        lock_acquired;
    ElementType value;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error_unlocked(ft_errno);
        return (ElementType());
    }
    if (this->_size == 0)
    {
        this->set_error_unlocked(FT_ERR_EMPTY);
        this->unlock_internal(lock_acquired);
        return (ElementType());
    }
    value = ft_move(this->_buffer[this->_head]);
    destroy_at(&this->_buffer[this->_head]);
    if (this->_capacity != 0)
        this->_head = (this->_head + 1) % this->_capacity;
    this->_size -= 1;
    this->set_error_unlocked(FT_ER_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (value);
}

template <typename ElementType>
bool ft_circular_buffer<ElementType>::is_full() const
{
    bool lock_acquired;
    bool result;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        const_cast<ft_circular_buffer<ElementType>*>(this)->set_error_unlocked(ft_errno);
        return (false);
    }
    result = (this->_size == this->_capacity);
    const_cast<ft_circular_buffer<ElementType>*>(this)->set_error_unlocked(FT_ER_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (result);
}

template <typename ElementType>
bool ft_circular_buffer<ElementType>::is_empty() const
{
    bool lock_acquired;
    bool result;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        const_cast<ft_circular_buffer<ElementType>*>(this)->set_error_unlocked(ft_errno);
        return (true);
    }
    result = (this->_size == 0);
    const_cast<ft_circular_buffer<ElementType>*>(this)->set_error_unlocked(FT_ER_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (result);
}

template <typename ElementType>
size_t ft_circular_buffer<ElementType>::size() const
{
    bool   lock_acquired;
    size_t current_size;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        const_cast<ft_circular_buffer<ElementType>*>(this)->set_error_unlocked(ft_errno);
        return (0);
    }
    current_size = this->_size;
    const_cast<ft_circular_buffer<ElementType>*>(this)->set_error_unlocked(FT_ER_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (current_size);
}

template <typename ElementType>
size_t ft_circular_buffer<ElementType>::capacity() const
{
    bool   lock_acquired;
    size_t current_capacity;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        const_cast<ft_circular_buffer<ElementType>*>(this)->set_error_unlocked(ft_errno);
        return (0);
    }
    current_capacity = this->_capacity;
    const_cast<ft_circular_buffer<ElementType>*>(this)->set_error_unlocked(FT_ER_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (current_capacity);
}

template <typename ElementType>
int ft_circular_buffer<ElementType>::enable_thread_safety()
{
    return (this->prepare_thread_safety());
}

template <typename ElementType>
void ft_circular_buffer<ElementType>::disable_thread_safety()
{
    this->teardown_thread_safety();
    this->set_error_unlocked(FT_ER_SUCCESSS);
    return ;
}

template <typename ElementType>
bool ft_circular_buffer<ElementType>::is_thread_safe() const
{
    bool enabled;

    enabled = (this->_thread_safe_enabled && this->_mutex != ft_nullptr);
    const_cast<ft_circular_buffer<ElementType>*>(this)->set_error_unlocked(FT_ER_SUCCESSS);
    return (enabled);
}

template <typename ElementType>
int ft_circular_buffer<ElementType>::lock(bool *lock_acquired) const
{
    int result;

    result = this->lock_internal(lock_acquired);
    if (result != 0)
        const_cast<ft_circular_buffer<ElementType>*>(this)->set_error_unlocked(ft_errno);
    else
        const_cast<ft_circular_buffer<ElementType>*>(this)->set_error_unlocked(FT_ER_SUCCESSS);
    return (result);
}

template <typename ElementType>
void ft_circular_buffer<ElementType>::unlock(bool lock_acquired) const
{
    int entry_errno;

    entry_errno = ft_errno;
    this->unlock_internal(lock_acquired);
    if (this->_mutex != ft_nullptr && this->_mutex->get_error() != FT_ER_SUCCESSS)
        const_cast<ft_circular_buffer<ElementType>*>(this)->set_error_unlocked(this->_mutex->get_error());
    else
    {
        ft_errno = entry_errno;
        const_cast<ft_circular_buffer<ElementType>*>(this)->set_error_unlocked(ft_errno);
    }
    return ;
}

template <typename ElementType>
int ft_circular_buffer<ElementType>::get_error() const
{
    return (this->_error_code);
}

template <typename ElementType>
const char* ft_circular_buffer<ElementType>::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}

template <typename ElementType>
void ft_circular_buffer<ElementType>::clear()
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error_unlocked(ft_errno);
        return ;
    }
    this->destroy_elements_locked();
    this->_head = 0;
    this->_tail = 0;
    this->_size = 0;
    this->set_error_unlocked(FT_ER_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename ElementType>
void ft_circular_buffer<ElementType>::destroy_elements_locked()
{
    size_t index;

    index = 0;
    while (index < this->_size)
    {
        size_t position;

        position = this->_head + index;
        if (this->_capacity != 0)
            position %= this->_capacity;
        destroy_at(&this->_buffer[position]);
        index += 1;
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
    this->_capacity = 0;
    this->_head = 0;
    this->_tail = 0;
    this->_size = 0;
    return ;
}

template <typename ElementType>
int ft_circular_buffer<ElementType>::lock_internal(bool *lock_acquired) const
{
    if (lock_acquired)
        *lock_acquired = false;
    if (!this->_thread_safe_enabled || this->_mutex == ft_nullptr)
    {
        ft_errno = FT_ER_SUCCESSS;
        return (0);
    }
    this->_mutex->lock(THREAD_ID);
    if (this->_mutex->get_error() != FT_ER_SUCCESSS)
    {
        if (this->_mutex->get_error() == FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            bool state_lock_acquired;

            state_lock_acquired = false;
            ft_errno = FT_ER_SUCCESSS;
            if (this->_mutex->lock_state(&state_lock_acquired) == 0)
                this->_mutex->unlock_state(state_lock_acquired);
            ft_errno = FT_ER_SUCCESSS;
            return (0);
        }
        ft_errno = this->_mutex->get_error();
        return (-1);
    }
    if (lock_acquired)
        *lock_acquired = true;
    ft_errno = FT_ER_SUCCESSS;
    return (0);
}

template <typename ElementType>
void ft_circular_buffer<ElementType>::unlock_internal(bool lock_acquired) const
{
    int entry_errno;

    if (!lock_acquired || this->_mutex == ft_nullptr)
        return ;
    entry_errno = ft_errno;
    this->_mutex->unlock(THREAD_ID);
    if (this->_mutex->get_error() != FT_ER_SUCCESSS)
    {
        ft_errno = this->_mutex->get_error();
        return ;
    }
    ft_errno = entry_errno;
    return ;
}

template <typename ElementType>
int ft_circular_buffer<ElementType>::prepare_thread_safety()
{
    void *memory_pointer;
    pt_mutex *mutex_pointer;

    if (this->_thread_safe_enabled && this->_mutex != ft_nullptr)
    {
        this->set_error_unlocked(FT_ER_SUCCESSS);
        return (0);
    }
    memory_pointer = cma_malloc(sizeof(pt_mutex));
    if (memory_pointer == ft_nullptr)
    {
        this->set_error_unlocked(FT_ERR_NO_MEMORY);
        return (-1);
    }
    mutex_pointer = new(memory_pointer) pt_mutex();
    if (mutex_pointer->get_error() != FT_ER_SUCCESSS)
    {
        int mutex_error;

        mutex_error = mutex_pointer->get_error();
        mutex_pointer->~pt_mutex();
        cma_free(memory_pointer);
        this->set_error_unlocked(mutex_error);
        return (-1);
    }
    this->_mutex = mutex_pointer;
    this->_thread_safe_enabled = true;
    this->set_error_unlocked(FT_ER_SUCCESSS);
    return (0);
}

template <typename ElementType>
void ft_circular_buffer<ElementType>::teardown_thread_safety()
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

#endif
