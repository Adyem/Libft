#ifndef FT_QUEUE_HPP
#define FT_QUEUE_HPP

#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../CMA/CMA.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread_internal.hpp"
#include "constructor.hpp"
#include "move.hpp"
#include <cstddef>

template <typename ElementType>
class ft_queue
{
    private:
        struct QueueNode
        {
            ElementType _data;
            QueueNode*  _next;
        };

        QueueNode*              _front;
        QueueNode*              _rear;
        size_t                  _size;
        mutable pt_recursive_mutex* _mutex;
        void    destroy_all_unlocked();
        int     lock_internal(bool *lock_acquired) const;
        int     unlock_internal(bool lock_acquired) const;
        int     prepare_thread_safety();
        void    teardown_thread_safety();

    public:
        ft_queue();
        ~ft_queue();

        ft_queue(const ft_queue&) = delete;
        ft_queue& operator=(const ft_queue&) = delete;

        ft_queue(ft_queue&& other) noexcept;
        ft_queue& operator=(ft_queue&& other) noexcept;

        int enable_thread_safety();
        void disable_thread_safety();
        bool is_thread_safe() const;
        int lock(bool *lock_acquired) const;
        void unlock(bool lock_acquired) const;

        void enqueue(const ElementType& value);
        void enqueue(ElementType&& value);
        ElementType dequeue();

        ElementType& front();
        const ElementType& front() const;

        size_t size() const;
        bool empty() const;

        void clear();

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex* get_mutex_for_validation() const noexcept;
#endif
};

template <typename ElementType>
ft_queue<ElementType>::ft_queue()
    : _front(ft_nullptr), _rear(ft_nullptr), _size(0),
      _mutex(ft_nullptr)
{
    return ;
}

