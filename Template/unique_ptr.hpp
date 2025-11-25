#ifndef UNIQUE_PTR_HPP
#define UNIQUE_PTR_HPP

#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "math.hpp"
#include "swap.hpp"
#include "template_concepts.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"
#include <cstddef>
#include <utility>
#include <type_traits>
#include <cstdlib>
#include <new>
#include "../Libft/libft.hpp"

template <typename ManagedType>
class ft_uniqueptr
{
    private:
        ManagedType* _managedPointer;
        size_t _arraySize;
        bool _isArrayType;
        mutable int _error_code;
        mutable pt_mutex *_mutex;
        bool _thread_safe_enabled;

        void destroy_locked();
        void set_error_unlocked(int error) const;
        int lock_internal(bool *lock_acquired) const;
        void unlock_internal(bool lock_acquired) const;
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

        ft_uniqueptr(ManagedType* pointer, bool isArray = false, size_t arraySize = 1);
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
        int get_error() const;
        const char* get_error_str() const;
        explicit operator bool() const noexcept;
        void swap(ft_uniqueptr& other);
        void set_error(int error) const;
        int enable_thread_safety();
        void disable_thread_safety();
        bool is_thread_safe() const;
        int lock(bool *lock_acquired) const;
        void unlock(bool lock_acquired) const;
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
      _error_code(ER_SUCCESS),
      _mutex(ft_nullptr),
      _thread_safe_enabled(false)
{
    if (!_managedPointer)
        this->set_error(FT_ERR_NO_MEMORY);
    return ;
}
#else
template <typename ManagedType>
template <typename... Args, typename>
ft_uniqueptr<ManagedType>::ft_uniqueptr(Args&&... args)
    : _managedPointer(new (std::nothrow) ManagedType(std::forward<Args>(args)...)),
      _arraySize(1),
      _isArrayType(false),
      _error_code(ER_SUCCESS),
      _mutex(ft_nullptr),
      _thread_safe_enabled(false)
{
    if (!_managedPointer)
        this->set_error(FT_ERR_NO_MEMORY);
    return ;
}
#endif

template <typename ManagedType>
ft_uniqueptr<ManagedType>::ft_uniqueptr(ManagedType* pointer, bool isArray,
        size_t arraySize)
    : _managedPointer(pointer),
      _arraySize(arraySize),
      _isArrayType(isArray),
      _error_code(ER_SUCCESS),
      _mutex(ft_nullptr),
      _thread_safe_enabled(false)
{
    return ;
}

template <typename ManagedType>
ft_uniqueptr<ManagedType>::ft_uniqueptr()
    : _managedPointer(ft_nullptr),
      _arraySize(0),
      _isArrayType(false),
      _error_code(ER_SUCCESS),
      _mutex(ft_nullptr),
      _thread_safe_enabled(false)
{
    return ;
}

template <typename ManagedType>
ft_uniqueptr<ManagedType>::ft_uniqueptr(size_t size)
    : _managedPointer(new (std::nothrow) ManagedType[size]),
      _arraySize(size),
      _isArrayType(true),
      _error_code(ER_SUCCESS),
      _mutex(ft_nullptr),
      _thread_safe_enabled(false)
{
    if (size > 0 && !_managedPointer)
        this->set_error(FT_ERR_NO_MEMORY);
    return ;
}

template <typename ManagedType>
ft_uniqueptr<ManagedType>::ft_uniqueptr(ft_uniqueptr&& other) noexcept
    : _managedPointer(ft_nullptr),
      _arraySize(0),
      _isArrayType(false),
      _error_code(ER_SUCCESS),
      _mutex(ft_nullptr),
      _thread_safe_enabled(false)
{
    bool other_lock_acquired;
    bool enable_thread_safety;

    other_lock_acquired = false;
    enable_thread_safety = false;
    if (other.lock_internal(&other_lock_acquired) != 0)
    {
        this->set_error_unlocked(ft_errno);
        return ;
    }
    enable_thread_safety = (other._thread_safe_enabled && other._mutex != ft_nullptr);
    this->_managedPointer = other._managedPointer;
    this->_arraySize = other._arraySize;
    this->_isArrayType = other._isArrayType;
    this->_error_code = other._error_code;
    other._managedPointer = ft_nullptr;
    other._arraySize = 0;
    other._isArrayType = false;
    other._error_code = ER_SUCCESS;
    other.unlock_internal(other_lock_acquired);
    other.teardown_thread_safety();
    if (enable_thread_safety)
    {
        if (this->prepare_thread_safety() != 0)
        {
            this->set_error_unlocked(ft_errno);
            other.set_error_unlocked(ft_errno);
            return ;
        }
    }
    this->set_error_unlocked(ER_SUCCESS);
    other.set_error_unlocked(ER_SUCCESS);
    return ;
}

template <typename ManagedType>
ft_uniqueptr<ManagedType>& ft_uniqueptr<ManagedType>::operator=(ft_uniqueptr&& other)
    noexcept
{
    if (this != &other)
    {
        ft_uniqueptr<ManagedType>* first;
        ft_uniqueptr<ManagedType>* second;
        bool first_lock_acquired;
        bool second_lock_acquired;
        bool other_thread_safety_enabled;

        first = this;
        second = &other;
        if (first > second)
        {
            ft_uniqueptr<ManagedType>* temp_pointer;

            temp_pointer = first;
            first = second;
            second = temp_pointer;
        }
        first_lock_acquired = false;
        if (first->lock_internal(&first_lock_acquired) != 0)
        {
            this->set_error_unlocked(ft_errno);
            other.set_error_unlocked(ft_errno);
            return (*this);
        }
        second_lock_acquired = false;
        if (second->lock_internal(&second_lock_acquired) != 0)
        {
            first->unlock_internal(first_lock_acquired);
            this->set_error_unlocked(ft_errno);
            other.set_error_unlocked(ft_errno);
            return (*this);
        }
        other_thread_safety_enabled = (other._thread_safe_enabled && other._mutex != ft_nullptr);
        this->destroy_locked();
        this->_managedPointer = other._managedPointer;
        this->_arraySize = other._arraySize;
        this->_isArrayType = other._isArrayType;
        this->_error_code = other._error_code;
        this->_thread_safe_enabled = other_thread_safety_enabled;
        other._managedPointer = ft_nullptr;
        other._arraySize = 0;
        other._isArrayType = false;
        other._error_code = ER_SUCCESS;
        other._thread_safe_enabled = false;
        other.unlock_internal(second_lock_acquired);
        this->unlock_internal(first_lock_acquired);
        other.teardown_thread_safety();
        this->teardown_thread_safety();
        if (other_thread_safety_enabled)
        {
            if (this->prepare_thread_safety() != 0)
            {
                this->set_error_unlocked(ft_errno);
                other.set_error_unlocked(ft_errno);
                return (*this);
            }
        }
        this->set_error_unlocked(ER_SUCCESS);
        other.set_error_unlocked(ER_SUCCESS);
    }
    return (*this);
}

template <typename ManagedType>
ft_uniqueptr<ManagedType>::~ft_uniqueptr()
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) == 0)
    {
        this->destroy_locked();
        this->unlock_internal(lock_acquired);
    }
    else
        this->set_error_unlocked(ft_errno);
    this->teardown_thread_safety();
    return ;
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
    this->_managedPointer = ft_nullptr;
    this->_arraySize = 0;
    this->_isArrayType = false;
    this->set_error_unlocked(ER_SUCCESS);
    return ;
}

