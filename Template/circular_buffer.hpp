#ifndef FT_CIRCULAR_BUFFER_HPP
#define FT_CIRCULAR_BUFFER_HPP

#include "constructor.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/nullptr.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/mutex.hpp"
#include <cstddef>
#include <utility>

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
        mutable pt_mutex _mutex;

        void set_error(int error) const;

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

        int get_error() const;
        const char* get_error_str() const;

        void clear();
};

template <typename ElementType>
ft_circular_buffer<ElementType>::ft_circular_buffer(size_t capacity)
    : _buffer(ft_nullptr), _capacity(capacity), _head(0), _tail(0), _size(0), _error_code(ER_SUCCESS)
{
    if (capacity == 0)
        return ;
    _buffer = static_cast<ElementType*>(cma_malloc(sizeof(ElementType) * capacity));
    if (_buffer == ft_nullptr)
    {
        this->set_error(CIRCULAR_BUFFER_ALLOC_FAIL);
        _capacity = 0;
    }
}

template <typename ElementType>
ft_circular_buffer<ElementType>::~ft_circular_buffer()
{
    this->clear();
    if (_buffer != ft_nullptr)
        cma_free(_buffer);
}

template <typename ElementType>
ft_circular_buffer<ElementType>::ft_circular_buffer(ft_circular_buffer&& other) noexcept
    : _buffer(other._buffer), _capacity(other._capacity), _head(other._head),
      _tail(other._tail), _size(other._size), _error_code(other._error_code)
{
    other._buffer = ft_nullptr;
    other._capacity = 0;
    other._head = 0;
    other._tail = 0;
    other._size = 0;
    other._error_code = ER_SUCCESS;
}

template <typename ElementType>
ft_circular_buffer<ElementType>& ft_circular_buffer<ElementType>::operator=(ft_circular_buffer&& other) noexcept
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
        if (this->_buffer != ft_nullptr)
            cma_free(this->_buffer);
        this->_buffer = other._buffer;
        this->_capacity = other._capacity;
        this->_head = other._head;
        this->_tail = other._tail;
        this->_size = other._size;
        this->_error_code = other._error_code;
        other._buffer = ft_nullptr;
        other._capacity = 0;
        other._head = 0;
        other._tail = 0;
        other._size = 0;
        other._error_code = ER_SUCCESS;
        other._mutex.unlock(THREAD_ID);
        this->_mutex.unlock(THREAD_ID);
    }
    return (*this);
}

template <typename ElementType>
void ft_circular_buffer<ElementType>::set_error(int error) const
{
    this->_error_code = error;
    ft_errno = error;
}

template <typename ElementType>
void ft_circular_buffer<ElementType>::push(const ElementType& value)
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->set_error(PT_ERR_MUTEX_OWNER);
        return ;
    }
    if (this->_size == this->_capacity)
    {
        this->set_error(CIRCULAR_BUFFER_FULL);
        this->_mutex.unlock(THREAD_ID);
        return ;
    }
    construct_at(&this->_buffer[this->_tail], value);
    this->_tail = (this->_tail + 1) % this->_capacity;
    ++this->_size;
    this->_mutex.unlock(THREAD_ID);
}

template <typename ElementType>
void ft_circular_buffer<ElementType>::push(ElementType&& value)
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->set_error(PT_ERR_MUTEX_OWNER);
        return ;
    }
    if (this->_size == this->_capacity)
    {
        this->set_error(CIRCULAR_BUFFER_FULL);
        this->_mutex.unlock(THREAD_ID);
        return ;
    }
    construct_at(&this->_buffer[this->_tail], std::move(value));
    this->_tail = (this->_tail + 1) % this->_capacity;
    ++this->_size;
    this->_mutex.unlock(THREAD_ID);
}

template <typename ElementType>
ElementType ft_circular_buffer<ElementType>::pop()
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->set_error(PT_ERR_MUTEX_OWNER);
        return (ElementType());
    }
    if (this->_size == 0)
    {
        this->set_error(CIRCULAR_BUFFER_EMPTY);
        this->_mutex.unlock(THREAD_ID);
        return (ElementType());
    }
    ElementType value = std::move(this->_buffer[this->_head]);
    destroy_at(&this->_buffer[this->_head]);
    this->_head = (this->_head + 1) % this->_capacity;
    --this->_size;
    this->_mutex.unlock(THREAD_ID);
    return (value);
}

template <typename ElementType>
bool ft_circular_buffer<ElementType>::is_full() const
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (false);
    bool res = (this->_size == this->_capacity);
    this->_mutex.unlock(THREAD_ID);
    return (res);
}

template <typename ElementType>
bool ft_circular_buffer<ElementType>::is_empty() const
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (true);
    bool res = (this->_size == 0);
    this->_mutex.unlock(THREAD_ID);
    return (res);
}

template <typename ElementType>
size_t ft_circular_buffer<ElementType>::size() const
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (0);
    size_t s = this->_size;
    this->_mutex.unlock(THREAD_ID);
    return (s);
}

template <typename ElementType>
size_t ft_circular_buffer<ElementType>::capacity() const
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (0);
    size_t c = this->_capacity;
    this->_mutex.unlock(THREAD_ID);
    return (c);
}

template <typename ElementType>
int ft_circular_buffer<ElementType>::get_error() const
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (this->_error_code);
    int err = this->_error_code;
    this->_mutex.unlock(THREAD_ID);
    return (err);
}

template <typename ElementType>
const char* ft_circular_buffer<ElementType>::get_error_str() const
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (ft_strerror(this->_error_code));
    int err = this->_error_code;
    this->_mutex.unlock(THREAD_ID);
    return (ft_strerror(err));
}

template <typename ElementType>
void ft_circular_buffer<ElementType>::clear()
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return ;
    size_t i = 0;
    while (i < this->_size)
    {
        destroy_at(&this->_buffer[(this->_head + i) % this->_capacity]);
        ++i;
    }
    this->_head = 0;
    this->_tail = 0;
    this->_size = 0;
    this->_mutex.unlock(THREAD_ID);
}

#endif
