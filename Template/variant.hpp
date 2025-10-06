#ifndef FT_VARIANT_HPP
#define FT_VARIANT_HPP

#include "constructor.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/mutex.hpp"
#include <type_traits>
#include <utility>
#include <cstddef>
#include <new>
#include <tuple>



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
        mutable pt_mutex _mutex;

        void set_error(int error) const;
        void destroy();

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
      _index(npos), _error_code(ER_SUCCESS)
{
    if (this->_data == ft_nullptr)
        this->set_error(VARIANT_ALLOC_FAIL);
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
    this->emplace<T>(std::move(value));
    return ;
}

template <typename... Types>
ft_variant<Types...>::~ft_variant()
{
    this->destroy();
    if (this->_data != ft_nullptr)
        cma_free(this->_data);
    return ;
}

template <typename... Types>
ft_variant<Types...>::ft_variant(ft_variant&& other) noexcept
    : _data(other._data), _index(other._index), _error_code(other._error_code)
{
    other._data = ft_nullptr;
    other._index = npos;
    other._error_code = ER_SUCCESS;
    return ;
}

template <typename... Types>
ft_variant<Types...>& ft_variant<Types...>::operator=(ft_variant&& other) noexcept
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
        this->destroy();
        if (this->_data != ft_nullptr)
            cma_free(this->_data);
        this->_data = other._data;
        this->_index = other._index;
        this->_error_code = other._error_code;
        other._data = ft_nullptr;
        other._index = npos;
        other._error_code = ER_SUCCESS;
        other._mutex.unlock(THREAD_ID);
        this->_mutex.unlock(THREAD_ID);
    }
    return (*this);
}

template <typename... Types>
void ft_variant<Types...>::destroy()
{
    if (this->_data == ft_nullptr || this->_index == npos)
        return ;
    variant_destroyer<0, Types...>::destroy(this->_index, this->_data);
    this->_index = npos;
    return ;
}

template <typename... Types>
template <typename T>
void ft_variant<Types...>::emplace(T&& value)
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        this->set_error(PT_ERR_MUTEX_OWNER);
        return ;
    }
    if (this->_data == ft_nullptr)
    {
        this->_data = static_cast<storage_t*>(cma_malloc(sizeof(storage_t)));
        if (this->_data == ft_nullptr)
        {
            this->set_error(VARIANT_ALLOC_FAIL);
            this->_mutex.unlock(THREAD_ID);
            return ;
        }
    }
    this->destroy();
    construct_at(reinterpret_cast<std::decay_t<T>*>(this->_data), std::forward<T>(value));
    this->_index = variant_index<std::decay_t<T>, Types...>::value;
    this->set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
    return ;
}

template <typename... Types>
template <typename T>
bool ft_variant<Types...>::holds_alternative() const
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        const_cast<ft_variant*>(this)->set_error(PT_ERR_MUTEX_OWNER);
        return (false);
    }
    size_t idx = variant_index<T, Types...>::value;
    bool result = (this->_index == idx);
    this->set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
    return (result);
}

template <typename... Types>
template <typename T>
T& ft_variant<Types...>::get()
{
    static T default_instance = T();
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        this->set_error(PT_ERR_MUTEX_OWNER);
        return (default_instance);
    }
    size_t idx = variant_index<T, Types...>::value;
    if (this->_index != idx)
    {
        this->set_error(VARIANT_BAD_ACCESS);
        this->_mutex.unlock(THREAD_ID);
        return (default_instance);
    }
    T& ref = *reinterpret_cast<T*>(this->_data);
    this->set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
    return (ref);
}

template <typename... Types>
template <typename T>
const T& ft_variant<Types...>::get() const
{
    static T default_instance = T();
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        const_cast<ft_variant*>(this)->set_error(PT_ERR_MUTEX_OWNER);
        return (default_instance);
    }
    size_t idx = variant_index<T, Types...>::value;
    if (this->_index != idx)
    {
        const_cast<ft_variant*>(this)->set_error(VARIANT_BAD_ACCESS);
        this->_mutex.unlock(THREAD_ID);
        return (default_instance);
    }
    const T& ref = *reinterpret_cast<const T*>(this->_data);
    this->set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
    return (ref);
}

template <typename... Types>
template <typename Visitor>
void ft_variant<Types...>::visit(Visitor&& vis)
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        this->set_error(PT_ERR_MUTEX_OWNER);
        return ;
    }
    if (this->_index == npos)
    {
        this->set_error(VARIANT_BAD_ACCESS);
        this->_mutex.unlock(THREAD_ID);
        return ;
    }
    variant_visitor<0, Types...>::apply(this->_index, this->_data, std::forward<Visitor>(vis));
    this->set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
    return ;
}

template <typename... Types>
void ft_variant<Types...>::reset()
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        this->set_error(this->_mutex.get_error());
        return ;
    }
    this->destroy();
    this->set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
    return ;
}

template <typename... Types>
int ft_variant<Types...>::get_error() const
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
        return (this->_error_code);
    int err = this->_error_code;
    this->_mutex.unlock(THREAD_ID);
    return (err);
}

template <typename... Types>
const char* ft_variant<Types...>::get_error_str() const
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
        return (ft_strerror(this->_error_code));
    int err = this->_error_code;
    this->_mutex.unlock(THREAD_ID);
    return (ft_strerror(err));
}

#endif 
