#ifndef FT_FUTURE_HPP
#define FT_FUTURE_HPP

#include "promise.hpp"
#include "shared_ptr.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"
#include "../PThread/unique_lock.hpp"
#include <chrono>
#include <utility>




template <typename ValueType>
class ft_future
{
    private:
        ft_promise<ValueType>* _promise;
        ft_sharedptr<ft_promise<ValueType> > _shared_promise;
        mutable pt_mutex _mutex;
        mutable int _error_code;

        void set_error(int error) const;
        bool wait_ready_locked(ft_unique_lock<pt_mutex>& guard) const;

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
        mutable pt_mutex _mutex;
        mutable int _error_code;

        void set_error(int error) const;
        bool wait_ready_locked(ft_unique_lock<pt_mutex>& guard) const;

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
    : _promise(ft_nullptr), _shared_promise(), _mutex(), _error_code(ER_SUCCESS)
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->set_error(ER_SUCCESS);
    guard.unlock();
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
    }
    return ;
}

template <typename ValueType>
ft_future<ValueType>::ft_future(const ft_future<ValueType> &other)
    : _promise(ft_nullptr), _shared_promise(), _mutex(), _error_code(ER_SUCCESS)
{
    ft_unique_lock<pt_mutex> guard(other._mutex);

    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_promise = other._promise;
    this->_shared_promise = other._shared_promise;
    this->set_error(other._error_code);
    guard.unlock();
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
    }
    return ;
}

template <typename ValueType>
ft_future<ValueType>::ft_future(ft_future<ValueType> &&other)
    : _promise(ft_nullptr), _shared_promise(), _mutex(), _error_code(ER_SUCCESS)
{
    ft_unique_lock<pt_mutex> guard(other._mutex);

    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_promise = other._promise;
    this->_shared_promise = std::move(other._shared_promise);
    this->set_error(other._error_code);
    other._promise = ft_nullptr;
    other.set_error(ER_SUCCESS);
    guard.unlock();
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
    }
    return ;
}

template <typename ValueType>
ft_future<ValueType>::ft_future(ft_promise<ValueType>& promise)
    : _promise(&promise), _shared_promise(), _mutex(), _error_code(ER_SUCCESS)
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->set_error(ER_SUCCESS);
    guard.unlock();
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
    }
    return ;
}

template <typename ValueType>
ft_future<ValueType>::ft_future(ft_sharedptr<ft_promise<ValueType> > promise_pointer)
    : _promise(promise_pointer.get()), _shared_promise(promise_pointer), _mutex(), _error_code(ER_SUCCESS)
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    if (!this->_promise)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        guard.unlock();
        if (guard.get_error() != ER_SUCCESS)
        {
            this->set_error(guard.get_error());
        }
        return ;
    }
    if (this->_shared_promise.hasError())
    {
        this->set_error(this->_shared_promise.get_error());
        guard.unlock();
        if (guard.get_error() != ER_SUCCESS)
        {
            this->set_error(guard.get_error());
        }
        return ;
    }
    this->set_error(ER_SUCCESS);
    guard.unlock();
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
    }
    return ;
}

template <typename ValueType>
ft_future<ValueType>::~ft_future()
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_promise = ft_nullptr;
    this->set_error(ER_SUCCESS);
    guard.unlock();
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
    }
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
    uintptr_t this_address;
    uintptr_t other_address;
    const ft_future<ValueType> *first;
    const ft_future<ValueType> *second;

    this_address = reinterpret_cast<uintptr_t>(this);
    other_address = reinterpret_cast<uintptr_t>(&other);
    if (this_address < other_address)
    {
        first = this;
        second = &other;
    }
    else
    {
        first = &other;
        second = this;
    }
    ft_unique_lock<pt_mutex> first_guard(first->_mutex);

    if (first_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(first_guard.get_error());
        return (*this);
    }
    ft_unique_lock<pt_mutex> second_guard(second->_mutex);

    if (second_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(second_guard.get_error());
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
    uintptr_t this_address;
    uintptr_t other_address;
    ft_future<ValueType> *first;
    ft_future<ValueType> *second;

    this_address = reinterpret_cast<uintptr_t>(this);
    other_address = reinterpret_cast<uintptr_t>(&other);
    if (this_address < other_address)
    {
        first = this;
        second = &other;
    }
    else
    {
        first = &other;
        second = this;
    }
    ft_unique_lock<pt_mutex> first_guard(first->_mutex);

    if (first_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(first_guard.get_error());
        return (*this);
    }
    ft_unique_lock<pt_mutex> second_guard(second->_mutex);

    if (second_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(second_guard.get_error());
        return (*this);
    }
    this->_promise = other._promise;
    this->_shared_promise = std::move(other._shared_promise);
    this->set_error(other._error_code);
    other._promise = ft_nullptr;
    other.set_error(ER_SUCCESS);
    return (*this);
}

