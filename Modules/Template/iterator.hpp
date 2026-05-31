#ifndef ITERATOR_HPP
#define ITERATOR_HPP

#include "../Basic/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Errno/errno_internal.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../Printf/printf.hpp"
#include <cstddef>
#include <cstdint>

template <typename ValueType>
class Iterator
{
    private:
        ValueType                 *_ptr;
        mutable pt_recursive_mutex *_mutex;
        uint8_t                    _initialised_state;
        static thread_local int32_t _last_error;

        static int32_t set_error(int32_t error_code) noexcept;
        int32_t lock_internal(ft_bool *lock_acquired) const;
        int32_t unlock_internal(ft_bool lock_acquired) const;
        static ValueType &fallback_reference() noexcept;

    public:
        class reference_proxy
        {
            private:
                ValueType *_pointer;
                int32_t   _error;

            public:
                reference_proxy(ValueType *pointer, int32_t error) noexcept;
                operator ValueType&() const noexcept;
                ValueType *operator->() const noexcept;
                int32_t get_error() const noexcept;
        };

        Iterator() noexcept;
        explicit Iterator(ValueType *pointer) noexcept;
        ~Iterator();

        Iterator(const Iterator &other) = delete;
        Iterator(Iterator &&other) = delete;
        Iterator &operator=(const Iterator &other) = delete;
        Iterator &operator=(Iterator &&other) = delete;

        int32_t initialize() noexcept;
        int32_t initialize(ValueType *pointer) noexcept;
        int32_t destroy() noexcept;
        int32_t move(Iterator<ValueType> &other) noexcept;

        Iterator& operator++() noexcept;
        ft_bool operator!=(const Iterator& other) const noexcept;
        reference_proxy operator*() const noexcept;
        ValueType *operator->() const noexcept;

        int32_t enable_thread_safety();
        int32_t disable_thread_safety();
        ft_bool is_thread_safe() const noexcept;

        int32_t lock(ft_bool *lock_acquired) const;
        void unlock(ft_bool lock_acquired) const;

        int32_t get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

template <typename ValueType>
int32_t Iterator<ValueType>::set_error(int32_t error_code) noexcept
{
    _last_error = error_code;
    return (error_code);
}

template <typename ValueType>
int32_t Iterator<ValueType>::lock_internal(ft_bool *lock_acquired) const
{
    int32_t lock_result;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = FT_FALSE;
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    lock_result = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_result != FT_ERR_SUCCESS)
        return (set_error(lock_result));
    if (lock_acquired != ft_nullptr)
        *lock_acquired = FT_TRUE;
    return (FT_ERR_SUCCESS);
}

template <typename ValueType>
int32_t Iterator<ValueType>::unlock_internal(ft_bool lock_acquired) const
{
    if (lock_acquired == FT_FALSE)
        return (FT_ERR_SUCCESS);
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (FT_ERR_SUCCESS);
}

template <typename ValueType>
ValueType &Iterator<ValueType>::fallback_reference() noexcept
{
    static ValueType fallback = ValueType();

    return (fallback);
}

template <typename ValueType>
Iterator<ValueType>::reference_proxy::reference_proxy(ValueType *pointer,
    int32_t error) noexcept : _pointer(pointer), _error(error)
{
    return ;
}

template <typename ValueType>
Iterator<ValueType>::reference_proxy::operator ValueType&() const noexcept
{
    if (this->_pointer == ft_nullptr)
        return (Iterator<ValueType>::fallback_reference());
    return (*this->_pointer);
}

template <typename ValueType>
ValueType *Iterator<ValueType>::reference_proxy::operator->() const noexcept
{
    return (this->_pointer);
}

template <typename ValueType>
int32_t Iterator<ValueType>::reference_proxy::get_error() const noexcept
{
    return (this->_error);
}

template <typename ValueType>
Iterator<ValueType>::Iterator() noexcept
    : _ptr(ft_nullptr), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    return ;
}

template <typename ValueType>
Iterator<ValueType>::Iterator(ValueType *pointer) noexcept
    : _ptr(ft_nullptr), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    uint32_t previous_error;

    previous_error = _last_error;
    (void)this->initialize(pointer);
    (void)set_error(previous_error);
    return ;
}

