#ifndef FT_QUEUE_HPP
#define FT_QUEUE_HPP

#include "constructor.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/nullptr.hpp"
#include <cstddef>
#include <utility>

// A simple queue implementation using a singly linked list.
// Provides basic queue operations with error handling similar to other
// template classes in the project.

template <typename ElementType>
class ft_queue
{
private:
    struct QueueNode
    {
        ElementType data;
        QueueNode* next;
    };

    QueueNode*  _front;
    QueueNode*  _rear;
    size_t      _size;
    mutable int _errorCode;

    void    setError(int error) const;

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

// Implementation

template <typename ElementType>
ft_queue<ElementType>::ft_queue()
    : _front(ft_nullptr), _rear(ft_nullptr), _size(0), _errorCode(ER_SUCCESS)
{
    return ;
}

template <typename ElementType>
ft_queue<ElementType>::~ft_queue()
{
    clear();
    return ;
}

template <typename ElementType>
ft_queue<ElementType>::ft_queue(ft_queue&& other) noexcept
    : _front(other._front), _rear(other._rear), _size(other._size), _errorCode(other._errorCode)
{
    other._front = ft_nullptr;
    other._rear = ft_nullptr;
    other._size = 0;
    other._errorCode = ER_SUCCESS;
    return ;
}

template <typename ElementType>
ft_queue<ElementType>& ft_queue<ElementType>::operator=(ft_queue&& other) noexcept
{
    if (this != &other)
    {
        clear();
        _front = other._front;
        _rear = other._rear;
        _size = other._size;
        _errorCode = other._errorCode;
        other._front = ft_nullptr;
        other._rear = ft_nullptr;
        other._size = 0;
        other._errorCode = ER_SUCCESS;
    }
    return (*this);
}

template <typename ElementType>
void ft_queue<ElementType>::setError(int error) const
{
    _errorCode = error;
    ft_errno = error;
    return ;
}

template <typename ElementType>
void ft_queue<ElementType>::enqueue(const ElementType& value)
{
    QueueNode* node = static_cast<QueueNode*>(cma_malloc(sizeof(QueueNode)));
    if (node == ft_nullptr)
    {
        setError(QUEUE_ALLOC_FAIL);
        return ;
    }
    construct_at(&node->data, value);
    node->next = ft_nullptr;
    if (_rear == ft_nullptr)
    {
        _front = node;
        _rear = node;
    }
    else
    {
        _rear->next = node;
        _rear = node;
    }
    ++_size;
    return ;
}

template <typename ElementType>
void ft_queue<ElementType>::enqueue(ElementType&& value)
{
    QueueNode* node = static_cast<QueueNode*>(cma_malloc(sizeof(QueueNode)));
    if (node == ft_nullptr)
    {
        setError(QUEUE_ALLOC_FAIL);
        return ;
    }
    construct_at(&node->data, std::move(value));
    node->next = ft_nullptr;
    if (_rear == ft_nullptr)
    {
        _front = node;
        _rear = node;
    }
    else
    {
        _rear->next = node;
        _rear = node;
    }
    ++_size;
    return ;
}

template <typename ElementType>
ElementType ft_queue<ElementType>::dequeue()
{
    if (_front == ft_nullptr)
    {
        setError(QUEUE_EMPTY);
        return (ElementType());
    }
    QueueNode* node = _front;
    _front = node->next;
    if (_front == ft_nullptr)
        _rear = ft_nullptr;
    ElementType value = std::move(node->data);
    destroy_at(&node->data);
    cma_free(node);
    --_size;
    return (value);
}

template <typename ElementType>
ElementType& ft_queue<ElementType>::front()
{
    static ElementType errorElement = ElementType();
    if (_front == ft_nullptr)
    {
        setError(QUEUE_EMPTY);
        return (errorElement);
    }
    return (_front->data);
}

template <typename ElementType>
const ElementType& ft_queue<ElementType>::front() const
{
    static ElementType errorElement = ElementType();
    if (_front == ft_nullptr)
    {
        setError(QUEUE_EMPTY);
        return (errorElement);
    }
    return (_front->data);
}

template <typename ElementType>
size_t ft_queue<ElementType>::size() const
{
    return (_size);
}

template <typename ElementType>
bool ft_queue<ElementType>::empty() const
{
    return (_size == 0);
}

template <typename ElementType>
int ft_queue<ElementType>::get_error() const
{
    return (_errorCode);
}

template <typename ElementType>
const char* ft_queue<ElementType>::get_error_str() const
{
    return (ft_strerror(_errorCode));
}

template <typename ElementType>
void ft_queue<ElementType>::clear()
{
    while (_front != ft_nullptr)
    {
        QueueNode* node = _front;
        _front = _front->next;
        destroy_at(&node->data);
        cma_free(node);
    }
    _rear = ft_nullptr;
    _size = 0;
    return ;
}

#endif
