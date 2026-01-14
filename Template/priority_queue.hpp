#ifndef FT_PRIORITY_QUEUE_HPP
#define FT_PRIORITY_QUEUE_HPP

#include "constructor.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"
#include "swap.hpp"
#include <cstddef>
#include <utility>
#include <functional>
#include "move.hpp"

template <typename ElementType, typename Compare = std::less<ElementType> >
class ft_priority_queue
{
    private:
        ElementType*         _data;
        size_t               _capacity;
        size_t               _size;
        Compare              _comp;
        mutable int          _error_code;
        mutable pt_mutex*    _mutex;
        bool                 _thread_safe_enabled;

        void    set_error(int error) const;
        bool    ensure_capacity(size_t desired);
        void    heapify_up(size_t index);
        void    heapify_down(size_t index);
        int     lock_internal(bool *lock_acquired) const;
        void    unlock_internal(bool lock_acquired) const;
        void    teardown_thread_safety();

    public:
        ft_priority_queue(size_t initialCapacity = 0, const Compare& comp = Compare());
        ~ft_priority_queue();

        ft_priority_queue(const ft_priority_queue&) = delete;
        ft_priority_queue& operator=(const ft_priority_queue&) = delete;

        ft_priority_queue(ft_priority_queue&& other) noexcept;
        ft_priority_queue& operator=(ft_priority_queue&& other) noexcept;

        int     enable_thread_safety();
        void    disable_thread_safety();
        bool    is_thread_safe() const;
        int     lock(bool *lock_acquired) const;
        void    unlock(bool lock_acquired) const;

        void push(const ElementType& value);
        void push(ElementType&& value);
        ElementType pop();

        ElementType& top();
        const ElementType& top() const;

        size_t size() const;
        bool empty() const;

        int get_error() const;
        const char* get_error_str() const;

        void clear();
};

template <typename ElementType, typename Compare>
ft_priority_queue<ElementType, Compare>::ft_priority_queue(size_t initialCapacity, const Compare& comp)
    : _data(ft_nullptr), _capacity(0), _size(0), _comp(comp), _error_code(FT_ERR_SUCCESSS),
      _mutex(ft_nullptr), _thread_safe_enabled(false)
{
    if (initialCapacity > 0)
    {
        this->_data = static_cast<ElementType*>(cma_malloc(sizeof(ElementType) * initialCapacity));
        if (this->_data == ft_nullptr)
        {
            this->set_error(FT_ERR_PRIORITY_QUEUE_NO_MEMORY);
            return ;
        }
        this->_capacity = initialCapacity;
    }
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

template <typename ElementType, typename Compare>
ft_priority_queue<ElementType, Compare>::~ft_priority_queue()
{
    this->clear();
    if (this->_data != ft_nullptr)
        cma_free(this->_data);
    this->teardown_thread_safety();
    return ;
}

template <typename ElementType, typename Compare>
ft_priority_queue<ElementType, Compare>::ft_priority_queue(ft_priority_queue&& other) noexcept
    : _data(other._data), _capacity(other._capacity), _size(other._size), _comp(other._comp),
      _error_code(other._error_code), _mutex(ft_nullptr),
      _thread_safe_enabled(false)
{
    bool other_thread_safe;

    other_thread_safe = (other._thread_safe_enabled && other._mutex != ft_nullptr);
    if (other_thread_safe)
    {
        if (this->enable_thread_safety() != 0)
        {
            other.teardown_thread_safety();
            other._data = ft_nullptr;
            other._capacity = 0;
            other._size = 0;
            other._error_code = FT_ERR_SUCCESSS;
            other._thread_safe_enabled = false;
            return ;
        }
    }
    other._data = ft_nullptr;
    other._capacity = 0;
    other._size = 0;
    other._error_code = FT_ERR_SUCCESSS;
    other.teardown_thread_safety();
    other._thread_safe_enabled = false;
    this->set_error(this->_error_code);
    return ;
}

template <typename ElementType, typename Compare>
ft_priority_queue<ElementType, Compare>& ft_priority_queue<ElementType, Compare>::operator=(ft_priority_queue&& other) noexcept
{
    if (this != &other)
    {
        bool other_thread_safe;

        this->clear();
        if (this->_data != ft_nullptr)
            cma_free(this->_data);
        this->teardown_thread_safety();
        this->_data = other._data;
        this->_capacity = other._capacity;
        this->_size = other._size;
        this->_comp = other._comp;
        this->_error_code = other._error_code;
        this->_mutex = ft_nullptr;
        this->_thread_safe_enabled = false;
        other_thread_safe = (other._thread_safe_enabled && other._mutex != ft_nullptr);
        if (other_thread_safe)
        {
            if (this->enable_thread_safety() != 0)
            {
                other.teardown_thread_safety();
                other._data = ft_nullptr;
                other._capacity = 0;
                other._size = 0;
                other._error_code = FT_ERR_SUCCESSS;
                other._thread_safe_enabled = false;
                return (*this);
            }
        }
        other._data = ft_nullptr;
        other._capacity = 0;
        other._size = 0;
        other._error_code = FT_ERR_SUCCESSS;
        other.teardown_thread_safety();
        other._thread_safe_enabled = false;
    }
    this->set_error(this->_error_code);
    return (*this);
}

template <typename ElementType, typename Compare>
void ft_priority_queue<ElementType, Compare>::set_error(int error) const
{
    this->_error_code = error;
    return ;
}

template <typename ElementType, typename Compare>
int ft_priority_queue<ElementType, Compare>::enable_thread_safety()
{
    void     *memory;
    pt_mutex *mutex_pointer;

    if (this->_thread_safe_enabled && this->_mutex != ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESSS);
        return (0);
    }
    memory = cma_malloc(sizeof(pt_mutex));
    if (memory == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return (-1);
    }
    mutex_pointer = new(memory) pt_mutex();
    if (mutex_pointer->get_error() != FT_ERR_SUCCESSS)
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
    this->set_error(FT_ERR_SUCCESSS);
    return (0);
}

