#ifndef UNIQUE_LOCK_HPP
#define UNIQUE_LOCK_HPP

#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Errno/errno_internal.hpp"
#include <atomic>
#include "mutex.hpp"

#ifndef PTHREAD_NO_PROMISE
#define UNIQUE_LOCK_DEFINED_PTHREAD_NO_PROMISE
#define PTHREAD_NO_PROMISE
#endif
#include "pthread.hpp"
#ifdef UNIQUE_LOCK_DEFINED_PTHREAD_NO_PROMISE
#undef PTHREAD_NO_PROMISE
#undef UNIQUE_LOCK_DEFINED_PTHREAD_NO_PROMISE
#endif

template <typename MutexType>
class ft_unique_lock
{
    private:
        MutexType *_mutex;
        bool _owns_lock;
        mutable pt_mutex _state_mutex;
        mutable ft_operation_error_stack _operation_errors = {{}, {}, 0};

        void record_error(int error, bool push_global = true) const noexcept;

    public:
        ft_unique_lock();
        explicit ft_unique_lock(MutexType &mutex);
        ~ft_unique_lock();

        ft_unique_lock(const ft_unique_lock &) = delete;
        ft_unique_lock &operator=(const ft_unique_lock &) = delete;

        ft_unique_lock(ft_unique_lock &&other);
        ft_unique_lock &operator=(ft_unique_lock &&other);

        void lock();
        void unlock();

        bool owns_lock() const;
        MutexType *mutex() const;
        int last_operation_error() const noexcept;
        const char *last_operation_error_str() const noexcept;
};

template <typename MutexType>
void ft_unique_lock<MutexType>::record_error(int error, bool push_global) const noexcept
{
    unsigned long long operation_id = 0;

    if (push_global)
        operation_id = ft_global_error_stack_push_entry(error);
    ft_operation_error_stack_push(&this->_operation_errors, error, operation_id);
    return ;
}

template <typename MutexType>
static int ft_unique_lock_pop_mutex_error(MutexType *mutex)
{
    int error_code = mutex->operation_error_pop_newest();

    ft_global_error_stack_pop_newest();
    return (error_code);
}

static int ft_unique_lock_pop_state_mutex_error(pt_mutex &mutex)
{
    int error_code = mutex.operation_error_pop_newest();

    ft_global_error_stack_pop_newest();
    return (error_code);
}

template <typename MutexType>
ft_unique_lock<MutexType>::ft_unique_lock()
    : _mutex(ft_nullptr)
    , _owns_lock(false)
{
    this->record_error(FT_ERR_SUCCESSS, false);
    return ;
}

template <typename MutexType>
ft_unique_lock<MutexType>::ft_unique_lock(MutexType &mutex)
    : _mutex(&mutex)
    , _owns_lock(false)
{
    this->lock();
    return ;
}

template <typename MutexType>
ft_unique_lock<MutexType>::~ft_unique_lock()
{
    bool needs_unlock;
    int state_error;

    needs_unlock = false;
    this->_state_mutex.lock(THREAD_ID);
    state_error = ft_unique_lock_pop_state_mutex_error(this->_state_mutex);
    if (state_error != FT_ERR_SUCCESSS)
    {
        this->record_error(state_error, false);
        return ;
    }
    if (this->_owns_lock && this->_mutex != ft_nullptr)
        needs_unlock = true;
    this->_state_mutex.unlock(THREAD_ID);
    state_error = ft_unique_lock_pop_state_mutex_error(this->_state_mutex);
    if (state_error != FT_ERR_SUCCESSS)
    {
        this->record_error(state_error, false);
        return ;
    }
    if (needs_unlock)
    {
        this->unlock();
        return ;
    }
    this->record_error(FT_ERR_SUCCESSS, false);
    return ;
}

template <typename MutexType>
ft_unique_lock<MutexType>::ft_unique_lock(ft_unique_lock &&other)
    : _mutex(ft_nullptr)
    , _owns_lock(false)
{
    MutexType *moved_mutex;
    bool moved_owns;
    int moved_error;
    int state_error;

    moved_mutex = ft_nullptr;
    moved_owns = false;
    moved_error = FT_ERR_SUCCESSS;
    other._state_mutex.lock(THREAD_ID);
    state_error = ft_unique_lock_pop_state_mutex_error(this->_state_mutex);
    if (state_error != FT_ERR_SUCCESSS)
    {
        this->record_error(state_error, false);
        return ;
    }
    moved_mutex = other._mutex;
    moved_owns = other._owns_lock;
    moved_error = other.last_operation_error();
    other._mutex = ft_nullptr;
    other._owns_lock = false;
    other._state_mutex.unlock(THREAD_ID);
    state_error = ft_unique_lock_pop_state_mutex_error(this->_state_mutex);
    if (state_error != FT_ERR_SUCCESSS)
    {
        this->record_error(state_error, false);
        return ;
    }
    this->_state_mutex.lock(THREAD_ID);
    state_error = ft_unique_lock_pop_state_mutex_error(this->_state_mutex);
    if (state_error != FT_ERR_SUCCESSS)
    {
        this->record_error(state_error, false);
        return ;
    }
    this->_mutex = moved_mutex;
    this->_owns_lock = moved_owns;
    this->_state_mutex.unlock(THREAD_ID);
    state_error = ft_unique_lock_pop_state_mutex_error(this->_state_mutex);
    if (state_error != FT_ERR_SUCCESSS)
    {
        this->record_error(state_error, false);
        return ;
    }
    this->record_error(moved_error, true);
    return ;
}

