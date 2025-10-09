#ifndef FT_SET_HPP
#define FT_SET_HPP

#include "constructor.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/mutex.hpp"
#include <cstddef>
#include <utility>

/*
** Complexity and iterator invalidation guarantees:
** - size, empty: O(1) without invalidation.
** - find: O(log n); does not invalidate iterators.
** - insert: O(n) due to shifting to maintain order; invalidates iterators at or after insertion point.
** - remove: O(n) with element compaction; invalidates iterators at or after removed position.
** - clear: O(n); invalidates all iterators and references.
** Thread safety: external synchronization required; mutex covers error reporting only.
*/



template <typename ElementType>
class ft_set
{
    private:
        ElementType*   _data;
        size_t         _capacity;
        size_t         _size;
        mutable int    _error_code;
        mutable pt_mutex _mutex;

        void    set_error(int error) const;
        bool    ensure_capacity(size_t desired_capacity);
        size_t  find_index(const ElementType& value) const; 
        size_t  lower_bound(const ElementType& value) const; 

    public:
        ft_set(size_t initial_capacity = 0);
        ~ft_set();

        ft_set(const ft_set&) = delete;
        ft_set& operator=(const ft_set&) = delete;

        ft_set(ft_set&& other) noexcept;
        ft_set& operator=(ft_set&& other) noexcept;

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
    : _data(ft_nullptr), _capacity(0), _size(0), _error_code(ER_SUCCESS)
{
    if (initial_capacity > 0)
    {
        this->_data = static_cast<ElementType*>(cma_malloc(sizeof(ElementType) * initial_capacity));
        if (this->_data == ft_nullptr)
            this->set_error(FT_ERR_NO_MEMORY);
        else
            this->_capacity = initial_capacity;
    }
    return ;
}

template <typename ElementType>
ft_set<ElementType>::~ft_set()
{
    if (this->_data != ft_nullptr)
    {
        size_t i = 0;
        while (i < this->_size)
        {
            destroy_at(&this->_data[i]);
            ++i;
        }
        cma_free(this->_data);
    }
    return ;
}

template <typename ElementType>
ft_set<ElementType>::ft_set(ft_set&& other) noexcept
    : _data(other._data), _capacity(other._capacity), _size(other._size),
      _error_code(other._error_code)
{
    other._data = ft_nullptr;
    other._capacity = 0;
    other._size = 0;
    other._error_code = ER_SUCCESS;
    return ;
}

template <typename ElementType>
ft_set<ElementType>& ft_set<ElementType>::operator=(ft_set&& other) noexcept
{
    if (this != &other)
    {
        if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
            return (*this);
        if (other._mutex.lock(THREAD_ID) != FT_SUCCESS)
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

template <typename ElementType>
void ft_set<ElementType>::set_error(int error) const
{
    this->_error_code = error;
    ft_errno = error;
    return ;
}

template <typename ElementType>
bool ft_set<ElementType>::ensure_capacity(size_t desired_capacity)
{
    if (desired_capacity <= this->_capacity)
        return (true);
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
        construct_at(&new_data[index], std::move(this->_data[index]));
        destroy_at(&this->_data[index]);
        ++index;
    }
    if (this->_data != ft_nullptr)
        cma_free(this->_data);
    this->_data = new_data;
    this->_capacity = new_capacity;
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
void ft_set<ElementType>::insert(const ElementType& value)
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        this->set_error(FT_ERR_MUTEX_NOT_OWNER);
        return ;
    }
    size_t position = lower_bound(value);
    if (position < this->_size && !(value < this->_data[position]) && !(this->_data[position] < value))
    {
        this->set_error(ER_SUCCESS);
        this->_mutex.unlock(THREAD_ID);
        return ;
    }
    if (!ensure_capacity(this->_size + 1))
    {
        this->_mutex.unlock(THREAD_ID);
        return ;
    }
    size_t index = this->_size;
    while (index > position)
    {
        construct_at(&this->_data[index], std::move(this->_data[index - 1]));
        destroy_at(&this->_data[index - 1]);
        --index;
    }
    construct_at(&this->_data[position], value);
    ++this->_size;
    this->set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
    return ;
}

template <typename ElementType>
void ft_set<ElementType>::insert(ElementType&& value)
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        this->set_error(FT_ERR_MUTEX_NOT_OWNER);
        return ;
    }
    size_t position = lower_bound(value);
    if (position < this->_size && !(value < this->_data[position]) && !(this->_data[position] < value))
    {
        this->set_error(ER_SUCCESS);
        this->_mutex.unlock(THREAD_ID);
        return ;
    }
    if (!ensure_capacity(this->_size + 1))
    {
        this->_mutex.unlock(THREAD_ID);
        return ;
    }
    size_t index = this->_size;
    while (index > position)
    {
        construct_at(&this->_data[index], std::move(this->_data[index - 1]));
        destroy_at(&this->_data[index - 1]);
        --index;
    }
    construct_at(&this->_data[position], std::move(value));
    ++this->_size;
    this->set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
    return ;
}

