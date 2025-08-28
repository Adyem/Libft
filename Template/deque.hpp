#ifndef FT_DEQUE_HPP
#define FT_DEQUE_HPP

#include "constructor.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/nullptr.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/mutex.hpp"
#include <cstddef>
#include <utility>

template <typename ElementType>
class ft_deque
{
    private:
        struct DequeNode
        {
            ElementType data;
            DequeNode* prev;
            DequeNode* next;
        };

        DequeNode*   _front;
        DequeNode*   _back;
        size_t       _size;
        mutable int  _errorCode;
        mutable pt_mutex _mutex;

        void    setError(int error) const;

    public:
        ft_deque();
        ~ft_deque();

        ft_deque(const ft_deque&) = delete;
        ft_deque& operator=(const ft_deque&) = delete;

        ft_deque(ft_deque&& other) noexcept;
        ft_deque& operator=(ft_deque&& other) noexcept;

        void push_front(const ElementType& value);
        void push_front(ElementType&& value);
        void push_back(const ElementType& value);
        void push_back(ElementType&& value);
        ElementType pop_front();
        ElementType pop_back();

        ElementType& front();
        const ElementType& front() const;
        ElementType& back();
        const ElementType& back() const;

        size_t size() const;
        bool empty() const;

        int get_error() const;
        const char* get_error_str() const;

        void clear();
};

template <typename ElementType>
ft_deque<ElementType>::ft_deque()
    : _front(ft_nullptr), _back(ft_nullptr), _size(0), _errorCode(ER_SUCCESS)
{
    return ;
}

template <typename ElementType>
ft_deque<ElementType>::~ft_deque()
{
    this->clear();
    return ;
}

template <typename ElementType>
ft_deque<ElementType>::ft_deque(ft_deque&& other) noexcept
    : _front(other._front), _back(other._back), _size(other._size), _errorCode(other._errorCode)
{
    other._front = ft_nullptr;
    other._back = ft_nullptr;
    other._size = 0;
    other._errorCode = ER_SUCCESS;
    return ;
}

template <typename ElementType>
ft_deque<ElementType>& ft_deque<ElementType>::operator=(ft_deque&& other) noexcept
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
        this->_back = other._back;
        this->_size = other._size;
        this->_errorCode = other._errorCode;
        other._front = ft_nullptr;
        other._back = ft_nullptr;
        other._size = 0;
        other._errorCode = ER_SUCCESS;
        other._mutex.unlock(THREAD_ID);
        this->_mutex.unlock(THREAD_ID);
    }
    return (*this);
}

template <typename ElementType>
void ft_deque<ElementType>::setError(int error) const
{
    this->_errorCode = error;
    ft_errno = error;
    return ;
}

template <typename ElementType>
void ft_deque<ElementType>::push_front(const ElementType& value)
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->setError(PT_ERR_MUTEX_OWNER);
        return ;
    }
    DequeNode* node = static_cast<DequeNode*>(cma_malloc(sizeof(DequeNode)));
    if (node == ft_nullptr)
    {
        this->setError(DEQUE_ALLOC_FAIL);
        this->_mutex.unlock(THREAD_ID);
        return ;
    }
    construct_at(&node->data, value);
    node->prev = ft_nullptr;
    node->next = this->_front;
    if (this->_front == ft_nullptr)
        this->_back = node;
    else
        this->_front->prev = node;
    this->_front = node;
    ++this->_size;
    this->_mutex.unlock(THREAD_ID);
    return ;
}

template <typename ElementType>
void ft_deque<ElementType>::push_front(ElementType&& value)
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->setError(PT_ERR_MUTEX_OWNER);
        return ;
    }
    DequeNode* node = static_cast<DequeNode*>(cma_malloc(sizeof(DequeNode)));
    if (node == ft_nullptr)
    {
        this->setError(DEQUE_ALLOC_FAIL);
        this->_mutex.unlock(THREAD_ID);
        return ;
    }
    construct_at(&node->data, std::move(value));
    node->prev = ft_nullptr;
    node->next = this->_front;
    if (this->_front == ft_nullptr)
        this->_back = node;
    else
        this->_front->prev = node;
    this->_front = node;
    ++this->_size;
    this->_mutex.unlock(THREAD_ID);
    return ;
}

template <typename ElementType>
void ft_deque<ElementType>::push_back(const ElementType& value)
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->setError(PT_ERR_MUTEX_OWNER);
        return ;
    }
    DequeNode* node = static_cast<DequeNode*>(cma_malloc(sizeof(DequeNode)));
    if (node == ft_nullptr)
    {
        this->setError(DEQUE_ALLOC_FAIL);
        this->_mutex.unlock(THREAD_ID);
        return ;
    }
    construct_at(&node->data, value);
    node->next = ft_nullptr;
    node->prev = this->_back;
    if (this->_back == ft_nullptr)
        this->_front = node;
    else
        this->_back->next = node;
    this->_back = node;
    ++this->_size;
    this->_mutex.unlock(THREAD_ID);
    return ;
}

