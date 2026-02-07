#ifndef FT_OPTIONAL_HPP
#define FT_OPTIONAL_HPP

#include "constructor.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread.hpp"
#include "../PThread/pthread_internal.hpp"
#include <type_traits>
#include "move.hpp"

template <typename T>
class ft_optional
{
    private:
        T*                      _value;
        mutable pt_recursive_mutex* _mutex;

        static T &fallback_reference() noexcept;
        static void sleep_backoff() noexcept;
        int  lock_mutex() const noexcept;
        int  unlock_mutex() const noexcept;
        int  prepare_thread_safety() noexcept;
        void teardown_thread_safety() noexcept;
        static int lock_pair(ft_optional &first, ft_optional &second,
                const ft_optional *&lower,
                const ft_optional *&upper) noexcept;
        static int unlock_pair(const ft_optional *lower,
                const ft_optional *upper) noexcept;

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
        int enable_thread_safety() noexcept;
        void disable_thread_safety() noexcept;

    #ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_validation() const noexcept;
    #endif
};

template <typename T>
ft_optional<T>::ft_optional()
    : _value(ft_nullptr), _mutex(ft_nullptr)
{
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

template <typename T>
ft_optional<T>::ft_optional(const T& value)
    : _value(ft_nullptr), _mutex(ft_nullptr)
{
    this->_value = static_cast<T*>(cma_malloc(sizeof(T)));
    if (this->_value == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return ;
    }
    construct_at(this->_value, value);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

template <typename T>
ft_optional<T>::ft_optional(T&& value)
    : _value(ft_nullptr), _mutex(ft_nullptr)
{
    this->_value = static_cast<T*>(cma_malloc(sizeof(T)));
    if (this->_value == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return ;
    }
    construct_at(this->_value, ft_move(value));
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

template <typename T>
ft_optional<T>::~ft_optional()
{
    this->reset();
    this->disable_thread_safety();
    return ;
}

template <typename T>
ft_optional<T>::ft_optional(ft_optional&& other) noexcept
    : _value(ft_nullptr), _mutex(ft_nullptr)
{
    const ft_optional *lower = ft_nullptr;
    const ft_optional *upper = ft_nullptr;
    int lock_error = ft_optional::lock_pair(*this, other, lower, upper);

    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    this->_value = other._value;
    other._value = ft_nullptr;
    int unlock_error = ft_optional::unlock_pair(lower, upper);

    if (unlock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(unlock_error);
        return ;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

template <typename T>
ft_optional<T>& ft_optional<T>::operator=(ft_optional&& other) noexcept
{
    if (this == &other)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (*this);
    }
    const ft_optional *lower = ft_nullptr;
    const ft_optional *upper = ft_nullptr;
    int lock_error = ft_optional::lock_pair(*this, other, lower, upper);

    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (*this);
    }
    if (this->_value != ft_nullptr)
    {
        destroy_at(this->_value);
        cma_free(this->_value);
        this->_value = ft_nullptr;
    }
    this->_value = other._value;
    other._value = ft_nullptr;
    int unlock_error = ft_optional::unlock_pair(lower, upper);

    if (unlock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(unlock_error);
        return (*this);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (*this);
}

template <typename T>
bool ft_optional<T>::has_value() const
{
    int lock_error = this->lock_mutex();
    bool result = false;

    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (false);
    }
    result = (this->_value != ft_nullptr);
    int unlock_error = this->unlock_mutex();

    if (unlock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(unlock_error);
        return (result);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}

template <typename T>
T& ft_optional<T>::value()
{
    int lock_error = this->lock_mutex();

    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (ft_optional<T>::fallback_reference());
    }
    if (this->_value == ft_nullptr)
    {
        int unlock_error = this->unlock_mutex();

        if (unlock_error != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(unlock_error);
            return (ft_optional<T>::fallback_reference());
        }
        ft_global_error_stack_push(FT_ERR_EMPTY);
        return (ft_optional<T>::fallback_reference());
    }
    T &reference = *(this->_value);
    int unlock_error = this->unlock_mutex();

    if (unlock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(unlock_error);
        return (ft_optional<T>::fallback_reference());
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (reference);
}

template <typename T>
const T& ft_optional<T>::value() const
{
    int lock_error = this->lock_mutex();

    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (ft_optional<T>::fallback_reference());
    }
    if (this->_value == ft_nullptr)
    {
        int unlock_error = this->unlock_mutex();

        if (unlock_error != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(unlock_error);
            return (ft_optional<T>::fallback_reference());
        }
        ft_global_error_stack_push(FT_ERR_EMPTY);
        return (ft_optional<T>::fallback_reference());
    }
    const T &reference = *(this->_value);
    int unlock_error = this->unlock_mutex();

    if (unlock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(unlock_error);
        return (ft_optional<T>::fallback_reference());
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (reference);
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
int ft_optional<T>::lock_mutex() const noexcept
{
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESSS);
    return (pt_recursive_mutex_lock_with_error(*this->_mutex));
}

template <typename T>
int ft_optional<T>::unlock_mutex() const noexcept
{
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESSS);
    return (pt_recursive_mutex_unlock_with_error(*this->_mutex));
}

template <typename T>
int ft_optional<T>::prepare_thread_safety() noexcept
{
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESSS);
    int result = pt_recursive_mutex_create_with_error(&this->_mutex);
    if (result != FT_ERR_SUCCESSS && this->_mutex != ft_nullptr)
        pt_recursive_mutex_destroy(&this->_mutex);
    return (result);
}

template <typename T>
void ft_optional<T>::teardown_thread_safety() noexcept
{
    pt_recursive_mutex_destroy(&this->_mutex);
}

template <typename T>
int ft_optional<T>::enable_thread_safety() noexcept
{
    int result = this->prepare_thread_safety();
    ft_global_error_stack_push(result);
    return (result);
}

template <typename T>
void ft_optional<T>::disable_thread_safety() noexcept
{
    this->teardown_thread_safety();
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

template <typename T>
int ft_optional<T>::lock_pair(ft_optional &first, ft_optional &second,
    const ft_optional *&lower,
    const ft_optional *&upper) noexcept
{
    lower = &first;
    upper = &second;
    if (&first == &second)
        return (first.lock_mutex());
    if (lower > upper)
    {
        const ft_optional *temporary = lower;

        lower = upper;
        upper = temporary;
    }
    while (true)
    {
        int lower_error = lower->lock_mutex();

        if (lower_error != FT_ERR_SUCCESSS)
            return (lower_error);
        int upper_error = upper->lock_mutex();

        if (upper_error == FT_ERR_SUCCESSS)
            return (FT_ERR_SUCCESSS);
        if (upper_error != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            lower->unlock_mutex();
            return (upper_error);
        }
        lower->unlock_mutex();
        ft_optional<T>::sleep_backoff();
    }
}

template <typename T>
int ft_optional<T>::unlock_pair(const ft_optional *lower, const ft_optional *upper) noexcept
{
    int error;
    int final_error = FT_ERR_SUCCESSS;

    if (upper != ft_nullptr)
    {
        error = upper->unlock_mutex();
        if (error != FT_ERR_SUCCESSS)
            final_error = error;
    }
    if (lower != ft_nullptr && lower != upper)
    {
        error = lower->unlock_mutex();
        if (error != FT_ERR_SUCCESSS && final_error == FT_ERR_SUCCESSS)
            final_error = error;
    }
    return (final_error);
}

template <typename T>
void ft_optional<T>::reset()
{
    int lock_error = this->lock_mutex();

    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    if (this->_value != ft_nullptr)
    {
        destroy_at(this->_value);
        cma_free(this->_value);
        this->_value = ft_nullptr;
    }
    int unlock_error = this->unlock_mutex();

    if (unlock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(unlock_error);
        return ;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

#ifdef LIBFT_TEST_BUILD
template <typename T>
pt_recursive_mutex *ft_optional<T>::get_mutex_for_validation() const noexcept
{
    return (this->_mutex);
}
#endif

#endif
