#ifndef FT_DEQUE_HPP
#define FT_DEQUE_HPP

#include "constructor.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread.hpp"
#include <cstddef>
#include "move.hpp"

template <typename ElementType>
class ft_deque
{
    private:
        struct DequeNode
        {
            ElementType _data;
            DequeNode*  _prev;
            DequeNode*  _next;
        };

        DequeNode* _front;
        DequeNode* _back;
        size_t     _size;
        mutable pt_recursive_mutex* _mutex;

        void destroy_all_unlocked();
        int  lock_internal(bool *lock_acquired) const;
        int  unlock_internal(bool lock_acquired) const;
        int  prepare_thread_safety();
        void teardown_thread_safety();

    public:
        ft_deque();
        ~ft_deque();

        ft_deque(const ft_deque&) = delete;
        ft_deque& operator=(const ft_deque&) = delete;

        ft_deque(ft_deque&& other) noexcept;
        ft_deque& operator=(ft_deque&& other) noexcept;

        int  enable_thread_safety();
        void disable_thread_safety();
        bool is_thread_safe() const;
        int  lock(bool *lock_acquired) const;
        void unlock(bool lock_acquired) const;

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

        void clear();

    #ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex* get_mutex_for_validation() const noexcept;
    #endif
};

template <typename ElementType>
ft_deque<ElementType>::ft_deque()
    : _front(ft_nullptr), _back(ft_nullptr), _size(0), _mutex(ft_nullptr)
{
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

template <typename ElementType>
ft_deque<ElementType>::~ft_deque()
{
    bool lock_acquired = false;
    int  lock_error = this->lock_internal(&lock_acquired);

    if (lock_error == FT_ERR_SUCCESS)
    {
        this->destroy_all_unlocked();
        this->_front = ft_nullptr;
        this->_back = ft_nullptr;
        this->_size = 0;
        int unlock_error = this->unlock_internal(lock_acquired);
        if (unlock_error != FT_ERR_SUCCESS)
            ft_global_error_stack_push(unlock_error);
        else
            ft_global_error_stack_push(FT_ERR_SUCCESS);
    }
    else
    {
        ft_global_error_stack_push(lock_error);
    }
    this->teardown_thread_safety();
    return ;
}

template <typename ElementType>
ft_deque<ElementType>::ft_deque(ft_deque<ElementType>&& other) noexcept
    : _front(ft_nullptr), _back(ft_nullptr), _size(0), _mutex(ft_nullptr)
{
    bool                other_thread_safe;
    bool                lock_acquired;
    int                 lock_error;

    other_thread_safe = (other._mutex != ft_nullptr);
    lock_acquired = false;
    lock_error = other.lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    this->_front = other._front;
    this->_back = other._back;
    this->_size = other._size;
    other._front = ft_nullptr;
    other._back = ft_nullptr;
    other._size = 0;
    other.unlock_internal(lock_acquired);
    other.teardown_thread_safety();
    if (other_thread_safe)
    {
        int enable_error;

        enable_error = this->enable_thread_safety();
        if (enable_error != FT_ERR_SUCCESS)
        {
            ft_global_error_stack_push(enable_error);
            return ;
        }
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

template <typename ElementType>
ft_deque<ElementType>& ft_deque<ElementType>::operator=(ft_deque<ElementType>&& other) noexcept
{
    if (this == &other)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESS);
        return (*this);
    }
    bool                other_thread_safe;
    bool                lock_acquired;
    int                 lock_error;
    bool                other_lock_acquired;
    int                 other_lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return (*this);
    }
    this->destroy_all_unlocked();
    this->_front = ft_nullptr;
    this->_back = ft_nullptr;
    this->_size = 0;
    this->unlock_internal(lock_acquired);
    this->teardown_thread_safety();
    other_lock_acquired = false;
    other_lock_error = other.lock_internal(&other_lock_acquired);
    if (other_lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(other_lock_error);
        return (*this);
    }
    this->_front = other._front;
    this->_back = other._back;
    this->_size = other._size;
    other._front = ft_nullptr;
    other._back = ft_nullptr;
    other._size = 0;
    other.unlock_internal(other_lock_acquired);
    other_thread_safe = (other._mutex != ft_nullptr);
    other.teardown_thread_safety();
    if (other_thread_safe)
    {
        int enable_error;

        enable_error = this->enable_thread_safety();
        if (enable_error != FT_ERR_SUCCESS)
        {
            ft_global_error_stack_push(enable_error);
            return (*this);
        }
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (*this);
}

template <typename ElementType>
int ft_deque<ElementType>::enable_thread_safety()
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
void ft_deque<ElementType>::disable_thread_safety()
{
    this->teardown_thread_safety();
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

template <typename ElementType>
bool ft_deque<ElementType>::is_thread_safe() const
{
    bool enabled;

    enabled = (this->_mutex != ft_nullptr);
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (enabled);
}

template <typename ElementType>
int ft_deque<ElementType>::lock(bool *lock_acquired) const
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
void ft_deque<ElementType>::unlock(bool lock_acquired) const
{
    int unlock_error;

    unlock_error = this->unlock_internal(lock_acquired);
    ft_global_error_stack_push(unlock_error);
    return ;
}

template <typename ElementType>
void ft_deque<ElementType>::push_front(const ElementType& value)
{
    DequeNode* node;
    bool       lock_acquired;
    int        lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    node = static_cast<DequeNode*>(cma_malloc(sizeof(DequeNode)));
    if (node == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        this->unlock_internal(lock_acquired);
        return ;
    }
    construct_at(&node->_data, value);
    node->_prev = ft_nullptr;
    node->_next = this->_front;
    if (this->_front == ft_nullptr)
        this->_back = node;
    else
        this->_front->_prev = node;
    this->_front = node;
    this->_size += 1;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename ElementType>
void ft_deque<ElementType>::push_front(ElementType&& value)
{
    DequeNode* node;
    bool       lock_acquired;
    int        lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    node = static_cast<DequeNode*>(cma_malloc(sizeof(DequeNode)));
    if (node == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        this->unlock_internal(lock_acquired);
        return ;
    }
    construct_at(&node->_data, ft_move(value));
    node->_prev = ft_nullptr;
    node->_next = this->_front;
    if (this->_front == ft_nullptr)
        this->_back = node;
    else
        this->_front->_prev = node;
    this->_front = node;
    this->_size += 1;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename ElementType>
void ft_deque<ElementType>::push_back(const ElementType& value)
{
    DequeNode* node;
    bool       lock_acquired;
    int        lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    node = static_cast<DequeNode*>(cma_malloc(sizeof(DequeNode)));
    if (node == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        this->unlock_internal(lock_acquired);
        return ;
    }
    construct_at(&node->_data, value);
    node->_next = ft_nullptr;
    node->_prev = this->_back;
    if (this->_back == ft_nullptr)
        this->_front = node;
    else
        this->_back->_next = node;
    this->_back = node;
    this->_size += 1;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename ElementType>
void ft_deque<ElementType>::push_back(ElementType&& value)
{
    DequeNode* node;
    bool       lock_acquired;
    int        lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    node = static_cast<DequeNode*>(cma_malloc(sizeof(DequeNode)));
    if (node == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        this->unlock_internal(lock_acquired);
        return ;
    }
    construct_at(&node->_data, ft_move(value));
    node->_next = ft_nullptr;
    node->_prev = this->_back;
    if (this->_back == ft_nullptr)
        this->_front = node;
    else
        this->_back->_next = node;
    this->_back = node;
    this->_size += 1;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename ElementType>
ElementType ft_deque<ElementType>::pop_front()
{
    bool        lock_acquired;
    DequeNode  *node;
    ElementType value;
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
    this->_front = node->_next;
    if (this->_front == ft_nullptr)
        this->_back = ft_nullptr;
    else
        this->_front->_prev = ft_nullptr;
    value = ft_move(node->_data);
    destroy_at(&node->_data);
    cma_free(node);
    this->_size -= 1;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (value);
}

template <typename ElementType>
ElementType ft_deque<ElementType>::pop_back()
{
    bool        lock_acquired;
    DequeNode  *node;
    ElementType value;
    int         lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return (ElementType());
    }
    if (this->_back == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_EMPTY);
        this->unlock_internal(lock_acquired);
        return (ElementType());
    }
    node = this->_back;
    this->_back = node->_prev;
    if (this->_back == ft_nullptr)
        this->_front = ft_nullptr;
    else
        this->_back->_next = ft_nullptr;
    value = ft_move(node->_data);
    destroy_at(&node->_data);
    cma_free(node);
    this->_size -= 1;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (value);
}

template <typename ElementType>
ElementType& ft_deque<ElementType>::front()
{
    static ElementType error_element = ElementType();
    bool               lock_acquired;
    ElementType       *value_pointer;
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
    value_pointer = &this->_front->_data;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (*value_pointer);
}

template <typename ElementType>
const ElementType& ft_deque<ElementType>::front() const
{
    static ElementType error_element = ElementType();
    bool               lock_acquired;
    const ElementType *value_pointer;
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
    value_pointer = &this->_front->_data;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (*value_pointer);
}

template <typename ElementType>
ElementType& ft_deque<ElementType>::back()
{
    static ElementType error_element = ElementType();
    bool               lock_acquired;
    ElementType       *value_pointer;
    int                lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return (error_element);
    }
    if (this->_back == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_EMPTY);
        this->unlock_internal(lock_acquired);
        return (error_element);
    }
    value_pointer = &this->_back->_data;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (*value_pointer);
}

template <typename ElementType>
const ElementType& ft_deque<ElementType>::back() const
{
    static ElementType error_element = ElementType();
    bool               lock_acquired;
    const ElementType *value_pointer;
    int                lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return (error_element);
    }
    if (this->_back == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_EMPTY);
        this->unlock_internal(lock_acquired);
        return (error_element);
    }
    value_pointer = &this->_back->_data;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (*value_pointer);
}

