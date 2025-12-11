#ifndef FT_TUPLE_HPP
#define FT_TUPLE_HPP

#include "constructor.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Libft/libft.hpp"
#include <tuple>
#include <utility>
#include <new>
#include <type_traits>
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"

template <typename... Types>
class ft_tuple
{
    private:
        using tuple_t = std::tuple<Types...>;
        tuple_t*        _data;
        mutable int     _error_code;
        mutable pt_mutex    *_mutex;
        bool                _thread_safe_enabled;

        void set_error_unlocked(int error) const;
        void set_error(int error) const;
        int  lock_internal(bool *lock_acquired) const;
        void unlock_internal(bool lock_acquired) const;
        int  prepare_thread_safety();
        void teardown_thread_safety();
        void destroy_locked();

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

        int get_error() const;
        const char* get_error_str() const;
};

template <typename... Types>
ft_tuple<Types...>::ft_tuple()
    : _data(ft_nullptr),
      _error_code(FT_ERR_SUCCESSS),
      _mutex(ft_nullptr),
      _thread_safe_enabled(false)
{
    return ;
}

template <typename... Types>
ft_tuple<Types...>::~ft_tuple()
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) == 0)
    {
        this->destroy_locked();
        this->set_error_unlocked(FT_ERR_SUCCESSS);
        this->unlock_internal(lock_acquired);
    }
    else
        this->set_error_unlocked(ft_errno);
    this->teardown_thread_safety();
    return ;
}

template <typename... Types>
ft_tuple<Types...>::ft_tuple(ft_tuple&& other) noexcept
    : _data(ft_nullptr),
      _error_code(FT_ERR_SUCCESSS),
      _mutex(ft_nullptr),
      _thread_safe_enabled(false)
{
    bool other_lock_acquired;
    pt_mutex *transferred_mutex;
    bool other_thread_safe;

    other_lock_acquired = false;
    transferred_mutex = ft_nullptr;
    other_thread_safe = false;
    if (other.lock_internal(&other_lock_acquired) != 0)
    {
        this->set_error_unlocked(ft_errno);
        return ;
    }
    transferred_mutex = other._mutex;
    other_thread_safe = (other._thread_safe_enabled && other._mutex != ft_nullptr);
    this->_data = other._data;
    this->_error_code = other._error_code;
    this->_mutex = ft_nullptr;
    this->_thread_safe_enabled = false;
    other._data = ft_nullptr;
    other._error_code = FT_ERR_SUCCESSS;
    other._mutex = ft_nullptr;
    other._thread_safe_enabled = false;
    other.unlock_internal(other_lock_acquired);
    if (transferred_mutex != ft_nullptr)
    {
        transferred_mutex->~pt_mutex();
        cma_free(transferred_mutex);
    }
    if (other_thread_safe)
    {
        if (this->enable_thread_safety() != 0)
            return ;
    }
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    return ;
}

template <typename... Types>
ft_tuple<Types...>& ft_tuple<Types...>::operator=(ft_tuple&& other) noexcept
{
    if (this != &other)
    {
        ft_tuple<Types...>* first;
        ft_tuple<Types...>* second;
        bool first_lock_acquired;
        bool second_lock_acquired;
        pt_mutex *previous_mutex;
        bool previous_thread_safe;
        pt_mutex *transferred_mutex;
        bool other_thread_safe;

        first = this;
        second = &other;
        if (first > second)
        {
            ft_tuple<Types...>* temp_pointer;

            temp_pointer = first;
            first = second;
            second = temp_pointer;
        }
        first_lock_acquired = false;
        if (first->lock_internal(&first_lock_acquired) != 0)
        {
            this->set_error_unlocked(ft_errno);
            other.set_error_unlocked(ft_errno);
            return (*this);
        }
        second_lock_acquired = false;
        if (second->lock_internal(&second_lock_acquired) != 0)
        {
            first->unlock_internal(first_lock_acquired);
            this->set_error_unlocked(ft_errno);
            other.set_error_unlocked(ft_errno);
            return (*this);
        }
        previous_mutex = this->_mutex;
        previous_thread_safe = this->_thread_safe_enabled;
        transferred_mutex = other._mutex;
        other_thread_safe = (other._thread_safe_enabled && other._mutex != ft_nullptr);
        this->destroy_locked();
        this->_data = other._data;
        this->_error_code = other._error_code;
        this->_mutex = ft_nullptr;
        this->_thread_safe_enabled = false;
        other._data = ft_nullptr;
        other._error_code = FT_ERR_SUCCESSS;
        other._mutex = ft_nullptr;
        other._thread_safe_enabled = false;
        other.unlock_internal(second_lock_acquired);
        this->unlock_internal(first_lock_acquired);
        if (previous_thread_safe && previous_mutex != ft_nullptr)
        {
            previous_mutex->~pt_mutex();
            cma_free(previous_mutex);
        }
        if (transferred_mutex != ft_nullptr)
        {
            transferred_mutex->~pt_mutex();
            cma_free(transferred_mutex);
        }
        if (other_thread_safe)
        {
            if (this->enable_thread_safety() != 0)
                return (*this);
        }
        this->set_error_unlocked(FT_ERR_SUCCESSS);
        other.set_error_unlocked(FT_ERR_SUCCESSS);
    }
    return (*this);
}

