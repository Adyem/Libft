#ifndef FT_TUPLE_HPP
#define FT_TUPLE_HPP

#include "constructor.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/basic.hpp"
#include <tuple>
#include <utility>
#include <new>
#include <type_traits>
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread.hpp"

template <typename... Types>
class ft_tuple
{
    private:
        using tuple_t = std::tuple<Types...>;

        tuple_t*                _data;
        mutable pt_recursive_mutex* _mutex;

        void destroy_locked();
        int  lock_internal(bool *lock_acquired) const;
        int  unlock_internal(bool lock_acquired) const;
        int  prepare_thread_safety();
        void teardown_thread_safety();

    public:
        ft_tuple();
        ~ft_tuple();

        ft_tuple(const ft_tuple&) = delete;
        ft_tuple& operator=(const ft_tuple&) = delete;

        ft_tuple(ft_tuple&& other) noexcept;
        ft_tuple& operator=(ft_tuple&& other) noexcept;

        template <typename... Args>
        explicit ft_tuple(Args&&... args);

        template <std::size_t I>
        typename std::tuple_element<I, tuple_t>::type& get();

        template <std::size_t I>
        const typename std::tuple_element<I, tuple_t>::type& get() const;

        template <typename T>
        T& get();

        template <typename T>
        const T& get() const;

        void reset();

        int enable_thread_safety();
        void disable_thread_safety();
        bool is_thread_safe() const;
        int lock(bool *lock_acquired) const;
        void unlock(bool lock_acquired) const;

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex* get_mutex_for_validation() const noexcept;
#endif
};

template <typename... Types>
ft_tuple<Types...>::ft_tuple()
    : _data(ft_nullptr), _mutex(ft_nullptr)
{
    ft_global_error_stack_push(FT_ERR_SUCCESS);
}

template <typename... Types>
ft_tuple<Types...>::~ft_tuple()
{
    bool lock_acquired = false;
    int lock_result = this->lock_internal(&lock_acquired);

    if (lock_result == FT_ERR_SUCCESS)
    {
        this->destroy_locked();
        this->unlock_internal(lock_acquired);
    }
    this->teardown_thread_safety();
    ft_global_error_stack_push(FT_ERR_SUCCESS);
}

template <typename... Types>
ft_tuple<Types...>::ft_tuple(ft_tuple&& other) noexcept
    : _data(ft_nullptr), _mutex(ft_nullptr)
{
    bool other_lock_acquired = false;
    int lock_result = other.lock_internal(&other_lock_acquired);

    if (lock_result != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_result);
        return ;
    }
    this->_data = other._data;
    bool other_thread_safe = (other._mutex != ft_nullptr);
    other._data = ft_nullptr;
    other.unlock_internal(other_lock_acquired);
    if (other_thread_safe)
    {
        if (this->enable_thread_safety() != FT_ERR_SUCCESS)
            return ;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
}

template <typename... Types>
ft_tuple<Types...>& ft_tuple<Types...>::operator=(ft_tuple&& other) noexcept
{
    if (this == &other)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESS);
        return (*this);
    }
    bool this_lock_acquired = false;
    int lock_result = this->lock_internal(&this_lock_acquired);

    if (lock_result != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_result);
        return (*this);
    }
    bool other_lock_acquired = false;
    lock_result = other.lock_internal(&other_lock_acquired);

    if (lock_result != FT_ERR_SUCCESS)
    {
        this->unlock_internal(this_lock_acquired);
        ft_global_error_stack_push(lock_result);
        return (*this);
    }
    this->destroy_locked();
    this->_data = other._data;
    other._data = ft_nullptr;
    other.unlock_internal(other_lock_acquired);
    this->unlock_internal(this_lock_acquired);
    this->teardown_thread_safety();
    if (other._mutex != ft_nullptr)
    {
        if (this->enable_thread_safety() != FT_ERR_SUCCESS)
            return (*this);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (*this);
}

