#ifndef FT_EVENT_EMITTER_HPP
#define FT_EVENT_EMITTER_HPP

#include "constructor.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Basic/basic.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread.hpp"
#include <cstddef>
#include <utility>

#include "move.hpp"
template <typename EventType, typename... Args>
class ft_event_emitter
{
    private:
        struct Listener
        {
            EventType _event;
            void (*_callback)(Args...);
        };

        Listener*                     _listeners;
        size_t                        _capacity;
        size_t                        _size;
        mutable pt_recursive_mutex*   _mutex;

        bool    ensure_capacity(size_t desired);
        bool    ensure_capacity_unlocked(size_t desired);
        void    clear_unlocked();
        int     lock_internal(bool *lock_acquired) const;
        int     unlock_internal(bool lock_acquired) const;
        int     prepare_thread_safety();
        void    teardown_thread_safety();

        template <typename, typename...>
        friend struct ft_event_emitter_test_helper;

    public:
        ft_event_emitter(size_t initial_capacity = 0);
        ~ft_event_emitter();

        ft_event_emitter(const ft_event_emitter&) = delete;
        ft_event_emitter& operator=(const ft_event_emitter&) = delete;

        ft_event_emitter(ft_event_emitter&& other) noexcept;
        ft_event_emitter& operator=(ft_event_emitter&& other) noexcept;

        void    on(const EventType& event, void (*callback)(Args...));
        void    emit(const EventType& event, Args... args);
        void    remove_listener(const EventType& event, void (*callback)(Args...));
        size_t  size() const;
        bool    empty() const;
        void    clear();
        int     enable_thread_safety();
        void    disable_thread_safety();
        bool    is_thread_safe() const;
        int     lock(bool *lock_acquired) const;
        void    unlock(bool lock_acquired) const;

    #ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_validation() const noexcept;
    #endif
};