template <typename ValueType>
bool ft_future<ValueType>::wait_ready_locked(ft_unique_lock<pt_mutex>& guard) const
{
    using namespace std::chrono;
    const auto start = steady_clock::now();

    (void)guard;

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
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_future<ValueType> *>(this)->set_error(guard.get_error());
        return (ValueType());
    }
    if (!this->_promise)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        guard.unlock();
        if (guard.get_error() != ER_SUCCESS)
        {
            const_cast<ft_future<ValueType> *>(this)->set_error(guard.get_error());
        }
        return (ValueType());
    }
    if (!this->wait_ready_locked(guard))
    {
        guard.unlock();
        if (guard.get_error() != ER_SUCCESS)
        {
            const_cast<ft_future<ValueType> *>(this)->set_error(guard.get_error());
        }
        return (ValueType());
    }
    ValueType value;

    value = this->_promise->get();
    this->set_error(ER_SUCCESS);
    guard.unlock();
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_future<ValueType> *>(this)->set_error(guard.get_error());
    }
    return (value);
}

template <typename ValueType>
void ft_future<ValueType>::wait() const
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_future<ValueType> *>(this)->set_error(guard.get_error());
        return ;
    }
    if (!this->_promise)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        guard.unlock();
        if (guard.get_error() != ER_SUCCESS)
        {
            const_cast<ft_future<ValueType> *>(this)->set_error(guard.get_error());
        }
        return ;
    }
    if (!this->wait_ready_locked(guard))
    {
        guard.unlock();
        if (guard.get_error() != ER_SUCCESS)
        {
            const_cast<ft_future<ValueType> *>(this)->set_error(guard.get_error());
        }
        return ;
    }
    guard.unlock();
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_future<ValueType> *>(this)->set_error(guard.get_error());
    }
    return ;
}

template <typename ValueType>
bool ft_future<ValueType>::valid() const
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_future<ValueType> *>(this)->set_error(guard.get_error());
        return (false);
    }
    bool is_valid;

    is_valid = (this->_promise != ft_nullptr);
    this->set_error(ER_SUCCESS);
    guard.unlock();
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_future<ValueType> *>(this)->set_error(guard.get_error());
        return (false);
    }
    return (is_valid);
}

template <typename ValueType>
int ft_future<ValueType>::get_error() const
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_future<ValueType> *>(this)->set_error(guard.get_error());
        return (guard.get_error());
    }
    int error;

    error = this->_error_code;
    this->set_error(error);
    guard.unlock();
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_future<ValueType> *>(this)->set_error(guard.get_error());
        return (guard.get_error());
    }
    return (error);
}

template <typename ValueType>
const char* ft_future<ValueType>::get_error_str() const
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_future<ValueType> *>(this)->set_error(guard.get_error());
        return (ft_strerror(guard.get_error()));
    }
    const char* error_string;

    error_string = ft_strerror(this->_error_code);
    this->set_error(this->_error_code);
    guard.unlock();
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_future<ValueType> *>(this)->set_error(guard.get_error());
        return (ft_strerror(guard.get_error()));
    }
    return (error_string);
}

inline void ft_future<void>::set_error(int error) const
{
    this->_error_code = error;
    ft_errno = error;
    return ;
}

inline bool ft_future<void>::wait_ready_locked(ft_unique_lock<pt_mutex>& guard) const
{
    using namespace std::chrono;
    const auto start = steady_clock::now();

    (void)guard;

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

inline ft_future<void>::ft_future()
    : _promise(ft_nullptr), _shared_promise(), _mutex(), _error_code(ER_SUCCESS)
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->set_error(ER_SUCCESS);
    guard.unlock();
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
    }
    return ;
}

inline ft_future<void>::ft_future(const ft_future<void> &other)
    : _promise(ft_nullptr), _shared_promise(), _mutex(), _error_code(ER_SUCCESS)
{
    ft_unique_lock<pt_mutex> guard(other._mutex);

    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_promise = other._promise;
    this->_shared_promise = other._shared_promise;
    this->set_error(other._error_code);
    guard.unlock();
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
    }
    return ;
}

inline ft_future<void>::ft_future(ft_future<void> &&other)
    : _promise(ft_nullptr), _shared_promise(), _mutex(), _error_code(ER_SUCCESS)
{
    ft_unique_lock<pt_mutex> guard(other._mutex);

    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_promise = other._promise;
    this->_shared_promise = std::move(other._shared_promise);
    this->set_error(other._error_code);
    other._promise = ft_nullptr;
    other.set_error(ER_SUCCESS);
    guard.unlock();
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
    }
    return ;
}

inline ft_future<void>::ft_future(ft_promise<void>& promise)
    : _promise(&promise), _shared_promise(), _mutex(), _error_code(ER_SUCCESS)
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->set_error(ER_SUCCESS);
    guard.unlock();
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
    }
    return ;
}