template <typename ManagedType>
ManagedType& ft_uniqueptr<ManagedType>::operator*()
{
    static ManagedType default_instance = ManagedType();
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error_unlocked(ft_errno);
        return (default_instance);
    }
    if (this->_managedPointer == ft_nullptr)
    {
        this->set_error_unlocked(FT_ERR_INVALID_POINTER);
        this->unlock_internal(lock_acquired);
        if constexpr (!std::is_abstract_v<ManagedType>)
            return (default_instance);
        else
        {
            static char dummy_buffer[sizeof(ManagedType)] = {0};
            this->unlock_internal(lock_acquired);
            return (*reinterpret_cast<ManagedType*>(dummy_buffer));
        }
    }
    ManagedType& reference = *this->_managedPointer;
    this->set_error_unlocked(ER_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (reference);
}

template <typename ManagedType>
const ManagedType& ft_uniqueptr<ManagedType>::operator*() const
{
    static ManagedType default_instance = ManagedType();
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        const_cast<ft_uniqueptr<ManagedType>*>(this)->set_error_unlocked(ft_errno);
        return (default_instance);
    }
    if (this->_managedPointer == ft_nullptr)
    {
        const_cast<ft_uniqueptr<ManagedType>*>(this)->set_error_unlocked(FT_ERR_INVALID_POINTER);
        const_cast<ft_uniqueptr<ManagedType>*>(this)->unlock_internal(lock_acquired);
        if constexpr (!std::is_abstract_v<ManagedType>)
            return (default_instance);
        else
        {
            static char dummy_buffer[sizeof(ManagedType)] = {0};
            const_cast<ft_uniqueptr<ManagedType>*>(this)->unlock_internal(lock_acquired);
            return (*reinterpret_cast<const ManagedType*>(dummy_buffer));
        }
    }
    const ManagedType& reference = *this->_managedPointer;
    this->set_error_unlocked(ER_SUCCESS);
    const_cast<ft_uniqueptr<ManagedType>*>(this)->unlock_internal(lock_acquired);
    return (reference);
}