template <typename ElementType>
ElementType* ft_set<ElementType>::find(const ElementType& value)
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        this->set_error(FT_ERR_MUTEX_NOT_OWNER);
        return (ft_nullptr);
    }
    size_t index = find_index(value);
    if (index == this->_size)
    {
        this->set_error(FT_ERR_NOT_FOUND);
        this->_mutex.unlock(THREAD_ID);
        return (ft_nullptr);
    }
    ElementType* result = &this->_data[index];
    this->set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
    return (result);
}

template <typename ElementType>
const ElementType* ft_set<ElementType>::find(const ElementType& value) const
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        const_cast<ft_set*>(this)->set_error(FT_ERR_MUTEX_NOT_OWNER);
        return (ft_nullptr);
    }
    size_t index = find_index(value);
    if (index == this->_size)
    {
        const_cast<ft_set*>(this)->set_error(FT_ERR_NOT_FOUND);
        this->_mutex.unlock(THREAD_ID);
        return (ft_nullptr);
    }
    ElementType* result = &this->_data[index];
    this->set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
    return (result);
}

template <typename ElementType>
void ft_set<ElementType>::remove(const ElementType& value)
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        this->set_error(FT_ERR_MUTEX_NOT_OWNER);
        return ;
    }
    size_t index = find_index(value);
    if (index == this->_size)
    {
        this->set_error(FT_ERR_NOT_FOUND);
        this->_mutex.unlock(THREAD_ID);
        return ;
    }
    destroy_at(&this->_data[index]);
    size_t current_index = index;
    while (current_index + 1 < this->_size)
    {
        construct_at(&this->_data[current_index], std::move(this->_data[current_index + 1]));
        destroy_at(&this->_data[current_index + 1]);
        ++current_index;
    }
    --this->_size;
    this->set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
    return ;
}

template <typename ElementType>
size_t ft_set<ElementType>::size() const
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        this->set_error(this->_mutex.get_error());
        return (0);
    }
    size_t current_size = this->_size;
    this->set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
    return (current_size);
}

template <typename ElementType>
bool ft_set<ElementType>::empty() const
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        this->set_error(this->_mutex.get_error());
        return (true);
    }
    bool result = (this->_size == 0);
    this->set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
    return (result);
}

template <typename ElementType>
int ft_set<ElementType>::get_error() const
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
        return (this->_error_code);
    int error = this->_error_code;
    this->_mutex.unlock(THREAD_ID);
    return (error);
}

template <typename ElementType>
const char* ft_set<ElementType>::get_error_str() const
{
    return (ft_strerror(this->get_error()));
}

template <typename ElementType>
void ft_set<ElementType>::clear()
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
        return ;
    size_t index = 0;
    while (index < this->_size)
    {
        destroy_at(&this->_data[index]);
        ++index;
    }
    this->_size = 0;
    this->set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
    return ;
}

#endif
