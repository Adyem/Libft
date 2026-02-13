#ifndef FT_STACK_HPP
#define FT_STACK_HPP

#include "constructor.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread.hpp"
#include <cstddef>
#include "move.hpp"

template <typename ElementType>
class ft_stack
{
    private:
        struct StackNode
        {
            ElementType _data;
            StackNode* _next;
        };

        StackNode* _top;
        size_t _size;
        mutable pt_recursive_mutex *_mutex;

        void destroy_all_unlocked();
        int lock_internal(bool *lock_acquired) const;
        int unlock_internal(bool lock_acquired) const;
        int prepare_thread_safety();
        void teardown_thread_safety();

    public:
        ft_stack();
        ~ft_stack();

        ft_stack(const ft_stack&) = delete;
        ft_stack& operator=(const ft_stack&) = delete;

        ft_stack(ft_stack&& other) noexcept;
        ft_stack& operator=(ft_stack&& other) noexcept;

        int enable_thread_safety();
        void disable_thread_safety();
        bool is_thread_safe() const;
        int lock(bool *lock_acquired) const;
        void unlock(bool lock_acquired) const;

        void push(const ElementType& value);
        void push(ElementType&& value);
        ElementType pop();

        ElementType& top();
        const ElementType& top() const;

        size_t size() const;
        bool empty() const;

        void clear();

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex* get_mutex_for_validation() const noexcept;
#endif
};

template <typename ElementType>
ft_stack<ElementType>::ft_stack()
    : _top(ft_nullptr), _size(0), _mutex(ft_nullptr)
{
    return ;
}

template <typename ElementType>
ft_stack<ElementType>::~ft_stack()
{
    this->clear();
    this->teardown_thread_safety();
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

template <typename ElementType>
ft_stack<ElementType>::ft_stack(ft_stack&& other) noexcept
    : _top(ft_nullptr), _size(0), _mutex(ft_nullptr)
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
    this->_top = other._top;
    this->_size = other._size;
    other_thread_safe = (other._mutex != ft_nullptr);
    other._top = ft_nullptr;
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
ft_stack<ElementType>& ft_stack<ElementType>::operator=(ft_stack&& other) noexcept
{
    if (this == &other)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESS);
        return (*this);
    }
    bool other_thread_safe;
    bool other_lock_acquired;
    int lock_error;

    this->clear();
    this->teardown_thread_safety();
    other_lock_acquired = false;
    lock_error = other.lock_internal(&other_lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return (*this);
    }
    this->_top = other._top;
    this->_size = other._size;
    other_thread_safe = (other._mutex != ft_nullptr);
    other._top = ft_nullptr;
    other._size = 0;
    other.unlock_internal(other_lock_acquired);
    other.teardown_thread_safety();
    if (other_thread_safe)
    {
        if (this->enable_thread_safety() != FT_ERR_SUCCESS)
            return (*this);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (*this);
}

template <typename ElementType>
void ft_stack<ElementType>::destroy_all_unlocked()
{
    StackNode *node;

    while (this->_top != ft_nullptr)
    {
        node = this->_top;
        this->_top = node->_next;
        destroy_at(&node->_data);
        cma_free(node);
    }
    this->_size = 0;
    return ;
}

template <typename ElementType>
int ft_stack<ElementType>::lock_internal(bool *lock_acquired) const
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
int ft_stack<ElementType>::unlock_internal(bool lock_acquired) const
{
    if (!lock_acquired || this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (pt_recursive_mutex_unlock_with_error(*this->_mutex));
}

template <typename ElementType>
int ft_stack<ElementType>::prepare_thread_safety()
{
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (pt_recursive_mutex_create_with_error(&this->_mutex));
}

template <typename ElementType>
void ft_stack<ElementType>::teardown_thread_safety()
{
    pt_recursive_mutex_destroy(&this->_mutex);
    return ;
}

template <typename ElementType>
int ft_stack<ElementType>::enable_thread_safety()
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
void ft_stack<ElementType>::disable_thread_safety()
{
    this->teardown_thread_safety();
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

template <typename ElementType>
bool ft_stack<ElementType>::is_thread_safe() const
{
    bool enabled;

    enabled = (this->_mutex != ft_nullptr);
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (enabled);
}

template <typename ElementType>
int ft_stack<ElementType>::lock(bool *lock_acquired) const
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
void ft_stack<ElementType>::unlock(bool lock_acquired) const
{
    int unlock_error;

    unlock_error = this->unlock_internal(lock_acquired);
    ft_global_error_stack_push(unlock_error);
    return ;
}

template <typename ElementType>
void ft_stack<ElementType>::push(const ElementType& value)
{
    StackNode *new_node;
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    new_node = static_cast<StackNode *>(cma_malloc(sizeof(StackNode)));
    if (new_node == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        this->unlock_internal(lock_acquired);
        return ;
    }
    construct_at(&new_node->_data, value);
    new_node->_next = this->_top;
    this->_top = new_node;
    this->_size += 1;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename ElementType>
void ft_stack<ElementType>::push(ElementType&& value)
{
    StackNode *new_node;
    bool lock_acquired;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    new_node = static_cast<StackNode *>(cma_malloc(sizeof(StackNode)));
    if (new_node == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        this->unlock_internal(lock_acquired);
        return ;
    }
    construct_at(&new_node->_data, ft_move(value));
    new_node->_next = this->_top;
    this->_top = new_node;
    this->_size += 1;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename ElementType>
ElementType ft_stack<ElementType>::pop()
{
    bool lock_acquired;
    StackNode *node;
    ElementType value;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return (ElementType());
    }
    if (this->_top == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_EMPTY);
        this->unlock_internal(lock_acquired);
        return (ElementType());
    }
    node = this->_top;
    this->_top = node->_next;
    value = ft_move(node->_data);
    destroy_at(&node->_data);
    cma_free(node);
    this->_size -= 1;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (value);
}

template <typename ElementType>
ElementType& ft_stack<ElementType>::top()
{
    static ElementType error_element = ElementType();
    bool lock_acquired;
    ElementType *value;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return (error_element);
    }
    if (this->_top == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_EMPTY);
        this->unlock_internal(lock_acquired);
        return (error_element);
    }
    value = &this->_top->_data;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (*value);
}

template <typename ElementType>
const ElementType& ft_stack<ElementType>::top() const
{
    static ElementType error_element = ElementType();
    bool lock_acquired;
    const ElementType *value;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return (error_element);
    }
    if (this->_top == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_EMPTY);
        this->unlock_internal(lock_acquired);
        return (error_element);
    }
    value = &this->_top->_data;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (*value);
}

template <typename ElementType>
size_t ft_stack<ElementType>::size() const
{
    size_t current_size;
    bool lock_acquired;
    int lock_error;

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
bool ft_stack<ElementType>::empty() const
{
    bool lock_acquired;
    bool is_empty;
    int lock_error;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return (true);
    }
    is_empty = (this->_top == ft_nullptr);
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (is_empty);
}

template <typename ElementType>
void ft_stack<ElementType>::clear()
{
    bool lock_acquired;
    int lock_error;

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
pt_recursive_mutex* ft_stack<ElementType>::get_mutex_for_validation() const noexcept
{
    return (this->_mutex);
}
#endif

#endif