template <typename ValueType>
Iterator<ValueType>::~Iterator()
{
    uint32_t previous_error;

    previous_error = _last_error;
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    if (this->_mutex != ft_nullptr)
        (void)this->disable_thread_safety();
    (void)set_error(previous_error);
    return ;
}

template <typename ValueType>
int32_t Iterator<ValueType>::initialize() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(this->_initialised_state, "Iterator::initialize", "called while object is already initialised");
        return (set_error(FT_ERR_INVALID_STATE));
    }
    this->_ptr = ft_nullptr;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (set_error(FT_ERR_SUCCESS));
}

template <typename ValueType>
int32_t Iterator<ValueType>::initialize(ValueType *pointer) noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(this->_initialised_state, "Iterator::initialize(pointer)", "called while object is already initialised");
        return (set_error(FT_ERR_INVALID_STATE));
    }
    this->_ptr = pointer;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    if (pointer == ft_nullptr)
        return (set_error(FT_ERR_INVALID_ARGUMENT));
    return (set_error(FT_ERR_SUCCESS));
}

template <typename ValueType>
int32_t Iterator<ValueType>::destroy() noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;
    int32_t first_error;
    int32_t disable_result;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
        return (set_error(FT_ERR_SUCCESS));
    first_error = FT_ERR_SUCCESS;
    disable_result = this->disable_thread_safety();
    if (disable_result != FT_ERR_SUCCESS)
        first_error = disable_result;
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS && first_error == FT_ERR_SUCCESS)
        first_error = lock_error;
    this->_ptr = ft_nullptr;
    (void)this->unlock_internal(lock_acquired);
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return (set_error(first_error));
}

template <typename ValueType>
int32_t Iterator<ValueType>::move(Iterator<ValueType> &other) noexcept
{
    int32_t destroy_result;

    if (this == &other)
        return (set_error(FT_ERR_SUCCESS));
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "Iterator::move",
            "source object is uninitialised");
        return (set_error(FT_ERR_INVALID_STATE));
    }
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        destroy_result = this->destroy();
        if (destroy_result != FT_ERR_SUCCESS)
            return (set_error(destroy_result));
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_ptr = ft_nullptr;
        this->_mutex = ft_nullptr;
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (set_error(FT_ERR_SUCCESS));
    }
    this->_ptr = other._ptr;
    this->_mutex = other._mutex;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    other._ptr = ft_nullptr;
    other._mutex = ft_nullptr;
    other._initialised_state = FT_CLASS_STATE_DESTROYED;
    return (set_error(FT_ERR_SUCCESS));
}

template <typename ValueType>
Iterator<ValueType>& Iterator<ValueType>::operator++() noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "Iterator::operator++");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        set_error(lock_error);
        return (*this);
    }
    if (this->_ptr == ft_nullptr)
    {
        (void)this->unlock_internal(lock_acquired);
        set_error(FT_ERR_INVALID_ARGUMENT);
        return (*this);
    }
    ++this->_ptr;
    (void)this->unlock_internal(lock_acquired);
    set_error(FT_ERR_SUCCESS);
    return (*this);
}

template <typename ValueType>
ft_bool Iterator<ValueType>::operator!=(const Iterator& other) const noexcept
{
    ft_bool this_lock_acquired;
    ft_bool other_lock_acquired;
    int32_t lock_error;
    ft_bool is_different;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "Iterator::operator!=");
    errno_abort_if_uninitialised_or_destroyed(other._initialised_state, "Iterator::operator!= other");
    this_lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&this_lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        set_error(lock_error);
        return (FT_FALSE);
    }
    other_lock_acquired = FT_FALSE;
    lock_error = other.lock_internal(&other_lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        (void)this->unlock_internal(this_lock_acquired);
        set_error(lock_error);
        return (FT_FALSE);
    }
    is_different = (this->_ptr != other._ptr);
    (void)other.unlock_internal(other_lock_acquired);
    (void)this->unlock_internal(this_lock_acquired);
    set_error(FT_ERR_SUCCESS);
    return (is_different);
}