template <typename MutexType>
ft_unique_lock<MutexType> &ft_unique_lock<MutexType>::operator=(ft_unique_lock &&other)
{
    MutexType *current_mutex;
    bool current_owns;
    MutexType *incoming_mutex;
    bool incoming_owns;
    int incoming_error;
    int state_error;

    if (this == &other)
        return (*this);
    current_mutex = ft_nullptr;
    current_owns = false;
    incoming_mutex = ft_nullptr;
    incoming_owns = false;
    incoming_error = FT_ERR_SUCCESSS;
    other._state_mutex.lock(THREAD_ID);
    state_error = ft_unique_lock_pop_state_mutex_error(this->_state_mutex);
    if (state_error != FT_ERR_SUCCESSS)
    {
        this->record_error(state_error, false);
        return (*this);
    }
    incoming_mutex = other._mutex;
    incoming_owns = other._owns_lock;
    incoming_error = other.last_operation_error();
    other._mutex = ft_nullptr;
    other._owns_lock = false;
    other._state_mutex.unlock(THREAD_ID);
    state_error = ft_unique_lock_pop_state_mutex_error(this->_state_mutex);
    if (state_error != FT_ERR_SUCCESSS)
    {
        this->record_error(state_error, false);
        return (*this);
    }
    this->_state_mutex.lock(THREAD_ID);
    state_error = ft_unique_lock_pop_state_mutex_error(this->_state_mutex);
    if (state_error != FT_ERR_SUCCESSS)
    {
        this->record_error(state_error, false);
        return (*this);
    }
    current_mutex = this->_mutex;
    current_owns = this->_owns_lock;
    if (current_owns)
        this->_owns_lock = false;
    this->_state_mutex.unlock(THREAD_ID);
    state_error = ft_unique_lock_pop_state_mutex_error(this->_state_mutex);
    if (state_error != FT_ERR_SUCCESSS)
    {
        this->record_error(state_error, false);
        return (*this);
    }
    if (current_owns && current_mutex != ft_nullptr)
    {
        current_mutex->unlock(THREAD_ID);
        int unlock_error;

        unlock_error = ft_unique_lock_pop_mutex_error<MutexType>(current_mutex);
        if (unlock_error != FT_ERR_SUCCESSS)
        {
            this->_state_mutex.lock(THREAD_ID);
            state_error = ft_unique_lock_pop_state_mutex_error(this->_state_mutex);
            if (state_error == FT_ERR_SUCCESSS)
            {
                this->_owns_lock = true;
                this->_state_mutex.unlock(THREAD_ID);
                state_error = ft_unique_lock_pop_state_mutex_error(this->_state_mutex);
                if (state_error != FT_ERR_SUCCESSS)
                    unlock_error = state_error;
            }
            else
                unlock_error = state_error;
            this->record_error(unlock_error, false);
            return (*this);
        }
    }
    this->_state_mutex.lock(THREAD_ID);
    state_error = ft_unique_lock_pop_state_mutex_error(this->_state_mutex);
    if (state_error != FT_ERR_SUCCESSS)
    {
        this->record_error(state_error, false);
        return (*this);
    }
    this->_mutex = incoming_mutex;
    this->_owns_lock = incoming_owns;
    this->_state_mutex.unlock(THREAD_ID);
    state_error = ft_unique_lock_pop_state_mutex_error(this->_state_mutex);
    if (state_error != FT_ERR_SUCCESSS)
    {
        this->record_error(state_error, false);
        return (*this);
    }
    this->record_error(incoming_error, true);
    return (*this);
}

template <typename MutexType>
void ft_unique_lock<MutexType>::lock()
{
    MutexType *mutex_pointer;
    bool perform_lock;
    int final_error;
    int state_error;

    mutex_pointer = ft_nullptr;
    perform_lock = false;
    final_error = FT_ERR_SUCCESSS;
    this->_state_mutex.lock(THREAD_ID);
    state_error = ft_unique_lock_pop_state_mutex_error(this->_state_mutex);
    if (state_error != FT_ERR_SUCCESSS)
    {
        this->record_error(state_error, false);
        return ;
    }
    if (this->_mutex == ft_nullptr)
        final_error = FT_ERR_INVALID_ARGUMENT;
    else if (this->_owns_lock)
        final_error = FT_ERR_MUTEX_ALREADY_LOCKED;
    else
    {
        mutex_pointer = this->_mutex;
        perform_lock = true;
    }
    this->_state_mutex.unlock(THREAD_ID);
    state_error = ft_unique_lock_pop_state_mutex_error(this->_state_mutex);
    if (state_error != FT_ERR_SUCCESSS)
    {
        this->record_error(state_error, false);
        return ;
    }
    if (!perform_lock)
    {
        this->record_error(final_error, true);
        return ;
    }
    mutex_pointer->lock(THREAD_ID);
    final_error = ft_unique_lock_pop_mutex_error<MutexType>(mutex_pointer);
    this->_state_mutex.lock(THREAD_ID);
    state_error = ft_unique_lock_pop_state_mutex_error(this->_state_mutex);
    if (state_error != FT_ERR_SUCCESSS)
    {
        this->record_error(state_error, false);
        return ;
    }
    if (final_error == FT_ERR_SUCCESSS)
        this->_owns_lock = true;
    else
        this->_owns_lock = false;
    this->_state_mutex.unlock(THREAD_ID);
    state_error = ft_unique_lock_pop_state_mutex_error(this->_state_mutex);
    if (state_error != FT_ERR_SUCCESSS)
        final_error = state_error;
    this->record_error(final_error, true);
    return ;
}

