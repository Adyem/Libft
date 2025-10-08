#ifndef FT_PROMISE_HPP
#define FT_PROMISE_HPP

#include "../Errno/errno.hpp"
#include <atomic>
#include "../PThread/pthread.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"
#include <utility>

int pt_thread_yield();

template <typename ValueType>
class ft_promise
{
    private:
        ValueType _value;
        std::atomic<bool> _ready;
        mutable pt_mutex _mutex;
        mutable int _error_code;

    protected:
        void set_error(int error) const;

    public:
        ft_promise();
        ~ft_promise();

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
        std::atomic<bool> _ready;
        mutable pt_mutex _mutex;
        mutable int _error_code;

        void set_error(int error) const;

    public:
        ft_promise();
        ~ft_promise();

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
    : _value(), _ready(false), _mutex(), _error_code(ER_SUCCESS)
{
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ValueType>
ft_promise<ValueType>::~ft_promise()
{
    this->set_error(ER_SUCCESS);
    return ;
}

inline ft_promise<void>::ft_promise()
    : _ready(false), _mutex(), _error_code(ER_SUCCESS)
{
    this->set_error(ER_SUCCESS);
    return ;
}

inline ft_promise<void>::~ft_promise()
{
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ValueType>
void ft_promise<ValueType>::set_value(const ValueType& value)
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_value = value;
    this->_ready.store(true, std::memory_order_release);
    this->set_error(ER_SUCCESS);
    return ;
}

inline void ft_promise<void>::set_value()
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_ready.store(true, std::memory_order_release);
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ValueType>
void ft_promise<ValueType>::set_value(ValueType&& value)
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_value = std::move(value);
    this->_ready.store(true, std::memory_order_release);
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ValueType>
ValueType ft_promise<ValueType>::get() const
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return (ValueType());
    }
    if (!this->_ready.load(std::memory_order_acquire))
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (ValueType());
    }
    ValueType value_copy(this->_value);
    this->set_error(ER_SUCCESS);
    return (value_copy);
}

inline void ft_promise<void>::get() const
{
    while (true)
    {
        ft_unique_lock<pt_mutex> guard(this->_mutex);
        if (guard.get_error() != ER_SUCCESS)
        {
            this->set_error(guard.get_error());
            return ;
        }
        if (this->_ready.load(std::memory_order_acquire))
        {
            this->set_error(ER_SUCCESS);
            return ;
        }
        this->set_error(ER_SUCCESS);
        guard.unlock();
        if (guard.get_error() != ER_SUCCESS)
        {
            this->set_error(guard.get_error());
            return ;
        }
        pt_thread_yield();
    }
}

template <typename ValueType>
bool ft_promise<ValueType>::is_ready() const
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return (false);
    }
    bool ready = this->_ready.load(std::memory_order_acquire);
    this->set_error(ER_SUCCESS);
    return (ready);
}

inline bool ft_promise<void>::is_ready() const
{
    while (true)
    {
        ft_unique_lock<pt_mutex> guard(this->_mutex);
        if (guard.get_error() != ER_SUCCESS)
        {
            this->set_error(guard.get_error());
            return (false);
        }
        if (this->_ready.load(std::memory_order_acquire))
        {
            this->set_error(ER_SUCCESS);
            return (true);
        }
        this->set_error(ER_SUCCESS);
        guard.unlock();
        if (guard.get_error() != ER_SUCCESS)
        {
            this->set_error(guard.get_error());
            return (false);
        }
        pt_thread_yield();
    }
}

template <typename ValueType>
int ft_promise<ValueType>::get_error() const
{
    const_cast<ft_promise<ValueType> *>(this)->set_error(this->_error_code);
    return (this->_error_code);
}

inline int ft_promise<void>::get_error() const
{
    const_cast<ft_promise<void> *>(this)->set_error(this->_error_code);
    return (this->_error_code);
}

template <typename ValueType>
const char* ft_promise<ValueType>::get_error_str() const
{
    const_cast<ft_promise<ValueType> *>(this)->set_error(this->_error_code);
    return (ft_strerror(this->_error_code));
}

inline const char *ft_promise<void>::get_error_str() const
{
    const_cast<ft_promise<void> *>(this)->set_error(this->_error_code);
    return (ft_strerror(this->_error_code));
}

#endif
