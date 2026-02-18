#ifndef FT_QUEUE_HPP
#define FT_QUEUE_HPP

#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../CMA/CMA.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include "constructor.hpp"
#include "move.hpp"
#include <cstddef>
#include <cstdint>
#include <new>

template <typename ElementType>
class ft_queue
{
    private:
        struct QueueNode
        {
            ElementType _data;
            QueueNode*  _next;
        };

        QueueNode*                  _front;
        QueueNode*                  _rear;
        size_t                      _size;
        mutable pt_recursive_mutex* _mutex;
        mutable uint8_t             _initialized_state;

        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;
        static thread_local int32_t _last_error;

        void destroy_all_unlocked();
        int lock_internal(bool *lock_acquired) const;
        int unlock_internal(bool lock_acquired) const;
        static int set_last_operation_error(int error_code) noexcept;

        void abort_lifecycle_error(const char *method_name,
            const char *reason) const;
        void abort_if_not_initialized(const char *method_name) const;

    public:
        class value_proxy
        {
            private:
                ft_queue<ElementType> * _parent_queue;
                ElementType *            _element_pointer;
                int32_t                  _last_error;
                int32_t                  _is_valid;

            public:
                value_proxy();
                value_proxy(ft_queue<ElementType> *parent_queue,
                    ElementType *element_pointer, int32_t error_code,
                    int32_t is_valid);
                ~value_proxy();

                ElementType *operator->();
                ElementType &operator*();
                operator ElementType() const;
                int32_t get_error() const;
                int32_t is_valid() const;
        };

        ft_queue();
        ~ft_queue();

        ft_queue(const ft_queue&) = delete;
        ft_queue& operator=(const ft_queue&) = delete;
        ft_queue(ft_queue&& other) = delete;
        ft_queue& operator=(ft_queue&& other) = delete;

        int initialize();
        int destroy();

        int enable_thread_safety();
        int disable_thread_safety();
        bool is_thread_safe() const;
        int lock(bool *lock_acquired) const;
        void unlock(bool lock_acquired) const;

        void enqueue(const ElementType& value);
        void enqueue(ElementType&& value);
        ElementType dequeue();

        ElementType& front();
        const ElementType& front() const;
        value_proxy front_proxy();

        size_t size() const;
        bool empty() const;

        void clear();

        static int last_operation_error() noexcept;
        static const char *last_operation_error_str() noexcept;

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex* get_mutex_for_validation() const noexcept;
#endif
};

template <typename ElementType>
thread_local int32_t ft_queue<ElementType>::_last_error = FT_ERR_SUCCESS;

template <typename ElementType>
int ft_queue<ElementType>::set_last_operation_error(int error_code) noexcept
{
    ft_queue<ElementType>::_last_error = error_code;
    return (error_code);
}

template <typename ElementType>
void ft_queue<ElementType>::abort_lifecycle_error(const char *method_name,
    const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_queue lifecycle error: %s: %s\n", method_name, reason);
    su_abort();
    return ;
}