template <typename ManagedType>
ManagedType* ft_uniqueptr<ManagedType>::operator->()
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error_unlocked(ft_errno);
        return (ft_nullptr);
    }
    if (this->_managedPointer == ft_nullptr)
    {
        this->set_error_unlocked(FT_ERR_INVALID_POINTER);
        this->unlock_internal(lock_acquired);
        return (ft_nullptr);
    }
    ManagedType* pointer = this->_managedPointer;
    this->set_error_unlocked(ER_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (pointer);
}

template <typename ManagedType>
const ManagedType* ft_uniqueptr<ManagedType>::operator->() const
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        const_cast<ft_uniqueptr<ManagedType>*>(this)->set_error_unlocked(ft_errno);
        return (ft_nullptr);
    }
    if (this->_managedPointer == ft_nullptr)
    {
        const_cast<ft_uniqueptr<ManagedType>*>(this)->set_error_unlocked(FT_ERR_INVALID_POINTER);
        const_cast<ft_uniqueptr<ManagedType>*>(this)->unlock_internal(lock_acquired);
        return (ft_nullptr);
    }
    const ManagedType* pointer = this->_managedPointer;
    this->set_error_unlocked(ER_SUCCESS);
    const_cast<ft_uniqueptr<ManagedType>*>(this)->unlock_internal(lock_acquired);
    return (pointer);
}

template <typename ManagedType>
ManagedType& ft_uniqueptr<ManagedType>::operator[](size_t index)
{
    static ManagedType default_instance = ManagedType();
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error_unlocked(ft_errno);
        return (default_instance);
    }
    if (!this->_isArrayType)
    {
        this->set_error_unlocked(FT_ERR_INVALID_OPERATION);
        this->unlock_internal(lock_acquired);
        if constexpr (!std::is_abstract_v<ManagedType>)
            return (default_instance);
        else
        {
            static char dummy_buffer[sizeof(ManagedType)] = {0};
            this->unlock_internal(lock_acquired);
            return (*reinterpret_cast<ManagedType*>(dummy_buffer));
        }
    }
    if (this->_managedPointer == ft_nullptr)
    {
        this->set_error_unlocked(FT_ERR_INVALID_POINTER);
        this->unlock_internal(lock_acquired);
        if constexpr (!std::is_abstract_v<ManagedType>)
            return (default_instance);
        else
        {
            static char dummy_buffer[sizeof(ManagedType)] = {0};
            this->unlock_internal(lock_acquired);
            return (*reinterpret_cast<ManagedType*>(dummy_buffer));
        }
    }
    if (index >= this->_arraySize)
    {
        this->set_error_unlocked(FT_ERR_OUT_OF_RANGE);
        this->unlock_internal(lock_acquired);
        if constexpr (!std::is_abstract_v<ManagedType>)
            return (default_instance);
        else
        {
            static char dummy_buffer[sizeof(ManagedType)] = {0};
            this->unlock_internal(lock_acquired);
            return (*reinterpret_cast<ManagedType*>(dummy_buffer));
        }
    }
    ManagedType& reference = this->_managedPointer[index];
    this->set_error_unlocked(ER_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (reference);
}

