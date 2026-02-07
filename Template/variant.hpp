#ifndef FT_VARIANT_HPP
#define FT_VARIANT_HPP

#include "constructor.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread_internal.hpp"
#include <type_traits>
#include <utility>
#include <cstddef>
#include <new>
#include <tuple>

#include "move.hpp"

template <typename T, typename... Ts>
struct variant_index;

template <typename T, typename... Ts>
struct variant_index<T, T, Ts...> : std::integral_constant<size_t, 0> {};

template <typename T, typename U, typename... Ts>
struct variant_index<T, U, Ts...>
    : std::integral_constant<size_t, 1 + variant_index<T, Ts...>::value> {};

template <size_t I, typename... Ts>
struct variant_destroyer;

template <size_t I>
struct variant_destroyer<I>
{
    static void destroy(size_t, void*) { return ; }
};

template <size_t I, typename T, typename... Ts>
struct variant_destroyer<I, T, Ts...>
{
    static void destroy(size_t index, void* data)
    {
        if (index == I)
            ::destroy_at(reinterpret_cast<T*>(data));
        else
            variant_destroyer<I + 1, Ts...>::destroy(index, data);
    }
};

template <size_t I, typename... Ts>
struct variant_visitor;

template <size_t I>
struct variant_visitor<I>
{
    template <typename Visitor>
    static void apply(size_t, void*, Visitor&&) { return ; }
};

template <size_t I, typename T, typename... Ts>
struct variant_visitor<I, T, Ts...>
{
    template <typename Visitor>
    static void apply(size_t index, void* data, Visitor&& vis)
    {
        if (index == I)
            vis(*reinterpret_cast<T*>(data));
        else
            variant_visitor<I + 1, Ts...>::apply(index, data, std::forward<Visitor>(vis));
    }
};

template <typename... Types>
class ft_variant
{
    private:
        using storage_t = typename std::aligned_union<0, Types...>::type;
        storage_t*      _data;
        size_t          _index;
        mutable pt_recursive_mutex* _mutex;

        void    destroy_unlocked();
        int     lock_internal(bool *lock_acquired) const;
        int     unlock_internal(bool lock_acquired) const;
        int     prepare_thread_safety();
        void    teardown_thread_safety();

    public:
        static constexpr size_t npos = static_cast<size_t>(-1);

        ft_variant();
        ~ft_variant();

        ft_variant(const ft_variant&) = delete;
        ft_variant& operator=(const ft_variant&) = delete;

        ft_variant(ft_variant&& other) noexcept;
        ft_variant& operator=(ft_variant&& other) noexcept;

        template <typename T>
        ft_variant(const T& value);
        template <typename T>
        ft_variant(T&& value);

        template <typename T>
        void emplace(T&& value);

        template <typename T>
        bool holds_alternative() const;

        template <typename T>
        T& get();
        template <typename T>
        const T& get() const;

