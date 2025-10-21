#ifndef FT_QUEUE_HPP
#define FT_QUEUE_HPP

#include "constructor.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Libft/libft.hpp"
#include <cstddef>
#include <utility>
#include "move.hpp"

template <typename ElementType>
class ft_queue
{
    private:
        struct QueueNode
        {
            ElementType _data;
            QueueNode* _next;
        };

        QueueNode*  _front;
        QueueNode*  _rear;
        size_t      _size;
        mutable int _error_code;

        void    set_error(int error) const;

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
    : _front(ft_nullptr), _rear(ft_nullptr), _size(0), _error_code(ER_SUCCESS)
{
    return ;
}

template <typename ElementType>
ft_queue<ElementType>::~ft_queue()
{
    this->clear();
    return ;
}

template <typename ElementType>
ft_queue<ElementType>::ft_queue(ft_queue&& other) noexcept
    : _front(other._front), _rear(other._rear), _size(other._size), _error_code(other._error_code)
{
    other._front = ft_nullptr;
    other._rear = ft_nullptr;
    other._size = 0;
    other._error_code = ER_SUCCESS;
    return ;
}

template <typename ElementType>
ft_queue<ElementType>& ft_queue<ElementType>::operator=(ft_queue&& other) noexcept
{
    if (this != &other)
    {
        this->clear();
        this->_front = other._front;
        this->_rear = other._rear;
        this->_size = other._size;
        this->_error_code = other._error_code;
        other._front = ft_nullptr;
        other._rear = ft_nullptr;
        other._size = 0;
        other._error_code = ER_SUCCESS;
    }
    return (*this);
}

template <typename ElementType>
void ft_queue<ElementType>::set_error(int error) const
{
    this->_error_code = error;
    ft_errno = error;
    return ;
}

template <typename ElementType>
void ft_queue<ElementType>::enqueue(const ElementType& value)
{
    QueueNode* node = static_cast<QueueNode*>(cma_malloc(sizeof(QueueNode)));
    if (node == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return ;
    }
    construct_at(&node->_data, value);
    node->_next = ft_nullptr;
    if (this->_rear == ft_nullptr)
    {
        this->_front = node;
        this->_rear = node;
    }
    else
    {
        this->_rear->_next = node;
        this->_rear = node;
    }
    ++this->_size;
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ElementType>
void ft_queue<ElementType>::enqueue(ElementType&& value)
{
    QueueNode* node = static_cast<QueueNode*>(cma_malloc(sizeof(QueueNode)));
    if (node == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return ;
    }
    construct_at(&node->_data, ft_move(value));
    node->_next = ft_nullptr;
    if (this->_rear == ft_nullptr)
    {
        this->_front = node;
        this->_rear = node;
    }
    else
    {
        this->_rear->_next = node;
        this->_rear = node;
    }
    ++this->_size;
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ElementType>
ElementType ft_queue<ElementType>::dequeue()
{
    if (this->_front == ft_nullptr)
    {
        this->set_error(FT_ERR_EMPTY);
        return (ElementType());
    }
    QueueNode* node = this->_front;
    this->_front = node->_next;
    if (this->_front == ft_nullptr)
        this->_rear = ft_nullptr;
    ElementType value = ft_move(node->_data);
    destroy_at(&node->_data);
    cma_free(node);
    --this->_size;
    this->set_error(ER_SUCCESS);
    return (value);
}

template <typename ElementType>
ElementType& ft_queue<ElementType>::front()
{
    static ElementType error_element = ElementType();
    if (this->_front == ft_nullptr)
    {
        this->set_error(FT_ERR_EMPTY);
        return (error_element);
    }
    ElementType& value = this->_front->_data;
    this->set_error(ER_SUCCESS);
    return (value);
}

template <typename ElementType>
const ElementType& ft_queue<ElementType>::front() const
{
    static ElementType error_element = ElementType();
    if (this->_front == ft_nullptr)
    {
        this->set_error(FT_ERR_EMPTY);
        return (error_element);
    }
    const ElementType& value = this->_front->_data;
    this->set_error(ER_SUCCESS);
    return (value);
}

template <typename ElementType>
size_t ft_queue<ElementType>::size() const
{
    const_cast<ft_queue<ElementType>*>(this)->set_error(ER_SUCCESS);
    return (this->_size);
}

template <typename ElementType>
bool ft_queue<ElementType>::empty() const
{
    const_cast<ft_queue<ElementType>*>(this)->set_error(ER_SUCCESS);
    return (this->_size == 0);
}

template <typename ElementType>
int ft_queue<ElementType>::get_error() const
{
    return (this->_error_code);
}

template <typename ElementType>
const char* ft_queue<ElementType>::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}

template <typename ElementType>
void ft_queue<ElementType>::clear()
{
    while (this->_front != ft_nullptr)
    {
        QueueNode* node = this->_front;
        this->_front = this->_front->_next;
        destroy_at(&node->_data);
        cma_free(node);
    }
    this->_rear = ft_nullptr;
    this->_size = 0;
    this->set_error(ER_SUCCESS);
    return ;
}

#endif