template <typename ElementType>
void ft_queue<ElementType>::abort_if_not_initialized(const char *method_name) const
{
    if (this->_initialized_state == ft_queue<ElementType>::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

template <typename ElementType>
ft_queue<ElementType>::value_proxy::value_proxy()
    : _parent_queue(ft_nullptr)
    , _element_pointer(ft_nullptr)
    , _last_error(FT_ERR_INVALID_STATE)
    , _is_valid(0)
{
    return ;
}

template <typename ElementType>
ft_queue<ElementType>::value_proxy::value_proxy(
    ft_queue<ElementType> *parent_queue,
    ElementType *element_pointer,
    int32_t error_code,
    int32_t is_valid)
    : _parent_queue(parent_queue)
    , _element_pointer(element_pointer)
    , _last_error(error_code)
    , _is_valid(is_valid)
{
    return ;
}

template <typename ElementType>
ft_queue<ElementType>::value_proxy::~value_proxy()
{
    return ;
}

template <typename ElementType>
ElementType *ft_queue<ElementType>::value_proxy::operator->()
{
    if (this->_is_valid == 0 || this->_element_pointer == ft_nullptr)
    {
        if (this->_parent_queue != ft_nullptr)
            this->_parent_queue->set_last_operation_error(this->_last_error);
        return (ft_nullptr);
    }
    if (this->_parent_queue != ft_nullptr)
        this->_parent_queue->set_last_operation_error(FT_ERR_SUCCESS);
    return (this->_element_pointer);
}

template <typename ElementType>
ElementType &ft_queue<ElementType>::value_proxy::operator*()
{
    if (this->_is_valid == 0 || this->_element_pointer == ft_nullptr)
    {
        static ElementType fallback_value = ElementType();

        if (this->_parent_queue != ft_nullptr)
            this->_parent_queue->set_last_operation_error(this->_last_error);
        return (fallback_value);
    }
    if (this->_parent_queue != ft_nullptr)
        this->_parent_queue->set_last_operation_error(FT_ERR_SUCCESS);
    return (*this->_element_pointer);
}

template <typename ElementType>
ft_queue<ElementType>::value_proxy::operator ElementType() const
{
    if (this->_is_valid == 0 || this->_element_pointer == ft_nullptr)
        return (ElementType());
    return (*this->_element_pointer);
}

template <typename ElementType>
int32_t ft_queue<ElementType>::value_proxy::get_error() const
{
    return (this->_last_error);
}

template <typename ElementType>
int32_t ft_queue<ElementType>::value_proxy::is_valid() const
{
    return (this->_is_valid);
}

template <typename ElementType>
ft_queue<ElementType>::ft_queue()
    : _front(ft_nullptr)
    , _rear(ft_nullptr)
    , _size(0)
    , _mutex(ft_nullptr)
    , _initialized_state(ft_queue<ElementType>::_state_uninitialized)
{
    return ;
}

template <typename ElementType>
ft_queue<ElementType>::~ft_queue()
{
    if (this->_initialized_state == ft_queue<ElementType>::_state_uninitialized)
    {
        this->abort_lifecycle_error("ft_queue::~ft_queue",
            "destructor called while object is uninitialized");
        return ;
    }
    if (this->_initialized_state == ft_queue<ElementType>::_state_initialized)
        (void)this->destroy();
    return ;
}

template <typename ElementType>
int ft_queue<ElementType>::initialize()
{
    if (this->_initialized_state == ft_queue<ElementType>::_state_initialized)
    {
        this->abort_lifecycle_error("ft_queue::initialize",
            "called while object is already initialized");
        return (ft_queue<ElementType>::set_last_operation_error(FT_ERR_INVALID_STATE));
    }
    this->_front = ft_nullptr;
    this->_rear = ft_nullptr;
    this->_size = 0;
    this->_mutex = ft_nullptr;
    this->_initialized_state = ft_queue<ElementType>::_state_initialized;
    return (ft_queue<ElementType>::set_last_operation_error(FT_ERR_SUCCESS));
}

template <typename ElementType>
int ft_queue<ElementType>::destroy()
{
    bool lock_acquired;
    int lock_error;
    int unlock_error;
    int mutex_destroy_error;

    if (this->_initialized_state != ft_queue<ElementType>::_state_initialized)
    {
        this->abort_lifecycle_error("ft_queue::destroy",
            "called while object is not initialized");
        return (ft_queue<ElementType>::set_last_operation_error(FT_ERR_INVALID_STATE));
    }
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (ft_queue<ElementType>::set_last_operation_error(lock_error));
    this->destroy_all_unlocked();
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
        return (ft_queue<ElementType>::set_last_operation_error(unlock_error));
    mutex_destroy_error = this->disable_thread_safety();
    this->_initialized_state = ft_queue<ElementType>::_state_destroyed;
    if (mutex_destroy_error != FT_ERR_SUCCESS)
        return (ft_queue<ElementType>::set_last_operation_error(mutex_destroy_error));
    return (ft_queue<ElementType>::set_last_operation_error(FT_ERR_SUCCESS));
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
    pt_recursive_mutex *mutex_pointer;
    int mutex_error;

    this->abort_if_not_initialized("ft_queue::enable_thread_safety");
    if (this->_mutex != ft_nullptr)
        return (ft_queue<ElementType>::set_last_operation_error(FT_ERR_SUCCESS));
    mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    if (mutex_pointer == ft_nullptr)
        return (ft_queue<ElementType>::set_last_operation_error(FT_ERR_NO_MEMORY));
    mutex_error = mutex_pointer->initialize();
    if (mutex_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        return (ft_queue<ElementType>::set_last_operation_error(mutex_error));
    }
    this->_mutex = mutex_pointer;
    return (ft_queue<ElementType>::set_last_operation_error(FT_ERR_SUCCESS));
}

template <typename ElementType>
int ft_queue<ElementType>::disable_thread_safety()
{
    int destroy_error;

    this->abort_if_not_initialized("ft_queue::disable_thread_safety");
    if (this->_mutex == ft_nullptr)
        return (ft_queue<ElementType>::set_last_operation_error(FT_ERR_SUCCESS));
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (ft_queue<ElementType>::set_last_operation_error(destroy_error));
}

template <typename ElementType>
bool ft_queue<ElementType>::is_thread_safe() const
{
    this->abort_if_not_initialized("ft_queue::is_thread_safe");
    ft_queue<ElementType>::set_last_operation_error(FT_ERR_SUCCESS);
    return (this->_mutex != ft_nullptr);
}

template <typename ElementType>
int ft_queue<ElementType>::lock(bool *lock_acquired) const
{
    int lock_error;

    this->abort_if_not_initialized("ft_queue::lock");
    lock_error = this->lock_internal(lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (ft_queue<ElementType>::set_last_operation_error(lock_error));
    return (ft_queue<ElementType>::set_last_operation_error(FT_ERR_SUCCESS));
}

template <typename ElementType>
void ft_queue<ElementType>::unlock(bool lock_acquired) const
{
    int unlock_error;

    this->abort_if_not_initialized("ft_queue::unlock");
    unlock_error = this->unlock_internal(lock_acquired);
    ft_queue<ElementType>::set_last_operation_error(unlock_error);
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
    result = this->_mutex->lock();
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
    return (this->_mutex->unlock());
}

template <typename ElementType>
void ft_queue<ElementType>::enqueue(const ElementType& value)
{
    QueueNode *node;
    bool lock_acquired;
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("ft_queue::enqueue(const ElementType&)");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_queue<ElementType>::set_last_operation_error(lock_error);
        return ;
    }
    node = static_cast<QueueNode *>(cma_malloc(sizeof(QueueNode)));
    if (node == ft_nullptr)
    {
        unlock_error = this->unlock_internal(lock_acquired);
        if (unlock_error != FT_ERR_SUCCESS)
            ft_queue<ElementType>::set_last_operation_error(unlock_error);
        else
            ft_queue<ElementType>::set_last_operation_error(FT_ERR_NO_MEMORY);
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
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        ft_queue<ElementType>::set_last_operation_error(unlock_error);
        return ;
    }
    ft_queue<ElementType>::set_last_operation_error(FT_ERR_SUCCESS);
    return ;
}

template <typename ElementType>
void ft_queue<ElementType>::enqueue(ElementType&& value)
{
    QueueNode *node;
    bool lock_acquired;
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("ft_queue::enqueue(ElementType&&)");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_queue<ElementType>::set_last_operation_error(lock_error);
        return ;
    }
    node = static_cast<QueueNode *>(cma_malloc(sizeof(QueueNode)));
    if (node == ft_nullptr)
    {
        unlock_error = this->unlock_internal(lock_acquired);
        if (unlock_error != FT_ERR_SUCCESS)
            ft_queue<ElementType>::set_last_operation_error(unlock_error);
        else
            ft_queue<ElementType>::set_last_operation_error(FT_ERR_NO_MEMORY);
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
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        ft_queue<ElementType>::set_last_operation_error(unlock_error);
        return ;
    }
    ft_queue<ElementType>::set_last_operation_error(FT_ERR_SUCCESS);
    return ;
}

template <typename ElementType>
ElementType ft_queue<ElementType>::dequeue()
{
    QueueNode   *node;
    ElementType result;
    bool        lock_acquired;
    int         lock_error;
    int         unlock_error;

    this->abort_if_not_initialized("ft_queue::dequeue");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_queue<ElementType>::set_last_operation_error(lock_error);
        return (ElementType());
    }
    if (this->_front == ft_nullptr)
    {
        unlock_error = this->unlock_internal(lock_acquired);
        if (unlock_error != FT_ERR_SUCCESS)
            ft_queue<ElementType>::set_last_operation_error(unlock_error);
        else
            ft_queue<ElementType>::set_last_operation_error(FT_ERR_EMPTY);
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
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        ft_queue<ElementType>::set_last_operation_error(unlock_error);
        return (ElementType());
    }
    ft_queue<ElementType>::set_last_operation_error(FT_ERR_SUCCESS);
    return (result);
}

