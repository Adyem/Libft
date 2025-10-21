#ifndef FT_OPTIONAL_HPP
#define FT_OPTIONAL_HPP

#include "constructor.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Libft/libft.hpp"
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

        void set_error(int error) const;
        static T &fallback_reference() noexcept;

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
    : _value(ft_nullptr), _error_code(ER_SUCCESS)
{
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename T>
ft_optional<T>::ft_optional(const T& value)
    : _value(ft_nullptr), _error_code(ER_SUCCESS)
{
    _value = static_cast<T*>(cma_malloc(sizeof(T)));
    if (_value == ft_nullptr)
        this->set_error(FT_ERR_NO_MEMORY);
    else
    {
        construct_at(_value, value);
        this->set_error(ER_SUCCESS);
    }
    return ;
}

template <typename T>
ft_optional<T>::ft_optional(T&& value)
    : _value(ft_nullptr), _error_code(ER_SUCCESS)
{
    _value = static_cast<T*>(cma_malloc(sizeof(T)));
    if (_value == ft_nullptr)
        this->set_error(FT_ERR_NO_MEMORY);
    else
    {
        construct_at(_value, ft_move(value));
        this->set_error(ER_SUCCESS);
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
    : _value(other._value), _error_code(other._error_code)
{
    other._value = ft_nullptr;
    other._error_code = ER_SUCCESS;
    this->set_error(this->_error_code);
    return ;
}

template <typename T>
ft_optional<T>& ft_optional<T>::operator=(ft_optional&& other) noexcept
{
    if (this != &other)
    {
        this->reset();
        this->_value = other._value;
        this->_error_code = other._error_code;
        other._value = ft_nullptr;
        other._error_code = ER_SUCCESS;
    }
    this->set_error(this->_error_code);
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
    bool result = (this->_value != ft_nullptr);
    const_cast<ft_optional*>(this)->set_error(ER_SUCCESS);
    return (result);
}

template <typename T>
T& ft_optional<T>::value()
{
    if (this->_value == ft_nullptr)
    {
        this->set_error(FT_ERR_EMPTY);
        return (ft_optional<T>::fallback_reference());
    }
    T& reference = *this->_value;
    this->set_error(ER_SUCCESS);
    return (reference);
}

template <typename T>
const T& ft_optional<T>::value() const
{
    if (this->_value == ft_nullptr)
    {
        const_cast<ft_optional*>(this)->set_error(FT_ERR_EMPTY);
        return (ft_optional<T>::fallback_reference());
    }
    const T& reference = *this->_value;
    const_cast<ft_optional*>(this)->set_error(ER_SUCCESS);
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
void ft_optional<T>::reset()
{
    if (this->_value != ft_nullptr)
    {
        destroy_at(this->_value);
        cma_free(this->_value);
        this->_value = ft_nullptr;
    }
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename T>
int ft_optional<T>::get_error() const
{
    return (this->_error_code);
}

template <typename T>
const char* ft_optional<T>::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}

#endif 
