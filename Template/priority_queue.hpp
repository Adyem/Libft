#ifndef FT_PRIORITY_QUEUE_HPP
#define FT_PRIORITY_QUEUE_HPP

#include "constructor.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/mutex.hpp"
#include "swap.hpp"
#include <cstddef>
#include <utility>
#include <functional>

/*
 * ft_priority_queue
 * A thread-safe priority queue implemented as a binary heap.
 * Provides push, pop, top, size, empty, and clear operations
 * with error reporting.
 */

template <typename ElementType, typename Compare = std::less<ElementType> >
class ft_priority_queue
{
    private:
        ElementType*   _data;
        size_t         _capacity;
        size_t         _size;
        Compare        _comp;
        mutable int    _error_code;
        mutable pt_mutex _mutex;

        void    set_error(int error) const;
        bool    ensure_capacity(size_t desired);
        void    heapify_up(size_t index);
        void    heapify_down(size_t index);

    public:
        ft_priority_queue(size_t initialCapacity = 0, const Compare& comp = Compare());
        ~ft_priority_queue();

        ft_priority_queue(const ft_priority_queue&) = delete;
        ft_priority_queue& operator=(const ft_priority_queue&) = delete;

        ft_priority_queue(ft_priority_queue&& other) noexcept;
        ft_priority_queue& operator=(ft_priority_queue&& other) noexcept;

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

// Implementation

template <typename ElementType, typename Compare>
ft_priority_queue<ElementType, Compare>::ft_priority_queue(size_t initialCapacity, const Compare& comp)
    : _data(ft_nullptr), _capacity(0), _size(0), _comp(comp), _error_code(ER_SUCCESS)
{
    if (initialCapacity > 0)
    {
        this->_data = static_cast<ElementType*>(cma_malloc(sizeof(ElementType) * initialCapacity));
        if (this->_data == ft_nullptr)
            this->set_error(PRIORITY_QUEUE_ALLOC_FAIL);
        else
            this->_capacity = initialCapacity;
    }
    return ;
}

template <typename ElementType, typename Compare>
ft_priority_queue<ElementType, Compare>::~ft_priority_queue()
{
    this->clear();
    if (this->_data != ft_nullptr)
        cma_free(this->_data);
    return ;
}

template <typename ElementType, typename Compare>
ft_priority_queue<ElementType, Compare>::ft_priority_queue(ft_priority_queue&& other) noexcept
    : _data(other._data), _capacity(other._capacity), _size(other._size), _comp(other._comp), _error_code(other._error_code)
{
    other._data = ft_nullptr;
    other._capacity = 0;
    other._size = 0;
    other._error_code = ER_SUCCESS;
    return ;
}

template <typename ElementType, typename Compare>
ft_priority_queue<ElementType, Compare>& ft_priority_queue<ElementType, Compare>::operator=(ft_priority_queue&& other) noexcept
{
    if (this != &other)
    {
        if (this->_mutex.lock(THREAD_ID) != SUCCES)
            return (*this);
        if (other._mutex.lock(THREAD_ID) != SUCCES)
        {
            this->_mutex.unlock(THREAD_ID);
            return (*this);
        }
        this->clear();
        if (this->_data != ft_nullptr)
            cma_free(this->_data);
        this->_data = other._data;
        this->_capacity = other._capacity;
        this->_size = other._size;
        this->_comp = other._comp;
        this->_error_code = other._error_code;
        other._data = ft_nullptr;
        other._capacity = 0;
        other._size = 0;
        other._error_code = ER_SUCCESS;
        other._mutex.unlock(THREAD_ID);
        this->_mutex.unlock(THREAD_ID);
    }
    return (*this);
}

template <typename ElementType, typename Compare>
void ft_priority_queue<ElementType, Compare>::set_error(int error) const
{
    this->_error_code = error;
    ft_errno = error;
    return ;
}

template <typename ElementType, typename Compare>
bool ft_priority_queue<ElementType, Compare>::ensure_capacity(size_t desired)
{
    if (desired <= this->_capacity)
        return (true);
    size_t newCap;
    if (this->_capacity == 0)
        newCap = 1;
    else
        newCap = this->_capacity * 2;
    while (newCap < desired)
        newCap *= 2;
    ElementType* newData = static_cast<ElementType*>(cma_malloc(sizeof(ElementType) * newCap));
    if (newData == ft_nullptr)
    {
        this->set_error(PRIORITY_QUEUE_ALLOC_FAIL);
        return (false);
    }
    size_t i = 0;
    while (i < this->_size)
    {
        construct_at(&newData[i], std::move(this->_data[i]));
        destroy_at(&this->_data[i]);
        ++i;
    }
    if (this->_data != ft_nullptr)
        cma_free(this->_data);
    this->_data = newData;
    this->_capacity = newCap;
    return (true);
}

template <typename ElementType, typename Compare>
void ft_priority_queue<ElementType, Compare>::heapify_up(size_t index)
{
    while (index > 0)
    {
        size_t parent = (index - 1) / 2;
        if (!this->_comp(this->_data[parent], this->_data[index]))
            break;
        ft_swap(this->_data[parent], this->_data[index]);
        index = parent;
    }
    return ;
}

template <typename ElementType, typename Compare>
void ft_priority_queue<ElementType, Compare>::heapify_down(size_t index)
{
    while (true)
    {
        size_t left = index * 2 + 1;
        size_t right = left + 1;
        size_t largest = index;
        if (left < this->_size && this->_comp(this->_data[largest], this->_data[left]))
            largest = left;
        if (right < this->_size && this->_comp(this->_data[largest], this->_data[right]))
            largest = right;
        if (largest == index)
            break;
        ft_swap(this->_data[index], this->_data[largest]);
        index = largest;
    }
    return ;
}

template <typename ElementType, typename Compare>
void ft_priority_queue<ElementType, Compare>::push(const ElementType& value)
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->set_error(PT_ERR_MUTEX_OWNER);
        return ;
    }
    if (!this->ensure_capacity(this->_size + 1))
    {
        this->_mutex.unlock(THREAD_ID);
        return ;
    }
    construct_at(&this->_data[this->_size], value);
    this->heapify_up(this->_size);
    ++this->_size;
    this->_mutex.unlock(THREAD_ID);
    return ;
}

