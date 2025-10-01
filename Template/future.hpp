#ifndef FT_FUTURE_HPP
#define FT_FUTURE_HPP

#include "promise.hpp"
#include "shared_ptr.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/pthread.hpp"
#include <chrono>




template <typename ValueType>
class ft_future
{
    private:
        ft_promise<ValueType>* _promise;
        ft_sharedptr<ft_promise<ValueType> > _shared_promise;
        mutable int _error_code;

        void set_error(int error) const;

    public:
        ft_future();
        explicit ft_future(ft_promise<ValueType>& promise);
        explicit ft_future(ft_sharedptr<ft_promise<ValueType> > promise_pointer);
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

    public:
        ft_future();
        explicit ft_future(ft_promise<void>& promise);
        explicit ft_future(ft_sharedptr<ft_promise<void> > promise_pointer);
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
        this->set_error(FUTURE_INVALID);
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
ValueType ft_future<ValueType>::get() const
{
    if (!this->valid())
    {
        this->set_error(FUTURE_INVALID);
        return (ValueType());
    }
    this->wait();
    if (this->_error_code != ER_SUCCESS)
        return (ValueType());
    ValueType value;

    value = this->_promise->get();
    this->set_error(ER_SUCCESS);
    return (value);
}

template <typename ValueType>
void ft_future<ValueType>::wait() const
{
    if (!this->valid())
    {
        this->set_error(FUTURE_INVALID);
        return ;
    }
    using namespace std::chrono;
    const auto start = steady_clock::now();
    while (!this->_promise->is_ready())
    {
        if (steady_clock::now() - start > seconds(1))
        {
            this->set_error(FUTURE_BROKEN);
            return ;
        }
        pt_thread_yield();
    }
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ValueType>
bool ft_future<ValueType>::valid() const
{
    const_cast<ft_future<ValueType> *>(this)->set_error(ER_SUCCESS);
    return (this->_promise != ft_nullptr);
}

template <typename ValueType>
int ft_future<ValueType>::get_error() const
{
    return (this->_error_code);
}

template <typename ValueType>
const char* ft_future<ValueType>::get_error_str() const
{
    return (ft_strerror(this->_error_code));
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
        this->set_error(FUTURE_INVALID);
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

inline void ft_future<void>::get() const
{
    if (!this->valid())
    {
        this->set_error(FUTURE_INVALID);
        return ;
    }
    this->wait();
    if (this->_error_code != ER_SUCCESS)
        return ;
    this->set_error(ER_SUCCESS);
    return ;
}

inline void ft_future<void>::wait() const
{
    if (!this->valid())
    {
        this->set_error(FUTURE_INVALID);
        return ;
    }
    using namespace std::chrono;
    const auto start = steady_clock::now();
    while (!this->_promise->is_ready())
    {
        if (steady_clock::now() - start > seconds(1))
        {
            this->set_error(FUTURE_BROKEN);
            return ;
        }
        pt_thread_yield();
    }
    this->set_error(ER_SUCCESS);
    return ;
}

inline bool ft_future<void>::valid() const
{
    const_cast<ft_future<void> *>(this)->set_error(ER_SUCCESS);
    return (this->_promise != ft_nullptr);
}

inline int ft_future<void>::get_error() const
{
    return (this->_error_code);
}

inline const char *ft_future<void>::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}

#endif
