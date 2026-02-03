#ifndef FT_STACK_HPP
#define FT_STACK_HPP

#include "constructor.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../Errno/errno_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"
#include <cstddef>
#include <utility>
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

            StackNode*          _top;
            size_t              _size;
            mutable pt_mutex   *_mutex;
            bool                _thread_safe_enabled;
            mutable ft_operation_error_stack _operation_errors;
            void                record_operation_error(int error_code) const noexcept;
            ft_operation_error_stack *get_operation_error_stack_for_validation() noexcept;
            const ft_operation_error_stack *get_operation_error_stack_for_validation() const noexcept;
            int                 last_operation_error() const noexcept;
            const char         *last_operation_error_str() const noexcept;
            int                 operation_error_at(ft_size_t index) const noexcept;
            const char         *operation_error_str_at(ft_size_t index) const noexcept;
            void                pop_operation_errors() noexcept;
            int                 pop_oldest_operation_error() noexcept;
            int                 pop_newest_operation_error() noexcept;

        int     lock_internal(bool *lock_acquired) const;
        void    unlock_internal(bool lock_acquired) const;
        void    teardown_thread_safety();

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
};

template <typename ElementType>
void ft_stack<ElementType>::record_operation_error(int error_code) const noexcept
{
    unsigned long long operation_id;

    operation_id = ft_errno_next_operation_id();
    ft_global_error_stack_push_entry_with_id(error_code, operation_id);
    ft_operation_error_stack_push(&this->_operation_errors, error_code, operation_id);
    return ;
}

template <typename ElementType>
ft_stack<ElementType>::ft_stack()
    : _top(ft_nullptr), _size(0), _mutex(ft_nullptr), _thread_safe_enabled(false),
      _operation_errors({{}, {}, 0})
{
    return ;
}

template <typename ElementType>
ft_stack<ElementType>::~ft_stack()
{
    this->clear();
    this->teardown_thread_safety();
    return ;
}

template <typename ElementType>
ft_stack<ElementType>::ft_stack(ft_stack&& other) noexcept
    : _top(ft_nullptr), _size(0), _mutex(ft_nullptr), _thread_safe_enabled(false)
{
    bool other_lock_acquired;
    bool other_thread_safe;

    other_lock_acquired = false;
    if (other.lock_internal(&other_lock_acquired) != 0)
    {
        this->record_operation_error(other.last_operation_error());
        return ;
    }
    this->_top = other._top;
    this->_size = other._size;
    other_thread_safe = other._thread_safe_enabled;
    other._top = ft_nullptr;
    other._size = 0;
    other.unlock_internal(other_lock_acquired);
    other.teardown_thread_safety();
    if (other_thread_safe)
    {
        if (this->enable_thread_safety() != 0)
            return ;
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
    return ;
}

template <typename ElementType>
ft_stack<ElementType>& ft_stack<ElementType>::operator=(ft_stack&& other) noexcept
{
    bool other_lock_acquired;
    bool other_thread_safe;

    if (this == &other)
        return (*this);
    this->clear();
    this->teardown_thread_safety();
    other_lock_acquired = false;
    if (other.lock_internal(&other_lock_acquired) != 0)
    {
        this->record_operation_error(other.last_operation_error());
        return (*this);
    }
    this->_top = other._top;
    this->_size = other._size;
    other_thread_safe = other._thread_safe_enabled;
    other._top = ft_nullptr;
    other._size = 0;
    other.unlock_internal(other_lock_acquired);
    other.teardown_thread_safety();
    if (other_thread_safe)
    {
        if (this->enable_thread_safety() != 0)
            return (*this);
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (*this);
}

template <typename ElementType>
int ft_stack<ElementType>::lock_internal(bool *lock_acquired) const
{
    int mutex_error;

    if (lock_acquired)
        *lock_acquired = false;
    if (!this->_thread_safe_enabled || this->_mutex == ft_nullptr)
    {
        this->record_operation_error(FT_ERR_SUCCESSS);
        return (0);
    }
    this->_mutex->lock(THREAD_ID);
    mutex_error = ft_global_error_stack_last_error();
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(mutex_error);
        return (-1);
    }
    if (lock_acquired)
        *lock_acquired = true;
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (0);
}

template <typename ElementType>
void ft_stack<ElementType>::unlock_internal(bool lock_acquired) const
{
    int mutex_error;

    if (!lock_acquired || this->_mutex == ft_nullptr)
        return ;
    this->_mutex->unlock(THREAD_ID);
    mutex_error = ft_global_error_stack_last_error();
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        this->record_operation_error(mutex_error);
        return ;
    }
    this->record_operation_error(FT_ERR_SUCCESSS);
    return ;
}

