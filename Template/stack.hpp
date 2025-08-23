#ifndef FT_STACK_HPP
#define FT_STACK_HPP

#include "constructor.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/nullptr.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/mutex.hpp"
#include <cstddef>
#include <utility>
#include <new>

#ifndef ft_move
# define ft_move std::move
#endif

template <typename ElementType>
class ft_stack
{
private:
    ElementType*     _data;
    size_t           _capacity;
    size_t           _size;
    mutable int      _errorCode;
    mutable pt_mutex _mutex;

    void    setError(int error) const;
    void    resize(size_t new_capacity);

public:
    ft_stack();
    ~ft_stack();

    ft_stack(const ft_stack&) = delete;
    ft_stack& operator=(const ft_stack&) = delete;

    ft_stack(ft_stack&& other) noexcept;
    ft_stack& operator=(ft_stack&& other) noexcept;

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

template <typename ElementType>
ft_stack<ElementType>::ft_stack()
    : _data(static_cast<ElementType*>(cma_malloc(sizeof(ElementType) * 16))),
      _capacity(16), _size(0), _errorCode(ER_SUCCESS)
{
    return ;
}

template <typename ElementType>
ft_stack<ElementType>::~ft_stack()
{
    this->clear();
    cma_free(this->_data);
}

template <typename ElementType>
ft_stack<ElementType>::ft_stack(ft_stack&& other) noexcept
    : _data(other._data), _capacity(other._capacity), _size(other._size),
      _errorCode(other._errorCode)
{
    other._data = ft_nullptr;
    other._capacity = 0;
    other._size = 0;
    other._errorCode = ER_SUCCESS;
}

template <typename ElementType>
ft_stack<ElementType>& ft_stack<ElementType>::operator=(ft_stack&& other) noexcept
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
void ft_stack<ElementType>::setError(int error) const
{
    this->_errorCode = error;
    ft_errno = error;
}

template <typename ElementType>
void ft_stack<ElementType>::resize(size_t new_capacity)
{
    ElementType* new_data = static_cast<ElementType*>(cma_malloc(sizeof(ElementType) * new_capacity));
    for (size_t i = 0; i < this->_size; ++i)
    {
        construct_at(&new_data[i], ft_move(this->_data[i]));
        destroy_at(&this->_data[i]);
    }
    cma_free(this->_data);
    this->_data = new_data;
    this->_capacity = new_capacity;
}

template <typename ElementType>
void ft_stack<ElementType>::push(const ElementType& value)
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->setError(PT_ERR_MUTEX_OWNER);
        return ;
    }
    if (this->_size == this->_capacity)
        this->resize(this->_capacity * 2);
    construct_at(&this->_data[this->_size], value);
    ++this->_size;
    this->_mutex.unlock(THREAD_ID);
}

template <typename ElementType>
void ft_stack<ElementType>::push(ElementType&& value)
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->setError(PT_ERR_MUTEX_OWNER);
        return ;
    }
    if (this->_size == this->_capacity)
        this->resize(this->_capacity * 2);
    construct_at(&this->_data[this->_size], ft_move(value));
    ++this->_size;
    this->_mutex.unlock(THREAD_ID);
}

template <typename ElementType>
ElementType ft_stack<ElementType>::pop()
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->setError(PT_ERR_MUTEX_OWNER);
        return (ElementType());
    }
    if (this->_size == 0)
    {
        this->setError(STACK_EMPTY);
        this->_mutex.unlock(THREAD_ID);
        return (ElementType());
    }
    --this->_size;
    ElementType value = ft_move(this->_data[this->_size]);
    destroy_at(&this->_data[this->_size]);
    this->_mutex.unlock(THREAD_ID);
    return (value);
}

template <typename ElementType>
ElementType& ft_stack<ElementType>::top()
{
    static ElementType errorElement = ElementType();
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->setError(PT_ERR_MUTEX_OWNER);
        return (errorElement);
    }
    if (this->_size == 0)
    {
        this->setError(STACK_EMPTY);
        this->_mutex.unlock(THREAD_ID);
        return (errorElement);
    }
    ElementType& ref = this->_data[this->_size - 1];
    this->_mutex.unlock(THREAD_ID);
    return (ref);
}

template <typename ElementType>
const ElementType& ft_stack<ElementType>::top() const
{
    static ElementType errorElement = ElementType();
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->setError(PT_ERR_MUTEX_OWNER);
        return (errorElement);
    }
    if (this->_size == 0)
    {
        this->setError(STACK_EMPTY);
        this->_mutex.unlock(THREAD_ID);
        return (errorElement);
    }
    ElementType& ref = this->_data[this->_size - 1];
    this->_mutex.unlock(THREAD_ID);
    return (ref);
}

template <typename ElementType>
size_t ft_stack<ElementType>::size() const
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (0);
    size_t s = this->_size;
    this->_mutex.unlock(THREAD_ID);
    return (s);
}

template <typename ElementType>
bool ft_stack<ElementType>::empty() const
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (true);
    bool res = (this->_size == 0);
    this->_mutex.unlock(THREAD_ID);
    return (res);
}

template <typename ElementType>
int ft_stack<ElementType>::get_error() const
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (this->_errorCode);
    int err = this->_errorCode;
    this->_mutex.unlock(THREAD_ID);
    return (err);
}

template <typename ElementType>
const char* ft_stack<ElementType>::get_error_str() const
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (ft_strerror(this->_errorCode));
    int err = this->_errorCode;
    this->_mutex.unlock(THREAD_ID);
    return (ft_strerror(err));
}

template <typename ElementType>
void ft_stack<ElementType>::clear()
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return ;
    for (size_t i = 0; i < this->_size; ++i)
        destroy_at(&this->_data[i]);
    this->_size = 0;
    this->_mutex.unlock(THREAD_ID);
}

#endif
