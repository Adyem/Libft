#ifndef FT_OPTIONAL_HPP
#define FT_OPTIONAL_HPP

#include "constructor.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"
#include "../PThread/pthread.hpp"
#include <utility>
#include <type_traits>
#include <new>

#include "move.hpp"
template <typename T>
class ft_optional
{
    private:
        T*          _value;
        mutable int _error_code;
        mutable pt_mutex _mutex;

        void set_error(int error) const;
        static T &fallback_reference() noexcept;
        static void sleep_backoff() noexcept;
        static void restore_errno(ft_unique_lock<pt_mutex> &guard,
            int entry_errno) noexcept;
        int lock_self(ft_unique_lock<pt_mutex> &guard) const noexcept;
        static int lock_pair(ft_optional &first, ft_optional &second,
            ft_unique_lock<pt_mutex> &first_guard,
            ft_unique_lock<pt_mutex> &second_guard) noexcept;

    public:
        ft_optional();
        ft_optional(const T& value);
        ft_optional(T&& value);
        ~ft_optional();

        ft_optional(const ft_optional&) = delete;
        ft_optional& operator=(const ft_optional&) = delete;

        ft_optional(ft_optional&& other) noexcept;
        ft_optional& operator=(ft_optional&& other) noexcept;

        bool has_value() const;
        T& value();
        const T& value() const;
        void reset();

        int get_error() const;
        const char* get_error_str() const;
};

template <typename T>
ft_optional<T>::ft_optional()
    : _value(ft_nullptr), _error_code(FT_ER_SUCCESSS), _mutex()
{
    this->set_error(FT_ER_SUCCESSS);
    return ;
}

template <typename T>
ft_optional<T>::ft_optional(const T& value)
    : _value(ft_nullptr), _error_code(FT_ER_SUCCESSS), _mutex()
{
    _value = static_cast<T*>(cma_malloc(sizeof(T)));
    if (_value == ft_nullptr)
        this->set_error(FT_ERR_NO_MEMORY);
    else
    {
        construct_at(_value, value);
        this->set_error(FT_ER_SUCCESSS);
    }
    return ;
}

template <typename T>
ft_optional<T>::ft_optional(T&& value)
    : _value(ft_nullptr), _error_code(FT_ER_SUCCESSS), _mutex()
{
    _value = static_cast<T*>(cma_malloc(sizeof(T)));
    if (_value == ft_nullptr)
        this->set_error(FT_ERR_NO_MEMORY);
    else
    {
        construct_at(_value, ft_move(value));
        this->set_error(FT_ER_SUCCESSS);
    }
    return ;
}

template <typename T>
ft_optional<T>::~ft_optional()
{
    this->reset();
    return ;
}

template <typename T>
ft_optional<T>::ft_optional(ft_optional&& other) noexcept
    : _value(ft_nullptr), _error_code(FT_ER_SUCCESSS), _mutex()
{
    int entry_errno;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    entry_errno = ft_errno;
    lock_error = other.lock_self(other_guard);
    if (lock_error != FT_ER_SUCCESSS)
    {
        this->_value = ft_nullptr;
        this->_error_code = lock_error;
        this->set_error(lock_error);
        ft_optional<T>::restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_value = other._value;
    this->_error_code = other._error_code;
    other._value = ft_nullptr;
    other._error_code = FT_ER_SUCCESSS;
    this->set_error(this->_error_code);
    other.set_error(FT_ER_SUCCESSS);
    ft_optional<T>::restore_errno(other_guard, entry_errno);
    return ;
}

template <typename T>
ft_optional<T>& ft_optional<T>::operator=(ft_optional&& other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
    {
        this->set_error(this->_error_code);
        return (*this);
    }
    entry_errno = ft_errno;
    lock_error = ft_optional<T>::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ER_SUCCESSS)
    {
        this->set_error(lock_error);
        ft_optional<T>::restore_errno(this_guard, entry_errno);
        ft_optional<T>::restore_errno(other_guard, entry_errno);
        return (*this);
    }
    if (this->_value != ft_nullptr)
    {
        destroy_at(this->_value);
        cma_free(this->_value);
        this->_value = ft_nullptr;
    }
    this->_value = other._value;
    this->_error_code = other._error_code;
    other._value = ft_nullptr;
    other._error_code = FT_ER_SUCCESSS;
    this->set_error(this->_error_code);
    other.set_error(FT_ER_SUCCESSS);
    ft_optional<T>::restore_errno(this_guard, entry_errno);
    ft_optional<T>::restore_errno(other_guard, entry_errno);
    return (*this);
}