template <typename ManagedType>
const ManagedType& ft_uniqueptr<ManagedType>::operator[](size_t index) const
{
    static ManagedType default_instance = ManagedType();
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        const_cast<ft_uniqueptr<ManagedType>*>(this)->set_error_unlocked(ft_errno);
        return (default_instance);
    }
    if (!this->_isArrayType)
    {
        const_cast<ft_uniqueptr<ManagedType>*>(this)->set_error_unlocked
            (FT_ERR_INVALID_OPERATION);
        const_cast<ft_uniqueptr<ManagedType>*>(this)->unlock_internal(lock_acquired);
        if constexpr (!std::is_abstract_v<ManagedType>)
            return (default_instance);
        else
        {
            static char dummy_buffer[sizeof(ManagedType)] = {0};
            const_cast<ft_uniqueptr<ManagedType>*>(this)->unlock_internal(lock_acquired);
            return (*reinterpret_cast<const ManagedType*>(dummy_buffer));
        }
    }
    if (this->_managedPointer == ft_nullptr)
    {
        const_cast<ft_uniqueptr<ManagedType>*>(this)->set_error_unlocked(FT_ERR_INVALID_POINTER);
        const_cast<ft_uniqueptr<ManagedType>*>(this)->unlock_internal(lock_acquired);
        if constexpr (!std::is_abstract_v<ManagedType>)
            return (default_instance);
        else
        {
            static char dummy_buffer[sizeof(ManagedType)] = {0};
            const_cast<ft_uniqueptr<ManagedType>*>(this)->unlock_internal(lock_acquired);
            return (*reinterpret_cast<const ManagedType*>(dummy_buffer));
        }
    }
    if (index >= this->_arraySize)
    {
        const_cast<ft_uniqueptr<ManagedType>*>(this)->set_error_unlocked
            (FT_ERR_OUT_OF_RANGE);
        const_cast<ft_uniqueptr<ManagedType>*>(this)->unlock_internal(lock_acquired);
        if constexpr (!std::is_abstract_v<ManagedType>)
            return (default_instance);
        else
        {
            static char dummy_buffer[sizeof(ManagedType)] = {0};
            const_cast<ft_uniqueptr<ManagedType>*>(this)->unlock_internal(lock_acquired);
            return (*reinterpret_cast<const ManagedType*>(dummy_buffer));
        }
    }
    const ManagedType& reference = this->_managedPointer[index];
    this->set_error_unlocked(ER_SUCCESS);
    const_cast<ft_uniqueptr<ManagedType>*>(this)->unlock_internal(lock_acquired);
    return (reference);
}

template <typename ManagedType>
ManagedType* ft_uniqueptr<ManagedType>::get()
{
    bool lock_acquired;
    ManagedType* pointer;

    lock_acquired = false;
    pointer = ft_nullptr;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error_unlocked(ft_errno);
        return (pointer);
    }
    pointer = this->_managedPointer;
    this->set_error_unlocked(ER_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (pointer);
}

template <typename ManagedType>
const ManagedType* ft_uniqueptr<ManagedType>::get() const
{
    bool lock_acquired;
    const ManagedType* pointer;

    lock_acquired = false;
    pointer = ft_nullptr;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        const_cast<ft_uniqueptr<ManagedType>*>(this)->set_error_unlocked(ft_errno);
        return (pointer);
    }
    pointer = this->_managedPointer;
    const_cast<ft_uniqueptr<ManagedType>*>(this)->set_error_unlocked(ER_SUCCESS);
    const_cast<ft_uniqueptr<ManagedType>*>(this)->unlock_internal(lock_acquired);
    return (pointer);
}

template <typename ManagedType>
ManagedType* ft_uniqueptr<ManagedType>::release()
{
    ManagedType* released_pointer;
    bool lock_acquired;

    released_pointer = ft_nullptr;
    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error_unlocked(ft_errno);
        return (released_pointer);
    }
    released_pointer = this->_managedPointer;
    this->_managedPointer = ft_nullptr;
    this->_arraySize = 0;
    this->_isArrayType = false;
    this->_error_code = ER_SUCCESS;
    this->set_error_unlocked(ER_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (released_pointer);
}