template <typename ValueType>
typename Iterator<ValueType>::reference_proxy Iterator<ValueType>::operator*() const noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;
    ValueType *pointer_value;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "Iterator::operator*");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (reference_proxy(ft_nullptr, set_error(lock_error)));
    pointer_value = this->_ptr;
    (void)this->unlock_internal(lock_acquired);
    if (pointer_value == ft_nullptr)
        return (reference_proxy(ft_nullptr, set_error(FT_ERR_INVALID_ARGUMENT)));
    return (reference_proxy(pointer_value, set_error(FT_ERR_SUCCESS)));
}

template <typename ValueType>
ValueType *Iterator<ValueType>::operator->() const noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;
    ValueType *pointer_value;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "Iterator::operator->");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        set_error(lock_error);
        return (ft_nullptr);
    }
    pointer_value = this->_ptr;
    (void)this->unlock_internal(lock_acquired);
    if (pointer_value == ft_nullptr)
    {
        set_error(FT_ERR_INVALID_ARGUMENT);
        return (ft_nullptr);
    }
    set_error(FT_ERR_SUCCESS);
    return (pointer_value);
}

template <typename ValueType>
int32_t Iterator<ValueType>::enable_thread_safety()
{
    pt_recursive_mutex *new_mutex;
    int32_t initialize_result;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "Iterator::enable_thread_safety");
    if (this->_mutex != ft_nullptr)
        return (set_error(FT_ERR_SUCCESS));
    new_mutex = new (std::nothrow) pt_recursive_mutex();
    if (new_mutex == ft_nullptr)
        return (set_error(FT_ERR_NO_MEMORY));
    initialize_result = new_mutex->initialize();
    if (initialize_result != FT_ERR_SUCCESS)
    {
        delete new_mutex;
        return (set_error(initialize_result));
    }
    this->_mutex = new_mutex;
    return (set_error(FT_ERR_SUCCESS));
}

template <typename ValueType>
int32_t Iterator<ValueType>::disable_thread_safety()
{
    pt_recursive_mutex *mutex_pointer;
    int32_t destroy_result;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED
        && this->_initialised_state != FT_CLASS_STATE_DESTROYED)
        return (set_error(FT_ERR_INVALID_STATE));
    mutex_pointer = this->_mutex;
    if (mutex_pointer == ft_nullptr)
        return (set_error(FT_ERR_SUCCESS));
    this->_mutex = ft_nullptr;
    destroy_result = mutex_pointer->destroy();
    delete mutex_pointer;
    if (destroy_result != FT_ERR_SUCCESS)
        return (set_error(destroy_result));
    return (set_error(FT_ERR_SUCCESS));
}

template <typename ValueType>
ft_bool Iterator<ValueType>::is_thread_safe() const noexcept
{
    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
    {
        set_error(FT_ERR_INVALID_STATE);
        return (FT_FALSE);
    }
    set_error(FT_ERR_SUCCESS);
    return (this->_mutex != ft_nullptr);
}

template <typename ValueType>
int32_t Iterator<ValueType>::lock(ft_bool *lock_acquired) const
{
    int32_t lock_result;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "Iterator::lock");
    lock_result = this->lock_internal(lock_acquired);
    return (set_error(lock_result));
}

template <typename ValueType>
void Iterator<ValueType>::unlock(ft_bool lock_acquired) const
{
    (void)this->unlock_internal(lock_acquired);
    set_error(FT_ERR_SUCCESS);
    return ;
}

template <typename ValueType>
int32_t Iterator<ValueType>::get_error() const noexcept
{
    errno_abort_if_uninitialised(this->_initialised_state, "Iterator::get_error");
    return (_last_error);
}

template <typename ValueType>
const char *Iterator<ValueType>::get_error_str() const noexcept
{
    errno_abort_if_uninitialised(this->_initialised_state,
        "Iterator::get_error_str");
    return (ft_strerror(_last_error));
}

template <typename ValueType>
thread_local int32_t Iterator<ValueType>::_last_error = FT_ERR_SUCCESS;

#endif
