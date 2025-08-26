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
        mutable pt_mutex _mutex;

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

template <typename ElementType>
ft_queue<ElementType>::ft_queue()
    : _front(ft_nullptr), _rear(ft_nullptr), _size(0), _errorCode(ER_SUCCESS)
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
        if (this->_mutex.lock(THREAD_ID) != SUCCES)
            return (*this);
        if (other._mutex.lock(THREAD_ID) != SUCCES)
        {
            this->_mutex.unlock(THREAD_ID);
            return (*this);
        }
        this->clear();
        this->_front = other._front;
        this->_rear = other._rear;
        this->_size = other._size;
        this->_errorCode = other._errorCode;
        other._front = ft_nullptr;
        other._rear = ft_nullptr;
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
    return ;
}

template <typename ElementType>
void ft_queue<ElementType>::enqueue(const ElementType& value)
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->setError(PT_ERR_MUTEX_OWNER);
        return ;
    }
    QueueNode* node = static_cast<QueueNode*>(cma_malloc(sizeof(QueueNode)));
    if (node == ft_nullptr)
    {
        this->setError(QUEUE_ALLOC_FAIL);
        this->_mutex.unlock(THREAD_ID);
        return ;
    }
    construct_at(&node->data, value);
    node->next = ft_nullptr;
    if (this->_rear == ft_nullptr)
    {
        this->_front = node;
        this->_rear = node;
    }
    else
    {
        this->_rear->next = node;
        this->_rear = node;
    }
    ++this->_size;
    this->_mutex.unlock(THREAD_ID);
    return ;
}

template <typename ElementType>
void ft_queue<ElementType>::enqueue(ElementType&& value)
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->setError(PT_ERR_MUTEX_OWNER);
        return ;
    }
    QueueNode* node = static_cast<QueueNode*>(cma_malloc(sizeof(QueueNode)));
    if (node == ft_nullptr)
    {
        this->setError(QUEUE_ALLOC_FAIL);
        this->_mutex.unlock(THREAD_ID);
        return ;
    }
    construct_at(&node->data, std::move(value));
    node->next = ft_nullptr;
    if (this->_rear == ft_nullptr)
    {
        this->_front = node;
        this->_rear = node;
    }
    else
    {
        this->_rear->next = node;
        this->_rear = node;
    }
    ++this->_size;
    this->_mutex.unlock(THREAD_ID);
    return ;
}

template <typename ElementType>
ElementType ft_queue<ElementType>::dequeue()
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->setError(PT_ERR_MUTEX_OWNER);
        return (ElementType());
    }
    if (this->_front == ft_nullptr)
    {
        this->setError(QUEUE_EMPTY);
        this->_mutex.unlock(THREAD_ID);
        return (ElementType());
    }
    QueueNode* node = this->_front;
    this->_front = node->next;
    if (this->_front == ft_nullptr)
        this->_rear = ft_nullptr;
    ElementType value = std::move(node->data);
    destroy_at(&node->data);
    cma_free(node);
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
    if (this->_front == ft_nullptr)
    {
        this->setError(QUEUE_EMPTY);
        this->_mutex.unlock(THREAD_ID);
        return (errorElement);
    }
    ElementType& ref = this->_front->data;
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
    if (this->_front == ft_nullptr)
    {
        this->setError(QUEUE_EMPTY);
        this->_mutex.unlock(THREAD_ID);
        return (errorElement);
    }
    ElementType& ref = this->_front->data;
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
    while (this->_front != ft_nullptr)
    {
        QueueNode* node = this->_front;
        this->_front = this->_front->next;
        destroy_at(&node->data);
        cma_free(node);
    }
    this->_rear = ft_nullptr;
    this->_size = 0;
    this->_mutex.unlock(THREAD_ID);
    return ;
}

#endif