template <typename ElementType>
void ft_stack<ElementType>::teardown_thread_safety()
{
    if (this->_mutex != ft_nullptr)
    {
        this->_mutex->~pt_mutex();
        cma_free(this->_mutex);
        this->_mutex = ft_nullptr;
    }
    this->_thread_safe_enabled = false;
    return ;
}

template <typename ElementType>
int ft_stack<ElementType>::enable_thread_safety()
{
    void     *memory;
    pt_mutex *mutex_pointer;

    if (this->_thread_safe_enabled && this->_mutex != ft_nullptr)
    {
        this->record_operation_error(FT_ERR_SUCCESSS);
        return (0);
    }
    memory = cma_malloc(sizeof(pt_mutex));
    if (memory == ft_nullptr)
    {
        this->record_operation_error(FT_ERR_NO_MEMORY);
        return (-1);
    }
    mutex_pointer = new(memory) pt_mutex();
    int mutex_error = ft_global_error_stack_last_error();
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        mutex_pointer->~pt_mutex();
        cma_free(memory);
        this->record_operation_error(mutex_error);
        return (-1);
    }
    this->_mutex = mutex_pointer;
    this->_thread_safe_enabled = true;
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (0);
}

template <typename ElementType>
void ft_stack<ElementType>::disable_thread_safety()
{
    this->teardown_thread_safety();
    this->record_operation_error(FT_ERR_SUCCESSS);
    return ;
}

template <typename ElementType>
bool ft_stack<ElementType>::is_thread_safe() const
{
    bool enabled;

    enabled = (this->_thread_safe_enabled && this->_mutex != ft_nullptr);
    this->record_operation_error(FT_ERR_SUCCESSS);
    return (enabled);
}

template <typename ElementType>
int ft_stack<ElementType>::lock(bool *lock_acquired) const
{
    int result;

    result = this->lock_internal(lock_acquired);
    if (result != 0)
        this->record_operation_error(this->last_operation_error());
    else
        this->record_operation_error(FT_ERR_SUCCESSS);
    return (result);
}

template <typename ElementType>
void ft_stack<ElementType>::unlock(bool lock_acquired) const
{
    this->unlock_internal(lock_acquired);
    int mutex_error = FT_ERR_SUCCESSS;
    if (this->_mutex != ft_nullptr && lock_acquired)
        mutex_error = ft_global_error_stack_last_error();
    if (mutex_error != FT_ERR_SUCCESSS)
        this->record_operation_error(mutex_error);
    else
        this->record_operation_error(FT_ERR_SUCCESSS);
    return ;
}

template <typename ElementType>
void ft_stack<ElementType>::push(const ElementType& value)
{
    StackNode *new_node;
    bool       lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->record_operation_error(this->last_operation_error());
        return ;
    }
    new_node = static_cast<StackNode *>(cma_malloc(sizeof(StackNode)));
    if (new_node == ft_nullptr)
    {
        this->record_operation_error(FT_ERR_NO_MEMORY);
        this->unlock_internal(lock_acquired);
        return ;
    }
    construct_at(&new_node->_data, value);
    new_node->_next = this->_top;
    this->_top = new_node;
    this->_size += 1;
    this->record_operation_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename ElementType>
void ft_stack<ElementType>::push(ElementType&& value)
{
    StackNode *new_node;
    bool       lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->record_operation_error(this->last_operation_error());
        return ;
    }
    new_node = static_cast<StackNode *>(cma_malloc(sizeof(StackNode)));
    if (new_node == ft_nullptr)
    {
        this->record_operation_error(FT_ERR_NO_MEMORY);
        this->unlock_internal(lock_acquired);
        return ;
    }
    construct_at(&new_node->_data, ft_move(value));
    new_node->_next = this->_top;
    this->_top = new_node;
    this->_size += 1;
    this->record_operation_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename ElementType>
ElementType ft_stack<ElementType>::pop()
{
    bool        lock_acquired;
    StackNode  *node;
    ElementType value;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->record_operation_error(this->last_operation_error());
        return (ElementType());
    }
    if (this->_top == ft_nullptr)
    {
        this->record_operation_error(FT_ERR_EMPTY);
        this->unlock_internal(lock_acquired);
        return (ElementType());
    }
    node = this->_top;
    this->_top = node->_next;
    value = ft_move(node->_data);
    destroy_at(&node->_data);
    cma_free(node);
    this->_size -= 1;
    this->record_operation_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (value);
}

