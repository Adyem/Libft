#ifndef FT_QUEUE_HPP
#define FT_QUEUE_HPP

#include "template_constructor.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/cpp_class_nullptr.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/pthread_mutex.hpp"
#include <cstddef>
#include <utility>

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
        mutable pt_mutex _mutex;

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
        this->_error_code = other._error_code;
        other._front = ft_nullptr;
        other._rear = ft_nullptr;
        other._size = 0;
        other._error_code = ER_SUCCESS;
        other._mutex.unlock(THREAD_ID);
        this->_mutex.unlock(THREAD_ID);
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
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->set_error(PT_ERR_MUTEX_OWNER);
        return ;
    }
    QueueNode* node = static_cast<QueueNode*>(cma_malloc(sizeof(QueueNode)));
    if (node == ft_nullptr)
    {
        this->set_error(QUEUE_ALLOC_FAIL);
        this->_mutex.unlock(THREAD_ID);
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
    this->_mutex.unlock(THREAD_ID);
    return ;
}

template <typename ElementType>
void ft_queue<ElementType>::enqueue(ElementType&& value)
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->set_error(PT_ERR_MUTEX_OWNER);
        return ;
    }
    QueueNode* node = static_cast<QueueNode*>(cma_malloc(sizeof(QueueNode)));
    if (node == ft_nullptr)
    {
        this->set_error(QUEUE_ALLOC_FAIL);
        this->_mutex.unlock(THREAD_ID);
        return ;
    }
    construct_at(&node->_data, std::move(value));
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
    this->_mutex.unlock(THREAD_ID);
    return ;
}

template <typename ElementType>
ElementType ft_queue<ElementType>::dequeue()
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->set_error(PT_ERR_MUTEX_OWNER);
        return (ElementType());
    }
    if (this->_front == ft_nullptr)
    {
        this->set_error(QUEUE_EMPTY);
        this->_mutex.unlock(THREAD_ID);
        return (ElementType());
    }
    QueueNode* node = this->_front;
    this->_front = node->_next;
    if (this->_front == ft_nullptr)
        this->_rear = ft_nullptr;
    ElementType value = std::move(node->_data);
    destroy_at(&node->_data);
    cma_free(node);
    --this->_size;
    this->_mutex.unlock(THREAD_ID);
    return (value);
}

template <typename ElementType>
ElementType& ft_queue<ElementType>::front()
{
    static ElementType error_element = ElementType();
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->set_error(PT_ERR_MUTEX_OWNER);
        return (error_element);
    }
    if (this->_front == ft_nullptr)
    {
        this->set_error(QUEUE_EMPTY);
        this->_mutex.unlock(THREAD_ID);
        return (error_element);
    }
    ElementType& value = this->_front->_data;
    this->_mutex.unlock(THREAD_ID);
    return (value);
}

template <typename ElementType>
const ElementType& ft_queue<ElementType>::front() const
{
    static ElementType error_element = ElementType();
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->set_error(PT_ERR_MUTEX_OWNER);
        return (error_element);
    }
    if (this->_front == ft_nullptr)
    {
        this->set_error(QUEUE_EMPTY);
        this->_mutex.unlock(THREAD_ID);
        return (error_element);
    }
    const ElementType& value = this->_front->_data;
    this->_mutex.unlock(THREAD_ID);
    return (value);
}

template <typename ElementType>
size_t ft_queue<ElementType>::size() const
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (0);
    size_t current_size = this->_size;
    this->_mutex.unlock(THREAD_ID);
    return (current_size);
}

template <typename ElementType>
bool ft_queue<ElementType>::empty() const
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (true);
    bool result = (this->_size == 0);
    this->_mutex.unlock(THREAD_ID);
    return (result);
}

template <typename ElementType>
int ft_queue<ElementType>::get_error() const
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (this->_error_code);
    int err = this->_error_code;
    this->_mutex.unlock(THREAD_ID);
    return (err);
}

template <typename ElementType>
const char* ft_queue<ElementType>::get_error_str() const
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (ft_strerror(this->_error_code));
    int err = this->_error_code;
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
        this->_front = this->_front->_next;
        destroy_at(&node->_data);
        cma_free(node);
    }
    this->_rear = ft_nullptr;
    this->_size = 0;
    this->_mutex.unlock(THREAD_ID);
    return ;
}

#endif
