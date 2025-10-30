#ifndef FT_DEQUE_HPP
#define FT_DEQUE_HPP

#include "constructor.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"
#include <cstddef>
#include <utility>
#include "move.hpp"

template <typename ElementType>
class ft_deque
{
    private:
        struct DequeNode
        {
            ElementType _data;
            DequeNode* _prev;
            DequeNode* _next;
        };

        DequeNode*           _front;
        DequeNode*           _back;
        size_t               _size;
        mutable int          _error_code;
        mutable pt_mutex*    _mutex;
        bool                 _thread_safe_enabled;

        void    set_error(int error) const;
        int     lock_internal(bool *lock_acquired) const;
        void    unlock_internal(bool lock_acquired) const;
        void    teardown_thread_safety();

    public:
        ft_deque();
        ~ft_deque();

        ft_deque(const ft_deque&) = delete;
        ft_deque& operator=(const ft_deque&) = delete;

        ft_deque(ft_deque&& other) noexcept;
        ft_deque& operator=(ft_deque&& other) noexcept;

        int     enable_thread_safety();
        void    disable_thread_safety();
        bool    is_thread_safe() const;
        int     lock(bool *lock_acquired) const;
        void    unlock(bool lock_acquired) const;

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
    : _front(ft_nullptr), _back(ft_nullptr), _size(0),
      _error_code(ER_SUCCESS), _mutex(ft_nullptr),
      _thread_safe_enabled(false)
{
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ElementType>
ft_deque<ElementType>::~ft_deque()
{
    this->clear();
    this->teardown_thread_safety();
    return ;
}

template <typename ElementType>
ft_deque<ElementType>::ft_deque(ft_deque&& other) noexcept
    : _front(other._front), _back(other._back), _size(other._size),
      _error_code(other._error_code), _mutex(other._mutex),
      _thread_safe_enabled(other._thread_safe_enabled)
{
    other._front = ft_nullptr;
    other._back = ft_nullptr;
    other._size = 0;
    other._error_code = ER_SUCCESS;
    other._mutex = ft_nullptr;
    other._thread_safe_enabled = false;
    return ;
}

template <typename ElementType>
ft_deque<ElementType>& ft_deque<ElementType>::operator=(ft_deque&& other) noexcept
{
    if (this != &other)
    {
        this->clear();
        this->teardown_thread_safety();
        this->_front = other._front;
        this->_back = other._back;
        this->_size = other._size;
        this->_error_code = other._error_code;
        this->_mutex = other._mutex;
        this->_thread_safe_enabled = other._thread_safe_enabled;
        other._front = ft_nullptr;
        other._back = ft_nullptr;
        other._size = 0;
        other._error_code = ER_SUCCESS;
        other._mutex = ft_nullptr;
        other._thread_safe_enabled = false;
    }
    this->set_error(ER_SUCCESS);
    return (*this);
}

template <typename ElementType>
void ft_deque<ElementType>::set_error(int error) const
{
    this->_error_code = error;
    ft_errno = error;
    return ;
}

template <typename ElementType>
int ft_deque<ElementType>::enable_thread_safety()
{
    void     *memory;
    pt_mutex *mutex_pointer;

    if (this->_thread_safe_enabled && this->_mutex != ft_nullptr)
    {
        this->set_error(ER_SUCCESS);
        return (0);
    }
    memory = cma_malloc(sizeof(pt_mutex));
    if (memory == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return (-1);
    }
    mutex_pointer = new(memory) pt_mutex();
    if (mutex_pointer->get_error() != ER_SUCCESS)
    {
        int mutex_error;

        mutex_error = mutex_pointer->get_error();
        mutex_pointer->~pt_mutex();
        cma_free(memory);
        this->set_error(mutex_error);
        return (-1);
    }
    this->_mutex = mutex_pointer;
    this->_thread_safe_enabled = true;
    this->set_error(ER_SUCCESS);
    return (0);
}

template <typename ElementType>
void ft_deque<ElementType>::disable_thread_safety()
{
    this->teardown_thread_safety();
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ElementType>
bool ft_deque<ElementType>::is_thread_safe() const
{
    bool enabled;

    enabled = (this->_thread_safe_enabled && this->_mutex != ft_nullptr);
    const_cast<ft_deque<ElementType> *>(this)->set_error(ER_SUCCESS);
    return (enabled);
}

template <typename ElementType>
int ft_deque<ElementType>::lock(bool *lock_acquired) const
{
    int result;

    result = this->lock_internal(lock_acquired);
    if (result != 0)
        const_cast<ft_deque<ElementType> *>(this)->set_error(ft_errno);
    else
        const_cast<ft_deque<ElementType> *>(this)->set_error(ER_SUCCESS);
    return (result);
}

template <typename ElementType>
void ft_deque<ElementType>::unlock(bool lock_acquired) const
{
    int entry_errno;

    entry_errno = ft_errno;
    this->unlock_internal(lock_acquired);
    if (this->_mutex != ft_nullptr && this->_mutex->get_error() != ER_SUCCESS)
        const_cast<ft_deque<ElementType> *>(this)->set_error(this->_mutex->get_error());
    else
    {
        ft_errno = entry_errno;
        const_cast<ft_deque<ElementType> *>(this)->set_error(ft_errno);
    }
    return ;
}

template <typename ElementType>
void ft_deque<ElementType>::push_front(const ElementType& value)
{
    DequeNode* node;
    bool       lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    node = static_cast<DequeNode*>(cma_malloc(sizeof(DequeNode)));
    if (node == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
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
    this->set_error(ER_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename ElementType>
void ft_deque<ElementType>::push_front(ElementType&& value)
{
    DequeNode* node;
    bool       lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    node = static_cast<DequeNode*>(cma_malloc(sizeof(DequeNode)));
    if (node == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
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
    this->set_error(ER_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename ElementType>
void ft_deque<ElementType>::push_back(const ElementType& value)
{
    DequeNode* node;
    bool       lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    node = static_cast<DequeNode*>(cma_malloc(sizeof(DequeNode)));
    if (node == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
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
    this->set_error(ER_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename ElementType>
void ft_deque<ElementType>::push_back(ElementType&& value)
{
    DequeNode* node;
    bool       lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    node = static_cast<DequeNode*>(cma_malloc(sizeof(DequeNode)));
    if (node == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
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
    this->set_error(ER_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename ElementType>
ElementType ft_deque<ElementType>::pop_front()
{
    bool        lock_acquired;
    DequeNode  *node;
    ElementType value;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
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
        this->_back = ft_nullptr;
    else
        this->_front->_prev = ft_nullptr;
    value = ft_move(node->_data);
    destroy_at(&node->_data);
    cma_free(node);
    this->_size -= 1;
    this->set_error(ER_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (value);
}

template <typename ElementType>
ElementType ft_deque<ElementType>::pop_back()
{
    bool        lock_acquired;
    DequeNode  *node;
    ElementType value;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return (ElementType());
    }
    if (this->_back == ft_nullptr)
    {
        this->set_error(FT_ERR_EMPTY);
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
    this->set_error(ER_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (value);
}

template <typename ElementType>
ElementType& ft_deque<ElementType>::front()
{
    static ElementType error_element = ElementType();
    bool               lock_acquired;
    ElementType       *value_pointer;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return (error_element);
    }
    if (this->_front == ft_nullptr)
    {
        this->set_error(FT_ERR_EMPTY);
        this->unlock_internal(lock_acquired);
        return (error_element);
    }
    value_pointer = &this->_front->_data;
    this->set_error(ER_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (*value_pointer);
}

template <typename ElementType>
const ElementType& ft_deque<ElementType>::front() const
{
    static ElementType error_element = ElementType();
    bool               lock_acquired;
    const ElementType *value_pointer;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        const_cast<ft_deque<ElementType> *>(this)->set_error(ft_errno);
        return (error_element);
    }
    if (this->_front == ft_nullptr)
    {
        const_cast<ft_deque<ElementType> *>(this)->set_error(FT_ERR_EMPTY);
        this->unlock_internal(lock_acquired);
        return (error_element);
    }
    value_pointer = &this->_front->_data;
    this->set_error(ER_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (*value_pointer);
}

template <typename ElementType>
ElementType& ft_deque<ElementType>::back()
{
    static ElementType error_element = ElementType();
    bool               lock_acquired;
    ElementType       *value_pointer;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return (error_element);
    }
    if (this->_back == ft_nullptr)
    {
        this->set_error(FT_ERR_EMPTY);
        this->unlock_internal(lock_acquired);
        return (error_element);
    }
    value_pointer = &this->_back->_data;
    this->set_error(ER_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (*value_pointer);
}

template <typename ElementType>
const ElementType& ft_deque<ElementType>::back() const
{
    static ElementType error_element = ElementType();
    bool               lock_acquired;
    const ElementType *value_pointer;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        const_cast<ft_deque<ElementType> *>(this)->set_error(ft_errno);
        return (error_element);
    }
    if (this->_back == ft_nullptr)
    {
        const_cast<ft_deque<ElementType> *>(this)->set_error(FT_ERR_EMPTY);
        this->unlock_internal(lock_acquired);
        return (error_element);
    }
    value_pointer = &this->_back->_data;
    this->set_error(ER_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (*value_pointer);
}

template <typename ElementType>
size_t ft_deque<ElementType>::size() const
{
    bool   lock_acquired;
    size_t current_size;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        const_cast<ft_deque<ElementType> *>(this)->set_error(ft_errno);
        return (0);
    }
    current_size = this->_size;
    const_cast<ft_deque<ElementType> *>(this)->set_error(ER_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (current_size);
}

template <typename ElementType>
bool ft_deque<ElementType>::empty() const
{
    bool lock_acquired;
    bool is_empty;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        const_cast<ft_deque<ElementType> *>(this)->set_error(ft_errno);
        return (true);
    }
    is_empty = (this->_size == 0);
    const_cast<ft_deque<ElementType> *>(this)->set_error(ER_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (is_empty);
}

template <typename ElementType>
int ft_deque<ElementType>::get_error() const
{
    return (this->_error_code);
}

template <typename ElementType>
const char* ft_deque<ElementType>::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}

template <typename ElementType>
void ft_deque<ElementType>::clear()
{
    bool       lock_acquired;
    DequeNode *node;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    while (this->_front != ft_nullptr)
    {
        node = this->_front;
        this->_front = this->_front->_next;
        destroy_at(&node->_data);
        cma_free(node);
    }
    this->_back = ft_nullptr;
    this->_size = 0;
    this->set_error(ER_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename ElementType>
int ft_deque<ElementType>::lock_internal(bool *lock_acquired) const
{
    if (lock_acquired)
        *lock_acquired = false;
    if (!this->_thread_safe_enabled || this->_mutex == ft_nullptr)
    {
        ft_errno = ER_SUCCESS;
        return (0);
    }
    this->_mutex->lock(THREAD_ID);
    if (this->_mutex->get_error() != ER_SUCCESS)
    {
        if (this->_mutex->get_error() == FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            bool state_lock_acquired;

            state_lock_acquired = false;
            ft_errno = ER_SUCCESS;
            if (this->_mutex->lock_state(&state_lock_acquired) == 0)
                this->_mutex->unlock_state(state_lock_acquired);
            ft_errno = ER_SUCCESS;
            return (0);
        }
        ft_errno = this->_mutex->get_error();
        return (-1);
    }
    if (lock_acquired)
        *lock_acquired = true;
    ft_errno = ER_SUCCESS;
    return (0);
}

template <typename ElementType>
void ft_deque<ElementType>::unlock_internal(bool lock_acquired) const
{
    int entry_errno;

    if (!lock_acquired || this->_mutex == ft_nullptr)
        return ;
    entry_errno = ft_errno;
    this->_mutex->unlock(THREAD_ID);
    if (this->_mutex->get_error() != ER_SUCCESS)
    {
        ft_errno = this->_mutex->get_error();
        return ;
    }
    ft_errno = entry_errno;
    return ;
}

template <typename ElementType>
void ft_deque<ElementType>::teardown_thread_safety()
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

#endif
