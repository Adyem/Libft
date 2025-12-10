 #ifndef ITERATOR_HPP
 # define ITERATOR_HPP

#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"

template <typename ValueType>
class Iterator
{
    private:
        ValueType* _ptr;
        mutable int _error_code;
        mutable pt_mutex* _state_mutex;
        bool _thread_safe_enabled;

        void set_error(int error_code) const noexcept;
        int lock_internal(bool *lock_acquired) const;
        void unlock_internal(bool lock_acquired) const;
        void teardown_thread_safety();

    public:
        Iterator(ValueType* ptr) noexcept;
        Iterator(const Iterator& other) noexcept;
        Iterator& operator=(const Iterator& other) noexcept;
        Iterator(Iterator&& other) noexcept;
        Iterator& operator=(Iterator&& other) noexcept;
        ~Iterator();
        Iterator operator++() noexcept;
        bool operator!=(const Iterator& other) const noexcept;
        ValueType& operator*() const noexcept;
        int get_error() const noexcept;
        const char* get_error_str() const noexcept;
        int enable_thread_safety();
        void disable_thread_safety();
        bool is_thread_safe_enabled() const noexcept;
        int lock(bool *lock_acquired) const;
        void unlock(bool lock_acquired) const;
};

template <typename ValueType>
Iterator<ValueType>::Iterator(ValueType* ptr) noexcept
    : _ptr(ptr), _error_code(FT_ER_SUCCESSS), _state_mutex(ft_nullptr),
    _thread_safe_enabled(false)
{
    if (this->_ptr == ft_nullptr)
        this->set_error(FT_ERR_INVALID_ARGUMENT);
    else
        this->set_error(FT_ER_SUCCESSS);
    return ;
}