template <typename MutexType>
void ft_unique_lock<MutexType>::unlock()
{
    MutexType *mutex_pointer;
    bool perform_unlock;
    int final_error;
    int state_error;

    mutex_pointer = ft_nullptr;
    perform_unlock = false;
    final_error = FT_ERR_SUCCESSS;
    this->_state_mutex.lock(THREAD_ID);
    state_error = ft_unique_lock_pop_state_mutex_error(this->_state_mutex);
    if (state_error != FT_ERR_SUCCESSS)
    {
        this->record_error(state_error, false);
        return ;
    }
    if (this->_mutex == ft_nullptr)
        final_error = FT_ERR_INVALID_ARGUMENT;
    else if (!this->_owns_lock)
        final_error = FT_ERR_INVALID_ARGUMENT;
    else
    {
        mutex_pointer = this->_mutex;
        this->_owns_lock = false;
        perform_unlock = true;
    }
    this->_state_mutex.unlock(THREAD_ID);
    state_error = ft_unique_lock_pop_state_mutex_error(this->_state_mutex);
    if (state_error != FT_ERR_SUCCESSS)
    {
        this->record_error(state_error, false);
        return ;
    }
    if (!perform_unlock)
    {
        this->record_error(final_error, true);
        return ;
    }
    mutex_pointer->unlock(THREAD_ID);
    final_error = ft_unique_lock_pop_mutex_error<MutexType>(mutex_pointer);
    if (final_error != FT_ERR_SUCCESSS)
    {
        this->_state_mutex.lock(THREAD_ID);
        state_error = ft_unique_lock_pop_state_mutex_error(this->_state_mutex);
        if (state_error == FT_ERR_SUCCESSS)
        {
            this->_owns_lock = true;
            this->_state_mutex.unlock(THREAD_ID);
            state_error = ft_unique_lock_pop_state_mutex_error(this->_state_mutex);
            if (state_error != FT_ERR_SUCCESSS)
                final_error = state_error;
        }
        else
            final_error = state_error;
        this->record_error(final_error, false);
        return ;
    }
    this->record_error(final_error, true);
    return ;
}

template <typename MutexType>
bool ft_unique_lock<MutexType>::owns_lock() const
{
    bool owns_lock_value;
    int state_error;

    owns_lock_value = false;
    this->_state_mutex.lock(THREAD_ID);
    state_error = ft_unique_lock_pop_state_mutex_error(this->_state_mutex);
    if (state_error != FT_ERR_SUCCESSS)
    {
        this->record_error(state_error, false);
        return (false);
    }
    owns_lock_value = this->_owns_lock;
    this->_state_mutex.unlock(THREAD_ID);
    state_error = ft_unique_lock_pop_state_mutex_error(this->_state_mutex);
    if (state_error != FT_ERR_SUCCESSS)
    {
        this->record_error(state_error, false);
        return (false);
    }
    this->record_error(FT_ERR_SUCCESSS, false);
    return (owns_lock_value);
}

template <typename MutexType>
MutexType *ft_unique_lock<MutexType>::mutex() const
{
    MutexType *mutex_pointer;
    int state_error;

    mutex_pointer = ft_nullptr;
    this->_state_mutex.lock(THREAD_ID);
    state_error = ft_unique_lock_pop_state_mutex_error(this->_state_mutex);
    if (state_error != FT_ERR_SUCCESSS)
    {
        this->record_error(state_error, false);
        return (ft_nullptr);
    }
    mutex_pointer = this->_mutex;
    this->_state_mutex.unlock(THREAD_ID);
    state_error = ft_unique_lock_pop_state_mutex_error(this->_state_mutex);
    if (state_error != FT_ERR_SUCCESSS)
    {
        this->record_error(state_error, false);
        return (ft_nullptr);
    }
    this->record_error(FT_ERR_SUCCESSS, true);
    return (mutex_pointer);
}

template <typename MutexType>
int ft_unique_lock<MutexType>::last_operation_error() const noexcept
{
    return (ft_operation_error_stack_last_error(&this->_operation_errors));
}

template <typename MutexType>
const char *ft_unique_lock<MutexType>::last_operation_error_str() const noexcept
{
    return (ft_strerror(this->last_operation_error()));
}

#endif