template <typename ElementType, typename Compare>
void ft_priority_queue<ElementType, Compare>::disable_thread_safety()
{
    this->teardown_thread_safety();
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

template <typename ElementType, typename Compare>
bool ft_priority_queue<ElementType, Compare>::is_thread_safe() const
{
    bool enabled;

    enabled = (this->_thread_safe_enabled && this->_mutex != ft_nullptr);
    const_cast<ft_priority_queue<ElementType, Compare> *>(this)->set_error(FT_ERR_SUCCESSS);
    return (enabled);
}

template <typename ElementType, typename Compare>
int ft_priority_queue<ElementType, Compare>::lock(bool *lock_acquired) const
{
    int result;

    result = this->lock_internal(lock_acquired);
    if (result != 0)
        const_cast<ft_priority_queue<ElementType, Compare> *>(this)->set_error(this->get_error());
    else
        const_cast<ft_priority_queue<ElementType, Compare> *>(this)->set_error(FT_ERR_SUCCESSS);
    return (result);
}

template <typename ElementType, typename Compare>
void ft_priority_queue<ElementType, Compare>::unlock(bool lock_acquired) const
{
    this->unlock_internal(lock_acquired);
    if (this->_mutex != ft_nullptr && this->_mutex->get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_priority_queue<ElementType, Compare> *>(this)->set_error(this->_mutex->get_error());
        return ;
    }
    const_cast<ft_priority_queue<ElementType, Compare> *>(this)->set_error(FT_ERR_SUCCESSS);
    return ;
}

template <typename ElementType, typename Compare>
int ft_priority_queue<ElementType, Compare>::lock_internal(bool *lock_acquired) const
{
    int mutex_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (!this->_thread_safe_enabled || this->_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESSS);
        return (0);
    }
    this->_mutex->lock(THREAD_ID);
    mutex_error = this->_mutex->get_error();
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        this->set_error(mutex_error);
        return (-1);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    this->set_error(FT_ERR_SUCCESSS);
    return (0);
}

template <typename ElementType, typename Compare>
void ft_priority_queue<ElementType, Compare>::unlock_internal(bool lock_acquired) const
{
    int mutex_error;

    if (!lock_acquired || this->_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESSS);
        return ;
    }
    this->_mutex->unlock(THREAD_ID);
    mutex_error = this->_mutex->get_error();
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        this->set_error(mutex_error);
        return ;
    }
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

template <typename ElementType, typename Compare>
void ft_priority_queue<ElementType, Compare>::teardown_thread_safety()
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

template <typename ElementType, typename Compare>
bool ft_priority_queue<ElementType, Compare>::ensure_capacity(size_t desired)
{
    if (desired <= this->_capacity)
    {
        this->set_error(FT_ERR_SUCCESSS);
        return (true);
    }
    size_t new_capacity;
    if (this->_capacity == 0)
        new_capacity = 1;
    else
        new_capacity = this->_capacity * 2;
    while (new_capacity < desired)
        new_capacity *= 2;
    ElementType* new_data = static_cast<ElementType*>(cma_malloc(sizeof(ElementType) * new_capacity));
    if (new_data == ft_nullptr)
    {
        this->set_error(FT_ERR_PRIORITY_QUEUE_NO_MEMORY);
        return (false);
    }
    size_t element_index = 0;
    while (element_index < this->_size)
    {
        construct_at(&new_data[element_index], ft_move(this->_data[element_index]));
        destroy_at(&this->_data[element_index]);
        element_index += 1;
    }
    if (this->_data != ft_nullptr)
        cma_free(this->_data);
    this->_data = new_data;
    this->_capacity = new_capacity;
    this->set_error(FT_ERR_SUCCESSS);
    return (true);
}

template <typename ElementType, typename Compare>
void ft_priority_queue<ElementType, Compare>::heapify_up(size_t index)
{
    while (index > 0)
    {
        size_t parent_index = (index - 1) / 2;
        if (!this->_comp(this->_data[parent_index], this->_data[index]))
            break;
        ft_swap(this->_data[parent_index], this->_data[index]);
        index = parent_index;
    }
    return ;
}