template <typename ElementType>
size_t ft_deque<ElementType>::size() const
{
    bool   lock_acquired;
    size_t current_size;
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
bool ft_deque<ElementType>::empty() const
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
void ft_deque<ElementType>::clear()
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
    this->_front = ft_nullptr;
    this->_back = ft_nullptr;
    this->_size = 0;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename ElementType>
void ft_deque<ElementType>::destroy_all_unlocked()
{
    while (this->_front != ft_nullptr)
    {
        DequeNode *node = this->_front;
        this->_front = this->_front->_next;
        destroy_at(&node->_data);
        cma_free(node);
    }
    return ;
}

template <typename ElementType>
int ft_deque<ElementType>::lock_internal(bool *lock_acquired) const
{
    if (lock_acquired)
        *lock_acquired = false;
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    int result = pt_recursive_mutex_lock_with_error(*this->_mutex);
    if (result != FT_ERR_SUCCESS)
        return (result);
    if (lock_acquired)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

template <typename ElementType>
int ft_deque<ElementType>::unlock_internal(bool lock_acquired) const
{
    if (!lock_acquired || this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (pt_recursive_mutex_unlock_with_error(*this->_mutex));
}

template <typename ElementType>
int ft_deque<ElementType>::prepare_thread_safety()
{
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (pt_recursive_mutex_create_with_error(&this->_mutex));
}

template <typename ElementType>
void ft_deque<ElementType>::teardown_thread_safety()
{
    pt_recursive_mutex_destroy(&this->_mutex);
    return ;
}

#ifdef LIBFT_TEST_BUILD
template <typename ElementType>
pt_recursive_mutex* ft_deque<ElementType>::get_mutex_for_validation() const noexcept
{
    return (this->_mutex);
}
#endif

#endif