inline ft_future<void>::ft_future(ft_sharedptr<ft_promise<void> > promise_pointer)
    : _promise(promise_pointer.get()), _shared_promise(promise_pointer), _mutex(), _error_code(ER_SUCCESS)
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    if (!this->_promise)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        guard.unlock();
        if (guard.get_error() != ER_SUCCESS)
        {
            this->set_error(guard.get_error());
        }
        return ;
    }
    if (this->_shared_promise.hasError())
    {
        this->set_error(this->_shared_promise.get_error());
        guard.unlock();
        if (guard.get_error() != ER_SUCCESS)
        {
            this->set_error(guard.get_error());
        }
        return ;
    }
    this->set_error(ER_SUCCESS);
    guard.unlock();
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
    }
    return ;
}

inline ft_future<void>::~ft_future()
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_promise = ft_nullptr;
    this->set_error(ER_SUCCESS);
    guard.unlock();
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
    }
    return ;
}

inline ft_future<void> &ft_future<void>::operator=(const ft_future<void> &other)
{
    if (this == &other)
    {
        this->set_error(ER_SUCCESS);
        return (*this);
    }
    uintptr_t this_address;
    uintptr_t other_address;
    const ft_future<void> *first;
    const ft_future<void> *second;

    this_address = reinterpret_cast<uintptr_t>(this);
    other_address = reinterpret_cast<uintptr_t>(&other);
    if (this_address < other_address)
    {
        first = this;
        second = &other;
    }
    else
    {
        first = &other;
        second = this;
    }
    ft_unique_lock<pt_mutex> first_guard(first->_mutex);

    if (first_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(first_guard.get_error());
        return (*this);
    }
    ft_unique_lock<pt_mutex> second_guard(second->_mutex);

    if (second_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(second_guard.get_error());
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
    uintptr_t this_address;
    uintptr_t other_address;
    ft_future<void> *first;
    ft_future<void> *second;

    this_address = reinterpret_cast<uintptr_t>(this);
    other_address = reinterpret_cast<uintptr_t>(&other);
    if (this_address < other_address)
    {
        first = this;
        second = &other;
    }
    else
    {
        first = &other;
        second = this;
    }
    ft_unique_lock<pt_mutex> first_guard(first->_mutex);

    if (first_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(first_guard.get_error());
        return (*this);
    }
    ft_unique_lock<pt_mutex> second_guard(second->_mutex);

    if (second_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(second_guard.get_error());
        return (*this);
    }
    this->_promise = other._promise;
    this->_shared_promise = std::move(other._shared_promise);
    this->set_error(other._error_code);
    other._promise = ft_nullptr;
    other.set_error(ER_SUCCESS);
    return (*this);
}

inline void ft_future<void>::get() const
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_future<void> *>(this)->set_error(guard.get_error());
        return ;
    }
    if (!this->_promise)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        guard.unlock();
        if (guard.get_error() != ER_SUCCESS)
        {
            const_cast<ft_future<void> *>(this)->set_error(guard.get_error());
        }
        return ;
    }
    if (!this->wait_ready_locked(guard))
    {
        guard.unlock();
        if (guard.get_error() != ER_SUCCESS)
        {
            const_cast<ft_future<void> *>(this)->set_error(guard.get_error());
        }
        return ;
    }
    this->set_error(ER_SUCCESS);
    guard.unlock();
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_future<void> *>(this)->set_error(guard.get_error());
    }
    return ;
}

inline void ft_future<void>::wait() const
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_future<void> *>(this)->set_error(guard.get_error());
        return ;
    }
    if (!this->_promise)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        guard.unlock();
        if (guard.get_error() != ER_SUCCESS)
        {
            const_cast<ft_future<void> *>(this)->set_error(guard.get_error());
        }
        return ;
    }
    if (!this->wait_ready_locked(guard))
    {
        guard.unlock();
        if (guard.get_error() != ER_SUCCESS)
        {
            const_cast<ft_future<void> *>(this)->set_error(guard.get_error());
        }
        return ;
    }
    guard.unlock();
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_future<void> *>(this)->set_error(guard.get_error());
    }
    return ;
}

inline bool ft_future<void>::valid() const
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_future<void> *>(this)->set_error(guard.get_error());
        return (false);
    }
    bool is_valid;

    is_valid = (this->_promise != ft_nullptr);
    this->set_error(ER_SUCCESS);
    guard.unlock();
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_future<void> *>(this)->set_error(guard.get_error());
        return (false);
    }
    return (is_valid);
}

inline int ft_future<void>::get_error() const
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_future<void> *>(this)->set_error(guard.get_error());
        return (guard.get_error());
    }
    int error;

    error = this->_error_code;
    this->set_error(error);
    guard.unlock();
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_future<void> *>(this)->set_error(guard.get_error());
        return (guard.get_error());
    }
    return (error);
}

inline const char *ft_future<void>::get_error_str() const
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_future<void> *>(this)->set_error(guard.get_error());
        return (ft_strerror(guard.get_error()));
    }
    const char *error_string;

    error_string = ft_strerror(this->_error_code);
    this->set_error(this->_error_code);
    guard.unlock();
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_future<void> *>(this)->set_error(guard.get_error());
        return (ft_strerror(guard.get_error()));
    }
    return (error_string);
}

#endif