template <typename ElementType, typename Compare>
void ft_priority_queue<ElementType, Compare>::heapify_down(size_t index)
{
    while (true)
    {
        size_t left_child_index = index * 2 + 1;
        size_t right_child_index = left_child_index + 1;
        size_t largest_index = index;
        if (left_child_index < this->_size && this->_comp(this->_data[largest_index], this->_data[left_child_index]))
            largest_index = left_child_index;
        if (right_child_index < this->_size && this->_comp(this->_data[largest_index], this->_data[right_child_index]))
            largest_index = right_child_index;
        if (largest_index == index)
            break;
        ft_swap(this->_data[index], this->_data[largest_index]);
        index = largest_index;
    }
    return ;
}

template <typename ElementType, typename Compare>
void ft_priority_queue<ElementType, Compare>::push(const ElementType& value)
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(this->get_error());
        return ;
    }
    if (!this->ensure_capacity(this->_size + 1))
    {
        this->unlock_internal(lock_acquired);
        return ;
    }
    construct_at(&this->_data[this->_size], value);
    this->heapify_up(this->_size);
    ++this->_size;
    this->set_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename ElementType, typename Compare>
void ft_priority_queue<ElementType, Compare>::push(ElementType&& value)
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(this->get_error());
        return ;
    }
    if (!this->ensure_capacity(this->_size + 1))
    {
        this->unlock_internal(lock_acquired);
        return ;
    }
    construct_at(&this->_data[this->_size], ft_move(value));
    this->heapify_up(this->_size);
    ++this->_size;
    this->set_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename ElementType, typename Compare>
ElementType ft_priority_queue<ElementType, Compare>::pop()
{
    ElementType value;
    bool        lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(this->get_error());
        return (ElementType());
    }
    if (this->_size == 0)
    {
        this->set_error(FT_ERR_PRIORITY_QUEUE_EMPTY);
        this->unlock_internal(lock_acquired);
        return (ElementType());
    }
    value = ft_move(this->_data[0]);
    destroy_at(&this->_data[0]);
    --this->_size;
    if (this->_size > 0)
    {
        construct_at(&this->_data[0], ft_move(this->_data[this->_size]));
        destroy_at(&this->_data[this->_size]);
        this->heapify_down(0);
    }
    this->set_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (value);
}

template <typename ElementType, typename Compare>
ElementType& ft_priority_queue<ElementType, Compare>::top()
{
    static ElementType error_element = ElementType();
    ElementType       *value;
    bool               lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(this->get_error());
        return (error_element);
    }
    if (this->_size == 0)
    {
        this->set_error(FT_ERR_PRIORITY_QUEUE_EMPTY);
        this->unlock_internal(lock_acquired);
        return (error_element);
    }
    value = &this->_data[0];
    this->set_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (*value);
}

template <typename ElementType, typename Compare>
const ElementType& ft_priority_queue<ElementType, Compare>::top() const
{
    static ElementType error_element = ElementType();
    const ElementType *value;
    bool               lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        const_cast<ft_priority_queue<ElementType, Compare> *>(this)->set_error(this->get_error());
        return (error_element);
    }
    if (this->_size == 0)
    {
        const_cast<ft_priority_queue<ElementType, Compare> *>(this)->set_error(FT_ERR_PRIORITY_QUEUE_EMPTY);
        this->unlock_internal(lock_acquired);
        return (error_element);
    }
    value = &this->_data[0];
    const_cast<ft_priority_queue<ElementType, Compare> *>(this)->set_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (*value);
}

template <typename ElementType, typename Compare>
size_t ft_priority_queue<ElementType, Compare>::size() const
{
    size_t current_size;
    bool   lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        const_cast<ft_priority_queue<ElementType, Compare> *>(this)->set_error(this->get_error());
        return (0);
    }
    current_size = this->_size;
    const_cast<ft_priority_queue<ElementType, Compare> *>(this)->set_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (current_size);
}

template <typename ElementType, typename Compare>
bool ft_priority_queue<ElementType, Compare>::empty() const
{
    bool result;
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        const_cast<ft_priority_queue<ElementType, Compare> *>(this)->set_error(this->get_error());
        return (true);
    }
    result = (this->_size == 0);
    const_cast<ft_priority_queue<ElementType, Compare> *>(this)->set_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (result);
}

template <typename ElementType, typename Compare>
int ft_priority_queue<ElementType, Compare>::get_error() const
{
    this->set_error(this->_error_code);
    return (this->_error_code);
}

template <typename ElementType, typename Compare>
const char* ft_priority_queue<ElementType, Compare>::get_error_str() const
{
    this->set_error(this->_error_code);
    return (ft_strerror(this->_error_code));
}

template <typename ElementType, typename Compare>
void ft_priority_queue<ElementType, Compare>::clear()
{
    size_t element_index;
    bool   lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(this->get_error());
        return ;
    }
    element_index = 0;
    while (element_index < this->_size)
    {
        destroy_at(&this->_data[element_index]);
        element_index += 1;
    }
    this->_size = 0;
    this->set_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return ;
}

#endif
