#ifndef FT_EVENT_EMITTER_HPP
#define FT_EVENT_EMITTER_HPP

#include "constructor.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/mutex.hpp"
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

        Listener*   _listeners;
        size_t      _capacity;
        size_t      _size;
        mutable int _error_code;
        mutable pt_mutex _mutex;

        void    set_error(int error) const;
        bool    ensure_capacity(size_t desired);

    public:
        ft_event_emitter(size_t initial_capacity = 0);
        ~ft_event_emitter();

        ft_event_emitter(const ft_event_emitter&) = delete;
        ft_event_emitter& operator=(const ft_event_emitter&) = delete;

        ft_event_emitter(ft_event_emitter&& other) noexcept;
        ft_event_emitter& operator=(ft_event_emitter&& other) noexcept;

        void on(const EventType& event, void (*callback)(Args...));
        void emit(const EventType& event, Args... args);
        void remove_listener(const EventType& event, void (*callback)(Args...));
        size_t size() const;
        bool empty() const;
        int get_error() const;
        const char* get_error_str() const;
        void clear();
};

template <typename EventType, typename... Args>
ft_event_emitter<EventType, Args...>::ft_event_emitter(size_t initial_capacity)
    : _listeners(ft_nullptr), _capacity(0), _size(0), _error_code(ER_SUCCESS)
{
    if (initial_capacity > 0)
    {
        this->_listeners = static_cast<Listener*>(cma_malloc(sizeof(Listener) * initial_capacity));
        if (this->_listeners == ft_nullptr)
            this->set_error(EVENT_EMITTER_ALLOC_FAIL);
        else
            this->_capacity = initial_capacity;
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
      _error_code(other._error_code)
{
    other._listeners = ft_nullptr;
    other._capacity = 0;
    other._size = 0;
    other._error_code = ER_SUCCESS;
    return ;
}

template <typename EventType, typename... Args>
ft_event_emitter<EventType, Args...>& ft_event_emitter<EventType, Args...>::operator=(ft_event_emitter&& other) noexcept
{
    if (this != &other)
    {
        if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
            return (*this);
        if (other._mutex.lock(THREAD_ID) != FT_SUCCESS)
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
        this->_error_code = other._error_code;
        other._listeners = ft_nullptr;
        other._capacity = 0;
        other._size = 0;
        other._error_code = ER_SUCCESS;
        other._mutex.unlock(THREAD_ID);
        this->_mutex.unlock(THREAD_ID);
    }
    return (*this);
}

template <typename EventType, typename... Args>
void ft_event_emitter<EventType, Args...>::set_error(int error) const
{
    this->_error_code = error;
    ft_errno = error;
    return ;
}

template <typename EventType, typename... Args>
bool ft_event_emitter<EventType, Args...>::ensure_capacity(size_t desired)
{
    if (desired <= this->_capacity)
        return (true);
    size_t new_capacity;
    if (this->_capacity == 0)
        new_capacity = 1;
    else
        new_capacity = this->_capacity * 2;
    while (new_capacity < desired)
        new_capacity *= 2;
    Listener* new_data = static_cast<Listener*>(cma_malloc(sizeof(Listener) * new_capacity));
    if (new_data == ft_nullptr)
    {
        this->set_error(EVENT_EMITTER_ALLOC_FAIL);
        return (false);
    }
    size_t listener_index = 0;
    while (listener_index < this->_size)
    {
        construct_at(&new_data[listener_index], ft_move(this->_listeners[listener_index]));
        destroy_at(&this->_listeners[listener_index]);
        ++listener_index;
    }
    if (this->_listeners != ft_nullptr)
        cma_free(this->_listeners);
    this->_listeners = new_data;
    this->_capacity = new_capacity;
    return (true);
}

template <typename EventType, typename... Args>
void ft_event_emitter<EventType, Args...>::on(const EventType& event, void (*callback)(Args...))
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
        return ;
    if (!this->ensure_capacity(this->_size + 1))
    {
        this->_mutex.unlock(THREAD_ID);
        return ;
    }
    construct_at(&this->_listeners[this->_size], Listener{event, callback});
    ++this->_size;
    this->_mutex.unlock(THREAD_ID);
    return ;
}

template <typename EventType, typename... Args>
void ft_event_emitter<EventType, Args...>::emit(const EventType& event, Args... args)
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
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
        this->set_error(EVENT_EMITTER_NOT_FOUND);
    this->_mutex.unlock(THREAD_ID);
    return ;
}

template <typename EventType, typename... Args>
void ft_event_emitter<EventType, Args...>::remove_listener(const EventType& event, void (*callback)(Args...))
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
        return ;
    size_t listener_index = 0;
    while (listener_index < this->_size)
    {
        if (this->_listeners[listener_index]._event == event && this->_listeners[listener_index]._callback == callback)
        {
            destroy_at(&this->_listeners[listener_index]);
            size_t shift_index = listener_index;
            while (shift_index + 1 < this->_size)
            {
                construct_at(&this->_listeners[shift_index], ft_move(this->_listeners[shift_index + 1]));
                destroy_at(&this->_listeners[shift_index + 1]);
                ++shift_index;
            }
            --this->_size;
            this->_mutex.unlock(THREAD_ID);
            return ;
        }
        ++listener_index;
    }
    this->set_error(EVENT_EMITTER_NOT_FOUND);
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
    return (this->_error_code);
}

template <typename EventType, typename... Args>
const char* ft_event_emitter<EventType, Args...>::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}

template <typename EventType, typename... Args>
void ft_event_emitter<EventType, Args...>::clear()
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
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