template <typename... Types>
template <typename... Args>
ft_tuple<Types...>::ft_tuple(Args&&... args)
    : _data(ft_nullptr), _mutex(ft_nullptr)
{
    this->_data = static_cast<tuple_t*>(cma_malloc(sizeof(tuple_t)));
    if (this->_data == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return ;
    }
    construct_at(this->_data, std::forward<Args>(args)...);
    ft_global_error_stack_push(FT_ERR_SUCCESS);
}

static inline void* tuple_default_buffer(size_t size)
{
    static char buffer[sizeof(void*) * 2];
    (void)size;
    return (buffer);
}

template <typename... Types>
template <std::size_t I>
typename std::tuple_element<I, typename ft_tuple<Types...>::tuple_t>::type&
ft_tuple<Types...>::get()
{
    using elem_t = typename std::tuple_element<I, tuple_t>::type;
    bool lock_acquired = false;
    int lock_result = this->lock_internal(&lock_acquired);

    if (lock_result != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_result);
        if constexpr (!std::is_abstract_v<elem_t>)
        {
            static elem_t default_value = elem_t();
            return (default_value);
        }
        static char default_buffer[sizeof(elem_t)] = {0};
        return (*reinterpret_cast<elem_t*>(default_buffer));
    }
    if (this->_data == ft_nullptr)
    {
        this->unlock_internal(lock_acquired);
        ft_global_error_stack_push(FT_ERR_INVALID_OPERATION);
        if constexpr (!std::is_abstract_v<elem_t>)
        {
            static elem_t default_value = elem_t();
            return (default_value);
        }
        static char default_buffer[sizeof(elem_t)] = {0};
        return (*reinterpret_cast<elem_t*>(default_buffer));
    }
    elem_t& ref = std::get<I>(*this->_data);
    this->unlock_internal(lock_acquired);
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (ref);
}

template <typename... Types>
template <std::size_t I>
const typename std::tuple_element<I, typename ft_tuple<Types...>::tuple_t>::type&
ft_tuple<Types...>::get() const
{
    using elem_t = typename std::tuple_element<I, tuple_t>::type;
    bool lock_acquired = false;
    int lock_result = this->lock_internal(&lock_acquired);

    if (lock_result != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_result);
        if constexpr (!std::is_abstract_v<elem_t>)
        {
            static elem_t default_value = elem_t();
            return (default_value);
        }
        static char default_buffer[sizeof(elem_t)] = {0};
        return (*reinterpret_cast<elem_t*>(default_buffer));
    }
    if (this->_data == ft_nullptr)
    {
        this->unlock_internal(lock_acquired);
        ft_global_error_stack_push(FT_ERR_INVALID_OPERATION);
        if constexpr (!std::is_abstract_v<elem_t>)
        {
            static elem_t default_value = elem_t();
            return (default_value);
        }
        static char default_buffer[sizeof(elem_t)] = {0};
        return (*reinterpret_cast<elem_t*>(default_buffer));
    }
    const elem_t& ref = std::get<I>(*this->_data);
    this->unlock_internal(lock_acquired);
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (ref);
}

template <typename... Types>
template <typename T>
T& ft_tuple<Types...>::get()
{
    bool lock_acquired = false;
    int lock_result = this->lock_internal(&lock_acquired);

    if (lock_result != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_result);
        if constexpr (!std::is_abstract_v<T>)
        {
            static T default_value = T();
            return (default_value);
        }
        static char default_buffer[sizeof(T)] = {0};
        return (*reinterpret_cast<T*>(default_buffer));
    }
    if (this->_data == ft_nullptr)
    {
        this->unlock_internal(lock_acquired);
        ft_global_error_stack_push(FT_ERR_INVALID_OPERATION);
        if constexpr (!std::is_abstract_v<T>)
        {
            static T default_value = T();
            return (default_value);
        }
        static char default_buffer[sizeof(T)] = {0};
        return (*reinterpret_cast<T*>(default_buffer));
    }
    T& ref = std::get<T>(*this->_data);
    this->unlock_internal(lock_acquired);
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (ref);
}

