#ifndef FT_TUPLE_HPP
#define FT_TUPLE_HPP

#include "constructor.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/nullptr.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/mutex.hpp"
#include <tuple>
#include <utility>
#include <new>
#include <type_traits>

/*
 * ft_tuple
 * Fixed-size heterogeneous collection with thread-safe access helpers.
 */

template <typename... Types>
class ft_tuple
{
    private:
        using tuple_t = std::tuple<Types...>;
        tuple_t*        _data;
        mutable int     _errorCode;
        mutable pt_mutex _mutex;

        void setError(int error) const;

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

        int get_error() const;
        const char* get_error_str() const;
};

/* Constructors & Destructor */

template <typename... Types>
ft_tuple<Types...>::ft_tuple()
    : _data(ft_nullptr), _errorCode(ER_SUCCESS)
{
    return ;
}

template <typename... Types>
ft_tuple<Types...>::~ft_tuple()
{
    this->reset();
    return ;
}

template <typename... Types>
ft_tuple<Types...>::ft_tuple(ft_tuple&& other) noexcept
    : _data(other._data), _errorCode(other._errorCode)
{
    other._data = ft_nullptr;
    other._errorCode = ER_SUCCESS;
    return ;
}

template <typename... Types>
ft_tuple<Types...>& ft_tuple<Types...>::operator=(ft_tuple&& other) noexcept
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
        this->reset();
        this->_data = other._data;
        this->_errorCode = other._errorCode;
        other._data = ft_nullptr;
        other._errorCode = ER_SUCCESS;
        other._mutex.unlock(THREAD_ID);
        this->_mutex.unlock(THREAD_ID);
    }
    return (*this);
}

/* setError */

template <typename... Types>
void ft_tuple<Types...>::setError(int error) const
{
    this->_errorCode = error;
    ft_errno = error;
    return ;
}

/* construction */

template <typename... Types>
template <typename... Args>
ft_tuple<Types...>::ft_tuple(Args&&... args)
    : _data(ft_nullptr), _errorCode(ER_SUCCESS)
{
    _data = static_cast<tuple_t*>(cma_malloc(sizeof(tuple_t)));
    if (_data == ft_nullptr)
        this->setError(TUPLE_ALLOC_FAIL);
    else
        construct_at(_data, std::forward<Args>(args)...);
    return ;
}

/* get by index */

template <typename... Types>
template <std::size_t I>
typename std::tuple_element<I, typename ft_tuple<Types...>::tuple_t>::type&
ft_tuple<Types...>::get()
{
    using elem_t = typename std::tuple_element<I, tuple_t>::type;
    static elem_t defaultInstance = elem_t();
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->setError(PT_ERR_MUTEX_OWNER);
        return (defaultInstance);
    }
    if (this->_data == ft_nullptr)
    {
        this->setError(TUPLE_BAD_ACCESS);
        this->_mutex.unlock(THREAD_ID);
        if constexpr (!std::is_abstract_v<elem_t>)
            return (defaultInstance);
        else
        {
            static char dummy_buffer[sizeof(elem_t)] = {0};
            return (*reinterpret_cast<elem_t*>(dummy_buffer));
        }
    }
    elem_t& ref = std::get<I>(*this->_data);
    this->_mutex.unlock(THREAD_ID);
    return (ref);
}

template <typename... Types>
template <std::size_t I>
const typename std::tuple_element<I, typename ft_tuple<Types...>::tuple_t>::type&
ft_tuple<Types...>::get() const
{
    using elem_t = typename std::tuple_element<I, tuple_t>::type;
    static elem_t defaultInstance = elem_t();
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        const_cast<ft_tuple*>(this)->setError(PT_ERR_MUTEX_OWNER);
        return (defaultInstance);
    }
    if (this->_data == ft_nullptr)
    {
        const_cast<ft_tuple*>(this)->setError(TUPLE_BAD_ACCESS);
        this->_mutex.unlock(THREAD_ID);
        if constexpr (!std::is_abstract_v<elem_t>)
            return (defaultInstance);
        else
        {
            static char dummy_buffer[sizeof(elem_t)] = {0};
            return (*reinterpret_cast<elem_t*>(dummy_buffer));
        }
    }
    const elem_t& ref = std::get<I>(*this->_data);
    this->_mutex.unlock(THREAD_ID);
    return (ref);
}

/* get by type */

template <typename... Types>
template <typename T>
T& ft_tuple<Types...>::get()
{
    static T defaultInstance = T();
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->setError(PT_ERR_MUTEX_OWNER);
        return (defaultInstance);
    }
    if (this->_data == ft_nullptr)
    {
        this->setError(TUPLE_BAD_ACCESS);
        this->_mutex.unlock(THREAD_ID);
        if constexpr (!std::is_abstract_v<T>)
            return (defaultInstance);
        else
        {
            static char dummy_buffer[sizeof(T)] = {0};
            return (*reinterpret_cast<T*>(dummy_buffer));
        }
    }
    T& ref = std::get<T>(*this->_data);
    this->_mutex.unlock(THREAD_ID);
    return (ref);
}

template <typename... Types>
template <typename T>
const T& ft_tuple<Types...>::get() const
{
    static T defaultInstance = T();
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        const_cast<ft_tuple*>(this)->setError(PT_ERR_MUTEX_OWNER);
        return (defaultInstance);
    }
    if (this->_data == ft_nullptr)
    {
        const_cast<ft_tuple*>(this)->setError(TUPLE_BAD_ACCESS);
        this->_mutex.unlock(THREAD_ID);
        if constexpr (!std::is_abstract_v<T>)
            return (defaultInstance);
        else
        {
            static char dummy_buffer[sizeof(T)] = {0};
            return (*reinterpret_cast<T*>(dummy_buffer));
        }
    }
    const T& ref = std::get<T>(*this->_data);
    this->_mutex.unlock(THREAD_ID);
    return (ref);
}

/* reset */

template <typename... Types>
void ft_tuple<Types...>::reset()
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->setError(PT_ERR_MUTEX_OWNER);
        return ;
    }
    if (this->_data != ft_nullptr)
    {
        destroy_at(this->_data);
        cma_free(this->_data);
        this->_data = ft_nullptr;
    }
    this->_mutex.unlock(THREAD_ID);
    return ;
}

/* error accessors */

template <typename... Types>
int ft_tuple<Types...>::get_error() const
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (this->_errorCode);
    int err = this->_errorCode;
    this->_mutex.unlock(THREAD_ID);
    return (err);
}

template <typename... Types>
const char* ft_tuple<Types...>::get_error_str() const
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (ft_strerror(this->_errorCode));
    int err = this->_errorCode;
    this->_mutex.unlock(THREAD_ID);
    return (ft_strerror(err));
}

#endif // FT_TUPLE_HPP
