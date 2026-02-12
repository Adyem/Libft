#ifndef UNIQUE_PTR_HPP
#define UNIQUE_PTR_HPP

#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "math.hpp"
#include "swap.hpp"
#include "template_concepts.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread_internal.hpp"
#include <cstddef>
#include <utility>
#include <type_traits>
#include <cstdlib>
#include <new>
#include "../Basic/basic.hpp"

template <typename ManagedType>
class ft_uniqueptr
{
    private:
        ManagedType* _managedPointer;
        size_t _arraySize;
        bool _isArrayType;
        mutable pt_recursive_mutex* _mutex;

        void destroy_locked();
        void release_locked();
        int lock_internal(bool *lock_acquired) const;
        int unlock_internal(bool lock_acquired) const;
        int prepare_thread_safety();
        void teardown_thread_safety();

    public:
#if FT_TEMPLATE_HAS_CONCEPTS
        template <typename... Args>
        ft_uniqueptr(Args&&... args)
            requires (!is_single_convertible_to_size_t<Args...>::value &&
                ft_constructible_from<ManagedType, Args&&...>);
#else
        template <typename... Args, typename = std::enable_if_t<
            !(is_single_convertible_to_size_t<Args...>::value) &&
            std::is_constructible_v<ManagedType, Args&&...>
            >>
        ft_uniqueptr(Args&&... args);
#endif

        ft_uniqueptr(ManagedType* pointer, bool arrayType = false, size_t arraySize = 1);
        ft_uniqueptr();
        ft_uniqueptr(size_t size);
        ft_uniqueptr(const ft_uniqueptr&) = delete;
        ft_uniqueptr& operator=(const ft_uniqueptr&) = delete;
        ft_uniqueptr(ft_uniqueptr&& other) noexcept;
        ft_uniqueptr& operator=(ft_uniqueptr&& other) noexcept;
        ~ft_uniqueptr();

        ManagedType& operator*();
        const ManagedType& operator*() const;
        ManagedType* operator->();
        const ManagedType* operator->() const;
        ManagedType& operator[](size_t index);
        const ManagedType& operator[](size_t index) const;

        ManagedType* get();
        const ManagedType* get() const;
        ManagedType* release();
        void reset(ManagedType* pointer = ft_nullptr, size_t size = 1,
                bool arrayType = false);
        bool hasError() const;
        explicit operator bool() const noexcept;
        void swap(ft_uniqueptr& other);

        int enable_thread_safety();
        void disable_thread_safety();
        bool is_thread_safe() const;
        int lock(bool *lock_acquired) const;
        void unlock(bool lock_acquired) const;

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex* get_mutex_for_validation() const noexcept;
#endif
};

#if FT_TEMPLATE_HAS_CONCEPTS
template <typename ManagedType>
template <typename... Args>
ft_uniqueptr<ManagedType>::ft_uniqueptr(Args&&... args)
    requires (!is_single_convertible_to_size_t<Args...>::value &&
        ft_constructible_from<ManagedType, Args&&...>)
    : _managedPointer(new (std::nothrow) ManagedType(std::forward<Args>(args)...)),
      _arraySize(1),
      _isArrayType(false),
      _mutex(ft_nullptr)
{
    if (!this->_managedPointer)
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return ;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}
#else
template <typename ManagedType>
template <typename... Args, typename>
ft_uniqueptr<ManagedType>::ft_uniqueptr(Args&&... args)
    : _managedPointer(new (std::nothrow) ManagedType(std::forward<Args>(args)...)),
      _arraySize(1),
      _isArrayType(false),
      _mutex(ft_nullptr)
{
    if (!this->_managedPointer)
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        return ;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}
#endif

