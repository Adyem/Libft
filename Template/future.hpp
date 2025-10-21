#ifndef FT_FUTURE_HPP
#define FT_FUTURE_HPP

#include "promise.hpp"
#include "shared_ptr.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/pthread.hpp"
#include <chrono>
#include <utility>

#include "move.hpp"
template <typename ValueType>
class ft_future
{
    private:
        ft_promise<ValueType>* _promise;
        ft_sharedptr<ft_promise<ValueType> > _shared_promise;
        mutable int _error_code;

        void set_error(int error) const;
        bool wait_ready() const;

    public:
        ft_future();
        ft_future(const ft_future &other);
        ft_future(ft_future &&other);
        explicit ft_future(ft_promise<ValueType>& promise);
        explicit ft_future(ft_sharedptr<ft_promise<ValueType> > promise_pointer);
        ~ft_future();
        ft_future &operator=(const ft_future &other);
        ft_future &operator=(ft_future &&other);
        ValueType get() const;
        void wait() const;
        bool valid() const;
        int get_error() const;
        const char* get_error_str() const;
};

template <>
class ft_future<void>
{
    private:
        ft_promise<void>* _promise;
        ft_sharedptr<ft_promise<void> > _shared_promise;
        mutable int _error_code;

        void set_error(int error) const;
        bool wait_ready() const;

    public:
        ft_future();
        ft_future(const ft_future &other);
        ft_future(ft_future &&other);
        explicit ft_future(ft_promise<void>& promise);
        explicit ft_future(ft_sharedptr<ft_promise<void> > promise_pointer);
        ~ft_future();
        ft_future &operator=(const ft_future &other);
        ft_future &operator=(ft_future &&other);
        void get() const;
        void wait() const;
        bool valid() const;
        int get_error() const;
        const char* get_error_str() const;
};

template <typename ValueType>
void ft_future<ValueType>::set_error(int error) const
{
    this->_error_code = error;
    ft_errno = error;
    return ;
}

template <typename ValueType>
ft_future<ValueType>::ft_future()
    : _promise(ft_nullptr), _shared_promise(), _error_code(ER_SUCCESS)
{
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ValueType>
ft_future<ValueType>::ft_future(const ft_future<ValueType> &other)
    : _promise(other._promise), _shared_promise(other._shared_promise), _error_code(other._error_code)
{
    this->set_error(this->_error_code);
    return ;
}

template <typename ValueType>
ft_future<ValueType>::ft_future(ft_future<ValueType> &&other)
    : _promise(other._promise), _shared_promise(ft_move(other._shared_promise)), _error_code(other._error_code)
{
    other._promise = ft_nullptr;
    other.set_error(ER_SUCCESS);
    this->set_error(this->_error_code);
    return ;
}

template <typename ValueType>
ft_future<ValueType>::ft_future(ft_promise<ValueType>& promise)
    : _promise(&promise), _shared_promise(), _error_code(ER_SUCCESS)
{
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ValueType>
ft_future<ValueType>::ft_future(ft_sharedptr<ft_promise<ValueType> > promise_pointer)
    : _promise(promise_pointer.get()), _shared_promise(promise_pointer), _error_code(ER_SUCCESS)
{
    if (!this->_promise)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return ;
    }
    if (this->_shared_promise.hasError())
    {
        this->set_error(this->_shared_promise.get_error());
        return ;
    }
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ValueType>
ft_future<ValueType>::~ft_future()
{
    this->_promise = ft_nullptr;
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ValueType>
ft_future<ValueType> &ft_future<ValueType>::operator=(const ft_future<ValueType> &other)
{
    if (this == &other)
    {
        this->set_error(ER_SUCCESS);
        return (*this);
    }
    this->_promise = other._promise;
    this->_shared_promise = other._shared_promise;
    this->set_error(other._error_code);
    return (*this);
}

template <typename ValueType>
ft_future<ValueType> &ft_future<ValueType>::operator=(ft_future<ValueType> &&other)
{
    if (this == &other)
    {
        this->set_error(ER_SUCCESS);
        return (*this);
    }
    this->_promise = other._promise;
    this->_shared_promise = ft_move(other._shared_promise);
    this->set_error(other._error_code);
    other._promise = ft_nullptr;
    other.set_error(ER_SUCCESS);
    return (*this);
}

template <typename ValueType>
bool ft_future<ValueType>::wait_ready() const
{
    using namespace std::chrono;
    const auto start = steady_clock::now();

    while (!this->_promise->is_ready())
    {
        if (steady_clock::now() - start > seconds(1))
        {
            this->set_error(FT_ERR_BROKEN_PROMISE);
            return (false);
        }
        pt_thread_yield();
    }
    this->set_error(ER_SUCCESS);
    return (true);
}

template <typename ValueType>
ValueType ft_future<ValueType>::get() const
{
    if (!this->_promise)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return (ValueType());
    }
    if (!this->wait_ready())
    {
        return (ValueType());
    }
    ValueType value;

    value = this->_promise->get();
    this->set_error(ER_SUCCESS);
    return (value);
}

template <typename ValueType>
void ft_future<ValueType>::wait() const
{
    if (!this->_promise)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return ;
    }
    (void)this->wait_ready();
    return ;
}

template <typename ValueType>
bool ft_future<ValueType>::valid() const
{
    bool is_valid;

    is_valid = (this->_promise != ft_nullptr);
    this->set_error(ER_SUCCESS);
    return (is_valid);
}

template <typename ValueType>
int ft_future<ValueType>::get_error() const
{
    int error;

    error = this->_error_code;
    this->set_error(error);
    return (error);
}

template <typename ValueType>
const char* ft_future<ValueType>::get_error_str() const
{
    const char* error_string;

    error_string = ft_strerror(this->_error_code);
    this->set_error(this->_error_code);
    return (error_string);
}

inline void ft_future<void>::set_error(int error) const
{
    this->_error_code = error;
    ft_errno = error;
    return ;
}

inline ft_future<void>::ft_future()
    : _promise(ft_nullptr), _shared_promise(), _error_code(ER_SUCCESS)
{
    this->set_error(ER_SUCCESS);
    return ;
}

inline ft_future<void>::ft_future(const ft_future<void> &other)
    : _promise(other._promise), _shared_promise(other._shared_promise), _error_code(other._error_code)
{
    this->set_error(this->_error_code);
    return ;
}

inline ft_future<void>::ft_future(ft_future<void> &&other)
    : _promise(other._promise), _shared_promise(ft_move(other._shared_promise)), _error_code(other._error_code)
{
    other._promise = ft_nullptr;
    other.set_error(ER_SUCCESS);
    this->set_error(this->_error_code);
    return ;
}

inline ft_future<void>::ft_future(ft_promise<void>& promise)
    : _promise(&promise), _shared_promise(), _error_code(ER_SUCCESS)
{
    this->set_error(ER_SUCCESS);
    return ;
}

inline ft_future<void>::ft_future(ft_sharedptr<ft_promise<void> > promise_pointer)
    : _promise(promise_pointer.get()), _shared_promise(promise_pointer), _error_code(ER_SUCCESS)
{
    if (!this->_promise)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return ;
    }
    if (this->_shared_promise.hasError())
    {
        this->set_error(this->_shared_promise.get_error());
        return ;
    }
    this->set_error(ER_SUCCESS);
    return ;
}

