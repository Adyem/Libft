#ifndef FT_PRIORITY_QUEUE_HPP
#define FT_PRIORITY_QUEUE_HPP

#include "constructor.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread_internal.hpp"
#include "swap.hpp"
#include <cstddef>
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
        mutable pt_recursive_mutex* _mutex;
        bool    ensure_capacity(size_t desired);
        void    heapify_up(size_t index);
        void    heapify_down(size_t index);
        int     lock_internal(bool *lock_acquired) const;
        int     unlock_internal(bool lock_acquired) const;
        int     prepare_thread_safety();
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

        void clear();

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex* get_mutex_for_validation() const noexcept;
#endif
};

template <typename ElementType, typename Compare>
ft_priority_queue<ElementType, Compare>::ft_priority_queue(size_t initialCapacity, const Compare& comp)
    : _data(ft_nullptr), _capacity(0), _size(0), _comp(comp), _mutex(ft_nullptr)
{
    if (initialCapacity > 0)
    {
        this->_data = static_cast<ElementType*>(cma_malloc(sizeof(ElementType) * initialCapacity));
        if (this->_data == ft_nullptr)
        {
            ft_global_error_stack_push(FT_ERR_PRIORITY_QUEUE_NO_MEMORY);
            return ;
        }
        this->_capacity = initialCapacity;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

template <typename ElementType, typename Compare>
ft_priority_queue<ElementType, Compare>::~ft_priority_queue()
{
    this->clear();
    if (this->_data != ft_nullptr)
        cma_free(this->_data);
    this->teardown_thread_safety();
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

template <typename ElementType, typename Compare>
ft_priority_queue<ElementType, Compare>::ft_priority_queue(ft_priority_queue&& other) noexcept
    : _data(other._data), _capacity(other._capacity), _size(other._size), _comp(other._comp),
      _mutex(ft_nullptr)
{
    bool other_thread_safe;

    other_thread_safe = (other._mutex != ft_nullptr);
    if (other_thread_safe)
    {
        if (this->enable_thread_safety() != FT_ERR_SUCCESSS)
        {
            other.teardown_thread_safety();
            other._data = ft_nullptr;
            other._capacity = 0;
            other._size = 0;
            return ;
        }
    }
    other._data = ft_nullptr;
    other._capacity = 0;
    other._size = 0;
    other.teardown_thread_safety();
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

template <typename ElementType, typename Compare>
ft_priority_queue<ElementType, Compare>& ft_priority_queue<ElementType, Compare>::operator=(ft_priority_queue&& other) noexcept
{
    if (this != &other)
    {
        bool other_thread_safe;
        bool other_lock_acquired;
        int lock_error;

        this->clear();
        if (this->_data != ft_nullptr)
            cma_free(this->_data);
        this->teardown_thread_safety();
        this->_data = ft_nullptr;
        this->_capacity = 0;
        this->_size = 0;
        this->_mutex = ft_nullptr;
        other_lock_acquired = false;
        lock_error = other.lock_internal(&other_lock_acquired);
        if (lock_error != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(lock_error);
            return (*this);
        }
        this->_data = other._data;
        this->_capacity = other._capacity;
        this->_size = other._size;
        this->_comp = other._comp;
        other_thread_safe = (other._mutex != ft_nullptr);
        other._data = ft_nullptr;
        other._capacity = 0;
        other._size = 0;
        other.unlock_internal(other_lock_acquired);
        other.teardown_thread_safety();
        if (other_thread_safe)
        {
            if (this->enable_thread_safety() != FT_ERR_SUCCESSS)
                return (*this);
        }
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (*this);
}

template <typename ElementType, typename Compare>
int ft_priority_queue<ElementType, Compare>::enable_thread_safety()
{
    int result;

    if (this->_mutex != ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (FT_ERR_SUCCESSS);
    }
    result = this->prepare_thread_safety();
    ft_global_error_stack_push(result);
    return (result);
}

template <typename ElementType, typename Compare>
void ft_priority_queue<ElementType, Compare>::disable_thread_safety()
{
    this->teardown_thread_safety();
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

template <typename ElementType, typename Compare>
bool ft_priority_queue<ElementType, Compare>::is_thread_safe() const
{
    bool enabled;

    enabled = (this->_mutex != ft_nullptr);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (enabled);
}

template <typename ElementType, typename Compare>
int ft_priority_queue<ElementType, Compare>::lock(bool *lock_acquired) const
{
    int result;

    result = this->lock_internal(lock_acquired);
    if (result != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(result);
        return (-1);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}

template <typename ElementType, typename Compare>
void ft_priority_queue<ElementType, Compare>::unlock(bool lock_acquired) const
{
    int unlock_error = this->unlock_internal(lock_acquired);
    ft_global_error_stack_push(unlock_error);
    return ;
}

template <typename ElementType, typename Compare>
int ft_priority_queue<ElementType, Compare>::lock_internal(bool *lock_acquired) const
{
    int result;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESSS);
    result = pt_recursive_mutex_lock_with_error(*this->_mutex);
    if (result != FT_ERR_SUCCESSS)
        return (result);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESSS);
}

template <typename ElementType, typename Compare>
int ft_priority_queue<ElementType, Compare>::unlock_internal(bool lock_acquired) const
{
    if (!lock_acquired || this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESSS);
    return (pt_recursive_mutex_unlock_with_error(*this->_mutex));
}

template <typename ElementType, typename Compare>
int ft_priority_queue<ElementType, Compare>::prepare_thread_safety()
{
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESSS);
    return (pt_recursive_mutex_create_with_error(&this->_mutex));
}

template <typename ElementType, typename Compare>
void ft_priority_queue<ElementType, Compare>::teardown_thread_safety()
{
    pt_recursive_mutex_destroy(&this->_mutex);
    return ;
}

template <typename ElementType, typename Compare>
bool ft_priority_queue<ElementType, Compare>::ensure_capacity(size_t desired)
{
    if (desired <= this->_capacity)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
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
        ft_global_error_stack_push(FT_ERR_PRIORITY_QUEUE_NO_MEMORY);
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
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
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
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
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
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename ElementType, typename Compare>
void ft_priority_queue<ElementType, Compare>::push(ElementType&& value)
{
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
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
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename ElementType, typename Compare>
ElementType ft_priority_queue<ElementType, Compare>::pop()
{
    ElementType value;
    bool        lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (ElementType());
    }
    if (this->_size == 0)
    {
        ft_global_error_stack_push(FT_ERR_PRIORITY_QUEUE_EMPTY);
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
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (value);
}

template <typename ElementType, typename Compare>
ElementType& ft_priority_queue<ElementType, Compare>::top()
{
    static ElementType error_element = ElementType();
    ElementType       *value;
    bool               lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (error_element);
    }
    if (this->_size == 0)
    {
        ft_global_error_stack_push(FT_ERR_PRIORITY_QUEUE_EMPTY);
        this->unlock_internal(lock_acquired);
        return (error_element);
    }
    value = &this->_data[0];
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (*value);
}

template <typename ElementType, typename Compare>
const ElementType& ft_priority_queue<ElementType, Compare>::top() const
{
    static ElementType error_element = ElementType();
    const ElementType *value;
    bool               lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (error_element);
    }
    if (this->_size == 0)
    {
        ft_global_error_stack_push(FT_ERR_PRIORITY_QUEUE_EMPTY);
        this->unlock_internal(lock_acquired);
        return (error_element);
    }
    value = &this->_data[0];
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (*value);
}

template <typename ElementType, typename Compare>
size_t ft_priority_queue<ElementType, Compare>::size() const
{
    size_t current_size;
    bool   lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (0);
    }
    current_size = this->_size;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (current_size);
}

template <typename ElementType, typename Compare>
bool ft_priority_queue<ElementType, Compare>::empty() const
{
    bool result;
    bool lock_acquired;
    int  lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (true);
    }
    result = (this->_size == 0);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (result);
}

template <typename ElementType, typename Compare>
void ft_priority_queue<ElementType, Compare>::clear()
{
    size_t element_index;
    bool   lock_acquired;
    int    lock_error;

    lock_acquired = false;

    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    element_index = 0;
    while (element_index < this->_size)
    {
        destroy_at(&this->_data[element_index]);
        element_index += 1;
    }
    this->_size = 0;
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return ;
}

#ifdef LIBFT_TEST_BUILD
template <typename ElementType, typename Compare>
pt_recursive_mutex* ft_priority_queue<ElementType, Compare>::get_mutex_for_validation() const noexcept
{
    return (this->_mutex);
}
#endif

#endif