template <typename ManagedType>
ft_uniqueptr<ManagedType>::ft_uniqueptr(ManagedType* pointer,
        bool arrayType, size_t arraySize)
    : _managedPointer(pointer),
      _arraySize(arraySize),
      _isArrayType(arrayType),
      _mutex(ft_nullptr)
{
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

template <typename ManagedType>
ft_uniqueptr<ManagedType>::ft_uniqueptr()
    : _managedPointer(ft_nullptr),
      _arraySize(0),
      _isArrayType(false),
      _mutex(ft_nullptr)
{
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

template <typename ManagedType>
ft_uniqueptr<ManagedType>::ft_uniqueptr(size_t size)
    : _managedPointer(new (std::nothrow) ManagedType[size]),
      _arraySize(size),
      _isArrayType(true),
      _mutex(ft_nullptr)
{
    if (size > 0 && !this->_managedPointer)
    {
        ft_global_error_stack_push(FT_ERR_NO_MEMORY);
        this->_arraySize = 0;
        return ;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

template <typename ManagedType>
ft_uniqueptr<ManagedType>::~ft_uniqueptr()
{
    bool lock_acquired = false;
    int  lock_error = this->lock_internal(&lock_acquired);

    if (lock_error == FT_ERR_SUCCESS)
    {
        this->destroy_locked();
        this->_managedPointer = ft_nullptr;
        this->_arraySize = 0;
        this->_isArrayType = false;
        int unlock_error = this->unlock_internal(lock_acquired);
        if (unlock_error != FT_ERR_SUCCESS)
            ft_global_error_stack_push(unlock_error);
        else
            ft_global_error_stack_push(FT_ERR_SUCCESS);
    }
    else
    {
        ft_global_error_stack_push(lock_error);
    }
    this->teardown_thread_safety();
    return ;
}

template <typename ManagedType>
ft_uniqueptr<ManagedType>::ft_uniqueptr(ft_uniqueptr<ManagedType>&& other) noexcept
    : _managedPointer(ft_nullptr),
      _arraySize(0),
      _isArrayType(false),
      _mutex(ft_nullptr)
{
    bool other_thread_safe = (other._mutex != ft_nullptr);
    bool lock_acquired = false;
    int  lock_error = other.lock_internal(&lock_acquired);

    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    this->_managedPointer = other._managedPointer;
    this->_arraySize = other._arraySize;
    this->_isArrayType = other._isArrayType;
    other._managedPointer = ft_nullptr;
    other._arraySize = 0;
    other._isArrayType = false;
    other.unlock_internal(lock_acquired);
    other.teardown_thread_safety();
    if (other_thread_safe)
    {
        int enable_error = this->enable_thread_safety();
        if (enable_error != FT_ERR_SUCCESS)
        {
            ft_global_error_stack_push(enable_error);
            return ;
        }
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

template <typename ManagedType>
ft_uniqueptr<ManagedType>& ft_uniqueptr<ManagedType>::operator=(ft_uniqueptr<ManagedType>&& other) noexcept
{
    if (this == &other)
    {
        ft_global_error_stack_push(FT_ERR_SUCCESS);
        return (*this);
    }
    ft_uniqueptr<ManagedType>* first = this;
    ft_uniqueptr<ManagedType>* second = &other;
    if (first > second)
    {
        ft_uniqueptr<ManagedType>* temp_pointer = first;
        first = second;
        second = temp_pointer;
    }
    bool first_lock_acquired = false;
    int first_lock_error = first->lock_internal(&first_lock_acquired);
    if (first_lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(first_lock_error);
        return (*this);
    }
    bool second_lock_acquired = false;
    int second_lock_error = second->lock_internal(&second_lock_acquired);
    if (second_lock_error != FT_ERR_SUCCESS)
    {
        first->unlock_internal(first_lock_acquired);
        ft_global_error_stack_push(second_lock_error);
        return (*this);
    }
    if (first == this)
    {
        this->destroy_locked();
        this->release_locked();
    }
    else
    {
        other.destroy_locked();
        other.release_locked();
    }
    this->_managedPointer = second->_managedPointer;
    this->_arraySize = second->_arraySize;
    this->_isArrayType = second->_isArrayType;
    second->_managedPointer = ft_nullptr;
    second->_arraySize = 0;
    second->_isArrayType = false;
    second->unlock_internal(second_lock_acquired);
    first->unlock_internal(first_lock_acquired);
    bool other_thread_safe = (other._mutex != ft_nullptr);
    other.teardown_thread_safety();
    this->teardown_thread_safety();
    if (other_thread_safe)
    {
        int enable_error = this->enable_thread_safety();
        if (enable_error != FT_ERR_SUCCESS)
        {
            ft_global_error_stack_push(enable_error);
            return (*this);
        }
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (*this);
}

template <typename ManagedType>
void ft_uniqueptr<ManagedType>::destroy_locked()
{
    if (this->_managedPointer)
    {
        if (this->_isArrayType)
            delete[] this->_managedPointer;
        else
            delete this->_managedPointer;
    }
    return ;
}

template <typename ManagedType>
void ft_uniqueptr<ManagedType>::release_locked()
{
    this->_managedPointer = ft_nullptr;
    this->_arraySize = 0;
    this->_isArrayType = false;
    return ;
}

template <typename ManagedType>
ManagedType& ft_uniqueptr<ManagedType>::operator*()
{
    static ManagedType default_instance = ManagedType();
    bool lock_acquired = false;
    int  lock_error = this->lock_internal(&lock_acquired);

    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return (default_instance);
    }
    if (this->_managedPointer == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_POINTER);
        this->unlock_internal(lock_acquired);
        return (default_instance);
    }
    ManagedType& reference = *this->_managedPointer;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (reference);
}

template <typename ManagedType>
const ManagedType& ft_uniqueptr<ManagedType>::operator*() const
{
    static ManagedType default_instance = ManagedType();
    bool lock_acquired = false;
    int  lock_error = this->lock_internal(&lock_acquired);

    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return (default_instance);
    }
    if (this->_managedPointer == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_POINTER);
        this->unlock_internal(lock_acquired);
        return (default_instance);
    }
    const ManagedType& reference = *this->_managedPointer;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (reference);
}

template <typename ManagedType>
ManagedType* ft_uniqueptr<ManagedType>::operator->()
{
    bool lock_acquired = false;
    int  lock_error = this->lock_internal(&lock_acquired);

    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return (ft_nullptr);
    }
    if (this->_managedPointer == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_POINTER);
        this->unlock_internal(lock_acquired);
        return (ft_nullptr);
    }
    ManagedType* pointer = this->_managedPointer;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (pointer);
}

template <typename ManagedType>
const ManagedType* ft_uniqueptr<ManagedType>::operator->() const
{
    bool lock_acquired = false;
    int  lock_error = this->lock_internal(&lock_acquired);

    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return (ft_nullptr);
    }
    if (this->_managedPointer == ft_nullptr)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_POINTER);
        this->unlock_internal(lock_acquired);
        return (ft_nullptr);
    }
    const ManagedType* pointer = this->_managedPointer;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (pointer);
}

