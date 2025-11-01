#ifndef FT_VARIANT_HPP
#define FT_VARIANT_HPP

#include "constructor.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"
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
            destroy_at(reinterpret_cast<T*>(data));
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
        mutable int     _error_code;
        mutable pt_mutex* _state_mutex;
        bool            _thread_safe_enabled;

        void set_error(int error) const;
        void destroy_unlocked();
        int lock_internal(bool *lock_acquired) const;
        void unlock_internal(bool lock_acquired) const;
        void teardown_thread_safety();

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

        int get_error() const;
        const char* get_error_str() const;
        int enable_thread_safety();
        void disable_thread_safety();
        bool is_thread_safe_enabled() const;
        int lock(bool *lock_acquired) const;
        void unlock(bool lock_acquired) const;
};

template <typename... Types>
void ft_variant<Types...>::set_error(int error) const
{
    this->_error_code = error;
    ft_errno = error;
    return ;
}

template <typename... Types>
ft_variant<Types...>::ft_variant()
    : _data(static_cast<storage_t*>(cma_malloc(sizeof(storage_t)))),
      _index(npos), _error_code(ER_SUCCESS), _state_mutex(ft_nullptr),
      _thread_safe_enabled(false)
{
    if (this->_data == ft_nullptr)
        this->set_error(FT_ERR_NO_MEMORY);
    return ;
}

template <typename... Types>
template <typename T>
ft_variant<Types...>::ft_variant(const T& value)
    : ft_variant()
{
    this->emplace<T>(value);
    return ;
}

template <typename... Types>
template <typename T>
ft_variant<Types...>::ft_variant(T&& value)
    : ft_variant()
{
    this->emplace<T>(ft_move(value));
    return ;
}