template <typename ElementType>
ElementType& ft_queue<ElementType>::front()
{
    static ElementType error_element = ElementType();
    ElementType       *value;
    bool               lock_acquired;
    int                lock_error;
    int                unlock_error;

    this->abort_if_not_initialized("ft_queue::front");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_queue<ElementType>::set_last_operation_error(lock_error);
        return (error_element);
    }
    if (this->_front == ft_nullptr)
    {
        unlock_error = this->unlock_internal(lock_acquired);
        if (unlock_error != FT_ERR_SUCCESS)
            ft_queue<ElementType>::set_last_operation_error(unlock_error);
        else
            ft_queue<ElementType>::set_last_operation_error(FT_ERR_EMPTY);
        return (error_element);
    }
    value = &this->_front->_data;
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        ft_queue<ElementType>::set_last_operation_error(unlock_error);
        return (error_element);
    }
    ft_queue<ElementType>::set_last_operation_error(FT_ERR_SUCCESS);
    return (*value);
}

template <typename ElementType>
const ElementType& ft_queue<ElementType>::front() const
{
    static ElementType error_element = ElementType();
    const ElementType *value;
    bool               lock_acquired;
    int                lock_error;
    int                unlock_error;

    this->abort_if_not_initialized("ft_queue::front const");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_queue<ElementType>::set_last_operation_error(lock_error);
        return (error_element);
    }
    if (this->_front == ft_nullptr)
    {
        unlock_error = this->unlock_internal(lock_acquired);
        if (unlock_error != FT_ERR_SUCCESS)
            ft_queue<ElementType>::set_last_operation_error(unlock_error);
        else
            ft_queue<ElementType>::set_last_operation_error(FT_ERR_EMPTY);
        return (error_element);
    }
    value = &this->_front->_data;
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        ft_queue<ElementType>::set_last_operation_error(unlock_error);
        return (error_element);
    }
    ft_queue<ElementType>::set_last_operation_error(FT_ERR_SUCCESS);
    return (*value);
}