template <typename ManagedType>
ManagedType& ft_uniqueptr<ManagedType>::operator[](size_t index)
{
    static ManagedType default_instance = ManagedType();
    bool lock_acquired = false;
    int  lock_error = this->lock_internal(&lock_acquired);

    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return (default_instance);
    }
    if (!this->_isArrayType || this->_managedPointer == ft_nullptr || index >= this->_arraySize)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_OPERATION);
        this->unlock_internal(lock_acquired);
        return (default_instance);
    }
    ManagedType& reference = this->_managedPointer[index];
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (reference);
}

template <typename ManagedType>
const ManagedType& ft_uniqueptr<ManagedType>::operator[](size_t index) const
{
    static ManagedType default_instance = ManagedType();
    bool lock_acquired = false;
    int  lock_error = this->lock_internal(&lock_acquired);

    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return (default_instance);
    }
    if (!this->_isArrayType || this->_managedPointer == ft_nullptr || index >= this->_arraySize)
    {
        ft_global_error_stack_push(FT_ERR_INVALID_OPERATION);
        this->unlock_internal(lock_acquired);
        return (default_instance);
    }
    const ManagedType& reference = this->_managedPointer[index];
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (reference);
}

template <typename ManagedType>
ManagedType* ft_uniqueptr<ManagedType>::get()
{
    bool lock_acquired = false;
    int  lock_error = this->lock_internal(&lock_acquired);

    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return (ft_nullptr);
    }
    ManagedType* pointer = this->_managedPointer;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (pointer);
}

template <typename ManagedType>
const ManagedType* ft_uniqueptr<ManagedType>::get() const
{
    bool lock_acquired = false;
    int  lock_error = this->lock_internal(&lock_acquired);

    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return (ft_nullptr);
    }
    const ManagedType* pointer = this->_managedPointer;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (pointer);
}

template <typename ManagedType>
ManagedType* ft_uniqueptr<ManagedType>::release()
{
    bool lock_acquired = false;
    int  lock_error = this->lock_internal(&lock_acquired);

    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return (ft_nullptr);
    }
    ManagedType* pointer = this->_managedPointer;
    this->release_locked();
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (pointer);
}

