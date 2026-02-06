#ifndef FT_QUEUE_HPP
#define FT_QUEUE_HPP

#include "constructor.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"
#include <cstddef>
#include <utility>
#include <new>
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

            QueueNode*          _front;
            QueueNode*          _rear;
            size_t              _size;
            mutable int         _error_code;
            mutable pt_mutex   *_mutex;
        bool                _thread_safe_enabled;

        void record_operation_error(int error_code) const noexcept;

        void    set_error(int error_code) const;
        int     lock_internal(bool *lock_acquired) const;
        int     unlock_internal(bool lock_acquired) const;
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

        int get_error() const;
        const char* get_error_str() const;

        void clear();
};

template <typename ElementType>
void ft_queue<ElementType>::record_operation_error(int error_code) const noexcept
{
    unsigned long long operation_id;

    operation_id = ft_errno_next_operation_id();
    ft_global_error_stack_push_entry_with_id(error_code, operation_id);
    return ;
}
template <typename ElementType>
ft_queue<ElementType>::ft_queue()
    : _front(ft_nullptr), _rear(ft_nullptr), _size(0), _error_code(FT_ERR_SUCCESSS),
      _mutex(ft_nullptr), _thread_safe_enabled(false)
{
    return ;
}

template <typename ElementType>
ft_queue<ElementType>::~ft_queue()
{
    this->clear();
    this->teardown_thread_safety();
    return ;
}

template <typename ElementType>
ft_queue<ElementType>::ft_queue(ft_queue&& other) noexcept
    : _front(ft_nullptr), _rear(ft_nullptr), _size(0),
      _error_code(FT_ERR_SUCCESSS), _mutex(ft_nullptr),
      _thread_safe_enabled(false)
{
    bool other_lock_acquired;
    bool other_thread_safe;

    other_lock_acquired = false;
    if (other.lock_internal(&other_lock_acquired) != 0)
    {
        this->set_error(other.get_error());
        return ;
    }
    this->_front = other._front;
    this->_rear = other._rear;
    this->_size = other._size;
    this->_error_code = other._error_code;
    other_thread_safe = other._thread_safe_enabled;
    other._front = ft_nullptr;
    other._rear = ft_nullptr;
    other._size = 0;
    other._error_code = FT_ERR_SUCCESSS;
    other.unlock_internal(other_lock_acquired);
    other.teardown_thread_safety();
    if (other_thread_safe)
    {
        if (this->enable_thread_safety() != 0)
            return ;
    }
    this->set_error(this->_error_code);
    return ;
}

template <typename ElementType>
ft_queue<ElementType>& ft_queue<ElementType>::operator=(ft_queue&& other) noexcept
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
        this->set_error(other.get_error());
        return (*this);
    }
    this->_front = other._front;
    this->_rear = other._rear;
    this->_size = other._size;
    this->_error_code = other._error_code;
    other_thread_safe = other._thread_safe_enabled;
    other._front = ft_nullptr;
    other._rear = ft_nullptr;
    other._size = 0;
    other._error_code = FT_ERR_SUCCESSS;
    other.unlock_internal(other_lock_acquired);
    other.teardown_thread_safety();
    if (other_thread_safe)
    {
        if (this->enable_thread_safety() != 0)
            return (*this);
    }
    this->set_error(this->_error_code);
    return (*this);
}

template <typename ElementType>
void ft_queue<ElementType>::set_error(int error_code) const
{
    this->_error_code = error_code;
    this->record_operation_error(error_code);
    return ;
}

template <typename ElementType>
int ft_queue<ElementType>::lock_internal(bool *lock_acquired) const
{
    int mutex_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (!this->_thread_safe_enabled || this->_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESSS);
        return (FT_ERR_SUCCESSS);
    }
    this->_mutex->lock(THREAD_ID);
    mutex_error = ft_global_error_stack_drop_last_error();
    ft_global_error_stack_drop_last_error();
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        this->set_error(mutex_error);
        return (mutex_error);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    this->set_error(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

template <typename ElementType>
int ft_queue<ElementType>::unlock_internal(bool lock_acquired) const
{
    if (!lock_acquired || this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESSS);
    this->_mutex->unlock(THREAD_ID);
    int mutex_error;

    mutex_error = ft_global_error_stack_drop_last_error();
    ft_global_error_stack_drop_last_error();
    return (mutex_error);
}

template <typename ElementType>
void ft_queue<ElementType>::teardown_thread_safety()
{
    if (this->_mutex != ft_nullptr)
    {
        delete this->_mutex;
        this->_mutex = ft_nullptr;
    }
    this->_thread_safe_enabled = false;
    return ;
}

template <typename ElementType>
int ft_queue<ElementType>::enable_thread_safety()
{
    if (this->_thread_safe_enabled && this->_mutex != ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESSS);
        return (0);
    }
    pt_mutex *mutex_pointer;

    mutex_pointer = new(std::nothrow) pt_mutex();
    if (mutex_pointer == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return (-1);
    }
        {
            int mutex_error;

            mutex_error = ft_global_error_stack_drop_last_error();
            ft_global_error_stack_drop_last_error();
            if (mutex_error != FT_ERR_SUCCESSS)
            {
                delete mutex_pointer;
                this->set_error(mutex_error);
                return (-1);
            }
        }
    this->_mutex = mutex_pointer;
    this->_thread_safe_enabled = true;
    this->set_error(FT_ERR_SUCCESSS);
    return (0);
}