template <typename T>
void ft_optional<T>::set_error(int error) const
{
    this->_error_code = error;
    ft_errno = error;
    return ;
}

template <typename T>
bool ft_optional<T>::has_value() const
{
    int entry_errno;
    ft_unique_lock<pt_mutex> guard;
    int lock_error;
    bool result;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != FT_ER_SUCCESSS)
    {
        const_cast<ft_optional*>(this)->set_error(lock_error);
        ft_optional<T>::restore_errno(guard, entry_errno);
        return (false);
    }
    result = (this->_value != ft_nullptr);
    const_cast<ft_optional*>(this)->set_error(FT_ER_SUCCESSS);
    ft_optional<T>::restore_errno(guard, entry_errno);
    return (result);
}

template <typename T>
T& ft_optional<T>::value()
{
    int entry_errno;
    ft_unique_lock<pt_mutex> guard;
    int lock_error;
    T *reference_pointer;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != FT_ER_SUCCESSS)
    {
        this->set_error(lock_error);
        ft_optional<T>::restore_errno(guard, entry_errno);
        return (ft_optional<T>::fallback_reference());
    }
    if (this->_value == ft_nullptr)
    {
        this->set_error(FT_ERR_EMPTY);
        ft_optional<T>::restore_errno(guard, entry_errno);
        return (ft_optional<T>::fallback_reference());
    }
    reference_pointer = this->_value;
    this->set_error(FT_ER_SUCCESSS);
    ft_optional<T>::restore_errno(guard, entry_errno);
    return (*reference_pointer);
}

template <typename T>
const T& ft_optional<T>::value() const
{
    int entry_errno;
    ft_unique_lock<pt_mutex> guard;
    int lock_error;
    const T *reference_pointer;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != FT_ER_SUCCESSS)
    {
        const_cast<ft_optional*>(this)->set_error(lock_error);
        ft_optional<T>::restore_errno(guard, entry_errno);
        return (ft_optional<T>::fallback_reference());
    }
    if (this->_value == ft_nullptr)
    {
        const_cast<ft_optional*>(this)->set_error(FT_ERR_EMPTY);
        ft_optional<T>::restore_errno(guard, entry_errno);
        return (ft_optional<T>::fallback_reference());
    }
    reference_pointer = this->_value;
    const_cast<ft_optional*>(this)->set_error(FT_ER_SUCCESSS);
    ft_optional<T>::restore_errno(guard, entry_errno);
    return (*reference_pointer);
}

template <typename T>
T &ft_optional<T>::fallback_reference() noexcept
{
    if constexpr (std::is_default_constructible_v<T>)
    {
        static T default_instance = T();
        return (default_instance);
    }
    else
    {
        alignas(T) static unsigned char storage[sizeof(T)] = {0};
        return (*reinterpret_cast<T*>(storage));
    }
}

template <typename T>
void ft_optional<T>::sleep_backoff() noexcept
{
    pt_thread_sleep(1);
    return ;
}

template <typename T>
void ft_optional<T>::restore_errno(ft_unique_lock<pt_mutex> &guard,
    int entry_errno) noexcept
{
    if (guard.owns_lock())
        guard.unlock();
    ft_errno = entry_errno;
    return ;
}

template <typename T>
int ft_optional<T>::lock_self(ft_unique_lock<pt_mutex> &guard) const noexcept
{
    guard = ft_unique_lock<pt_mutex>(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        ft_errno = guard.get_error();
        return (guard.get_error());
    }
    ft_errno = FT_ER_SUCCESSS;
    return (FT_ER_SUCCESSS);
}

