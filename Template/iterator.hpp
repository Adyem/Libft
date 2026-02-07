#ifndef ITERATOR_HPP
#define ITERATOR_HPP

#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread_internal.hpp"

template <typename ValueType>
class Iterator
{
    private:
        ValueType* _ptr;
        mutable pt_recursive_mutex* _mutex;

        int lock_internal(bool *lock_acquired) const;
        int unlock_internal(bool lock_acquired) const;
        int prepare_thread_safety();
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
        int enable_thread_safety();
        void disable_thread_safety();
        bool is_thread_safe_enabled() const noexcept;
        int lock(bool *lock_acquired) const;
        void unlock(bool lock_acquired) const;

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex* get_mutex_for_validation() const noexcept;
#endif
};

template <typename ValueType>
Iterator<ValueType>::Iterator(ValueType* ptr) noexcept
    : _ptr(ptr),
      _mutex(ft_nullptr)
{
    if (this->_ptr == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
    }
    else
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
    }
}

template <typename ValueType>
Iterator<ValueType>::Iterator(const Iterator& other) noexcept
    : _ptr(ft_nullptr),
      _mutex(ft_nullptr)
{
    bool other_lock_acquired = false;
    int lock_result = other.lock_internal(&other_lock_acquired);

    if (lock_result != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_result);
        return ;
    }
    this->_ptr = other._ptr;
    other.unlock_internal(other_lock_acquired);
    if (other._mutex != ft_nullptr)
    {
        if (this->enable_thread_safety() != FT_ERR_SUCCESSS)
            return ;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
}

template <typename ValueType>
Iterator<ValueType>& Iterator<ValueType>::operator=(const Iterator& other) noexcept
{
    if (this == &other)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (*this);
    }
    bool this_lock_acquired = false;
    int lock_result = this->lock_internal(&this_lock_acquired);

    if (lock_result != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_result);
        return (*this);
    }
    bool other_lock_acquired = false;
    lock_result = other.lock_internal(&other_lock_acquired);

    if (lock_result != FT_ERR_SUCCESSS)
    {
        this->unlock_internal(this_lock_acquired);
        ft_global_error_stack_push(lock_result);
        return (*this);
    }
    this->_ptr = other._ptr;
    other.unlock_internal(other_lock_acquired);
    this->unlock_internal(this_lock_acquired);
    bool other_thread_safe = (other._mutex != ft_nullptr);
    if (other_thread_safe)
    {
        if (this->enable_thread_safety() != FT_ERR_SUCCESSS)
            return (*this);
    }
    else
    {
        this->disable_thread_safety();
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (*this);
}

template <typename ValueType>
Iterator<ValueType>::Iterator(Iterator&& other) noexcept
    : _ptr(ft_nullptr),
      _mutex(ft_nullptr)
{
    bool other_lock_acquired = false;
    int lock_result = other.lock_internal(&other_lock_acquired);

    if (lock_result != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_result);
        return ;
    }
    bool other_thread_safe = (other._mutex != ft_nullptr);
    this->_ptr = other._ptr;
    other._ptr = ft_nullptr;
    other.unlock_internal(other_lock_acquired);
    other.teardown_thread_safety();
    if (other_thread_safe)
    {
        if (this->enable_thread_safety() != FT_ERR_SUCCESSS)
            return ;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
}

template <typename ValueType>
Iterator<ValueType>& Iterator<ValueType>::operator=(Iterator&& other) noexcept
{
    if (this == &other)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (*this);
    }
    bool this_lock_acquired = false;
    int lock_result = this->lock_internal(&this_lock_acquired);

    if (lock_result != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_result);
        return (*this);
    }
    bool other_lock_acquired = false;
    lock_result = other.lock_internal(&other_lock_acquired);

    if (lock_result != FT_ERR_SUCCESSS)
    {
        this->unlock_internal(this_lock_acquired);
        ft_global_error_stack_push(lock_result);
        return (*this);
    }
    this->_ptr = other._ptr;
    bool other_thread_safe = (other._mutex != ft_nullptr);
    other._ptr = ft_nullptr;
    other.unlock_internal(other_lock_acquired);
    other.teardown_thread_safety();
    this->unlock_internal(this_lock_acquired);
    this->teardown_thread_safety();
    if (other_thread_safe)
    {
        if (this->enable_thread_safety() != FT_ERR_SUCCESSS)
            return (*this);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (*this);
}

template <typename ValueType>
Iterator<ValueType>::~Iterator()
{
    this->teardown_thread_safety();
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
}

template <typename ValueType>
Iterator<ValueType> Iterator<ValueType>::operator++() noexcept
{
    bool lock_acquired = false;
    int lock_result = this->lock_internal(&lock_acquired);

    if (lock_result != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_result);
        return (*this);
    }
    if (this->_ptr == ft_nullptr)
    {
        this->unlock_internal(lock_acquired);
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (*this);
    }
    ++this->_ptr;
    this->unlock_internal(lock_acquired);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (*this);
}