template <typename ElementType>
void ft_queue<ElementType>::disable_thread_safety()
{
    this->teardown_thread_safety();
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

template <typename ElementType>
bool ft_queue<ElementType>::is_thread_safe() const
{
    bool enabled;

    enabled = (this->_thread_safe_enabled && this->_mutex != ft_nullptr);
    const_cast<ft_queue<ElementType> *>(this)->set_error(FT_ERR_SUCCESSS);
    return (enabled);
}

template <typename ElementType>
int ft_queue<ElementType>::lock(bool *lock_acquired) const
{
    int result;

    result = this->lock_internal(lock_acquired);
    if (result != FT_ERR_SUCCESSS)
        return (-1);
    return (0);
}

template <typename ElementType>
void ft_queue<ElementType>::unlock(bool lock_acquired) const
{
    int unlock_error;

    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESSS)
        const_cast<ft_queue<ElementType> *>(this)->set_error(unlock_error);
    else
        const_cast<ft_queue<ElementType> *>(this)->set_error(FT_ERR_SUCCESSS);
    return ;
}

template <typename ElementType>
void ft_queue<ElementType>::enqueue(const ElementType& value)
{
    QueueNode *node;
    bool       lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(this->get_error());
        return ;
    }
    node = static_cast<QueueNode *>(cma_malloc(sizeof(QueueNode)));
    if (node == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
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
    this->set_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename ElementType>
void ft_queue<ElementType>::enqueue(ElementType&& value)
{
    QueueNode *node;
    bool       lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(this->get_error());
        return ;
    }
    node = static_cast<QueueNode *>(cma_malloc(sizeof(QueueNode)));
    if (node == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
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
    this->set_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename ElementType>
ElementType ft_queue<ElementType>::dequeue()
{
    QueueNode  *node;
    ElementType value;
    bool        lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(this->get_error());
        return (ElementType());
    }
    if (this->_front == ft_nullptr)
    {
        this->set_error(FT_ERR_EMPTY);
        this->unlock_internal(lock_acquired);
        return (ElementType());
    }
    node = this->_front;
    this->_front = node->_next;
    if (this->_front == ft_nullptr)
        this->_rear = ft_nullptr;
    value = ft_move(node->_data);
    destroy_at(&node->_data);
    cma_free(node);
    this->_size -= 1;
    this->set_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (value);
}

template <typename ElementType>
ElementType& ft_queue<ElementType>::front()
{
    static ElementType error_element = ElementType();
    ElementType       *value;
    bool               lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(this->get_error());
        return (error_element);
    }
    if (this->_front == ft_nullptr)
    {
        this->set_error(FT_ERR_EMPTY);
        this->unlock_internal(lock_acquired);
        return (error_element);
    }
    value = &this->_front->_data;
    this->set_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (*value);
}

template <typename ElementType>
const ElementType& ft_queue<ElementType>::front() const
{
    static ElementType error_element = ElementType();
    const ElementType *value;
    bool               lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        const_cast<ft_queue<ElementType> *>(this)->set_error(this->get_error());
        return (error_element);
    }
    if (this->_front == ft_nullptr)
    {
        const_cast<ft_queue<ElementType> *>(this)->set_error(FT_ERR_EMPTY);
        this->unlock_internal(lock_acquired);
        return (error_element);
    }
    value = &this->_front->_data;
    const_cast<ft_queue<ElementType> *>(this)->set_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (*value);
}

template <typename ElementType>
size_t ft_queue<ElementType>::size() const
{
    size_t current_size;
    bool   lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        const_cast<ft_queue<ElementType> *>(this)->set_error(this->get_error());
        return (0);
    }
    current_size = this->_size;
    const_cast<ft_queue<ElementType> *>(this)->set_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (current_size);
}

template <typename ElementType>
bool ft_queue<ElementType>::empty() const
{
    bool is_empty;
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        const_cast<ft_queue<ElementType> *>(this)->set_error(this->get_error());
        return (true);
    }
    is_empty = (this->_size == 0);
    const_cast<ft_queue<ElementType> *>(this)->set_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (is_empty);
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
    QueueNode *node;
    bool       lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(this->get_error());
        return ;
    }
    while (this->_front != ft_nullptr)
    {
        node = this->_front;
        this->_front = this->_front->_next;
        destroy_at(&node->_data);
        cma_free(node);
    }
    this->_rear = ft_nullptr;
    this->_size = 0;
    this->set_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return ;
}

#endif