template <typename ElementType, typename Compare>
void ft_priority_queue<ElementType, Compare>::push(ElementType&& value)
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->set_error(PT_ERR_MUTEX_OWNER);
        return ;
    }
    if (!this->ensure_capacity(this->_size + 1))
    {
        this->_mutex.unlock(THREAD_ID);
        return ;
    }
    construct_at(&this->_data[this->_size], std::move(value));
    this->heapify_up(this->_size);
    ++this->_size;
    this->_mutex.unlock(THREAD_ID);
    return ;
}

template <typename ElementType, typename Compare>
ElementType ft_priority_queue<ElementType, Compare>::pop()
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->set_error(PT_ERR_MUTEX_OWNER);
        return (ElementType());
    }
    if (this->_size == 0)
    {
        this->set_error(PRIORITY_QUEUE_EMPTY);
        this->_mutex.unlock(THREAD_ID);
        return (ElementType());
    }
    ElementType topValue = std::move(this->_data[0]);
    destroy_at(&this->_data[0]);
    --this->_size;
    if (this->_size > 0)
    {
        construct_at(&this->_data[0], std::move(this->_data[this->_size]));
        destroy_at(&this->_data[this->_size]);
        this->heapify_down(0);
    }
    this->_mutex.unlock(THREAD_ID);
    return (topValue);
}

template <typename ElementType, typename Compare>
ElementType& ft_priority_queue<ElementType, Compare>::top()
{
    static ElementType error_element = ElementType();
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->set_error(PT_ERR_MUTEX_OWNER);
        return (error_element);
    }
    if (this->_size == 0)
    {
        this->set_error(PRIORITY_QUEUE_EMPTY);
        this->_mutex.unlock(THREAD_ID);
        return (error_element);
    }
    ElementType& value = this->_data[0];
    this->_mutex.unlock(THREAD_ID);
    return (value);
}

template <typename ElementType, typename Compare>
const ElementType& ft_priority_queue<ElementType, Compare>::top() const
{
    static ElementType error_element = ElementType();
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->set_error(PT_ERR_MUTEX_OWNER);
        return (error_element);
    }
    if (this->_size == 0)
    {
        this->set_error(PRIORITY_QUEUE_EMPTY);
        this->_mutex.unlock(THREAD_ID);
        return (error_element);
    }
    const ElementType& value = this->_data[0];
    this->_mutex.unlock(THREAD_ID);
    return (value);
}

template <typename ElementType, typename Compare>
size_t ft_priority_queue<ElementType, Compare>::size() const
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (0);
    size_t current_size = this->_size;
    this->_mutex.unlock(THREAD_ID);
    return (current_size);
}

template <typename ElementType, typename Compare>
bool ft_priority_queue<ElementType, Compare>::empty() const
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (true);
    bool result = (this->_size == 0);
    this->_mutex.unlock(THREAD_ID);
    return (result);
}

template <typename ElementType, typename Compare>
int ft_priority_queue<ElementType, Compare>::get_error() const
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (this->_error_code);
    int err = this->_error_code;
    this->_mutex.unlock(THREAD_ID);
    return (err);
}

template <typename ElementType, typename Compare>
const char* ft_priority_queue<ElementType, Compare>::get_error_str() const
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (ft_strerror(this->_error_code));
    int err = this->_error_code;
    this->_mutex.unlock(THREAD_ID);
    return (ft_strerror(err));
}

template <typename ElementType, typename Compare>
void ft_priority_queue<ElementType, Compare>::clear()
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return ;
    size_t i = 0;
    while (i < this->_size)
    {
        destroy_at(&this->_data[i]);
        ++i;
    }
    this->_size = 0;
    this->_mutex.unlock(THREAD_ID);
    return ;
}

#endif