template <typename EventType, typename... Args>
ft_event_emitter<EventType, Args...>::ft_event_emitter(size_t initial_capacity)
    : _listeners(ft_nullptr), _capacity(0), _size(0), _mutex(ft_nullptr)
{
    if (initial_capacity > 0)
    {
        this->_listeners = static_cast<Listener*>(cma_malloc(sizeof(Listener) * initial_capacity));
        if (this->_listeners == ft_nullptr)
        {
            ft_global_error_stack_push(FT_ERR_NO_MEMORY);
            return ;
        }
        this->_capacity = initial_capacity;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

template <typename EventType, typename... Args>
ft_event_emitter<EventType, Args...>::~ft_event_emitter()
{
    this->clear_unlocked();
    if (this->_listeners != ft_nullptr)
    {
        cma_free(this->_listeners);
        this->_listeners = ft_nullptr;
    }
    this->_capacity = 0;
    this->teardown_thread_safety();
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

template <typename EventType, typename... Args>
ft_event_emitter<EventType, Args...>::ft_event_emitter(ft_event_emitter&& other) noexcept
    : _listeners(ft_nullptr), _capacity(0), _size(0), _mutex(ft_nullptr)
{
    bool other_lock_acquired = false;
    int lock_error = other.lock_internal(&other_lock_acquired);

    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    this->_listeners = other._listeners;
    this->_capacity = other._capacity;
    this->_size = other._size;
    bool other_thread_safe = (other._mutex != ft_nullptr);
    other._listeners = ft_nullptr;
    other._capacity = 0;
    other._size = 0;
    other.unlock_internal(other_lock_acquired);
    other.teardown_thread_safety();
    if (other_thread_safe)
    {
        if (this->enable_thread_safety() != FT_ERR_SUCCESS)
            return ;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

template <typename EventType, typename... Args>
ft_event_emitter<EventType, Args...>& ft_event_emitter<EventType, Args...>::operator=(ft_event_emitter&& other) noexcept
{
    if (this == &other)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESS);
        return (*this);
    }
    bool this_lock_acquired = false;
    int this_lock_error = this->lock_internal(&this_lock_acquired);
    if (this_lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(this_lock_error);
        return (*this);
    }
    bool other_lock_acquired = false;
    int other_lock_error = other.lock_internal(&other_lock_acquired);
    if (other_lock_error != FT_ERR_SUCCESS)
    {
        this->unlock_internal(this_lock_acquired);
        ft_global_error_stack_push(other_lock_error);
        return (*this);
    }
    this->clear_unlocked();
    if (this->_listeners != ft_nullptr)
        cma_free(this->_listeners);
    this->_listeners = other._listeners;
    this->_capacity = other._capacity;
    this->_size = other._size;
    bool other_thread_safe = (other._mutex != ft_nullptr);
    other._listeners = ft_nullptr;
    other._capacity = 0;
    other._size = 0;
    other.unlock_internal(other_lock_acquired);
    other.teardown_thread_safety();
    this->unlock_internal(this_lock_acquired);
    this->teardown_thread_safety();
    if (other_thread_safe)
    {
        if (this->enable_thread_safety() != FT_ERR_SUCCESS)
        {
            ft_global_error_stack_push(FT_ERR_INVALID_STATE);
            return (*this);
        }
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (*this);
}

template <typename EventType, typename... Args>
int ft_event_emitter<EventType, Args...>::enable_thread_safety()
{
    int result;

    result = this->prepare_thread_safety();
    ft_global_error_stack_push(result);
    return (result);
}

template <typename EventType, typename... Args>
void ft_event_emitter<EventType, Args...>::disable_thread_safety()
{
    this->teardown_thread_safety();
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

template <typename EventType, typename... Args>
bool ft_event_emitter<EventType, Args...>::is_thread_safe() const
{
    bool enabled;

    enabled = (this->_mutex != ft_nullptr);
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (enabled);
}

template <typename EventType, typename... Args>
int ft_event_emitter<EventType, Args...>::lock(bool *lock_acquired) const
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

template <typename EventType, typename... Args>
void ft_event_emitter<EventType, Args...>::unlock(bool lock_acquired) const
{
    int unlock_error;

    unlock_error = this->unlock_internal(lock_acquired);
    ft_global_error_stack_push(unlock_error);
    return ;
}

template <typename EventType, typename... Args>
void ft_event_emitter<EventType, Args...>::on(const EventType& event, void (*callback)(Args...))
{
    bool lock_acquired = false;
    int lock_error = this->lock_internal(&lock_acquired);

    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    if (!this->ensure_capacity_unlocked(this->_size + 1))
    {
        this->unlock_internal(lock_acquired);
        return ;
    }
    construct_at(&this->_listeners[this->_size], Listener{event, callback});
    this->_size += 1;
    int unlock_error = this->unlock_internal(lock_acquired);

    if (unlock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(unlock_error);
        return ;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

template <typename EventType, typename... Args>
void ft_event_emitter<EventType, Args...>::emit(const EventType& event, Args... args)
{
    bool lock_acquired = false;
    int lock_error = this->lock_internal(&lock_acquired);

    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    bool found = false;
    size_t listener_index = 0;

    while (listener_index < this->_size)
    {
        if (this->_listeners[listener_index]._event == event)
        {
            found = true;
            this->_listeners[listener_index]._callback(args...);
        }
        listener_index += 1;
    }
    int unlock_error = this->unlock_internal(lock_acquired);

    if (unlock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(unlock_error);
        return ;
    }
    if (!found)
    {
        ft_global_error_stack_push(FT_ERR_NOT_FOUND);
        return ;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

template <typename EventType, typename... Args>
void ft_event_emitter<EventType, Args...>::remove_listener(const EventType& event, void (*callback)(Args...))
{
    bool lock_acquired = false;
    int lock_error = this->lock_internal(&lock_acquired);

    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    size_t listener_index = 0;

    while (listener_index < this->_size)
    {
        if (this->_listeners[listener_index]._event == event &&
            this->_listeners[listener_index]._callback == callback)
        {
            destroy_at(&this->_listeners[listener_index]);
            size_t shift_index = listener_index;

            while (shift_index + 1 < this->_size)
            {
                construct_at(&this->_listeners[shift_index],
                             ft_move(this->_listeners[shift_index + 1]));
                destroy_at(&this->_listeners[shift_index + 1]);
                shift_index += 1;
            }
            this->_size -= 1;
            int unlock_error = this->unlock_internal(lock_acquired);

            if (unlock_error != FT_ERR_SUCCESS)
            {
                ft_global_error_stack_push(unlock_error);
                return ;
            }
            ft_global_error_stack_push(FT_ERR_SUCCESS);
            return ;
        }
        listener_index += 1;
    }
    int unlock_error = this->unlock_internal(lock_acquired);

    if (unlock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(unlock_error);
        return ;
    }
    ft_global_error_stack_push(FT_ERR_NOT_FOUND);
    return ;
}

template <typename EventType, typename... Args>
size_t ft_event_emitter<EventType, Args...>::size() const
{
    bool lock_acquired = false;
    int lock_error = this->lock_internal(&lock_acquired);

    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return (0);
    }
    size_t current_size = this->_size;
    int unlock_error = this->unlock_internal(lock_acquired);

    if (unlock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(unlock_error);
        return (0);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (current_size);
}

template <typename EventType, typename... Args>
bool ft_event_emitter<EventType, Args...>::empty() const
{
    bool lock_acquired = false;
    int lock_error = this->lock_internal(&lock_acquired);

    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return (true);
    }
    bool is_empty = (this->_size == 0);
    int unlock_error = this->unlock_internal(lock_acquired);

    if (unlock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(unlock_error);
        return (true);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (is_empty);
}

template <typename EventType, typename... Args>
void ft_event_emitter<EventType, Args...>::clear()
{
    bool lock_acquired = false;
    int lock_error = this->lock_internal(&lock_acquired);

    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    this->clear_unlocked();
    int unlock_error = this->unlock_internal(lock_acquired);

    if (unlock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(unlock_error);
        return ;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

template <typename EventType, typename... Args>
bool ft_event_emitter<EventType, Args...>::ensure_capacity(size_t desired)
{
    if (desired <= this->_capacity)
        return (true);
    return (this->ensure_capacity_unlocked(desired));
}

template <typename EventType, typename... Args>
bool ft_event_emitter<EventType, Args...>::ensure_capacity_unlocked(size_t desired)
{
    if (desired <= this->_capacity)
        return (true);
    size_t maximum_capacity = FT_SYSTEM_SIZE_MAX / sizeof(Listener);

    if (desired > maximum_capacity)
    {
        ft_global_error_stack_push(FT_ERR_OUT_OF_RANGE);
        return (false);
    }
    if (this->_capacity > maximum_capacity)
        this->_capacity = maximum_capacity;
    size_t new_capacity;

    if (this->_capacity == 0)
        new_capacity = 1;
    else
        new_capacity = this->_capacity;
    while (new_capacity < desired)
    {
        if (new_capacity >= maximum_capacity)
        {
            new_capacity = maximum_capacity;
            break ;
        }
        if (new_capacity > maximum_capacity / 2)
            new_capacity = maximum_capacity;
        else
            new_capacity *= 2;
    }
    Listener* new_data = static_cast<Listener*>(cma_malloc(sizeof(Listener) * new_capacity));

    if (new_data == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return (false);
    }
    size_t listener_index = 0;

    while (listener_index < this->_size)
    {
        construct_at(&new_data[listener_index], ft_move(this->_listeners[listener_index]));
        destroy_at(&this->_listeners[listener_index]);
        listener_index += 1;
    }
    if (this->_listeners != ft_nullptr)
        cma_free(this->_listeners);
    this->_listeners = new_data;
    this->_capacity = new_capacity;
    return (true);
}

template <typename EventType, typename... Args>
void ft_event_emitter<EventType, Args...>::clear_unlocked()
{
    size_t listener_index = 0;

    while (listener_index < this->_size)
    {
        destroy_at(&this->_listeners[listener_index]);
        listener_index += 1;
    }
    this->_size = 0;
    return ;
}

template <typename EventType, typename... Args>
int ft_event_emitter<EventType, Args...>::lock_internal(bool *lock_acquired) const
{
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    int result = pt_recursive_mutex_lock_with_error(*this->_mutex);

    if (result != FT_ERR_SUCCESS)
        return (result);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

template <typename EventType, typename... Args>
int ft_event_emitter<EventType, Args...>::unlock_internal(bool lock_acquired) const
{
    if (!lock_acquired || this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (pt_recursive_mutex_unlock_with_error(*this->_mutex));
}

template <typename EventType, typename... Args>
int ft_event_emitter<EventType, Args...>::prepare_thread_safety()
{
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    pt_recursive_mutex *mutex_pointer = ft_nullptr;
    int creation_result = pt_recursive_mutex_create_with_error(&mutex_pointer);

    if (creation_result != FT_ERR_SUCCESS)
        return (creation_result);
    this->_mutex = mutex_pointer;
    return (FT_ERR_SUCCESS);
}

template <typename EventType, typename... Args>
void ft_event_emitter<EventType, Args...>::teardown_thread_safety()
{
    pt_recursive_mutex_destroy(&this->_mutex);
    return ;
}

#ifdef LIBFT_TEST_BUILD
template <typename EventType, typename... Args>
pt_recursive_mutex* ft_event_emitter<EventType, Args...>::get_mutex_for_validation() const noexcept
{
    return (this->_mutex);
}
#endif

template <typename EventType, typename... Args>
struct ft_event_emitter_test_helper
{
    static bool ensure_capacity(ft_event_emitter<EventType, Args...> &emitter, size_t desired)
    {
        bool lock_acquired = false;
        int lock_error = emitter.lock_internal(&lock_acquired);

        if (lock_error != FT_ERR_SUCCESS)
        {
            ft_global_error_stack_push(lock_error);
            return (false);
        }
        bool result = emitter.ensure_capacity_unlocked(desired);
        int unlock_error = emitter.unlock_internal(lock_acquired);

        if (unlock_error != FT_ERR_SUCCESS)
        {
            ft_global_error_stack_push(unlock_error);
            return (false);
        }
        return (result);
    }
};

#endif