        template <typename Visitor>
        void visit(Visitor&& vis);

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
ft_variant<Types...>::ft_variant()
    : _data(static_cast<storage_t*>(cma_malloc(sizeof(storage_t)))),
      _index(npos), _mutex(ft_nullptr)
{
    if (this->_data == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return ;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
}

template <typename... Types>
ft_variant<Types...>::~ft_variant()
{
    storage_t *data_pointer = this->_data;
    bool lock_acquired = false;
    int lock_error = this->lock_internal(&lock_acquired);

    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        if (data_pointer != ft_nullptr)
            cma_free(data_pointer);
        this->teardown_thread_safety();
        return ;
    }
    this->destroy_unlocked();
    this->_data = ft_nullptr;
    this->unlock_internal(lock_acquired);
    if (data_pointer != ft_nullptr)
        cma_free(data_pointer);
    this->teardown_thread_safety();
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
}

template <typename... Types>
ft_variant<Types...>::ft_variant(ft_variant&& other) noexcept
    : _data(ft_nullptr), _index(npos), _mutex(ft_nullptr)
{
    bool lock_acquired = false;
    int lock_error = other.lock_internal(&lock_acquired);

    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    this->_data = other._data;
    this->_index = other._index;
    other._data = ft_nullptr;
    other._index = npos;
    other.unlock_internal(lock_acquired);
    bool other_thread_safe = (other._mutex != ft_nullptr);
    other.teardown_thread_safety();
    if (other_thread_safe)
    {
        if (this->enable_thread_safety() != FT_ERR_SUCCESSS)
            return ;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
}

template <typename... Types>
ft_variant<Types...>& ft_variant<Types...>::operator=(ft_variant&& other) noexcept
{
    if (this == &other)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (*this);
    }
    bool this_lock_acquired = false;
    int lock_error = this->lock_internal(&this_lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (*this);
    }
    bool other_lock_acquired = false;
    lock_error = other.lock_internal(&other_lock_acquired);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->unlock_internal(this_lock_acquired);
        ft_global_error_stack_push(lock_error);
        return (*this);
    }
    storage_t *previous_data = this->_data;
    bool previous_thread_safe = (this->_mutex != ft_nullptr);
    this->destroy_unlocked();
    this->_data = other._data;
    this->_index = other._index;
    other._data = ft_nullptr;
    other._index = npos;
    this->unlock_internal(this_lock_acquired);
    other.unlock_internal(other_lock_acquired);
    bool other_thread_safe = (other._mutex != ft_nullptr);
    other.teardown_thread_safety();
    if (previous_data != ft_nullptr && previous_data != this->_data)
        cma_free(previous_data);
    if (previous_thread_safe)
        this->disable_thread_safety();
    if (other_thread_safe)
    {
        if (this->enable_thread_safety() != FT_ERR_SUCCESSS)
            return (*this);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (*this);
}

template <typename... Types>
template <typename T>
ft_variant<Types...>::ft_variant(const T& value)
    : ft_variant()
{
    this->emplace<T>(value);
}

template <typename... Types>
template <typename T>
ft_variant<Types...>::ft_variant(T&& value)
    : ft_variant()
{
    this->emplace<T>(ft_move(value));
}

template <typename... Types>
template <typename T>
void ft_variant<Types...>::emplace(T&& value)
{
    bool lock_acquired = false;
    int lock_error = this->lock_internal(&lock_acquired);

    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    if (this->_data == ft_nullptr)
    {
        this->_data = static_cast<storage_t*>(cma_malloc(sizeof(storage_t)));
        if (this->_data == ft_nullptr)
        {
            this->unlock_internal(lock_acquired);
            ft_global_error_stack_push(FT_ERR_NO_MEMORY);
            return ;
        }
    }
    this->destroy_unlocked();
    construct_at(reinterpret_cast<std::decay_t<T>*>(this->_data), std::forward<T>(value));
    this->_index = variant_index<std::decay_t<T>, Types...>::value;
    this->unlock_internal(lock_acquired);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
}

template <typename... Types>
template <typename T>
bool ft_variant<Types...>::holds_alternative() const
{
    bool lock_acquired = false;
    int lock_error = this->lock_internal(&lock_acquired);

    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (false);
    }
    size_t idx = variant_index<T, Types...>::value;
    bool result = (this->_index == idx);
    this->unlock_internal(lock_acquired);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}

template <typename... Types>
template <typename T>
T& ft_variant<Types...>::get()
{
    static T default_instance = T();
    bool lock_acquired = false;
    int lock_error = this->lock_internal(&lock_acquired);

    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (default_instance);
    }
    size_t idx = variant_index<T, Types...>::value;
    if (this->_index != idx)
    {
        this->unlock_internal(lock_acquired);
        ft_global_error_stack_push(FT_ERR_INVALID_OPERATION);
        return (default_instance);
    }
    T *value_pointer = reinterpret_cast<T*>(this->_data);
    this->unlock_internal(lock_acquired);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (*value_pointer);
}

