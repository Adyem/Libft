#ifndef FT_OPTIONAL_HPP
#define FT_OPTIONAL_HPP

#include "constructor.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/nullptr.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/mutex.hpp"
#include <utility>
#include <type_traits>
#include <new>

/*
 * ft_optional
 * Represents an optional value with thread-safe access and error reporting.
 */

template <typename T>
class ft_optional
{
    private:
        T*          _value;
        mutable int _errorCode;
        mutable pt_mutex _mutex;

        void setError(int error) const;

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
    : _value(ft_nullptr), _errorCode(ER_SUCCESS)
{
    return ;
}

template <typename T>
ft_optional<T>::ft_optional(const T& value)
    : _value(ft_nullptr), _errorCode(ER_SUCCESS)
{
    _value = static_cast<T*>(cma_malloc(sizeof(T)));
    if (_value == ft_nullptr)
        this->setError(OPTIONAL_ALLOC_FAIL);
    else
        construct_at(_value, value);
    return ;
}

template <typename T>
ft_optional<T>::ft_optional(T&& value)
    : _value(ft_nullptr), _errorCode(ER_SUCCESS)
{
    _value = static_cast<T*>(cma_malloc(sizeof(T)));
    if (_value == ft_nullptr)
        this->setError(OPTIONAL_ALLOC_FAIL);
    else
        construct_at(_value, std::move(value));
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
    : _value(other._value), _errorCode(other._errorCode)
{
    other._value = ft_nullptr;
    other._errorCode = ER_SUCCESS;
    return ;
}

template <typename T>
ft_optional<T>& ft_optional<T>::operator=(ft_optional&& other) noexcept
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
        this->_value = other._value;
        this->_errorCode = other._errorCode;
        other._value = ft_nullptr;
        other._errorCode = ER_SUCCESS;
        other._mutex.unlock(THREAD_ID);
        this->_mutex.unlock(THREAD_ID);
    }
    return (*this);
}

template <typename T>
void ft_optional<T>::setError(int error) const
{
    this->_errorCode = error;
    ft_errno = error;
    return ;
}

template <typename T>
bool ft_optional<T>::has_value() const
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        const_cast<ft_optional*>(this)->setError(PT_ERR_MUTEX_OWNER);
        return (false);
    }
    bool result = (this->_value != ft_nullptr);
    this->_mutex.unlock(THREAD_ID);
    return (result);
}

template <typename T>
T& ft_optional<T>::value()
{
    static T defaultInstance = T();
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->setError(PT_ERR_MUTEX_OWNER);
        return (defaultInstance);
    }
    if (this->_value == ft_nullptr)
    {
        this->setError(OPTIONAL_EMPTY);
        this->_mutex.unlock(THREAD_ID);
        if constexpr (!std::is_abstract_v<T>)
            return (defaultInstance);
        else
        {
            static char dummy_buffer[sizeof(T)] = {0};
            return (*reinterpret_cast<T*>(dummy_buffer));
        }
    }
    T& ref = *this->_value;
    this->_mutex.unlock(THREAD_ID);
    return (ref);
}

template <typename T>
const T& ft_optional<T>::value() const
{
    static T defaultInstance = T();
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        const_cast<ft_optional*>(this)->setError(PT_ERR_MUTEX_OWNER);
        return (defaultInstance);
    }
    if (this->_value == ft_nullptr)
    {
        const_cast<ft_optional*>(this)->setError(OPTIONAL_EMPTY);
        this->_mutex.unlock(THREAD_ID);
        if constexpr (!std::is_abstract_v<T>)
            return (defaultInstance);
        else
        {
            static char dummy_buffer[sizeof(T)] = {0};
            return (*reinterpret_cast<T*>(dummy_buffer));
        }
    }
    const T& ref = *this->_value;
    this->_mutex.unlock(THREAD_ID);
    return (ref);
}

template <typename T>
void ft_optional<T>::reset()
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->setError(PT_ERR_MUTEX_OWNER);
        return ;
    }
    if (this->_value != ft_nullptr)
    {
        destroy_at(this->_value);
        cma_free(this->_value);
        this->_value = ft_nullptr;
    }
    this->_mutex.unlock(THREAD_ID);
    return ;
}

template <typename T>
int ft_optional<T>::get_error() const
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (this->_errorCode);
    int err = this->_errorCode;
    this->_mutex.unlock(THREAD_ID);
    return (err);
}

template <typename T>
const char* ft_optional<T>::get_error_str() const
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
        return (ft_strerror(this->_errorCode));
    int err = this->_errorCode;
    this->_mutex.unlock(THREAD_ID);
    return (ft_strerror(err));
}

#endif // FT_OPTIONAL_HPP
