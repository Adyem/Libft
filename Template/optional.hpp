#ifndef FT_OPTIONAL_HPP
#define FT_OPTIONAL_HPP

#include "constructor.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/mutex.hpp"
#include <utility>
#include <type_traits>
#include <new>



template <typename T>
class ft_optional
{
    private:
        T*          _value;
        mutable int _error_code;
        mutable pt_mutex _mutex;

        void set_error(int error) const;

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
        this->set_error(OPTIONAL_ALLOC_FAIL);
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
        this->set_error(OPTIONAL_ALLOC_FAIL);
    else
    {
        construct_at(_value, std::move(value));
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
        if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
        {
            this->set_error(PT_ERR_MUTEX_OWNER);
            return (*this);
        }
        if (other._mutex.lock(THREAD_ID) != FT_SUCCESS)
        {
            this->set_error(PT_ERR_MUTEX_OWNER);
            this->_mutex.unlock(THREAD_ID);
            return (*this);
        }
        this->reset();
        this->_value = other._value;
        this->_error_code = other._error_code;
        other._value = ft_nullptr;
        other._error_code = ER_SUCCESS;
        other._mutex.unlock(THREAD_ID);
        this->_mutex.unlock(THREAD_ID);
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
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        const_cast<ft_optional*>(this)->set_error(PT_ERR_MUTEX_OWNER);
        return (false);
    }
    bool result = (this->_value != ft_nullptr);
    const_cast<ft_optional*>(this)->set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
    return (result);
}

template <typename T>
T& ft_optional<T>::value()
{
    static T default_instance = T();
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        this->set_error(PT_ERR_MUTEX_OWNER);
        return (default_instance);
    }
    if (this->_value == ft_nullptr)
    {
        this->set_error(OPTIONAL_EMPTY);
        this->_mutex.unlock(THREAD_ID);
        if constexpr (!std::is_abstract_v<T>)
            return (default_instance);
        else
        {
            static char dummy_buffer[sizeof(T)] = {0};
            return (*reinterpret_cast<T*>(dummy_buffer));
        }
    }
    T& reference = *this->_value;
    this->set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
    return (reference);
}

template <typename T>
const T& ft_optional<T>::value() const
{
    static T default_instance = T();
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        const_cast<ft_optional*>(this)->set_error(PT_ERR_MUTEX_OWNER);
        return (default_instance);
    }
    if (this->_value == ft_nullptr)
    {
        const_cast<ft_optional*>(this)->set_error(OPTIONAL_EMPTY);
        this->_mutex.unlock(THREAD_ID);
        if constexpr (!std::is_abstract_v<T>)
            return (default_instance);
        else
        {
            static char dummy_buffer[sizeof(T)] = {0};
            return (*reinterpret_cast<T*>(dummy_buffer));
        }
    }
    const T& reference = *this->_value;
    const_cast<ft_optional*>(this)->set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
    return (reference);
}

template <typename T>
void ft_optional<T>::reset()
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        this->set_error(PT_ERR_MUTEX_OWNER);
        return ;
    }
    if (this->_value != ft_nullptr)
    {
        destroy_at(this->_value);
        cma_free(this->_value);
        this->_value = ft_nullptr;
    }
    this->set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
    return ;
}

template <typename T>
int ft_optional<T>::get_error() const
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        const_cast<ft_optional*>(this)->set_error(PT_ERR_MUTEX_OWNER);
        return (this->_error_code);
    }
    int error = this->_error_code;
    const_cast<ft_optional*>(this)->set_error(error);
    this->_mutex.unlock(THREAD_ID);
    return (error);
}

template <typename T>
const char* ft_optional<T>::get_error_str() const
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        const_cast<ft_optional*>(this)->set_error(PT_ERR_MUTEX_OWNER);
        return (ft_strerror(this->_error_code));
    }
    int error = this->_error_code;
    const_cast<ft_optional*>(this)->set_error(error);
    this->_mutex.unlock(THREAD_ID);
    return (ft_strerror(error));
}

#endif 