template <typename... Types>
template <typename T>
const T& ft_variant<Types...>::get() const
{
    static T default_instance = T();
    bool lock_acquired = false;
    int lock_error = this->lock_internal(&lock_acquired);

    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return (default_instance);
    }
    size_t idx = variant_index<T, Types...>::value;
    if (this->_index != idx)
    {
        this->unlock_internal(lock_acquired);
        ft_global_error_stack_push(FT_ERR_INVALID_OPERATION);
        return (default_instance);
    }
    const T *value_pointer = reinterpret_cast<const T*>(this->_data);
    this->unlock_internal(lock_acquired);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (*value_pointer);
}

template <typename... Types>
template <typename Visitor>
void ft_variant<Types...>::visit(Visitor&& vis)
{
    bool lock_acquired = false;
    int lock_error = this->lock_internal(&lock_acquired);

    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    if (this->_index == npos)
    {
        this->unlock_internal(lock_acquired);
        ft_global_error_stack_push(FT_ERR_INVALID_OPERATION);
        return ;
    }
    variant_visitor<0, Types...>::apply(this->_index, this->_data, std::forward<Visitor>(vis));
    this->unlock_internal(lock_acquired);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

template <typename... Types>
void ft_variant<Types...>::reset()
{
    bool lock_acquired = false;
    int lock_error = this->lock_internal(&lock_acquired);

    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    this->destroy_unlocked();
    this->unlock_internal(lock_acquired);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

template <typename... Types>
void ft_variant<Types...>::destroy_unlocked()
{
    if (this->_data == ft_nullptr || this->_index == npos)
        return ;
    variant_destroyer<0, Types...>::destroy(this->_index, this->_data);
    this->_index = npos;
    return ;
}

template <typename... Types>
int ft_variant<Types...>::enable_thread_safety()
{
    if (this->_mutex != ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (FT_ERR_SUCCESSS);
    }
    int result = this->prepare_thread_safety();
    ft_global_error_stack_push(result);
    return (result);
}

template <typename... Types>
void ft_variant<Types...>::disable_thread_safety()
{
    this->teardown_thread_safety();
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

template <typename... Types>
bool ft_variant<Types...>::is_thread_safe() const
{
    bool enabled = (this->_mutex != ft_nullptr);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (enabled);
}

template <typename... Types>
int ft_variant<Types...>::lock(bool *lock_acquired) const
{
    int result = this->lock_internal(lock_acquired);
    ft_global_error_stack_push(result);
    if (result != FT_ERR_SUCCESSS)
        return (-1);
    return (0);
}

template <typename... Types>
void ft_variant<Types...>::unlock(bool lock_acquired) const
{
    int result = this->unlock_internal(lock_acquired);
    ft_global_error_stack_push(result);
    return ;
}

template <typename... Types>
int ft_variant<Types...>::lock_internal(bool *lock_acquired) const
{
    int result;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESSS);
    result = pt_recursive_mutex_lock_with_error(*this->_mutex);
    if (result != FT_ERR_SUCCESSS)
        return (result);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESSS);
}

template <typename... Types>
int ft_variant<Types...>::unlock_internal(bool lock_acquired) const
{
    if (!lock_acquired || this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESSS);
    return (pt_recursive_mutex_unlock_with_error(*this->_mutex));
}

template <typename... Types>
int ft_variant<Types...>::prepare_thread_safety()
{
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESSS);
    return (pt_recursive_mutex_create_with_error(&this->_mutex));
}

template <typename... Types>
void ft_variant<Types...>::teardown_thread_safety()
{
    pt_recursive_mutex_destroy(&this->_mutex);
}

#ifdef LIBFT_TEST_BUILD
template <typename... Types>
pt_recursive_mutex* ft_variant<Types...>::get_mutex_for_validation() const noexcept
{
    return (this->_mutex);
}
#endif

#endif