template <typename... Types>
template <typename T>
const T& ft_tuple<Types...>::get() const
{
    bool lock_acquired = false;
    int lock_result = this->lock_internal(&lock_acquired);

    if (lock_result != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_result);
        if constexpr (!std::is_abstract_v<T>)
        {
            static T default_value = T();
            return (default_value);
        }
        static char default_buffer[sizeof(T)] = {0};
        return (*reinterpret_cast<T*>(default_buffer));
    }
    if (this->_data == ft_nullptr)
    {
        this->unlock_internal(lock_acquired);
        ft_global_error_stack_push(FT_ERR_INVALID_OPERATION);
        if constexpr (!std::is_abstract_v<T>)
        {
            static T default_value = T();
            return (default_value);
        }
        static char default_buffer[sizeof(T)] = {0};
        return (*reinterpret_cast<T*>(default_buffer));
    }
    const T& ref = std::get<T>(*this->_data);
    this->unlock_internal(lock_acquired);
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (ref);
}

template <typename... Types>
void ft_tuple<Types...>::reset()
{
    bool lock_acquired = false;
    int lock_result = this->lock_internal(&lock_acquired);

    if (lock_result != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_result);
        return ;
    }
    this->destroy_locked();
    this->unlock_internal(lock_acquired);
    ft_global_error_stack_push(FT_ERR_SUCCESS);
}

template <typename... Types>
int ft_tuple<Types...>::enable_thread_safety()
{
    if (this->_mutex != ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    int result = this->prepare_thread_safety();

    ft_global_error_stack_push(result);
    return (result);
}

template <typename... Types>
void ft_tuple<Types...>::disable_thread_safety()
{
    this->teardown_thread_safety();
    ft_global_error_stack_push(FT_ERR_SUCCESS);
}

template <typename... Types>
bool ft_tuple<Types...>::is_thread_safe() const
{
    bool enabled = (this->_mutex != ft_nullptr);

    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (enabled);
}

template <typename... Types>
int ft_tuple<Types...>::lock(bool *lock_acquired) const
{
    int result = this->lock_internal(lock_acquired);

    ft_global_error_stack_push(result);
    if (result != FT_ERR_SUCCESS)
        return (-1);
    return (0);
}

template <typename... Types>
void ft_tuple<Types...>::unlock(bool lock_acquired) const
{
    int result = this->unlock_internal(lock_acquired);

    ft_global_error_stack_push(result);
}

template <typename... Types>
int ft_tuple<Types...>::lock_internal(bool *lock_acquired) const
{
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    int result = pt_recursive_mutex_lock_with_error(*this->_mutex);

    if (result == FT_ERR_SUCCESS && lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (result);
}

template <typename... Types>
int ft_tuple<Types...>::unlock_internal(bool lock_acquired) const
{
    if (!lock_acquired || this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (pt_recursive_mutex_unlock_with_error(*this->_mutex));
}

template <typename... Types>
void ft_tuple<Types...>::destroy_locked()
{
    if (this->_data != ft_nullptr)
    {
        ::destroy_at(this->_data);
        cma_free(this->_data);
        this->_data = ft_nullptr;
    }
}

template <typename... Types>
int ft_tuple<Types...>::prepare_thread_safety()
{
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    int result = pt_recursive_mutex_create_with_error(&this->_mutex);

    if (result != FT_ERR_SUCCESS && this->_mutex != ft_nullptr)
        pt_recursive_mutex_destroy(&this->_mutex);
    return (result);
}

template <typename... Types>
void ft_tuple<Types...>::teardown_thread_safety()
{
    pt_recursive_mutex_destroy(&this->_mutex);
}

#ifdef LIBFT_TEST_BUILD

template <typename... Types>
pt_recursive_mutex* ft_tuple<Types...>::get_mutex_for_validation() const noexcept
{
    return (this->_mutex);
}
#endif

#endif