template <typename T>
int ft_optional<T>::lock_pair(ft_optional &first, ft_optional &second,
    ft_unique_lock<pt_mutex> &first_guard,
    ft_unique_lock<pt_mutex> &second_guard) noexcept
{
    ft_optional *ordered_first;
    ft_optional *ordered_second;
    bool swapped;

    if (&first == &second)
    {
        ft_unique_lock<pt_mutex> single_guard(first._mutex);

        if (single_guard.get_error() != FT_ER_SUCCESSS)
        {
            ft_errno = single_guard.get_error();
            return (single_guard.get_error());
        }
        first_guard = ft_move(single_guard);
        second_guard = ft_unique_lock<pt_mutex>();
        ft_errno = FT_ER_SUCCESSS;
        return (FT_ER_SUCCESSS);
    }
    ordered_first = &first;
    ordered_second = &second;
    swapped = false;
    if (ordered_first > ordered_second)
    {
        ft_optional *temporary;

        temporary = ordered_first;
        ordered_first = ordered_second;
        ordered_second = temporary;
        swapped = true;
    }
    while (true)
    {
        ft_unique_lock<pt_mutex> lower_guard(ordered_first->_mutex);

        if (lower_guard.get_error() != FT_ER_SUCCESSS)
        {
            ft_errno = lower_guard.get_error();
            return (lower_guard.get_error());
        }
        ft_unique_lock<pt_mutex> upper_guard(ordered_second->_mutex);
        if (upper_guard.get_error() == FT_ER_SUCCESSS)
        {
            if (!swapped)
            {
                first_guard = ft_move(lower_guard);
                second_guard = ft_move(upper_guard);
            }
            else
            {
                first_guard = ft_move(upper_guard);
                second_guard = ft_move(lower_guard);
            }
            ft_errno = FT_ER_SUCCESSS;
            return (FT_ER_SUCCESSS);
        }
        if (upper_guard.get_error() != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            ft_errno = upper_guard.get_error();
            return (upper_guard.get_error());
        }
        if (lower_guard.owns_lock())
            lower_guard.unlock();
        ft_optional<T>::sleep_backoff();
    }
}

template <typename T>
void ft_optional<T>::reset()
{
    int entry_errno;
    ft_unique_lock<pt_mutex> guard;
    int lock_error;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != FT_ER_SUCCESSS)
    {
        this->set_error(lock_error);
        ft_optional<T>::restore_errno(guard, entry_errno);
        return ;
    }
    if (this->_value != ft_nullptr)
    {
        destroy_at(this->_value);
        cma_free(this->_value);
        this->_value = ft_nullptr;
    }
    this->set_error(FT_ER_SUCCESSS);
    ft_optional<T>::restore_errno(guard, entry_errno);
    return ;
}

template <typename T>
int ft_optional<T>::get_error() const
{
    int entry_errno;
    ft_unique_lock<pt_mutex> guard;
    int lock_error;
    int error_code;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != FT_ER_SUCCESSS)
    {
        const_cast<ft_optional*>(this)->set_error(lock_error);
        ft_optional<T>::restore_errno(guard, entry_errno);
        return (lock_error);
    }
    error_code = this->_error_code;
    const_cast<ft_optional*>(this)->set_error(error_code);
    ft_optional<T>::restore_errno(guard, entry_errno);
    return (error_code);
}

template <typename T>
const char* ft_optional<T>::get_error_str() const
{
    int entry_errno;
    ft_unique_lock<pt_mutex> guard;
    int lock_error;
    int error_code;
    const char *result;

    entry_errno = ft_errno;
    lock_error = this->lock_self(guard);
    if (lock_error != FT_ER_SUCCESSS)
    {
        const_cast<ft_optional*>(this)->set_error(lock_error);
        ft_optional<T>::restore_errno(guard, entry_errno);
        return (ft_strerror(lock_error));
    }
    error_code = this->_error_code;
    result = ft_strerror(error_code);
    const_cast<ft_optional*>(this)->set_error(error_code);
    ft_optional<T>::restore_errno(guard, entry_errno);
    return (result);
}

#endif 
