#ifndef FT_FUTURE_HPP
#define FT_FUTURE_HPP

#include "promise.hpp"
#include "../Errno/errno.hpp"
#include <thread>
#include <chrono>

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
        this->_errorCode = error;
        ft_errno = error;
    }

public:
    ft_future() : _promise(nullptr), _errorCode(ER_SUCCESS) {}
    explicit ft_future(ft_promise<ValueType>& promise) : _promise(&promise), _errorCode(ER_SUCCESS) {}

    ValueType get() const
    {
        if (!this->valid())
        {
            this->setError(FUTURE_INVALID);
            return (ValueType());
        }
        this->wait();
        if (this->_errorCode != ER_SUCCESS)
            return (ValueType());
        return (this->_promise->get());
    }

    void wait() const
    {
        if (!this->valid())
        {
            this->setError(FUTURE_INVALID);
            return;
        }
        using namespace std::chrono;
        const auto start = steady_clock::now();
        while (!this->_promise->is_ready())
        {
            if (steady_clock::now() - start > seconds(1))
            {
                this->setError(FUTURE_BROKEN);
                return;
            }
            std::this_thread::yield();
        }
    }

    bool valid() const
    {
        return (this->_promise != nullptr);
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