template <typename ValueType>
Iterator<ValueType>::Iterator(const Iterator& other) noexcept
    : _ptr(ft_nullptr), _error_code(FT_ER_SUCCESSS), _state_mutex(ft_nullptr),
    _thread_safe_enabled(false)
{
    bool other_lock_acquired;
    int other_error_code;

    other_lock_acquired = false;
    other_error_code = FT_ER_SUCCESSS;
    if (other.lock_internal(&other_lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    this->_ptr = other._ptr;
    other_error_code = other._error_code;
    other.unlock_internal(other_lock_acquired);
    if (other._thread_safe_enabled && other._state_mutex != ft_nullptr)
    {
        if (this->enable_thread_safety() != 0)
            return ;
    }
    this->set_error(other_error_code);
    return ;
}

template <typename ValueType>
Iterator<ValueType>& Iterator<ValueType>::operator=(const Iterator& other) noexcept
{
    bool this_lock_acquired;
    bool other_lock_acquired;
    int other_error_code;

    if (this == &other)
    {
        this->set_error(this->_error_code);
        return (*this);
    }
    this_lock_acquired = false;
    if (this->lock_internal(&this_lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return (*this);
    }
    other_lock_acquired = false;
    if (other.lock_internal(&other_lock_acquired) != 0)
    {
        this->unlock_internal(this_lock_acquired);
        this->set_error(ft_errno);
        return (*this);
    }
    this->_ptr = other._ptr;
    other_error_code = other._error_code;
    other.unlock_internal(other_lock_acquired);
    this->unlock_internal(this_lock_acquired);
    if (other._thread_safe_enabled && other._state_mutex != ft_nullptr)
    {
        if (!this->_thread_safe_enabled || this->_state_mutex == ft_nullptr)
        {
            if (this->enable_thread_safety() != 0)
                return (*this);
        }
    }
    else if (this->_thread_safe_enabled)
        this->disable_thread_safety();
    this->set_error(other_error_code);
    return (*this);
}

template <typename ValueType>
Iterator<ValueType>::Iterator(Iterator&& other) noexcept
    : _ptr(ft_nullptr), _error_code(FT_ER_SUCCESSS), _state_mutex(ft_nullptr),
    _thread_safe_enabled(false)
{
    bool other_lock_acquired;
    int other_error_code;
    bool other_thread_safe;

    other_lock_acquired = false;
    if (other.lock_internal(&other_lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    other_error_code = other._error_code;
    this->_ptr = other._ptr;
    other_thread_safe = (other._thread_safe_enabled && other._state_mutex != ft_nullptr);
    other._ptr = ft_nullptr;
    other._error_code = FT_ER_SUCCESSS;
    other.unlock_internal(other_lock_acquired);
    other.teardown_thread_safety();
    if (other_thread_safe)
    {
        if (this->enable_thread_safety() != 0)
            return ;
    }
    this->set_error(other_error_code);
    return ;
}

template <typename ValueType>
Iterator<ValueType>& Iterator<ValueType>::operator=(Iterator&& other) noexcept
{
    bool this_lock_acquired;
    bool other_lock_acquired;
    int other_error_code;
    bool other_thread_safe;

    if (this == &other)
    {
        this->set_error(this->_error_code);
        return (*this);
    }
    this_lock_acquired = false;
    if (this->lock_internal(&this_lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return (*this);
    }
    other_lock_acquired = false;
    if (other.lock_internal(&other_lock_acquired) != 0)
    {
        this->unlock_internal(this_lock_acquired);
        this->set_error(ft_errno);
        return (*this);
    }
    other_error_code = other._error_code;
    this->_ptr = other._ptr;
    other_thread_safe = (other._thread_safe_enabled && other._state_mutex != ft_nullptr);
    other._ptr = ft_nullptr;
    other._error_code = FT_ER_SUCCESSS;
    other.unlock_internal(other_lock_acquired);
    other.teardown_thread_safety();
    this->unlock_internal(this_lock_acquired);
    this->teardown_thread_safety();
    if (other_thread_safe)
    {
        if (this->enable_thread_safety() != 0)
            return (*this);
    }
    this->set_error(other_error_code);
    return (*this);
}

template <typename ValueType>
Iterator<ValueType>::~Iterator()
{
    this->teardown_thread_safety();
    this->set_error(FT_ER_SUCCESSS);
    return ;
}

template <typename ValueType>
Iterator<ValueType> Iterator<ValueType>::operator++() noexcept
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return (*this);
    }
    if (this->_ptr == ft_nullptr)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        this->unlock_internal(lock_acquired);
        return (*this);
    }
    ++this->_ptr;
    this->unlock_internal(lock_acquired);
    this->set_error(FT_ER_SUCCESSS);
    return (*this);
}

template <typename ValueType>
bool Iterator<ValueType>::operator!=(const Iterator& other) const noexcept
{
    const Iterator<ValueType> *first_iterator;
    const Iterator<ValueType> *second_iterator;
    bool first_lock_acquired;
    bool second_lock_acquired;
    bool result;

    if (this == &other)
    {
        first_lock_acquired = false;
        if (this->lock_internal(&first_lock_acquired) != 0)
        {
            const_cast<Iterator<ValueType> *>(this)->set_error(ft_errno);
            return (false);
        }
        this->unlock_internal(first_lock_acquired);
        const_cast<Iterator<ValueType> *>(this)->set_error(FT_ER_SUCCESSS);
        return (false);
    }
    first_iterator = this;
    second_iterator = &other;
    if (first_iterator > second_iterator)
    {
        first_iterator = &other;
        second_iterator = this;
    }
    first_lock_acquired = false;
    if (first_iterator->lock_internal(&first_lock_acquired) != 0)
    {
        const_cast<Iterator<ValueType> *>(this)->set_error(ft_errno);
        return (false);
    }
    second_lock_acquired = false;
    if (second_iterator != first_iterator)
    {
        if (second_iterator->lock_internal(&second_lock_acquired) != 0)
        {
            first_iterator->unlock_internal(first_lock_acquired);
            const_cast<Iterator<ValueType> *>(this)->set_error(ft_errno);
            return (false);
        }
    }
    result = (this->_ptr != other._ptr);
    if (second_iterator != first_iterator)
        second_iterator->unlock_internal(second_lock_acquired);
    first_iterator->unlock_internal(first_lock_acquired);
    const_cast<Iterator<ValueType> *>(this)->set_error(FT_ER_SUCCESSS);
    return (result);
}

template <typename ValueType>
ValueType& Iterator<ValueType>::operator*() const noexcept
{
    bool lock_acquired;
    ValueType *value_pointer;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        const_cast<Iterator<ValueType> *>(this)->set_error(ft_errno);
        static ValueType default_value = ValueType();
        return (default_value);
    }
    if (this->_ptr == ft_nullptr)
    {
        const_cast<Iterator<ValueType> *>(this)->set_error(FT_ERR_INVALID_ARGUMENT);
        this->unlock_internal(lock_acquired);
        static ValueType default_value = ValueType();
        return (default_value);
    }
    value_pointer = this->_ptr;
    this->unlock_internal(lock_acquired);
    const_cast<Iterator<ValueType> *>(this)->set_error(FT_ER_SUCCESSS);
    return (*value_pointer);
}

template <typename ValueType>
int Iterator<ValueType>::get_error() const noexcept
{
    return (this->_error_code);
}

template <typename ValueType>
const char* Iterator<ValueType>::get_error_str() const noexcept
{
    return (ft_strerror(this->_error_code));
}

template <typename ValueType>
int Iterator<ValueType>::enable_thread_safety()
{
    void     *memory;
    pt_mutex *state_mutex;

    if (this->_thread_safe_enabled && this->_state_mutex != ft_nullptr)
    {
        this->set_error(FT_ER_SUCCESSS);
        return (0);
    }
    memory = cma_malloc(sizeof(pt_mutex));
    if (memory == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return (-1);
    }
    state_mutex = new(memory) pt_mutex();
    if (state_mutex->get_error() != FT_ER_SUCCESSS)
    {
        int mutex_error;

        mutex_error = state_mutex->get_error();
        state_mutex->~pt_mutex();
        cma_free(memory);
        this->set_error(mutex_error);
        return (-1);
    }
    this->_state_mutex = state_mutex;
    this->_thread_safe_enabled = true;
    this->set_error(FT_ER_SUCCESSS);
    return (0);
}

template <typename ValueType>
void Iterator<ValueType>::disable_thread_safety()
{
    this->teardown_thread_safety();
    this->set_error(FT_ER_SUCCESSS);
    return ;
}

template <typename ValueType>
bool Iterator<ValueType>::is_thread_safe_enabled() const noexcept
{
    bool enabled;

    enabled = (this->_thread_safe_enabled && this->_state_mutex != ft_nullptr);
    return (enabled);
}

template <typename ValueType>
int Iterator<ValueType>::lock(bool *lock_acquired) const
{
    int entry_errno;
    int result;

    entry_errno = ft_errno;
    result = this->lock_internal(lock_acquired);
    if (result != 0)
    {
        const_cast<Iterator<ValueType> *>(this)->set_error(ft_errno);
        return (result);
    }
    this->_error_code = FT_ER_SUCCESSS;
    ft_errno = entry_errno;
    return (result);
}

template <typename ValueType>
void Iterator<ValueType>::unlock(bool lock_acquired) const
{
    int entry_errno;

    entry_errno = ft_errno;
    this->unlock_internal(lock_acquired);
    if (lock_acquired && this->_state_mutex != ft_nullptr && this->_state_mutex->get_error() != FT_ER_SUCCESSS)
    {
        const_cast<Iterator<ValueType> *>(this)->set_error(this->_state_mutex->get_error());
        return ;
    }
    this->_error_code = FT_ER_SUCCESSS;
    ft_errno = entry_errno;
    return ;
}

template <typename ValueType>
void Iterator<ValueType>::set_error(int error_code) const noexcept
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}

template <typename ValueType>
int Iterator<ValueType>::lock_internal(bool *lock_acquired) const
{
    int entry_errno;

    entry_errno = ft_errno;
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (!this->_thread_safe_enabled || this->_state_mutex == ft_nullptr)
    {
        ft_errno = entry_errno;
        return (0);
    }
    this->_state_mutex->lock(THREAD_ID);
    if (this->_state_mutex->get_error() != FT_ER_SUCCESSS)
    {
        ft_errno = this->_state_mutex->get_error();
        return (-1);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    ft_errno = entry_errno;
    return (0);
}

template <typename ValueType>
void Iterator<ValueType>::unlock_internal(bool lock_acquired) const
{
    int entry_errno;

    if (!lock_acquired || this->_state_mutex == ft_nullptr)
        return ;
    entry_errno = ft_errno;
    this->_state_mutex->unlock(THREAD_ID);
    if (this->_state_mutex->get_error() != FT_ER_SUCCESSS)
    {
        ft_errno = this->_state_mutex->get_error();
        return ;
    }
    ft_errno = entry_errno;
    return ;
}

template <typename ValueType>
void Iterator<ValueType>::teardown_thread_safety()
{
    if (this->_state_mutex != ft_nullptr)
    {
        this->_state_mutex->~pt_mutex();
        cma_free(this->_state_mutex);
        this->_state_mutex = ft_nullptr;
    }
    this->_thread_safe_enabled = false;
    return ;
}

#endif