template <typename ElementType>
void ft_deque<ElementType>::push_back(ElementType&& value)
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->setError(PT_ERR_MUTEX_OWNER);
        return ;
    }
    DequeNode* node = static_cast<DequeNode*>(cma_malloc(sizeof(DequeNode)));
    if (node == ft_nullptr)
    {
        this->setError(DEQUE_ALLOC_FAIL);
        this->_mutex.unlock(THREAD_ID);
        return ;
    }
    construct_at(&node->data, std::move(value));
    node->next = ft_nullptr;
    node->prev = this->_back;
    if (this->_back == ft_nullptr)
        this->_front = node;
    else
        this->_back->next = node;
    this->_back = node;
    ++this->_size;
    this->_mutex.unlock(THREAD_ID);
    return ;
}

template <typename ElementType>
ElementType ft_deque<ElementType>::pop_front()
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->setError(PT_ERR_MUTEX_OWNER);
        return (ElementType());
    }
    if (this->_front == ft_nullptr)
    {
        this->setError(DEQUE_EMPTY);
        this->_mutex.unlock(THREAD_ID);
        return (ElementType());
    }
    DequeNode* node = this->_front;
    this->_front = node->next;
    if (this->_front == ft_nullptr)
        this->_back = ft_nullptr;
    else
        this->_front->prev = ft_nullptr;
    ElementType value = std::move(node->data);
    destroy_at(&node->data);
    cma_free(node);
    --this->_size;
    this->_mutex.unlock(THREAD_ID);
    return (value);
}

template <typename ElementType>
ElementType ft_deque<ElementType>::pop_back()
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->setError(PT_ERR_MUTEX_OWNER);
        return (ElementType());
    }
    if (this->_back == ft_nullptr)
    {
        this->setError(DEQUE_EMPTY);
        this->_mutex.unlock(THREAD_ID);
        return (ElementType());
    }
    DequeNode* node = this->_back;
    this->_back = node->prev;
    if (this->_back == ft_nullptr)
        this->_front = ft_nullptr;
    else
        this->_back->next = ft_nullptr;
    ElementType value = std::move(node->data);
    destroy_at(&node->data);
    cma_free(node);
    --this->_size;
    this->_mutex.unlock(THREAD_ID);
    return (value);
}

template <typename ElementType>
ElementType& ft_deque<ElementType>::front()
{
    static ElementType errorElement = ElementType();
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->setError(PT_ERR_MUTEX_OWNER);
        return (errorElement);
    }
    if (this->_front == ft_nullptr)
    {
        this->setError(DEQUE_EMPTY);
        this->_mutex.unlock(THREAD_ID);
        return (errorElement);
    }
    ElementType& ref = this->_front->data;
    this->_mutex.unlock(THREAD_ID);
    return (ref);
}

template <typename ElementType>
const ElementType& ft_deque<ElementType>::front() const
{
    static ElementType errorElement = ElementType();
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->setError(PT_ERR_MUTEX_OWNER);
        return (errorElement);
    }
    if (this->_front == ft_nullptr)
    {
        this->setError(DEQUE_EMPTY);
        this->_mutex.unlock(THREAD_ID);
        return (errorElement);
    }
    ElementType& ref = this->_front->data;
    this->_mutex.unlock(THREAD_ID);
    return (ref);
}

template <typename ElementType>
ElementType& ft_deque<ElementType>::back()
{
    static ElementType errorElement = ElementType();
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->setError(PT_ERR_MUTEX_OWNER);
        return (errorElement);
    }
    if (this->_back == ft_nullptr)
    {
        this->setError(DEQUE_EMPTY);
        this->_mutex.unlock(THREAD_ID);
        return (errorElement);
    }
    ElementType& ref = this->_back->data;
    this->_mutex.unlock(THREAD_ID);
    return (ref);
}

template <typename ElementType>
const ElementType& ft_deque<ElementType>::back() const
{
    static ElementType errorElement = ElementType();
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->setError(PT_ERR_MUTEX_OWNER);
        return (errorElement);
    }
    if (this->_back == ft_nullptr)
    {
        this->setError(DEQUE_EMPTY);
        this->_mutex.unlock(THREAD_ID);
        return (errorElement);
    }
    ElementType& ref = this->_back->data;
    this->_mutex.unlock(THREAD_ID);
    return (ref);
}

template <typename ElementType>
size_t ft_deque<ElementType>::size() const
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (0);
    size_t s = this->_size;
    this->_mutex.unlock(THREAD_ID);
    return (s);
}

template <typename ElementType>
bool ft_deque<ElementType>::empty() const
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (true);
    bool res = (this->_size == 0);
    this->_mutex.unlock(THREAD_ID);
    return (res);
}

template <typename ElementType>
int ft_deque<ElementType>::get_error() const
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (this->_errorCode);
    int err = this->_errorCode;
    this->_mutex.unlock(THREAD_ID);
    return (err);
}

template <typename ElementType>
const char* ft_deque<ElementType>::get_error_str() const
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (ft_strerror(this->_errorCode));
    int err = this->_errorCode;
    this->_mutex.unlock(THREAD_ID);
    return (ft_strerror(err));
}

template <typename ElementType>
void ft_deque<ElementType>::clear()
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return ;
    while (this->_front != ft_nullptr)
    {
        DequeNode* node = this->_front;
        this->_front = this->_front->next;
        destroy_at(&node->data);
        cma_free(node);
    }
    this->_back = ft_nullptr;
    this->_size = 0;
    this->_mutex.unlock(THREAD_ID);
    return ;
}

#endif
