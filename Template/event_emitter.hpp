#ifndef FT_EVENT_EMITTER_HPP
#define FT_EVENT_EMITTER_HPP

#include "constructor.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/nullptr.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/mutex.hpp"
#include <cstddef>
#include <utility>

/*
 * ft_event_emitter
 * Observer pattern utility allowing registration of callbacks
 * for specific events and emitting those events with arguments.
 */

template <typename EventType, typename... Args>
class ft_event_emitter
{
    private:
        struct Listener
        {
            EventType _event;
            void (*_callback)(Args...);
        };

        Listener*   _listeners;
        size_t      _capacity;
        size_t      _size;
        mutable int _errorCode;
        mutable pt_mutex _mutex;

        void    setError(int error) const;
        bool    ensure_capacity(size_t desired);

    public:
        ft_event_emitter(size_t initialCapacity = 0);
        ~ft_event_emitter();

        ft_event_emitter(const ft_event_emitter&) = delete;
        ft_event_emitter& operator=(const ft_event_emitter&) = delete;

        ft_event_emitter(ft_event_emitter&& other) noexcept;
        ft_event_emitter& operator=(ft_event_emitter&& other) noexcept;

        void on(const EventType& event, void (*cb)(Args...));
        void emit(const EventType& event, Args... args);
        void remove_listener(const EventType& event, void (*cb)(Args...));
        size_t size() const;
        bool empty() const;
        int get_error() const;
        const char* get_error_str() const;
        void clear();
};

template <typename EventType, typename... Args>
ft_event_emitter<EventType, Args...>::ft_event_emitter(size_t initialCapacity)
    : _listeners(ft_nullptr), _capacity(0), _size(0), _errorCode(ER_SUCCESS)
{
    if (initialCapacity > 0)
    {
        this->_listeners = static_cast<Listener*>(cma_malloc(sizeof(Listener) * initialCapacity));
        if (this->_listeners == ft_nullptr)
            this->setError(EVENT_EMITTER_ALLOC_FAIL);
        else
            this->_capacity = initialCapacity;
    }
    return ;
}

template <typename EventType, typename... Args>
ft_event_emitter<EventType, Args...>::~ft_event_emitter()
{
    this->clear();
    if (this->_listeners != ft_nullptr)
        cma_free(this->_listeners);
    return ;
}

template <typename EventType, typename... Args>
ft_event_emitter<EventType, Args...>::ft_event_emitter(ft_event_emitter&& other) noexcept
    : _listeners(other._listeners), _capacity(other._capacity), _size(other._size),
      _errorCode(other._errorCode)
{
    other._listeners = ft_nullptr;
    other._capacity = 0;
    other._size = 0;
    other._errorCode = ER_SUCCESS;
    return ;
}

template <typename EventType, typename... Args>
ft_event_emitter<EventType, Args...>& ft_event_emitter<EventType, Args...>::operator=(ft_event_emitter&& other) noexcept
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
        if (this->_listeners != ft_nullptr)
            cma_free(this->_listeners);
        this->_listeners = other._listeners;
        this->_capacity = other._capacity;
        this->_size = other._size;
        this->_errorCode = other._errorCode;
        other._listeners = ft_nullptr;
        other._capacity = 0;
        other._size = 0;
        other._errorCode = ER_SUCCESS;
        other._mutex.unlock(THREAD_ID);
        this->_mutex.unlock(THREAD_ID);
    }
    return (*this);
}

template <typename EventType, typename... Args>
void ft_event_emitter<EventType, Args...>::setError(int error) const
{
    this->_errorCode = error;
    ft_errno = error;
    return ;
}

template <typename EventType, typename... Args>
bool ft_event_emitter<EventType, Args...>::ensure_capacity(size_t desired)
{
    if (desired <= this->_capacity)
        return (true);
    size_t newCap = (this->_capacity == 0) ? 1 : this->_capacity * 2;
    while (newCap < desired)
        newCap *= 2;
    Listener* newData = static_cast<Listener*>(cma_malloc(sizeof(Listener) * newCap));
    if (newData == ft_nullptr)
    {
        this->setError(EVENT_EMITTER_ALLOC_FAIL);
        return (false);
    }
    size_t listener_index = 0;
    while (listener_index < this->_size)
    {
        construct_at(&newData[listener_index], std::move(this->_listeners[listener_index]));
        destroy_at(&this->_listeners[listener_index]);
        ++listener_index;
    }
    if (this->_listeners != ft_nullptr)
        cma_free(this->_listeners);
    this->_listeners = newData;
    this->_capacity = newCap;
    return (true);
}

template <typename EventType, typename... Args>
void ft_event_emitter<EventType, Args...>::on(const EventType& event, void (*cb)(Args...))
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return ;
    if (!this->ensure_capacity(this->_size + 1))
    {
        this->_mutex.unlock(THREAD_ID);
        return ;
    }
    construct_at(&this->_listeners[this->_size], Listener{event, cb});
    ++this->_size;
    this->_mutex.unlock(THREAD_ID);
    return ;
}

template <typename EventType, typename... Args>
void ft_event_emitter<EventType, Args...>::emit(const EventType& event, Args... args)
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return ;
    bool found = false;
    size_t listener_index = 0;
    while (listener_index < this->_size)
    {
        if (this->_listeners[listener_index]._event == event)
        {
            found = true;
            this->_listeners[listener_index]._callback(args...);
        }
        ++listener_index;
    }
    if (!found)
        this->setError(EVENT_EMITTER_NOT_FOUND);
    this->_mutex.unlock(THREAD_ID);
    return ;
}

template <typename EventType, typename... Args>
void ft_event_emitter<EventType, Args...>::remove_listener(const EventType& event, void (*cb)(Args...))
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return ;
    size_t listener_index = 0;
    while (listener_index < this->_size)
    {
        if (this->_listeners[listener_index]._event == event && this->_listeners[listener_index]._callback == cb)
        {
            destroy_at(&this->_listeners[listener_index]);
            size_t shift_index = listener_index;
            while (shift_index + 1 < this->_size)
            {
                construct_at(&this->_listeners[shift_index], std::move(this->_listeners[shift_index + 1]));
                destroy_at(&this->_listeners[shift_index + 1]);
                ++shift_index;
            }
            --this->_size;
            this->_mutex.unlock(THREAD_ID);
            return ;
        }
        ++listener_index;
    }
    this->setError(EVENT_EMITTER_NOT_FOUND);
    this->_mutex.unlock(THREAD_ID);
    return ;
}

template <typename EventType, typename... Args>
size_t ft_event_emitter<EventType, Args...>::size() const
{
    return (this->_size);
}

template <typename EventType, typename... Args>
bool ft_event_emitter<EventType, Args...>::empty() const
{
    return (this->_size == 0);
}

template <typename EventType, typename... Args>
int ft_event_emitter<EventType, Args...>::get_error() const
{
    return (this->_errorCode);
}

template <typename EventType, typename... Args>
const char* ft_event_emitter<EventType, Args...>::get_error_str() const
{
    return (ft_strerror(this->_errorCode));
}

template <typename EventType, typename... Args>
void ft_event_emitter<EventType, Args...>::clear()
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return ;
    size_t listener_index = 0;
    while (listener_index < this->_size)
    {
        destroy_at(&this->_listeners[listener_index]);
        ++listener_index;
    }
    this->_size = 0;
    this->_mutex.unlock(THREAD_ID);
    return ;
}

#endif