template <typename... Types>
ft_variant<Types...>::~ft_variant()
{
    bool lock_acquired;
    storage_t *data_pointer;

    lock_acquired = false;
    data_pointer = this->_data;
    if (this->lock_internal(&lock_acquired) == 0)
    {
        this->destroy_unlocked();
        this->_data = ft_nullptr;
        this->unlock_internal(lock_acquired);
    }
    else
        this->_data = ft_nullptr;
    if (data_pointer != ft_nullptr)
        cma_free(data_pointer);
    this->teardown_thread_safety();
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename... Types>
ft_variant<Types...>::ft_variant(ft_variant&& other) noexcept
    : _data(ft_nullptr), _index(npos), _error_code(ER_SUCCESS),
      _state_mutex(ft_nullptr), _thread_safe_enabled(false)
{
    bool lock_acquired;
    storage_t *transferred_data;
    size_t transferred_index;
    int transferred_error;
    pt_mutex *transferred_mutex;
    bool transferred_thread_safe;

    lock_acquired = false;
    if (other.lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    transferred_data = other._data;
    transferred_index = other._index;
    transferred_error = other._error_code;
    transferred_mutex = other._state_mutex;
    transferred_thread_safe = other._thread_safe_enabled;
    this->_data = transferred_data;
    this->_index = transferred_index;
    this->_error_code = transferred_error;
    this->_state_mutex = transferred_mutex;
    this->_thread_safe_enabled = transferred_thread_safe;
    other._data = ft_nullptr;
    other._index = npos;
    other._error_code = ER_SUCCESS;
    other._thread_safe_enabled = false;
    other.unlock_internal(lock_acquired);
    other._state_mutex = ft_nullptr;
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename... Types>
ft_variant<Types...>& ft_variant<Types...>::operator=(ft_variant&& other) noexcept
{
    bool this_lock_acquired;
    bool other_lock_acquired;
    storage_t *previous_data;
    pt_mutex *previous_mutex;
    bool previous_thread_safe;
    storage_t *transferred_data;
    size_t transferred_index;
    int transferred_error;
    pt_mutex *transferred_mutex;
    bool transferred_thread_safe;

    if (this == &other)
    {
        this->set_error(this->_error_code);
        return (*this);
    }
    this_lock_acquired = false;
    if (this->lock_internal(&this_lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return (*this);
    }
    other_lock_acquired = false;
    if (other.lock_internal(&other_lock_acquired) != 0)
    {
        this->unlock_internal(this_lock_acquired);
        this->set_error(ft_errno);
        return (*this);
    }
    previous_data = this->_data;
    previous_mutex = this->_state_mutex;
    previous_thread_safe = this->_thread_safe_enabled;
    this->destroy_unlocked();
    transferred_data = other._data;
    transferred_index = other._index;
    transferred_error = other._error_code;
    transferred_mutex = other._state_mutex;
    transferred_thread_safe = other._thread_safe_enabled;
    this->_data = transferred_data;
    this->_index = transferred_index;
    this->_error_code = transferred_error;
    this->_thread_safe_enabled = false;
    other._data = ft_nullptr;
    other._index = npos;
    other._error_code = ER_SUCCESS;
    other._thread_safe_enabled = false;
    other.unlock_internal(other_lock_acquired);
    other._state_mutex = ft_nullptr;
    this->unlock_internal(this_lock_acquired);
    if (previous_data != ft_nullptr && previous_data != this->_data)
        cma_free(previous_data);
    if (previous_thread_safe && previous_mutex != ft_nullptr && previous_mutex != transferred_mutex)
    {
        previous_mutex->~pt_mutex();
        cma_free(previous_mutex);
    }
    this->_state_mutex = transferred_mutex;
    this->_thread_safe_enabled = transferred_thread_safe;
    this->set_error(transferred_error);
    return (*this);
}

template <typename... Types>
template <typename T>
void ft_variant<Types...>::emplace(T&& value)
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    if (this->_data == ft_nullptr)
    {
        this->_data = static_cast<storage_t*>(cma_malloc(sizeof(storage_t)));
        if (this->_data == ft_nullptr)
        {
            this->set_error(FT_ERR_NO_MEMORY);
            this->unlock_internal(lock_acquired);
            return ;
        }
    }
    this->destroy_unlocked();
    construct_at(reinterpret_cast<std::decay_t<T>*>(this->_data), std::forward<T>(value));
    this->_index = variant_index<std::decay_t<T>, Types...>::value;
    this->unlock_internal(lock_acquired);
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename... Types>
template <typename T>
bool ft_variant<Types...>::holds_alternative() const
{
    bool lock_acquired;
    size_t idx;
    bool result;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        const_cast<ft_variant*>(this)->set_error(ft_errno);
        return (false);
    }
    idx = variant_index<T, Types...>::value;
    result = (this->_index == idx);
    this->unlock_internal(lock_acquired);
    const_cast<ft_variant*>(this)->set_error(ER_SUCCESS);
    return (result);
}

template <typename... Types>
template <typename T>
T& ft_variant<Types...>::get()
{
    static T default_instance = T();
    bool lock_acquired;
    size_t idx;
    T *value_pointer;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return (default_instance);
    }
    idx = variant_index<T, Types...>::value;
    if (this->_index != idx)
    {
        this->set_error(FT_ERR_INVALID_OPERATION);
        this->unlock_internal(lock_acquired);
        return (default_instance);
    }
    value_pointer = reinterpret_cast<T*>(this->_data);
    this->unlock_internal(lock_acquired);
    this->set_error(ER_SUCCESS);
    return (*value_pointer);
}

template <typename... Types>
template <typename T>
const T& ft_variant<Types...>::get() const
{
    static T default_instance = T();
    bool lock_acquired;
    size_t idx;
    const T *value_pointer;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        const_cast<ft_variant*>(this)->set_error(ft_errno);
        return (default_instance);
    }
    idx = variant_index<T, Types...>::value;
    if (this->_index != idx)
    {
        const_cast<ft_variant*>(this)->set_error(FT_ERR_INVALID_OPERATION);
        this->unlock_internal(lock_acquired);
        return (default_instance);
    }
    value_pointer = reinterpret_cast<const T*>(this->_data);
    this->unlock_internal(lock_acquired);
    this->set_error(ER_SUCCESS);
    return (*value_pointer);
}

template <typename... Types>
template <typename Visitor>
void ft_variant<Types...>::visit(Visitor&& vis)
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    if (this->_index == npos)
    {
        this->set_error(FT_ERR_INVALID_OPERATION);
        this->unlock_internal(lock_acquired);
        return ;
    }
    variant_visitor<0, Types...>::apply(this->_index, this->_data, std::forward<Visitor>(vis));
    this->unlock_internal(lock_acquired);
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename... Types>
void ft_variant<Types...>::reset()
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    this->destroy_unlocked();
    this->unlock_internal(lock_acquired);
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename... Types>
int ft_variant<Types...>::get_error() const
{
    return (this->_error_code);
}

template <typename... Types>
const char* ft_variant<Types...>::get_error_str() const
{
    return (ft_strerror(this->_error_code));
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
    void *memory;
    pt_mutex *state_mutex;

    if (this->_thread_safe_enabled && this->_state_mutex != ft_nullptr)
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
    state_mutex = new(memory) pt_mutex();
    if (state_mutex->get_error() != ER_SUCCESS)
    {
        int mutex_error;

        mutex_error = state_mutex->get_error();
        state_mutex->~pt_mutex();
        cma_free(memory);
        this->set_error(mutex_error);
        return (-1);
    }
    this->_state_mutex = state_mutex;
    this->_thread_safe_enabled = true;
    this->set_error(ER_SUCCESS);
    return (0);
}

template <typename... Types>
void ft_variant<Types...>::disable_thread_safety()
{
    this->teardown_thread_safety();
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename... Types>
bool ft_variant<Types...>::is_thread_safe_enabled() const
{
    bool enabled;

    enabled = (this->_thread_safe_enabled && this->_state_mutex != ft_nullptr);
    this->set_error(ER_SUCCESS);
    return (enabled);
}

template <typename... Types>
int ft_variant<Types...>::lock(bool *lock_acquired) const
{
    int entry_errno;
    int result;

    entry_errno = ft_errno;
    result = this->lock_internal(lock_acquired);
    if (result != 0)
    {
        const_cast<ft_variant*>(this)->set_error(ft_errno);
        return (result);
    }
    this->_error_code = ER_SUCCESS;
    ft_errno = entry_errno;
    return (result);
}

template <typename... Types>
void ft_variant<Types...>::unlock(bool lock_acquired) const
{
    int entry_errno;
    int mutex_error;

    entry_errno = ft_errno;
    this->unlock_internal(lock_acquired);
    if (!lock_acquired || this->_state_mutex == ft_nullptr)
    {
        this->_error_code = ER_SUCCESS;
        ft_errno = entry_errno;
        return ;
    }
    mutex_error = this->_state_mutex->get_error();
    if (mutex_error != ER_SUCCESS)
    {
        const_cast<ft_variant*>(this)->set_error(mutex_error);
        return ;
    }
    this->_error_code = ER_SUCCESS;
    ft_errno = entry_errno;
    return ;
}

template <typename... Types>
int ft_variant<Types...>::lock_internal(bool *lock_acquired) const
{
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (!this->_thread_safe_enabled || this->_state_mutex == ft_nullptr)
    {
        ft_errno = ER_SUCCESS;
        return (0);
    }
    this->_state_mutex->lock(THREAD_ID);
    if (this->_state_mutex->get_error() != ER_SUCCESS)
    {
        ft_errno = this->_state_mutex->get_error();
        return (-1);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    ft_errno = ER_SUCCESS;
    return (0);
}

template <typename... Types>
void ft_variant<Types...>::unlock_internal(bool lock_acquired) const
{
    int entry_errno;

    if (!lock_acquired || this->_state_mutex == ft_nullptr)
        return ;
    entry_errno = ft_errno;
    this->_state_mutex->unlock(THREAD_ID);
    if (this->_state_mutex->get_error() != ER_SUCCESS)
    {
        ft_errno = this->_state_mutex->get_error();
        return ;
    }
    ft_errno = entry_errno;
    return ;
}

template <typename... Types>
void ft_variant<Types...>::teardown_thread_safety()
{
    if (this->_state_mutex != ft_nullptr)
    {
        this->_state_mutex->~pt_mutex();
        cma_free(this->_state_mutex);
        this->_state_mutex = ft_nullptr;
    }
    this->_thread_safe_enabled = false;
    return ;
}

#endif 
