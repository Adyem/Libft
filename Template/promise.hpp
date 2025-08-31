#ifndef FT_PROMISE_HPP
#define FT_PROMISE_HPP

#include "../Errno/errno.hpp"
#include <atomic>
#include <utility>

template <typename ValueType>
class ft_promise
{
private:
    ValueType _value;
    std::atomic_bool _ready;
    mutable int _errorCode;

    void setError(int error) const
    {
        this->_errorCode = error;
        ft_errno = error;
    }

public:
    ft_promise() : _value(), _ready(false), _errorCode(ER_SUCCESS) {}

    void set_value(const ValueType& value)
    {
        this->_value = value;
        this->_ready.store(true, std::memory_order_release);
    }

    void set_value(ValueType&& value)
    {
        this->_value = std::move(value);
        this->_ready.store(true, std::memory_order_release);
    }

    ValueType get() const
    {
        if (!this->_ready.load(std::memory_order_acquire))
        {
            this->setError(FT_EINVAL);
            return (ValueType());
        }
        return (this->_value);
    }

    bool is_ready() const
    {
        return (this->_ready.load(std::memory_order_acquire));
    }

    int get_error() const
    {
        return (this->_errorCode);
    }

    const char* get_error_str() const
    {
        return (ft_strerror(this->_errorCode));
    }
};

#endif