template <typename... Types>
void ft_tuple<Types...>::set_error_unlocked(int error) const
{
    this->_error_code = error;
    ft_errno = error;
    return ;
}

template <typename... Types>
void ft_tuple<Types...>::set_error(int error) const
{
    this->set_error_unlocked(error);
    return ;
}

template <typename... Types>
template <typename... Args>
ft_tuple<Types...>::ft_tuple(Args&&... args)
    : _data(ft_nullptr),
      _error_code(FT_ERR_SUCCESSS),
      _mutex(ft_nullptr),
      _thread_safe_enabled(false)
{
    _data = static_cast<tuple_t*>(cma_malloc(sizeof(tuple_t)));
    if (_data == ft_nullptr)
        this->set_error_unlocked(FT_ERR_NO_MEMORY);
    else
        construct_at(_data, std::forward<Args>(args)...);
    return ;
}

template <typename... Types>
template <std::size_t I>
typename std::tuple_element<I, typename ft_tuple<Types...>::tuple_t>::type&
ft_tuple<Types...>::get()
{
    using elem_t = typename std::tuple_element<I, tuple_t>::type;
    static elem_t default_instance = elem_t();
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error_unlocked(ft_errno);
        if constexpr (!std::is_abstract_v<elem_t>)
            return (default_instance);
        else
        {
            static char dummy_buffer[sizeof(elem_t)] = {0};
            return (*reinterpret_cast<elem_t*>(dummy_buffer));
        }
    }
    if (this->_data == ft_nullptr)
    {
        this->set_error_unlocked(FT_ERR_INVALID_OPERATION);
        this->unlock_internal(lock_acquired);
        if constexpr (!std::is_abstract_v<elem_t>)
            return (default_instance);
        else
        {
            static char dummy_buffer[sizeof(elem_t)] = {0};
            return (*reinterpret_cast<elem_t*>(dummy_buffer));
        }
    }
    elem_t& ref = std::get<I>(*this->_data);
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (ref);
}

template <typename... Types>
template <std::size_t I>
const typename std::tuple_element<I, typename ft_tuple<Types...>::tuple_t>::type&
ft_tuple<Types...>::get() const
{
    using elem_t = typename std::tuple_element<I, tuple_t>::type;
    static elem_t default_instance = elem_t();
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error_unlocked(ft_errno);
        if constexpr (!std::is_abstract_v<elem_t>)
            return (default_instance);
        else
        {
            static char dummy_buffer[sizeof(elem_t)] = {0};
            return (*reinterpret_cast<elem_t*>(dummy_buffer));
        }
    }
    if (this->_data == ft_nullptr)
    {
        const_cast<ft_tuple*>(this)->set_error_unlocked(FT_ERR_INVALID_OPERATION);
        this->unlock_internal(lock_acquired);
        if constexpr (!std::is_abstract_v<elem_t>)
            return (default_instance);
        else
        {
            static char dummy_buffer[sizeof(elem_t)] = {0};
            return (*reinterpret_cast<elem_t*>(dummy_buffer));
        }
    }
    const elem_t& ref = std::get<I>(*this->_data);
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (ref);
}

template <typename... Types>
template <typename T>
T& ft_tuple<Types...>::get()
{
    static T default_instance = T();
    bool     lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error_unlocked(ft_errno);
        if constexpr (!std::is_abstract_v<T>)
            return (default_instance);
        else
        {
            static char dummy_buffer[sizeof(T)] = {0};
            return (*reinterpret_cast<T*>(dummy_buffer));
        }
    }
    if (this->_data == ft_nullptr)
    {
        this->set_error_unlocked(FT_ERR_INVALID_OPERATION);
        this->unlock_internal(lock_acquired);
        if constexpr (!std::is_abstract_v<T>)
            return (default_instance);
        else
        {
            static char dummy_buffer[sizeof(T)] = {0};
            return (*reinterpret_cast<T*>(dummy_buffer));
        }
    }
    T& ref = std::get<T>(*this->_data);
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (ref);
}

template <typename... Types>
template <typename T>
const T& ft_tuple<Types...>::get() const
{
    static T default_instance = T();
    bool     lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error_unlocked(ft_errno);
        if constexpr (!std::is_abstract_v<T>)
            return (default_instance);
        else
        {
            static char dummy_buffer[sizeof(T)] = {0};
            return (*reinterpret_cast<T*>(dummy_buffer));
        }
    }
    if (this->_data == ft_nullptr)
    {
        const_cast<ft_tuple*>(this)->set_error_unlocked(FT_ERR_INVALID_OPERATION);
        this->unlock_internal(lock_acquired);
        if constexpr (!std::is_abstract_v<T>)
            return (default_instance);
        else
        {
            static char dummy_buffer[sizeof(T)] = {0};
            return (*reinterpret_cast<T*>(dummy_buffer));
        }
    }
    const T& ref = std::get<T>(*this->_data);
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return (ref);
}