template <typename ElementType>
ft_queue<ElementType>::~ft_queue()
{
    this->clear();
    this->teardown_thread_safety();
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

template <typename ElementType>
ft_queue<ElementType>::ft_queue(ft_queue&& other) noexcept
    : _front(ft_nullptr), _rear(ft_nullptr), _size(0),
      _mutex(ft_nullptr)
{
    bool lock_acquired;
    bool other_thread_safe;
    int lock_error;

    lock_acquired = false;
    lock_error = other.lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    this->_front = other._front;
    this->_rear = other._rear;
    this->_size = other._size;
    other_thread_safe = (other._mutex != ft_nullptr);
    other._front = ft_nullptr;
    other._rear = ft_nullptr;
    other._size = 0;
    other.unlock_internal(lock_acquired);
    other.teardown_thread_safety();
    if (other_thread_safe)
    {
        if (this->enable_thread_safety() != FT_ERR_SUCCESS)
        {
            return ;
        }
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

template <typename ElementType>
ft_queue<ElementType>& ft_queue<ElementType>::operator=(ft_queue&& other) noexcept
{
    if (this == &other)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESS);
        return (*this);
    }
    bool lock_acquired;
    bool other_lock_acquired;
    bool other_thread_safe;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return (*this);
    }
    this->destroy_all_unlocked();
    this->_front = ft_nullptr;
    this->_rear = ft_nullptr;
    this->_size = 0;
    {
        int unlock_error = this->unlock_internal(lock_acquired);

        if (unlock_error != FT_ERR_SUCCESS)
        {
            ft_global_error_stack_push(unlock_error);
            return (*this);
        }
    }
    this->teardown_thread_safety();
    other_lock_acquired = false;
    lock_error = other.lock_internal(&other_lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return (*this);
    }
    this->_front = other._front;
    this->_rear = other._rear;
    this->_size = other._size;
    other_thread_safe = (other._mutex != ft_nullptr);
    other._front = ft_nullptr;
    other._rear = ft_nullptr;
    other._size = 0;
    other.unlock_internal(other_lock_acquired);
    other.teardown_thread_safety();
    if (other_thread_safe)
    {
        if (this->enable_thread_safety() != FT_ERR_SUCCESS)
        {
            return (*this);
        }
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (*this);
}

template <typename ElementType>
void ft_queue<ElementType>::destroy_all_unlocked()
{
    QueueNode *node;

    while (this->_front != ft_nullptr)
    {
        node = this->_front;
        this->_front = this->_front->_next;
        destroy_at(&node->_data);
        cma_free(node);
    }
    this->_rear = ft_nullptr;
    this->_size = 0;
    return ;
}

template <typename ElementType>
int ft_queue<ElementType>::enable_thread_safety()
{
    int result;

    if (this->_mutex != ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    result = this->prepare_thread_safety();
    ft_global_error_stack_push(result);
    return (result);
}

template <typename ElementType>
void ft_queue<ElementType>::disable_thread_safety()
{
    this->teardown_thread_safety();
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

template <typename ElementType>
bool ft_queue<ElementType>::is_thread_safe() const
{
    bool enabled;

    enabled = (this->_mutex != ft_nullptr);
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (enabled);
}

template <typename ElementType>
int ft_queue<ElementType>::lock(bool *lock_acquired) const
{
    int result;

    result = this->lock_internal(lock_acquired);
    if (result != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(result);
        return (-1);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (0);
}

template <typename ElementType>
void ft_queue<ElementType>::unlock(bool lock_acquired) const
{
    int unlock_error;

    unlock_error = this->unlock_internal(lock_acquired);
    ft_global_error_stack_push(unlock_error);
    return ;
}

template <typename ElementType>
int ft_queue<ElementType>::lock_internal(bool *lock_acquired) const
{
    int result;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    result = pt_recursive_mutex_lock_with_error(*this->_mutex);
    if (result != FT_ERR_SUCCESS)
        return (result);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

template <typename ElementType>
int ft_queue<ElementType>::unlock_internal(bool lock_acquired) const
{
    if (!lock_acquired || this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (pt_recursive_mutex_unlock_with_error(*this->_mutex));
}

template <typename ElementType>
int ft_queue<ElementType>::prepare_thread_safety()
{
    pt_recursive_mutex *mutex_pointer;
    int mutex_error;

    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    mutex_pointer = ft_nullptr;
    mutex_error = pt_recursive_mutex_create_with_error(&mutex_pointer);
    if (mutex_error != FT_ERR_SUCCESS)
        return (mutex_error);
    this->_mutex = mutex_pointer;
    return (FT_ERR_SUCCESS);
}

template <typename ElementType>
void ft_queue<ElementType>::teardown_thread_safety()
{
    pt_recursive_mutex_destroy(&this->_mutex);
    return ;
}

template <typename ElementType>
void ft_queue<ElementType>::enqueue(const ElementType& value)
{
    QueueNode *node;
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    node = static_cast<QueueNode *>(cma_malloc(sizeof(QueueNode)));
    if (node == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        this->unlock_internal(lock_acquired);
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
    this->_size += 1;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename ElementType>
void ft_queue<ElementType>::enqueue(ElementType&& value)
{
    QueueNode *node;
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    node = static_cast<QueueNode *>(cma_malloc(sizeof(QueueNode)));
    if (node == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        this->unlock_internal(lock_acquired);
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
    this->_size += 1;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename ElementType>
ElementType ft_queue<ElementType>::dequeue()
{
    QueueNode  *node;
    ElementType result;
    bool        lock_acquired;
    int         lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return (ElementType());
    }
    if (this->_front == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_EMPTY);
        this->unlock_internal(lock_acquired);
        return (ElementType());
    }
    node = this->_front;
    result = ft_move(node->_data);
    destroy_at(&node->_data);
    this->_front = node->_next;
    if (this->_front == ft_nullptr)
        this->_rear = ft_nullptr;
    cma_free(node);
    this->_size -= 1;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (result);
}

template <typename ElementType>
ElementType& ft_queue<ElementType>::front()
{
    static ElementType error_element = ElementType();
    ElementType       *value;
    bool               lock_acquired;
    int                lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return (error_element);
    }
    if (this->_front == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_EMPTY);
        this->unlock_internal(lock_acquired);
        return (error_element);
    }
    value = &this->_front->_data;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (*value);
}

template <typename ElementType>
const ElementType& ft_queue<ElementType>::front() const
{
    static ElementType error_element = ElementType();
    const ElementType *value;
    bool               lock_acquired;
    int                lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return (error_element);
    }
    if (this->_front == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_EMPTY);
        this->unlock_internal(lock_acquired);
        return (error_element);
    }
    value = &this->_front->_data;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (*value);
}

template <typename ElementType>
size_t ft_queue<ElementType>::size() const
{
    size_t current_size;
    bool   lock_acquired;
    int    lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return (0);
    }
    current_size = this->_size;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (current_size);
}

template <typename ElementType>
bool ft_queue<ElementType>::empty() const
{
    bool lock_acquired;
    bool is_empty;
    int  lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return (true);
    }
    is_empty = (this->_size == 0);
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (is_empty);
}

template <typename ElementType>
void ft_queue<ElementType>::clear()
{
    bool lock_acquired;
    int  lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    this->destroy_all_unlocked();
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

#ifdef LIBFT_TEST_BUILD
template <typename ElementType>
pt_recursive_mutex* ft_queue<ElementType>::get_mutex_for_validation() const noexcept
{
    return (this->_mutex);
}
#endif

#endif
