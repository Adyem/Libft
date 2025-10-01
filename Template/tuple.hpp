#ifndef FT_TUPLE_HPP
#define FT_TUPLE_HPP

#include "constructor.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/mutex.hpp"
#include <tuple>
#include <utility>
#include <new>
#include <type_traits>



template <typename... Types>
class ft_tuple
{
    private:
        using tuple_t = std::tuple<Types...>;
        tuple_t*        _data;
        mutable int     _error_code;
        mutable pt_mutex _mutex;

        void set_error(int error) const;

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



template <typename... Types>
ft_tuple<Types...>::ft_tuple()
    : _data(ft_nullptr), _error_code(ER_SUCCESS)
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
    : _data(other._data), _error_code(other._error_code)
{
    other._data = ft_nullptr;
    other._error_code = ER_SUCCESS;
    return ;
}

template <typename... Types>
ft_tuple<Types...>& ft_tuple<Types...>::operator=(ft_tuple&& other) noexcept
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
        this->reset();
        this->_data = other._data;
        this->_error_code = other._error_code;
        other._data = ft_nullptr;
        other._error_code = ER_SUCCESS;
        other._mutex.unlock(THREAD_ID);
        this->_mutex.unlock(THREAD_ID);
    }
    return (*this);
}



template <typename... Types>
void ft_tuple<Types...>::set_error(int error) const
{
    this->_error_code = error;
    ft_errno = error;
    return ;
}



template <typename... Types>
template <typename... Args>
ft_tuple<Types...>::ft_tuple(Args&&... args)
    : _data(ft_nullptr), _error_code(ER_SUCCESS)
{
    _data = static_cast<tuple_t*>(cma_malloc(sizeof(tuple_t)));
    if (_data == ft_nullptr)
        this->set_error(TUPLE_ALLOC_FAIL);
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
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        this->set_error(PT_ERR_MUTEX_OWNER);
        return (default_instance);
    }
    if (this->_data == ft_nullptr)
    {
        this->set_error(TUPLE_BAD_ACCESS);
        this->_mutex.unlock(THREAD_ID);
        if constexpr (!std::is_abstract_v<elem_t>)
            return (default_instance);
        else
        {
            static char dummy_buffer[sizeof(elem_t)] = {0};
            return (*reinterpret_cast<elem_t*>(dummy_buffer));
        }
    }
    elem_t& ref = std::get<I>(*this->_data);
    this->set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
    return (ref);
}

template <typename... Types>
template <std::size_t I>
const typename std::tuple_element<I, typename ft_tuple<Types...>::tuple_t>::type&
ft_tuple<Types...>::get() const
{
    using elem_t = typename std::tuple_element<I, tuple_t>::type;
    static elem_t default_instance = elem_t();
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        const_cast<ft_tuple*>(this)->set_error(PT_ERR_MUTEX_OWNER);
        return (default_instance);
    }
    if (this->_data == ft_nullptr)
    {
        const_cast<ft_tuple*>(this)->set_error(TUPLE_BAD_ACCESS);
        this->_mutex.unlock(THREAD_ID);
        if constexpr (!std::is_abstract_v<elem_t>)
            return (default_instance);
        else
        {
            static char dummy_buffer[sizeof(elem_t)] = {0};
            return (*reinterpret_cast<elem_t*>(dummy_buffer));
        }
    }
    const elem_t& ref = std::get<I>(*this->_data);
    this->set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
    return (ref);
}



template <typename... Types>
template <typename T>
T& ft_tuple<Types...>::get()
{
    static T default_instance = T();
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        this->set_error(PT_ERR_MUTEX_OWNER);
        return (default_instance);
    }
    if (this->_data == ft_nullptr)
    {
        this->set_error(TUPLE_BAD_ACCESS);
        this->_mutex.unlock(THREAD_ID);
        if constexpr (!std::is_abstract_v<T>)
            return (default_instance);
        else
        {
            static char dummy_buffer[sizeof(T)] = {0};
            return (*reinterpret_cast<T*>(dummy_buffer));
        }
    }
    T& ref = std::get<T>(*this->_data);
    this->set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
    return (ref);
}

template <typename... Types>
template <typename T>
const T& ft_tuple<Types...>::get() const
{
    static T default_instance = T();
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        const_cast<ft_tuple*>(this)->set_error(PT_ERR_MUTEX_OWNER);
        return (default_instance);
    }
    if (this->_data == ft_nullptr)
    {
        const_cast<ft_tuple*>(this)->set_error(TUPLE_BAD_ACCESS);
        this->_mutex.unlock(THREAD_ID);
        if constexpr (!std::is_abstract_v<T>)
            return (default_instance);
        else
        {
            static char dummy_buffer[sizeof(T)] = {0};
            return (*reinterpret_cast<T*>(dummy_buffer));
        }
    }
    const T& ref = std::get<T>(*this->_data);
    this->set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
    return (ref);
}



template <typename... Types>
void ft_tuple<Types...>::reset()
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        this->set_error(PT_ERR_MUTEX_OWNER);
        return ;
    }
    if (this->_data != ft_nullptr)
    {
        destroy_at(this->_data);
        cma_free(this->_data);
        this->_data = ft_nullptr;
    }
    this->set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
    return ;
}



template <typename... Types>
int ft_tuple<Types...>::get_error() const
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
        return (this->_error_code);
    int err = this->_error_code;
    this->_mutex.unlock(THREAD_ID);
    return (err);
}

template <typename... Types>
const char* ft_tuple<Types...>::get_error_str() const
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
        return (ft_strerror(this->_error_code));
    int err = this->_error_code;
    this->_mutex.unlock(THREAD_ID);
    return (ft_strerror(err));
}

#endif 
