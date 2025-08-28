#ifndef FT_FUTURE_HPP
#define FT_FUTURE_HPP

#include "promise.hpp"
#include "../Errno/errno.hpp"
#include <thread>

// ft_future: waits on an associated ft_promise and retrieves its value
// Provides get, wait, and validity checks with error reporting

template <typename ValueType>
class ft_future
{
private:
    ft_promise<ValueType>* _promise;
    mutable int _errorCode;

    void setError(int error) const
    {
        _errorCode = error;
        ft_errno = error;
    }

public:
    ft_future() : _promise(nullptr), _errorCode(ER_SUCCESS) {}
    explicit ft_future(ft_promise<ValueType>& promise) : _promise(&promise), _errorCode(ER_SUCCESS) {}

    ValueType get() const
    {
        if (!valid())
        {
            setError(FUTURE_INVALID);
            return (ValueType());
        }
        wait();
        return (_promise->get());
    }

    void wait() const
    {
        if (!valid())
        {
            setError(FUTURE_INVALID);
            return;
        }
        while (!_promise->is_ready())
            std::this_thread::yield();
    }

    bool valid() const
    {
        return (_promise != nullptr);
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

#endif