template <typename ManagedType>
void ft_uniqueptr<ManagedType>::reset(ManagedType* pointer, size_t size, bool arrayType)
{
    bool lock_acquired = false;
    int  lock_error = this->lock_internal(&lock_acquired);

    if (lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    this->destroy_locked();
    this->_managedPointer = pointer;
    this->_arraySize = size;
    this->_isArrayType = arrayType;
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename ManagedType>
bool ft_uniqueptr<ManagedType>::hasError() const
{
    int last_error = ft_global_error_stack_peek_last_error();
    return (last_error != FT_ERR_SUCCESS);
}

template <typename ManagedType>
ft_uniqueptr<ManagedType>::operator bool() const noexcept
{
    return (this->_managedPointer != ft_nullptr);
}

template <typename ManagedType>
void ft_uniqueptr<ManagedType>::swap(ft_uniqueptr& other)
{
    ft_uniqueptr<ManagedType>* first = this;
    ft_uniqueptr<ManagedType>* second = &other;
    if (first > second)
    {
        ft_uniqueptr<ManagedType>* temp_pointer = first;
        first = second;
        second = temp_pointer;
    }
    bool first_lock_acquired = false;
    int first_lock_error = first->lock_internal(&first_lock_acquired);
    if (first_lock_error != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(first_lock_error);
        return ;
    }
    bool second_lock_acquired = false;
    int second_lock_error = second->lock_internal(&second_lock_acquired);
    if (second_lock_error != FT_ERR_SUCCESS)
    {
        first->unlock_internal(first_lock_acquired);
        ft_global_error_stack_push(second_lock_error);
        return ;
    }
    ft_swap(this->_managedPointer, other._managedPointer);
    ft_swap(this->_arraySize, other._arraySize);
    ft_swap(this->_isArrayType, other._isArrayType);
    ft_swap(this->_mutex, other._mutex);
    second->unlock_internal(second_lock_acquired);
    first->unlock_internal(first_lock_acquired);
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

template <typename ManagedType>
int ft_uniqueptr<ManagedType>::lock_internal(bool *lock_acquired) const
{
    if (lock_acquired)
        *lock_acquired = false;
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    int result = pt_recursive_mutex_lock_with_error(*this->_mutex);
    if (result != FT_ERR_SUCCESS)
        return (result);
    if (lock_acquired)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

template <typename ManagedType>
int ft_uniqueptr<ManagedType>::unlock_internal(bool lock_acquired) const
{
    if (!lock_acquired || this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (pt_recursive_mutex_unlock_with_error(*this->_mutex));
}

template <typename ManagedType>
int ft_uniqueptr<ManagedType>::prepare_thread_safety()
{
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (pt_recursive_mutex_create_with_error(&this->_mutex));
}

template <typename ManagedType>
void ft_uniqueptr<ManagedType>::teardown_thread_safety()
{
    pt_recursive_mutex_destroy(&this->_mutex);
    return ;
}

template <typename ManagedType>
int ft_uniqueptr<ManagedType>::enable_thread_safety()
{
    int result = this->prepare_thread_safety();
    ft_global_error_stack_push(result);
    return (result);
}

template <typename ManagedType>
void ft_uniqueptr<ManagedType>::disable_thread_safety()
{
    this->teardown_thread_safety();
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return ;
}

template <typename ManagedType>
bool ft_uniqueptr<ManagedType>::is_thread_safe() const
{
    bool enabled = (this->_mutex != ft_nullptr);
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (enabled);
}

template <typename ManagedType>
int ft_uniqueptr<ManagedType>::lock(bool *lock_acquired) const
{
    int result = this->lock_internal(lock_acquired);
    if (result != FT_ERR_SUCCESS)
    {
        ft_global_error_stack_push(result);
        return (-1);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESS);
    return (0);
}

template <typename ManagedType>
void ft_uniqueptr<ManagedType>::unlock(bool lock_acquired) const
{
    int unlock_error = this->unlock_internal(lock_acquired);
    ft_global_error_stack_push(unlock_error);
    return ;
}

#ifdef LIBFT_TEST_BUILD
template <typename ManagedType>
pt_recursive_mutex* ft_uniqueptr<ManagedType>::get_mutex_for_validation() const noexcept
{
    return (this->_mutex);
}
#endif

#endif
