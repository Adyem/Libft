#ifndef FT_PROMISE_HPP
#define FT_PROMISE_HPP

#include "../Errno/errno.hpp"
#include "atomic.hpp"
#include <utility>
#include "move.hpp"

template <typename ValueType>
class ft_promise
{
    private:
        ValueType _value;
        ft_atomic<bool> _ready;
        mutable int _error_code;

    protected:
        void set_error(int error) const;

    public:
        ft_promise();

        void set_value(const ValueType& value);
        void set_value(ValueType&& value);
        ValueType get() const;
        bool is_ready() const;
        int get_error() const;
        const char* get_error_str() const;
};

template <>
class ft_promise<void>
{
    private:
        ft_atomic<bool> _ready;
        mutable int _error_code;

        void set_error(int error) const;

    public:
        ft_promise();

        void set_value();
        void get() const;
        bool is_ready() const;
        int get_error() const;
        const char *get_error_str() const;
};

template <typename ValueType>
void ft_promise<ValueType>::set_error(int error) const
{
    this->_error_code = error;
    ft_errno = error;
    return ;
}

inline void ft_promise<void>::set_error(int error) const
{
    this->_error_code = error;
    ft_errno = error;
    return ;
}

template <typename ValueType>
ft_promise<ValueType>::ft_promise()
    : _value(), _ready(false), _error_code(ER_SUCCESS)
{
    return ;
}

inline ft_promise<void>::ft_promise()
    : _ready(false), _error_code(ER_SUCCESS)
{
    return ;
}

template <typename ValueType>
void ft_promise<ValueType>::set_value(const ValueType& value)
{
    this->_value = value;
    this->_ready.store(true, std::memory_order_release);
    this->set_error(ER_SUCCESS);
    return ;
}

inline void ft_promise<void>::set_value()
{
    this->_ready.store(true, std::memory_order_release);
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ValueType>
void ft_promise<ValueType>::set_value(ValueType&& value)
{
    this->_value = ft_move(value);
    this->_ready.store(true, std::memory_order_release);
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ValueType>
ValueType ft_promise<ValueType>::get() const
{
    if (!this->_ready.load(std::memory_order_acquire))
    {
        this->set_error(FT_EINVAL);
        return (ValueType());
    }
    return (this->_value);
}

inline void ft_promise<void>::get() const
{
    if (!this->_ready.load(std::memory_order_acquire))
    {
        const_cast<ft_promise<void> *>(this)->set_error(FT_EINVAL);
        return ;
    }
    return ;
}

template <typename ValueType>
bool ft_promise<ValueType>::is_ready() const
{
    return (this->_ready.load(std::memory_order_acquire));
}

inline bool ft_promise<void>::is_ready() const
{
    return (this->_ready.load(std::memory_order_acquire));
}

template <typename ValueType>
int ft_promise<ValueType>::get_error() const
{
    return (this->_error_code);
}

inline int ft_promise<void>::get_error() const
{
    return (this->_error_code);
}

template <typename ValueType>
const char* ft_promise<ValueType>::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}

inline const char *ft_promise<void>::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}

#endif
