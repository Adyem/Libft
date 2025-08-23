#ifndef FT_QUEUE_HPP
#define FT_QUEUE_HPP

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
class ft_queue
{
private:
    ElementType*       _data;
    size_t             _capacity;
    size_t             _head;
    size_t             _tail;
    size_t             _size;
    mutable int        _errorCode;
    mutable pt_mutex   _mutex;

    void    setError(int error) const;
    void    resize(size_t new_capacity);

public:
    ft_queue();
    ~ft_queue();

    ft_queue(const ft_queue&) = delete;
    ft_queue& operator=(const ft_queue&) = delete;

    ft_queue(ft_queue&& other) noexcept;
    ft_queue& operator=(ft_queue&& other) noexcept;

    void enqueue(const ElementType& value);
    void enqueue(ElementType&& value);
    ElementType dequeue();

    ElementType& front();
    const ElementType& front() const;

    size_t size() const;
    bool empty() const;

    int get_error() const;
    const char* get_error_str() const;

    void clear();
};

template <typename ElementType>
ft_queue<ElementType>::ft_queue()
    : _data(static_cast<ElementType*>(cma_malloc(sizeof(ElementType) * 16))),
      _capacity(16), _head(0), _tail(0), _size(0), _errorCode(ER_SUCCESS)
{
    return ;
}

template <typename ElementType>
ft_queue<ElementType>::~ft_queue()
{
    this->clear();
    cma_free(this->_data);
}

template <typename ElementType>
ft_queue<ElementType>::ft_queue(ft_queue&& other) noexcept
    : _data(other._data), _capacity(other._capacity), _head(other._head),
      _tail(other._tail), _size(other._size), _errorCode(other._errorCode)
{
    other._data = ft_nullptr;
    other._capacity = 0;
    other._head = 0;
    other._tail = 0;
    other._size = 0;
    other._errorCode = ER_SUCCESS;
}

template <typename ElementType>
ft_queue<ElementType>& ft_queue<ElementType>::operator=(ft_queue&& other) noexcept
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
        this->_head = other._head;
        this->_tail = other._tail;
        this->_size = other._size;
        this->_errorCode = other._errorCode;
        other._data = ft_nullptr;
        other._capacity = 0;
        other._head = 0;
        other._tail = 0;
        other._size = 0;
        other._errorCode = ER_SUCCESS;
        other._mutex.unlock(THREAD_ID);
        this->_mutex.unlock(THREAD_ID);
    }
    return (*this);
}

template <typename ElementType>
void ft_queue<ElementType>::setError(int error) const
{
    this->_errorCode = error;
    ft_errno = error;
}

template <typename ElementType>
void ft_queue<ElementType>::resize(size_t new_capacity)
{
    ElementType* new_data = static_cast<ElementType*>(cma_malloc(sizeof(ElementType) * new_capacity));
    for (size_t i = 0; i < this->_size; ++i)
    {
        size_t idx = (this->_head + i) & (this->_capacity - 1);
        construct_at(&new_data[i], ft_move(this->_data[idx]));
        destroy_at(&this->_data[idx]);
    }
    cma_free(this->_data);
    this->_data = new_data;
    this->_capacity = new_capacity;
    this->_head = 0;
    this->_tail = this->_size;
}

template <typename ElementType>
void ft_queue<ElementType>::enqueue(const ElementType& value)
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->setError(PT_ERR_MUTEX_OWNER);
        return ;
    }
    if (this->_size == this->_capacity)
        this->resize(this->_capacity * 2);
    construct_at(&this->_data[this->_tail], value);
    this->_tail = (this->_tail + 1) & (this->_capacity - 1);
    ++this->_size;
    this->_mutex.unlock(THREAD_ID);
}

template <typename ElementType>
void ft_queue<ElementType>::enqueue(ElementType&& value)
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->setError(PT_ERR_MUTEX_OWNER);
        return ;
    }
    if (this->_size == this->_capacity)
        this->resize(this->_capacity * 2);
    construct_at(&this->_data[this->_tail], ft_move(value));
    this->_tail = (this->_tail + 1) & (this->_capacity - 1);
    ++this->_size;
    this->_mutex.unlock(THREAD_ID);
}

template <typename ElementType>
ElementType ft_queue<ElementType>::dequeue()
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->setError(PT_ERR_MUTEX_OWNER);
        return (ElementType());
    }
    if (this->_size == 0)
    {
        this->setError(QUEUE_EMPTY);
        this->_mutex.unlock(THREAD_ID);
        return (ElementType());
    }
    ElementType value = ft_move(this->_data[this->_head]);
    destroy_at(&this->_data[this->_head]);
    this->_head = (this->_head + 1) & (this->_capacity - 1);
    --this->_size;
    this->_mutex.unlock(THREAD_ID);
    return (value);
}

template <typename ElementType>
ElementType& ft_queue<ElementType>::front()
{
    static ElementType errorElement = ElementType();
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->setError(PT_ERR_MUTEX_OWNER);
        return (errorElement);
    }
    if (this->_size == 0)
    {
        this->setError(QUEUE_EMPTY);
        this->_mutex.unlock(THREAD_ID);
        return (errorElement);
    }
    ElementType& ref = this->_data[this->_head];
    this->_mutex.unlock(THREAD_ID);
    return (ref);
}

template <typename ElementType>
const ElementType& ft_queue<ElementType>::front() const
{
    static ElementType errorElement = ElementType();
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->setError(PT_ERR_MUTEX_OWNER);
        return (errorElement);
    }
    if (this->_size == 0)
    {
        this->setError(QUEUE_EMPTY);
        this->_mutex.unlock(THREAD_ID);
        return (errorElement);
    }
    ElementType& ref = this->_data[this->_head];
    this->_mutex.unlock(THREAD_ID);
    return (ref);
}

template <typename ElementType>
size_t ft_queue<ElementType>::size() const
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (0);
    size_t s = this->_size;
    this->_mutex.unlock(THREAD_ID);
    return (s);
}

template <typename ElementType>
bool ft_queue<ElementType>::empty() const
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (true);
    bool res = (this->_size == 0);
    this->_mutex.unlock(THREAD_ID);
    return (res);
}

template <typename ElementType>
int ft_queue<ElementType>::get_error() const
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (this->_errorCode);
    int err = this->_errorCode;
    this->_mutex.unlock(THREAD_ID);
    return (err);
}

template <typename ElementType>
const char* ft_queue<ElementType>::get_error_str() const
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (ft_strerror(this->_errorCode));
    int err = this->_errorCode;
    this->_mutex.unlock(THREAD_ID);
    return (ft_strerror(err));
}

template <typename ElementType>
void ft_queue<ElementType>::clear()
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return ;
    for (size_t i = 0; i < this->_size; ++i)
    {
        size_t idx = (this->_head + i) & (this->_capacity - 1);
        destroy_at(&this->_data[idx]);
    }
    this->_head = 0;
    this->_tail = 0;
    this->_size = 0;
    this->_mutex.unlock(THREAD_ID);
}

#endif