template <typename ElementType>
typename ft_queue<ElementType>::value_proxy ft_queue<ElementType>::front_proxy()
{
    ElementType *value;
    bool lock_acquired;
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("ft_queue::front_proxy");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_queue<ElementType>::set_last_operation_error(lock_error);
        return (typename ft_queue<ElementType>::value_proxy(this,
            ft_nullptr, lock_error, 0));
    }
    if (this->_front == ft_nullptr)
    {
        unlock_error = this->unlock_internal(lock_acquired);
        if (unlock_error != FT_ERR_SUCCESS)
        {
            ft_queue<ElementType>::set_last_operation_error(unlock_error);
            return (typename ft_queue<ElementType>::value_proxy(this,
                ft_nullptr, unlock_error, 0));
        }
        ft_queue<ElementType>::set_last_operation_error(FT_ERR_EMPTY);
        return (typename ft_queue<ElementType>::value_proxy(this,
            ft_nullptr, FT_ERR_EMPTY, 0));
    }
    value = &this->_front->_data;
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        ft_queue<ElementType>::set_last_operation_error(unlock_error);
        return (typename ft_queue<ElementType>::value_proxy(this,
            ft_nullptr, unlock_error, 0));
    }
    ft_queue<ElementType>::set_last_operation_error(FT_ERR_SUCCESS);
    return (typename ft_queue<ElementType>::value_proxy(this,
        value, FT_ERR_SUCCESS, 1));
}

template <typename ElementType>
size_t ft_queue<ElementType>::size() const
{
    size_t current_size;
    bool   lock_acquired;
    int    lock_error;
    int    unlock_error;

    this->abort_if_not_initialized("ft_queue::size");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_queue<ElementType>::set_last_operation_error(lock_error);
        return (0);
    }
    current_size = this->_size;
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        ft_queue<ElementType>::set_last_operation_error(unlock_error);
        return (0);
    }
    ft_queue<ElementType>::set_last_operation_error(FT_ERR_SUCCESS);
    return (current_size);
}

template <typename ElementType>
bool ft_queue<ElementType>::empty() const
{
    bool lock_acquired;
    bool is_empty;
    int  lock_error;
    int  unlock_error;

    this->abort_if_not_initialized("ft_queue::empty");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_queue<ElementType>::set_last_operation_error(lock_error);
        return (true);
    }
    is_empty = (this->_size == 0);
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        ft_queue<ElementType>::set_last_operation_error(unlock_error);
        return (true);
    }
    ft_queue<ElementType>::set_last_operation_error(FT_ERR_SUCCESS);
    return (is_empty);
}

template <typename ElementType>
void ft_queue<ElementType>::clear()
{
    bool lock_acquired;
    int  lock_error;
    int  unlock_error;

    this->abort_if_not_initialized("ft_queue::clear");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_queue<ElementType>::set_last_operation_error(lock_error);
        return ;
    }
    this->destroy_all_unlocked();
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        ft_queue<ElementType>::set_last_operation_error(unlock_error);
        return ;
    }
    ft_queue<ElementType>::set_last_operation_error(FT_ERR_SUCCESS);
    return ;
}

template <typename ElementType>
int ft_queue<ElementType>::last_operation_error() noexcept
{
    return (ft_queue<ElementType>::_last_error);
}

template <typename ElementType>
const char *ft_queue<ElementType>::last_operation_error_str() noexcept
{
    return (ft_strerror(ft_queue<ElementType>::last_operation_error()));
}

#ifdef LIBFT_TEST_BUILD
template <typename ElementType>
pt_recursive_mutex* ft_queue<ElementType>::get_mutex_for_validation() const noexcept
{
    return (this->_mutex);
}
#endif

#endif