template <typename ValueType>
bool Iterator<ValueType>::operator!=(const Iterator& other) const noexcept
{
    if (this == &other)
    {
        bool lock_acquired = false;
        int lock_result = this->lock_internal(&lock_acquired);

        if (lock_result != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(lock_result);
            return (false);
        }
        this->unlock_internal(lock_acquired);
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (false);
    }
    const Iterator<ValueType> *first_iterator = this;
    const Iterator<ValueType> *second_iterator = &other;

    if (first_iterator > second_iterator)
    {
        first_iterator = &other;
        second_iterator = this;
    }
    bool first_lock_acquired = false;
    int first_lock_result = first_iterator->lock_internal(&first_lock_acquired);

    if (first_lock_result != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(first_lock_result);
        return (false);
    }
    bool second_lock_acquired = false;
    int second_lock_result = FT_ERR_SUCCESSS;

    if (second_iterator != first_iterator)
    {
        second_lock_result = second_iterator->lock_internal(&second_lock_acquired);
        if (second_lock_result != FT_ERR_SUCCESSS)
        {
            first_iterator->unlock_internal(first_lock_acquired);
            ft_global_error_stack_push(second_lock_result);
            return (false);
        }
    }
    bool result = (this->_ptr != other._ptr);
    if (second_iterator != first_iterator)
    {
        second_iterator->unlock_internal(second_lock_acquired);
    }
    first_iterator->unlock_internal(first_lock_acquired);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}

template <typename ValueType>
ValueType& Iterator<ValueType>::operator*() const noexcept
{
    static ValueType default_value = ValueType();
    bool lock_acquired = false;
    int lock_result = this->lock_internal(&lock_acquired);

    if (lock_result != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_result);
        return (default_value);
    }
    if (this->_ptr == ft_nullptr)
    {
        this->unlock_internal(lock_acquired);
        ft_global_error_stack_push(FT_ERR_INVALID_ARGUMENT);
        return (default_value);
    }
    ValueType& result = *this->_ptr;
    this->unlock_internal(lock_acquired);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (result);
}

template <typename ValueType>
int Iterator<ValueType>::enable_thread_safety()
{
    if (this->_mutex != ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return (FT_ERR_SUCCESSS);
    }
    int result = this->prepare_thread_safety();

    ft_global_error_stack_push(result);
    return (result);
}

template <typename ValueType>
void Iterator<ValueType>::disable_thread_safety()
{
    this->teardown_thread_safety();
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

template <typename ValueType>
bool Iterator<ValueType>::is_thread_safe_enabled() const noexcept
{
    bool enabled = (this->_mutex != ft_nullptr);

    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (enabled);
}

template <typename ValueType>
int Iterator<ValueType>::lock(bool *lock_acquired) const
{
    int result = this->lock_internal(lock_acquired);

    ft_global_error_stack_push(result);
    if (result != FT_ERR_SUCCESSS)
    {
        return (-1);
    }
    return (0);
}

template <typename ValueType>
void Iterator<ValueType>::unlock(bool lock_acquired) const
{
    int result = this->unlock_internal(lock_acquired);

    ft_global_error_stack_push(result);
    return ;
}

template <typename ValueType>
int Iterator<ValueType>::lock_internal(bool *lock_acquired) const
{
    if (lock_acquired != ft_nullptr)
    {
        *lock_acquired = false;
    }
    if (this->_mutex == ft_nullptr)
    {
        return (FT_ERR_SUCCESSS);
    }
    int result = pt_recursive_mutex_lock_with_error(*this->_mutex);

    if (result == FT_ERR_SUCCESSS && lock_acquired != ft_nullptr)
    {
        *lock_acquired = true;
    }
    return (result);
}

template <typename ValueType>
int Iterator<ValueType>::unlock_internal(bool lock_acquired) const
{
    if (!lock_acquired || this->_mutex == ft_nullptr)
    {
        return (FT_ERR_SUCCESSS);
    }
    return (pt_recursive_mutex_unlock_with_error(*this->_mutex));
}

template <typename ValueType>
int Iterator<ValueType>::prepare_thread_safety()
{
    if (this->_mutex != ft_nullptr)
    {
        return (FT_ERR_SUCCESSS);
    }
    int result = pt_recursive_mutex_create_with_error(&this->_mutex);

    if (result != FT_ERR_SUCCESSS && this->_mutex != ft_nullptr)
    {
        pt_recursive_mutex_destroy(&this->_mutex);
    }
    return (result);
}

template <typename ValueType>
void Iterator<ValueType>::teardown_thread_safety()
{
    pt_recursive_mutex_destroy(&this->_mutex);
    return ;
}

#ifdef LIBFT_TEST_BUILD

template <typename ValueType>
pt_recursive_mutex* Iterator<ValueType>::get_mutex_for_validation() const noexcept
{
    return (this->_mutex);
}
#endif

#endif
