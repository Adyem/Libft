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
    {
        this->set_error(ER_SUCCESS);
        return ;
    }
    _buffer = static_cast<ElementType*>(cma_malloc(sizeof(ElementType) * capacity));
    if (_buffer == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        _capacity = 0;
        return ;
    }
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ElementType>
ft_circular_buffer<ElementType>::~ft_circular_buffer()
{
    this->clear();
    if (_buffer != ft_nullptr)
        cma_free(_buffer);
    this->set_error(ER_SUCCESS);
    return ;
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
    if (this->_size == this->_capacity)
    {
        this->set_error(FT_ERR_FULL);
        return ;
    }
    construct_at(&this->_buffer[this->_tail], value);
    this->_tail = (this->_tail + 1) % this->_capacity;
    ++this->_size;
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ElementType>
void ft_circular_buffer<ElementType>::push(ElementType&& value)
{
    if (this->_size == this->_capacity)
    {
        this->set_error(FT_ERR_FULL);
        return ;
    }
    construct_at(&this->_buffer[this->_tail], ft_move(value));
    this->_tail = (this->_tail + 1) % this->_capacity;
    ++this->_size;
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ElementType>
ElementType ft_circular_buffer<ElementType>::pop()
{
    if (this->_size == 0)
    {
        this->set_error(FT_ERR_EMPTY);
        return (ElementType());
    }
    ElementType value = ft_move(this->_buffer[this->_head]);
    destroy_at(&this->_buffer[this->_head]);
    this->_head = (this->_head + 1) % this->_capacity;
    --this->_size;
    this->set_error(ER_SUCCESS);
    return (value);
}

template <typename ElementType>
bool ft_circular_buffer<ElementType>::is_full() const
{
    bool res;

    res = (this->_size == this->_capacity);
    const_cast<ft_circular_buffer<ElementType> *>(this)->set_error(ER_SUCCESS);
    return (res);
}

template <typename ElementType>
bool ft_circular_buffer<ElementType>::is_empty() const
{
    bool res;

    res = (this->_size == 0);
    const_cast<ft_circular_buffer<ElementType> *>(this)->set_error(ER_SUCCESS);
    return (res);
}

template <typename ElementType>
size_t ft_circular_buffer<ElementType>::size() const
{
    size_t current_size;

    current_size = this->_size;
    const_cast<ft_circular_buffer<ElementType> *>(this)->set_error(ER_SUCCESS);
    return (current_size);
}

template <typename ElementType>
size_t ft_circular_buffer<ElementType>::capacity() const
{
    size_t current_capacity;

    current_capacity = this->_capacity;
    const_cast<ft_circular_buffer<ElementType> *>(this)->set_error(ER_SUCCESS);
    return (current_capacity);
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
    size_t i = 0;
    while (i < this->_size)
    {
        destroy_at(&this->_buffer[(this->_head + i) % this->_capacity]);
        ++i;
    }
    this->_head = 0;
    this->_tail = 0;
    this->_size = 0;
    this->set_error(ER_SUCCESS);
    return ;
}

#endif
