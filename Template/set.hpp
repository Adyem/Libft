#ifndef FT_SET_HPP
#define FT_SET_HPP

#include "constructor.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/nullptr.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/mutex.hpp"
#include <cstddef>
#include <utility>

/*
 * ft_set
 * Sorted collection of unique values. Provides basic insertion,
 * lookup and removal with thread-safe error reporting.
 */

template <typename ElementType>
class ft_set
{
    private:
        ElementType*   _data;
        size_t         _capacity;
        size_t         _size;
        mutable int    _errorCode;
        mutable pt_mutex _mutex;

        void    setError(int error) const;
        bool    ensure_capacity(size_t desired);
        size_t  find_index(const ElementType& value) const; // caller must hold lock
        size_t  lower_bound(const ElementType& value) const; // caller must hold lock

    public:
        ft_set(size_t initialCapacity = 0);
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
ft_set<ElementType>::ft_set(size_t initialCapacity)
    : _data(ft_nullptr), _capacity(0), _size(0), _errorCode(ER_SUCCESS)
{
    if (initialCapacity > 0)
    {
        this->_data = static_cast<ElementType*>(cma_malloc(sizeof(ElementType) * initialCapacity));
        if (this->_data == ft_nullptr)
            this->setError(SET_ALLOC_FAIL);
        else
            this->_capacity = initialCapacity;
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
      _errorCode(other._errorCode)
{
    other._data = ft_nullptr;
    other._capacity = 0;
    other._size = 0;
    other._errorCode = ER_SUCCESS;
    return ;
}

template <typename ElementType>
ft_set<ElementType>& ft_set<ElementType>::operator=(ft_set&& other) noexcept
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
        this->_errorCode = other._errorCode;
        other._data = ft_nullptr;
        other._capacity = 0;
        other._size = 0;
        other._errorCode = ER_SUCCESS;
        other._mutex.unlock(THREAD_ID);
        this->_mutex.unlock(THREAD_ID);
    }
    return (*this);
}

template <typename ElementType>
void ft_set<ElementType>::setError(int error) const
{
    this->_errorCode = error;
    ft_errno = error;
    return ;
}

template <typename ElementType>
bool ft_set<ElementType>::ensure_capacity(size_t desired)
{
    if (desired <= this->_capacity)
        return (true);
    size_t newCap = (this->_capacity == 0) ? 1 : this->_capacity * 2;
    while (newCap < desired)
        newCap *= 2;
    ElementType* newData = static_cast<ElementType*>(cma_malloc(sizeof(ElementType) * newCap));
    if (newData == ft_nullptr)
    {
        this->setError(SET_ALLOC_FAIL);
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
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->setError(PT_ERR_MUTEX_OWNER);
        return ;
    }
    size_t pos = lower_bound(value);
    if (pos < this->_size && !(value < this->_data[pos]) && !(this->_data[pos] < value))
    {
        this->_mutex.unlock(THREAD_ID);
        return ;
    }
    if (!ensure_capacity(this->_size + 1))
    {
        this->_mutex.unlock(THREAD_ID);
        return ;
    }
    size_t i = this->_size;
    while (i > pos)
    {
        construct_at(&this->_data[i], std::move(this->_data[i - 1]));
        destroy_at(&this->_data[i - 1]);
        --i;
    }
    construct_at(&this->_data[pos], value);
    ++this->_size;
    this->_mutex.unlock(THREAD_ID);
    return ;
}

template <typename ElementType>
void ft_set<ElementType>::insert(ElementType&& value)
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->setError(PT_ERR_MUTEX_OWNER);
        return ;
    }
    size_t pos = lower_bound(value);
    if (pos < this->_size && !(value < this->_data[pos]) && !(this->_data[pos] < value))
    {
        this->_mutex.unlock(THREAD_ID);
        return ;
    }
    if (!ensure_capacity(this->_size + 1))
    {
        this->_mutex.unlock(THREAD_ID);
        return ;
    }
    size_t i = this->_size;
    while (i > pos)
    {
        construct_at(&this->_data[i], std::move(this->_data[i - 1]));
        destroy_at(&this->_data[i - 1]);
        --i;
    }
    construct_at(&this->_data[pos], std::move(value));
    ++this->_size;
    this->_mutex.unlock(THREAD_ID);
    return ;
}

template <typename ElementType>
ElementType* ft_set<ElementType>::find(const ElementType& value)
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->setError(PT_ERR_MUTEX_OWNER);
        return (ft_nullptr);
    }
    size_t idx = find_index(value);
    if (idx == this->_size)
    {
        this->setError(SET_NOT_FOUND);
        this->_mutex.unlock(THREAD_ID);
        return (ft_nullptr);
    }
    ElementType* res = &this->_data[idx];
    this->_mutex.unlock(THREAD_ID);
    return (res);
}

template <typename ElementType>
const ElementType* ft_set<ElementType>::find(const ElementType& value) const
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        const_cast<ft_set*>(this)->setError(PT_ERR_MUTEX_OWNER);
        return (ft_nullptr);
    }
    size_t idx = find_index(value);
    if (idx == this->_size)
    {
        const_cast<ft_set*>(this)->setError(SET_NOT_FOUND);
        this->_mutex.unlock(THREAD_ID);
        return (ft_nullptr);
    }
    ElementType* res = &this->_data[idx];
    this->_mutex.unlock(THREAD_ID);
    return (res);
}

template <typename ElementType>
void ft_set<ElementType>::remove(const ElementType& value)
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->setError(PT_ERR_MUTEX_OWNER);
        return ;
    }
    size_t idx = find_index(value);
    if (idx == this->_size)
    {
        this->setError(SET_NOT_FOUND);
        this->_mutex.unlock(THREAD_ID);
        return ;
    }
    destroy_at(&this->_data[idx]);
    size_t i = idx;
    while (i + 1 < this->_size)
    {
        construct_at(&this->_data[i], std::move(this->_data[i + 1]));
        destroy_at(&this->_data[i + 1]);
        ++i;
    }
    --this->_size;
    this->_mutex.unlock(THREAD_ID);
    return ;
}

template <typename ElementType>
size_t ft_set<ElementType>::size() const
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (0);
    size_t s = this->_size;
    this->_mutex.unlock(THREAD_ID);
    return (s);
}

template <typename ElementType>
bool ft_set<ElementType>::empty() const
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (true);
    bool res = (this->_size == 0);
    this->_mutex.unlock(THREAD_ID);
    return (res);
}

template <typename ElementType>
int ft_set<ElementType>::get_error() const
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (this->_errorCode);
    int err = this->_errorCode;
    this->_mutex.unlock(THREAD_ID);
    return (err);
}

template <typename ElementType>
const char* ft_set<ElementType>::get_error_str() const
{
    return (ft_strerror(this->get_error()));
}

template <typename ElementType>
void ft_set<ElementType>::clear()
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