inline ft_future<void>::~ft_future()
{
    this->_promise = ft_nullptr;
    this->set_error(ER_SUCCESS);
    return ;
}

inline ft_future<void> &ft_future<void>::operator=(const ft_future<void> &other)
{
    if (this == &other)
    {
        this->set_error(ER_SUCCESS);
        return (*this);
    }
    this->_promise = other._promise;
    this->_shared_promise = other._shared_promise;
    this->set_error(other._error_code);
    return (*this);
}

inline ft_future<void> &ft_future<void>::operator=(ft_future<void> &&other)
{
    if (this == &other)
    {
        this->set_error(ER_SUCCESS);
        return (*this);
    }
    this->_promise = other._promise;
    this->_shared_promise = ft_move(other._shared_promise);
    this->set_error(other._error_code);
    other._promise = ft_nullptr;
    other.set_error(ER_SUCCESS);
    return (*this);
}

inline bool ft_future<void>::wait_ready() const
{
    using namespace std::chrono;
    const auto start = steady_clock::now();

    while (!this->_promise->is_ready())
    {
        if (steady_clock::now() - start > seconds(1))
        {
            this->set_error(FT_ERR_BROKEN_PROMISE);
            return (false);
        }
        pt_thread_yield();
    }
    this->set_error(ER_SUCCESS);
    return (true);
}

inline void ft_future<void>::get() const
{
    if (!this->_promise)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return ;
    }
    if (!this->wait_ready())
    {
        return ;
    }
    this->set_error(ER_SUCCESS);
    return ;
}

inline void ft_future<void>::wait() const
{
    if (!this->_promise)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return ;
    }
    (void)this->wait_ready();
    return ;
}

inline bool ft_future<void>::valid() const
{
    bool is_valid;

    is_valid = (this->_promise != ft_nullptr);
    this->set_error(ER_SUCCESS);
    return (is_valid);
}

inline int ft_future<void>::get_error() const
{
    int error;

    error = this->_error_code;
    this->set_error(error);
    return (error);
}

inline const char *ft_future<void>::get_error_str() const
{
    const char *error_string;

    error_string = ft_strerror(this->_error_code);
    this->set_error(this->_error_code);
    return (error_string);
}

#endif
