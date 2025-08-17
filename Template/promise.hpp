#ifndef FT_PROMISE_HPP
#define FT_PROMISE_HPP

#include "../Errno/errno.hpp"
#include <atomic>
#include <utility>

// Simple promise-like container for asynchronous value sharing
// stores a value of type ValueType and an atomic flag indicating readiness

template <typename ValueType>
class ft_promise
{
private:
    ValueType _value;
    std::atomic_bool _ready;
    mutable int _errorCode;

    void setError(int error) const
    {
        _errorCode = error;
        ft_errno = error;
    }

public:
    ft_promise() : _value(), _ready(false), _errorCode(ER_SUCCESS) {}

    void set_value(const ValueType& value)
    {
        _value = value;
        _ready.store(true, std::memory_order_release);
    }

    void set_value(ValueType&& value)
    {
        _value = std::move(value);
        _ready.store(true, std::memory_order_release);
    }

    ValueType get() const
    {
        if (!_ready.load(std::memory_order_acquire))
        {
            setError(FT_EINVAL);
            return (ValueType());
        }
        return (_value);
    }

    bool is_ready() const
    {
        return (_ready.load(std::memory_order_acquire));
    }

    int get_error() const
    {
        return (_errorCode);
    }

    const char* get_error_str() const
    {
        return (ft_strerror(_errorCode));
    }
};

#endif // FT_PROMISE_HPP