template <typename ManagedType>
void ft_uniqueptr<ManagedType>::reset(ManagedType* pointer, size_t size, bool arrayType)
{
    if (this->_managedPointer)
    {
        if (this->_isArrayType)
            delete[] this->_managedPointer;
        else
            delete this->_managedPointer;
    }
    this->_managedPointer = pointer;
    this->_arraySize = size;
    this->_isArrayType = arrayType;
    this->_error_code = ER_SUCCESS;
    this->set_error(ER_SUCCESS);
}

template <typename ManagedType>
bool ft_uniqueptr<ManagedType>::hasError() const
{
    bool has_error;

    has_error = (this->_error_code != ER_SUCCESS);
    const_cast<ft_uniqueptr<ManagedType>*>(this)->set_error_unlocked(this->_error_code);
    return (has_error);
}

template <typename ManagedType>
int ft_uniqueptr<ManagedType>::get_error() const
{
    this->set_error_unlocked(this->_error_code);
    return (this->_error_code);
}

template <typename ManagedType>
const char* ft_uniqueptr<ManagedType>::get_error_str() const
{
    this->set_error_unlocked(this->_error_code);
    return (ft_strerror(this->_error_code));
}

template <typename ManagedType>
ft_uniqueptr<ManagedType>::operator bool() const noexcept
{
    return (_managedPointer != ft_nullptr);
}

template <typename ManagedType>
void ft_uniqueptr<ManagedType>::swap(ft_uniqueptr& other)
{
    ft_uniqueptr<ManagedType>* first;
    ft_uniqueptr<ManagedType>* second;
    bool first_lock_acquired;
    bool second_lock_acquired;

    first = this;
    second = &other;
    if (first > second)
    {
        ft_uniqueptr<ManagedType>* temp_pointer;

        temp_pointer = first;
        first = second;
        second = temp_pointer;
    }
    first_lock_acquired = false;
    if (first->lock_internal(&first_lock_acquired) != 0)
    {
        this->set_error_unlocked(ft_errno);
        other.set_error_unlocked(ft_errno);
        return ;
    }
    second_lock_acquired = false;
    if (second->lock_internal(&second_lock_acquired) != 0)
    {
        first->unlock_internal(first_lock_acquired);
        this->set_error_unlocked(ft_errno);
        other.set_error_unlocked(ft_errno);
        return ;
    }
    ft_swap(this->_managedPointer, other._managedPointer);
    ft_swap(this->_arraySize, other._arraySize);
    ft_swap(this->_isArrayType, other._isArrayType);
    ft_swap(this->_error_code, other._error_code);
    ft_swap(this->_mutex, other._mutex);
    ft_swap(this->_thread_safe_enabled, other._thread_safe_enabled);
    this->set_error_unlocked(ER_SUCCESS);
    other.set_error_unlocked(ER_SUCCESS);
    second->unlock_internal(second_lock_acquired);
    first->unlock_internal(first_lock_acquired);
    return ;
}

template <typename ManagedType>
void ft_uniqueptr<ManagedType>::set_error(int error) const
{
    this->set_error_unlocked(error);
    return ;
}

template <typename ManagedType>
void ft_uniqueptr<ManagedType>::set_error_unlocked(int error) const
{
    ft_errno = error;
    this->_error_code = error;
    return ;
}

template <typename ManagedType>
int ft_uniqueptr<ManagedType>::lock_internal(bool *lock_acquired) const
{
    if (lock_acquired)
        *lock_acquired = false;
    if (!this->_thread_safe_enabled || this->_mutex == ft_nullptr)
    {
        ft_errno = ER_SUCCESS;
        return (0);
    }
    this->_mutex->lock(THREAD_ID);
    if (this->_mutex->get_error() != ER_SUCCESS)
    {
        if (this->_mutex->get_error() == FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            bool state_lock_acquired;

            state_lock_acquired = false;
            ft_errno = ER_SUCCESS;
            if (this->_mutex->lock_state(&state_lock_acquired) == 0)
                this->_mutex->unlock_state(state_lock_acquired);
            ft_errno = ER_SUCCESS;
            return (0);
        }
        ft_errno = this->_mutex->get_error();
        return (-1);
    }
    if (lock_acquired)
        *lock_acquired = true;
    ft_errno = ER_SUCCESS;
    return (0);
}