template <typename... Types>
void ft_tuple<Types...>::reset()
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error_unlocked(ft_errno);
        return ;
    }
    this->destroy_locked();
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename... Types>
int ft_tuple<Types...>::enable_thread_safety()
{
    return (this->prepare_thread_safety());
}

template <typename... Types>
void ft_tuple<Types...>::disable_thread_safety()
{
    this->teardown_thread_safety();
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    return ;
}

template <typename... Types>
bool ft_tuple<Types...>::is_thread_safe() const
{
    bool enabled;

    enabled = (this->_thread_safe_enabled && this->_mutex != ft_nullptr);
    const_cast<ft_tuple*>(this)->set_error_unlocked(FT_ERR_SUCCESSS);
    return (enabled);
}

template <typename... Types>
int ft_tuple<Types...>::lock(bool *lock_acquired) const
{
    int result;

    result = this->lock_internal(lock_acquired);
    if (result != 0)
        const_cast<ft_tuple*>(this)->set_error_unlocked(ft_errno);
    else
        const_cast<ft_tuple*>(this)->set_error_unlocked(FT_ERR_SUCCESSS);
    return (result);
}

template <typename... Types>
void ft_tuple<Types...>::unlock(bool lock_acquired) const
{
    int entry_errno;

    entry_errno = ft_errno;
    this->unlock_internal(lock_acquired);
    if (this->_mutex != ft_nullptr && this->_mutex->get_error() != FT_ERR_SUCCESSS)
        const_cast<ft_tuple*>(this)->set_error_unlocked(this->_mutex->get_error());
    else
    {
        ft_errno = entry_errno;
        const_cast<ft_tuple*>(this)->set_error_unlocked(ft_errno);
    }
    return ;
}

template <typename... Types>
int ft_tuple<Types...>::get_error() const
{
    return (this->_error_code);
}

template <typename... Types>
const char* ft_tuple<Types...>::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}

template <typename... Types>
void ft_tuple<Types...>::destroy_locked()
{
    if (this->_data != ft_nullptr)
    {
        destroy_at(this->_data);
        cma_free(this->_data);
        this->_data = ft_nullptr;
    }
    return ;
}

template <typename... Types>
int ft_tuple<Types...>::lock_internal(bool *lock_acquired) const
{
    if (lock_acquired)
        *lock_acquired = false;
    if (!this->_thread_safe_enabled || this->_mutex == ft_nullptr)
    {
        ft_errno = FT_ERR_SUCCESSS;
        return (0);
    }
    this->_mutex->lock(THREAD_ID);
    if (this->_mutex->get_error() != FT_ERR_SUCCESSS)
    {
        if (this->_mutex->get_error() == FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            bool state_lock_acquired;

            state_lock_acquired = false;
            ft_errno = FT_ERR_SUCCESSS;
            if (this->_mutex->lock_state(&state_lock_acquired) == 0)
                this->_mutex->unlock_state(state_lock_acquired);
            ft_errno = FT_ERR_SUCCESSS;
            return (0);
        }
        ft_errno = this->_mutex->get_error();
        return (-1);
    }
    if (lock_acquired)
        *lock_acquired = true;
    ft_errno = FT_ERR_SUCCESSS;
    return (0);
}

template <typename... Types>
void ft_tuple<Types...>::unlock_internal(bool lock_acquired) const
{
    int entry_errno;

    if (!lock_acquired || this->_mutex == ft_nullptr)
        return ;
    entry_errno = ft_errno;
    this->_mutex->unlock(THREAD_ID);
    if (this->_mutex->get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = this->_mutex->get_error();
        return ;
    }
    ft_errno = entry_errno;
    return ;
}

template <typename... Types>
int ft_tuple<Types...>::prepare_thread_safety()
{
    void *memory_pointer;
    pt_mutex *mutex_pointer;

    if (this->_thread_safe_enabled && this->_mutex != ft_nullptr)
    {
        this->set_error_unlocked(FT_ERR_SUCCESSS);
        return (0);
    }
    memory_pointer = cma_malloc(sizeof(pt_mutex));
    if (memory_pointer == ft_nullptr)
    {
        this->set_error_unlocked(FT_ERR_NO_MEMORY);
        return (-1);
    }
    mutex_pointer = new(memory_pointer) pt_mutex();
    if (mutex_pointer->get_error() != FT_ERR_SUCCESSS)
    {
        int mutex_error;

        mutex_error = mutex_pointer->get_error();
        mutex_pointer->~pt_mutex();
        cma_free(memory_pointer);
        this->set_error_unlocked(mutex_error);
        return (-1);
    }
    this->_mutex = mutex_pointer;
    this->_thread_safe_enabled = true;
    this->set_error_unlocked(FT_ERR_SUCCESSS);
    return (0);
}

template <typename... Types>
void ft_tuple<Types...>::teardown_thread_safety()
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