template <typename ElementType>
ElementType& ft_stack<ElementType>::top()
{
    static ElementType error_element = ElementType();
    bool               lock_acquired;
    ElementType       *value;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->record_operation_error(this->last_operation_error());
        return (error_element);
    }
    if (this->_top == ft_nullptr)
    {
        this->record_operation_error(FT_ERR_EMPTY);
        this->unlock_internal(lock_acquired);
        return (error_element);
    }
    value = &this->_top->_data;
    this->record_operation_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (*value);
}

template <typename ElementType>
const ElementType& ft_stack<ElementType>::top() const
{
    static ElementType error_element = ElementType();
    bool               lock_acquired;
    const ElementType *value;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->record_operation_error(this->last_operation_error());
        return (error_element);
    }
    if (this->_top == ft_nullptr)
    {
        this->record_operation_error(FT_ERR_EMPTY);
        this->unlock_internal(lock_acquired);
        return (error_element);
    }
    value = &this->_top->_data;
    this->record_operation_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (*value);
}

template <typename ElementType>
size_t ft_stack<ElementType>::size() const
{
    bool   lock_acquired;
    size_t current_size;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->record_operation_error(this->last_operation_error());
        return (0);
    }
    current_size = this->_size;
    this->record_operation_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (current_size);
}

template <typename ElementType>
bool ft_stack<ElementType>::empty() const
{
    bool lock_acquired;
    bool is_empty;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->record_operation_error(this->last_operation_error());
        return (true);
    }
    is_empty = (this->_top == ft_nullptr);
    this->record_operation_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (is_empty);
}

template <typename ElementType>
void ft_stack<ElementType>::clear()
{
    StackNode *node;
    bool       lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->record_operation_error(this->last_operation_error());
        return ;
    }
    while (this->_top != ft_nullptr)
    {
        node = this->_top;
        this->_top = node->_next;
        destroy_at(&node->_data);
        cma_free(node);
    }
    this->_size = 0;
    this->record_operation_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename ElementType>
ft_operation_error_stack *ft_stack<ElementType>::get_operation_error_stack_for_validation() noexcept
{
    return (&this->_operation_errors);
}

template <typename ElementType>
const ft_operation_error_stack *ft_stack<ElementType>::get_operation_error_stack_for_validation() const noexcept
{
    return (&this->_operation_errors);
}

template <typename ElementType>
int ft_stack<ElementType>::last_operation_error() const noexcept
{
    return (ft_operation_error_stack_last_error(&this->_operation_errors));
}

template <typename ElementType>
const char *ft_stack<ElementType>::last_operation_error_str() const noexcept
{
    int error_code;
    const char *error_string;

    error_code = ft_operation_error_stack_last_error(&this->_operation_errors);
    error_string = ft_strerror(error_code);
    if (!error_string)
        error_string = "unknown error";
    return (error_string);
}

template <typename ElementType>
int ft_stack<ElementType>::operation_error_at(ft_size_t index) const noexcept
{
    return (ft_operation_error_stack_error_at(&this->_operation_errors, index));
}

template <typename ElementType>
const char *ft_stack<ElementType>::operation_error_str_at(ft_size_t index) const noexcept
{
    int error_code;
    const char *error_string;

    error_code = ft_operation_error_stack_error_at(&this->_operation_errors, index);
    error_string = ft_strerror(error_code);
    if (!error_string)
        error_string = "unknown error";
    return (error_string);
}

template <typename ElementType>
void ft_stack<ElementType>::pop_operation_errors() noexcept
{
    ft_operation_error_stack_pop_all(&this->_operation_errors);
    ft_global_error_stack_pop_all();
    return ;
}

template <typename ElementType>
int ft_stack<ElementType>::pop_oldest_operation_error() noexcept
{
    ft_global_error_stack_pop_last();
    return (ft_operation_error_stack_pop_last(&this->_operation_errors));
}

template <typename ElementType>
int ft_stack<ElementType>::pop_newest_operation_error() noexcept
{
    ft_global_error_stack_pop_newest();
    return (ft_operation_error_stack_pop_newest(&this->_operation_errors));
}

#endif