template <typename ManagedType>
void ft_uniqueptr<ManagedType>::unlock_internal(bool lock_acquired) const
{
    int entry_errno;

    if (!lock_acquired || this->_mutex == ft_nullptr)
        return ;
    entry_errno = ft_errno;
    this->_mutex->unlock(THREAD_ID);
    if (this->_mutex->get_error() != ER_SUCCESS)
    {
        ft_errno = this->_mutex->get_error();
        return ;
    }
    ft_errno = entry_errno;
    return ;
}

template <typename ManagedType>
int ft_uniqueptr<ManagedType>::prepare_thread_safety()
{
    void *memory_pointer;
    pt_mutex *mutex_pointer;

    if (this->_thread_safe_enabled && this->_mutex != ft_nullptr)
    {
        this->set_error_unlocked(ER_SUCCESS);
        return (0);
    }
    memory_pointer = std::malloc(sizeof(pt_mutex));
    if (memory_pointer == ft_nullptr)
    {
        this->set_error_unlocked(FT_ERR_NO_MEMORY);
        return (-1);
    }
    mutex_pointer = new(memory_pointer) pt_mutex();
    if (mutex_pointer->get_error() != ER_SUCCESS)
    {
        int mutex_error;

        mutex_error = mutex_pointer->get_error();
        mutex_pointer->~pt_mutex();
        std::free(memory_pointer);
        this->set_error_unlocked(mutex_error);
        return (-1);
    }
    this->_mutex = mutex_pointer;
    this->_thread_safe_enabled = true;
    this->set_error_unlocked(ER_SUCCESS);
    return (0);
}

template <typename ManagedType>
void ft_uniqueptr<ManagedType>::teardown_thread_safety()
{
    if (this->_mutex != ft_nullptr)
    {
        this->_mutex->~pt_mutex();
        std::free(this->_mutex);
        this->_mutex = ft_nullptr;
    }
    this->_thread_safe_enabled = false;
    return ;
}

template <typename ManagedType>
int ft_uniqueptr<ManagedType>::enable_thread_safety()
{
    return (this->prepare_thread_safety());
}

template <typename ManagedType>
void ft_uniqueptr<ManagedType>::disable_thread_safety()
{
    this->teardown_thread_safety();
    this->set_error_unlocked(ER_SUCCESS);
    return ;
}

template <typename ManagedType>
bool ft_uniqueptr<ManagedType>::is_thread_safe() const
{
    bool enabled;

    enabled = (this->_thread_safe_enabled && this->_mutex != ft_nullptr);
    const_cast<ft_uniqueptr<ManagedType>*>(this)->set_error_unlocked(ER_SUCCESS);
    return (enabled);
}

template <typename ManagedType>
int ft_uniqueptr<ManagedType>::lock(bool *lock_acquired) const
{
    int result;

    result = this->lock_internal(lock_acquired);
    if (result != 0)
        const_cast<ft_uniqueptr<ManagedType>*>(this)->set_error_unlocked(ft_errno);
    else
        const_cast<ft_uniqueptr<ManagedType>*>(this)->set_error_unlocked(ER_SUCCESS);
    return (result);
}

template <typename ManagedType>
void ft_uniqueptr<ManagedType>::unlock(bool lock_acquired) const
{
    int entry_errno;

    entry_errno = ft_errno;
    this->unlock_internal(lock_acquired);
    if (this->_mutex != ft_nullptr && this->_mutex->get_error() != ER_SUCCESS)
        const_cast<ft_uniqueptr<ManagedType>*>(this)->set_error_unlocked(this->_mutex->get_error());
    else
    {
        ft_errno = entry_errno;
        const_cast<ft_uniqueptr<ManagedType>*>(this)->set_error_unlocked(ft_errno);
    }
    return ;
}

#endif
