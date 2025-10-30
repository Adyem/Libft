#ifndef SHARED_PTR_HPP
#define SHARED_PTR_HPP

#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "math.hpp"
#include "swap.hpp"
#include "template_concepts.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread.hpp"
#include <stdint.h>
#include <cstddef>
#include <utility>
#include <type_traits>
#include <cstdlib>
#include <new>

template <typename ManagedType>
class ft_sharedptr
{
    private:
        ManagedType* _managedPointer;
        int* _referenceCount;
        size_t _arraySize;
        bool _isArrayType;
        mutable int _error_code;
        mutable pt_mutex *_mutex;
        bool _thread_safe_enabled;

        void release_current_locked();
        void set_error_unlocked(int error) const;
        int lock_internal(bool *lock_acquired) const;
        void unlock_internal(bool lock_acquired) const;
        int prepare_thread_safety();
        void teardown_thread_safety();

    public:
        template <typename U> friend class ft_sharedptr;

#if FT_TEMPLATE_HAS_CONCEPTS
        template <typename... Args>
        ft_sharedptr(Args&&... args)
            requires (!is_single_convertible_to_size_t<Args...>::value &&
                ft_constructible_from<ManagedType, Args&&...>);
#else
        template <typename... Args, typename = std::enable_if_t<
            !(is_single_convertible_to_size_t<Args...>::value) &&
            std::is_constructible_v<ManagedType, Args&&...>
            >>
        ft_sharedptr(Args&&... args);
#endif

        ft_sharedptr(ManagedType* pointer, bool isArray = false, size_t arraySize = 1);
        ft_sharedptr();
        ft_sharedptr(size_t size);
        ft_sharedptr(const ft_sharedptr<ManagedType>& other);
        ft_sharedptr(ft_sharedptr<ManagedType>&& other) noexcept;
        ~ft_sharedptr();

        ft_sharedptr<ManagedType>& operator=(ft_sharedptr<ManagedType>&& other) noexcept;
        ft_sharedptr<ManagedType>& operator=(const ft_sharedptr<ManagedType>& other);

#if FT_TEMPLATE_HAS_CONCEPTS
        template <typename Other>
        ft_sharedptr(const ft_sharedptr<Other>& other)
            requires ft_convertible_to<Other*, ManagedType*>;

        template <typename Other>
        ft_sharedptr(ft_sharedptr<Other>&& other) noexcept
            requires ft_convertible_to<Other*, ManagedType*>;
#else
        template <typename Other, typename = std::enable_if_t<std::is_convertible_v<Other*,
                 ManagedType*>>>
        ft_sharedptr(const ft_sharedptr<Other>& other);

        template <typename Other, typename = std::enable_if_t<std::is_convertible_v<Other*,
                 ManagedType*>>>
        ft_sharedptr(ft_sharedptr<Other>&& other) noexcept;
#endif

        ManagedType& operator*();
        const ManagedType& operator*() const;
        ManagedType* operator->();
        const ManagedType* operator->() const;

        ManagedType& operator[](size_t index);
        const ManagedType& operator[](size_t index) const;

        int use_count() const;
        bool hasError() const;
        void set_error(int error) const;
        int get_error() const;
        const char* get_error_str() const;
        ManagedType* get();
        const ManagedType* get() const;
        bool unique() const;
        explicit operator bool() const noexcept;

        void reset(ManagedType* pointer = ft_nullptr, size_t size = 1,
            bool arrayType = false);
        void swap(ft_sharedptr<ManagedType>& other);
        void remove(int index);
        void add(const ManagedType& element);

        int enable_thread_safety();
        void disable_thread_safety();
        bool is_thread_safe() const;
        int lock(bool *lock_acquired) const;
        void unlock(bool lock_acquired) const;
};

#if FT_TEMPLATE_HAS_CONCEPTS
template <typename ManagedType>
template <typename... Args>
ft_sharedptr<ManagedType>::ft_sharedptr(Args&&... args)
    requires (!is_single_convertible_to_size_t<Args...>::value &&
        ft_constructible_from<ManagedType, Args&&...>)
    : _managedPointer(ft_nullptr),
      _referenceCount(ft_nullptr),
      _arraySize(0),
      _isArrayType(false),
      _error_code(ER_SUCCESS),
      _mutex(ft_nullptr),
      _thread_safe_enabled(false)
{
    int* reference_count;

    reference_count = new (std::nothrow) int;
    if (!reference_count)
    {
        this->set_error_unlocked(FT_ERR_NO_MEMORY);
        return ;
    }
    ManagedType* pointer = new (std::nothrow) ManagedType(std::forward<Args>(args)...);
    if (!pointer)
    {
        delete reference_count;
        this->set_error_unlocked(FT_ERR_NO_MEMORY);
        return ;
    }
    *reference_count = 1;
    this->_managedPointer = pointer;
    this->_referenceCount = reference_count;
    this->_arraySize = 1;
    this->_isArrayType = false;
    this->set_error_unlocked(ER_SUCCESS);
    return ;
}
#else
template <typename ManagedType>
template <typename... Args, typename>
ft_sharedptr<ManagedType>::ft_sharedptr(Args&&... args)
    : _managedPointer(ft_nullptr),
      _referenceCount(ft_nullptr),
      _arraySize(0),
      _isArrayType(false),
      _error_code(ER_SUCCESS),
      _mutex(ft_nullptr),
      _thread_safe_enabled(false)
{
    int* reference_count;

    reference_count = new (std::nothrow) int;
    if (!reference_count)
    {
        this->set_error_unlocked(FT_ERR_NO_MEMORY);
        return ;
    }
    ManagedType* pointer = new (std::nothrow) ManagedType(std::forward<Args>(args)...);
    if (!pointer)
    {
        delete reference_count;
        this->set_error_unlocked(FT_ERR_NO_MEMORY);
        return ;
    }
    *reference_count = 1;
    this->_managedPointer = pointer;
    this->_referenceCount = reference_count;
    this->_arraySize = 1;
    this->_isArrayType = false;
    this->set_error_unlocked(ER_SUCCESS);
    return ;
}
#endif

template <typename ManagedType>
ft_sharedptr<ManagedType>::ft_sharedptr(ManagedType* pointer, bool isArray, size_t arraySize)
    : _managedPointer(pointer),
      _referenceCount(ft_nullptr),
      _arraySize(isArray ? arraySize : (pointer ? 1 : 0)),
      _isArrayType(isArray),
      _error_code(ER_SUCCESS),
      _mutex(ft_nullptr),
      _thread_safe_enabled(false)
{
    if (pointer)
    {
        this->_referenceCount = new (std::nothrow) int;
        if (!this->_referenceCount)
        {
            if (isArray)
                delete[] pointer;
            else
                delete pointer;
            this->_managedPointer = ft_nullptr;
            this->_arraySize = 0;
            this->_isArrayType = false;
            this->set_error_unlocked(FT_ERR_NO_MEMORY);
            return ;
        }
        *this->_referenceCount = 1;
    }
    this->set_error_unlocked(ER_SUCCESS);
    return ;
}

template <typename ManagedType>
ft_sharedptr<ManagedType>::ft_sharedptr()
    : _managedPointer(ft_nullptr),
      _referenceCount(ft_nullptr),
      _arraySize(0),
      _isArrayType(false),
      _error_code(ER_SUCCESS),
      _mutex(ft_nullptr),
      _thread_safe_enabled(false)
{
    this->set_error_unlocked(ER_SUCCESS);
    return ;
}

template <typename ManagedType>
ft_sharedptr<ManagedType>::ft_sharedptr(size_t size)
    : _managedPointer(ft_nullptr),
      _referenceCount(ft_nullptr),
      _arraySize(size),
      _isArrayType(true),
      _error_code(ER_SUCCESS),
      _mutex(ft_nullptr),
      _thread_safe_enabled(false)
{
    if (size == 0)
    {
        this->set_error_unlocked(ER_SUCCESS);
        return ;
    }
    this->_referenceCount = new (std::nothrow) int;
    if (!this->_referenceCount)
    {
        this->_arraySize = 0;
        this->_isArrayType = false;
        this->set_error_unlocked(FT_ERR_NO_MEMORY);
        return ;
    }
    this->_managedPointer = new (std::nothrow) ManagedType[size];
    if (!this->_managedPointer)
    {
        delete this->_referenceCount;
        this->_referenceCount = ft_nullptr;
        this->_arraySize = 0;
        this->_isArrayType = false;
        this->set_error_unlocked(FT_ERR_NO_MEMORY);
        return ;
    }
    *this->_referenceCount = 1;
    this->set_error_unlocked(ER_SUCCESS);
    return ;
}

template <typename ManagedType>
ft_sharedptr<ManagedType>::ft_sharedptr(const ft_sharedptr<ManagedType>& other)
    : _managedPointer(ft_nullptr),
      _referenceCount(ft_nullptr),
      _arraySize(0),
      _isArrayType(false),
      _error_code(ER_SUCCESS),
      _mutex(ft_nullptr),
      _thread_safe_enabled(false)
{
    bool other_lock_acquired;

    other_lock_acquired = false;
    if (other.lock_internal(&other_lock_acquired) != 0)
    {
        this->set_error_unlocked(ft_errno);
        return ;
    }
    this->_managedPointer = other._managedPointer;
    this->_referenceCount = other._referenceCount;
    this->_arraySize = other._arraySize;
    this->_isArrayType = other._isArrayType;
    this->_error_code = other._error_code;
    if (this->_referenceCount)
    {
        ++(*this->_referenceCount);
        this->set_error_unlocked(ER_SUCCESS);
    }
    else
        this->set_error_unlocked(other._error_code);
    other.unlock_internal(other_lock_acquired);
    return ;
}

template <typename ManagedType>
ft_sharedptr<ManagedType>::ft_sharedptr(ft_sharedptr<ManagedType>&& other) noexcept
    : _managedPointer(ft_nullptr),
      _referenceCount(ft_nullptr),
      _arraySize(0),
      _isArrayType(false),
      _error_code(ER_SUCCESS),
      _mutex(ft_nullptr),
      _thread_safe_enabled(false)
{
    bool other_lock_acquired;

    other_lock_acquired = false;
    if (other.lock_internal(&other_lock_acquired) != 0)
    {
        this->set_error_unlocked(ft_errno);
        return ;
    }
    this->_managedPointer = other._managedPointer;
    this->_referenceCount = other._referenceCount;
    this->_arraySize = other._arraySize;
    this->_isArrayType = other._isArrayType;
    this->_error_code = other._error_code;
    other._managedPointer = ft_nullptr;
    other._referenceCount = ft_nullptr;
    other._arraySize = 0;
    other._isArrayType = false;
    other._error_code = ER_SUCCESS;
    this->set_error_unlocked(ER_SUCCESS);
    other.set_error_unlocked(ER_SUCCESS);
    other.unlock_internal(other_lock_acquired);
    return ;
}

template <typename ManagedType>
ft_sharedptr<ManagedType>::~ft_sharedptr()
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) == 0)
    {
        this->release_current_locked();
        this->unlock_internal(lock_acquired);
    }
    else
        this->set_error_unlocked(ft_errno);
    this->teardown_thread_safety();
    return ;
}

template <typename ManagedType>
void ft_sharedptr<ManagedType>::release_current_locked()
{
    if (this->_referenceCount)
    {
        --(*this->_referenceCount);
        if (*this->_referenceCount <= 0)
        {
            if (this->_managedPointer)
            {
                if (this->_isArrayType)
                    delete[] this->_managedPointer;
                else
                    delete this->_managedPointer;
            }
            delete this->_referenceCount;
        }
    }
    else if (this->_managedPointer)
    {
        if (this->_isArrayType)
            delete[] this->_managedPointer;
        else
            delete this->_managedPointer;
    }
    this->_managedPointer = ft_nullptr;
    this->_referenceCount = ft_nullptr;
    this->_arraySize = 0;
    this->_isArrayType = false;
    this->set_error_unlocked(ER_SUCCESS);
    return ;
}

template <typename ManagedType>
ft_sharedptr<ManagedType>& ft_sharedptr<ManagedType>::operator=(ft_sharedptr<ManagedType>&& other) noexcept
{
    if (this == &other)
    {
        bool self_lock_acquired;

        self_lock_acquired = false;
        if (this->lock_internal(&self_lock_acquired) == 0)
        {
            this->set_error_unlocked(ER_SUCCESS);
            this->unlock_internal(self_lock_acquired);
        }
        else
            this->set_error_unlocked(ft_errno);
        return (*this);
    }
    bool this_lock_acquired;
    bool other_lock_acquired;

    this_lock_acquired = false;
    if (this->lock_internal(&this_lock_acquired) != 0)
    {
        this->set_error_unlocked(ft_errno);
        return (*this);
    }
    other_lock_acquired = false;
    if (other.lock_internal(&other_lock_acquired) != 0)
    {
        this->unlock_internal(this_lock_acquired);
        this->set_error_unlocked(ft_errno);
        return (*this);
    }
    this->release_current_locked();
    this->_managedPointer = other._managedPointer;
    this->_referenceCount = other._referenceCount;
    this->_arraySize = other._arraySize;
    this->_isArrayType = other._isArrayType;
    this->_error_code = other._error_code;
    other._managedPointer = ft_nullptr;
    other._referenceCount = ft_nullptr;
    other._arraySize = 0;
    other._isArrayType = false;
    other._error_code = ER_SUCCESS;
    this->set_error_unlocked(ER_SUCCESS);
    other.set_error_unlocked(ER_SUCCESS);
    other.unlock_internal(other_lock_acquired);
    this->unlock_internal(this_lock_acquired);
    return (*this);
}

template <typename ManagedType>
ft_sharedptr<ManagedType>& ft_sharedptr<ManagedType>::operator=(const ft_sharedptr<ManagedType>& other)
{
    if (this == &other)
    {
        bool self_lock_acquired;

        self_lock_acquired = false;
        if (this->lock_internal(&self_lock_acquired) == 0)
        {
            this->set_error_unlocked(ER_SUCCESS);
            this->unlock_internal(self_lock_acquired);
        }
        else
            this->set_error_unlocked(ft_errno);
        return (*this);
    }
    bool this_lock_acquired;
    bool other_lock_acquired;

    this_lock_acquired = false;
    if (this->lock_internal(&this_lock_acquired) != 0)
    {
        this->set_error_unlocked(ft_errno);
        return (*this);
    }
    other_lock_acquired = false;
    if (other.lock_internal(&other_lock_acquired) != 0)
    {
        this->unlock_internal(this_lock_acquired);
        this->set_error_unlocked(ft_errno);
        return (*this);
    }
    this->release_current_locked();
    this->_managedPointer = other._managedPointer;
    this->_referenceCount = other._referenceCount;
    this->_arraySize = other._arraySize;
    this->_isArrayType = other._isArrayType;
    this->_error_code = other._error_code;
    if (this->_referenceCount)
    {
        ++(*this->_referenceCount);
        this->set_error_unlocked(ER_SUCCESS);
    }
    else
        this->set_error_unlocked(other._error_code);
    other.unlock_internal(other_lock_acquired);
    this->unlock_internal(this_lock_acquired);
    return (*this);
}

#if FT_TEMPLATE_HAS_CONCEPTS
template <typename ManagedType>
template <typename Other>
ft_sharedptr<ManagedType>::ft_sharedptr(const ft_sharedptr<Other>& other)
    requires ft_convertible_to<Other*, ManagedType*>
    : _managedPointer(ft_nullptr),
      _referenceCount(ft_nullptr),
      _arraySize(0),
      _isArrayType(false),
      _error_code(ER_SUCCESS),
      _mutex(ft_nullptr),
      _thread_safe_enabled(false)
{
    bool other_lock_acquired;

    other_lock_acquired = false;
    if (other.lock_internal(&other_lock_acquired) != 0)
    {
        this->set_error_unlocked(ft_errno);
        return ;
    }
    this->_managedPointer = other._managedPointer;
    this->_referenceCount = other._referenceCount;
    this->_arraySize = other._arraySize;
    this->_isArrayType = other._isArrayType;
    this->_error_code = other._error_code;
    if (this->_referenceCount)
    {
        ++(*this->_referenceCount);
        this->set_error_unlocked(ER_SUCCESS);
    }
    else
        this->set_error_unlocked(other._error_code);
    other.unlock_internal(other_lock_acquired);
    return ;
}
#else
template <typename ManagedType>
template <typename Other, typename>
ft_sharedptr<ManagedType>::ft_sharedptr(const ft_sharedptr<Other>& other)
    : _managedPointer(ft_nullptr),
      _referenceCount(ft_nullptr),
      _arraySize(0),
      _isArrayType(false),
      _error_code(ER_SUCCESS),
      _mutex(ft_nullptr),
      _thread_safe_enabled(false)
{
    bool other_lock_acquired;

    other_lock_acquired = false;
    if (other.lock_internal(&other_lock_acquired) != 0)
    {
        this->set_error_unlocked(ft_errno);
        return ;
    }
    this->_managedPointer = other._managedPointer;
    this->_referenceCount = other._referenceCount;
    this->_arraySize = other._arraySize;
    this->_isArrayType = other._isArrayType;
    this->_error_code = other._error_code;
    if (this->_referenceCount)
    {
        ++(*this->_referenceCount);
        this->set_error_unlocked(ER_SUCCESS);
    }
    else
        this->set_error_unlocked(other._error_code);
    other.unlock_internal(other_lock_acquired);
    return ;
}
#endif

#if FT_TEMPLATE_HAS_CONCEPTS
template <typename ManagedType>
template <typename Other>
ft_sharedptr<ManagedType>::ft_sharedptr(ft_sharedptr<Other>&& other) noexcept
    requires ft_convertible_to<Other*, ManagedType*>
    : _managedPointer(ft_nullptr),
      _referenceCount(ft_nullptr),
      _arraySize(0),
      _isArrayType(false),
      _error_code(ER_SUCCESS),
      _mutex(ft_nullptr),
      _thread_safe_enabled(false)
{
    bool other_lock_acquired;

    other_lock_acquired = false;
    if (other.lock_internal(&other_lock_acquired) != 0)
    {
        this->set_error_unlocked(ft_errno);
        return ;
    }
    this->_managedPointer = other._managedPointer;
    this->_referenceCount = other._referenceCount;
    this->_arraySize = other._arraySize;
    this->_isArrayType = other._isArrayType;
    this->_error_code = other._error_code;
    other._managedPointer = ft_nullptr;
    other._referenceCount = ft_nullptr;
    other._arraySize = 0;
    other._isArrayType = false;
    other._error_code = ER_SUCCESS;
    this->set_error_unlocked(ER_SUCCESS);
    other.set_error_unlocked(ER_SUCCESS);
    other.unlock_internal(other_lock_acquired);
    return ;
}
#else
template <typename ManagedType>
template <typename Other, typename>
ft_sharedptr<ManagedType>::ft_sharedptr(ft_sharedptr<Other>&& other) noexcept
    : _managedPointer(ft_nullptr),
      _referenceCount(ft_nullptr),
      _arraySize(0),
      _isArrayType(false),
      _error_code(ER_SUCCESS),
      _mutex(ft_nullptr),
      _thread_safe_enabled(false)
{
    bool other_lock_acquired;

    other_lock_acquired = false;
    if (other.lock_internal(&other_lock_acquired) != 0)
    {
        this->set_error_unlocked(ft_errno);
        return ;
    }
    this->_managedPointer = other._managedPointer;
    this->_referenceCount = other._referenceCount;
    this->_arraySize = other._arraySize;
    this->_isArrayType = other._isArrayType;
    this->_error_code = other._error_code;
    other._managedPointer = ft_nullptr;
    other._referenceCount = ft_nullptr;
    other._arraySize = 0;
    other._isArrayType = false;
    other._error_code = ER_SUCCESS;
    this->set_error_unlocked(ER_SUCCESS);
    other.set_error_unlocked(ER_SUCCESS);
    other.unlock_internal(other_lock_acquired);
    return ;
}
#endif

template <typename ManagedType>
ManagedType& ft_sharedptr<ManagedType>::operator*()
{
    bool lock_acquired;
    ManagedType* pointer;

    lock_acquired = false;
    pointer = ft_nullptr;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error_unlocked(ft_errno);
        if constexpr (!std::is_abstract_v<ManagedType>)
        {
            static ManagedType default_instance;
            return (default_instance);
        }
        else
        {
            static char dummy_buffer[sizeof(ManagedType)] = {0};
            return (*reinterpret_cast<ManagedType*>(dummy_buffer));
        }
    }
    pointer = this->_managedPointer;
    if (!pointer)
    {
        this->set_error_unlocked(FT_ERR_INVALID_POINTER);
        this->unlock_internal(lock_acquired);
        if constexpr (!std::is_abstract_v<ManagedType>)
        {
            static ManagedType default_instance;
            return (default_instance);
        }
        else
        {
            static char dummy_buffer[sizeof(ManagedType)] = {0};
            return (*reinterpret_cast<ManagedType*>(dummy_buffer));
        }
    }
    this->set_error_unlocked(ER_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (*pointer);
}

template <typename ManagedType>
const ManagedType& ft_sharedptr<ManagedType>::operator*() const
{
    bool lock_acquired;
    const ManagedType* pointer;

    lock_acquired = false;
    pointer = ft_nullptr;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        const_cast<ft_sharedptr<ManagedType>*>(this)->set_error_unlocked(ft_errno);
        if constexpr (!std::is_abstract_v<ManagedType>)
        {
            static ManagedType default_instance;
            return (default_instance);
        }
        else
        {
            static char dummy_buffer[sizeof(ManagedType)] = {0};
            return (*reinterpret_cast<const ManagedType*>(dummy_buffer));
        }
    }
    pointer = this->_managedPointer;
    if (!pointer)
    {
        const_cast<ft_sharedptr<ManagedType>*>(this)->set_error_unlocked(FT_ERR_INVALID_POINTER);
        this->unlock_internal(lock_acquired);
        if constexpr (!std::is_abstract_v<ManagedType>)
        {
            static ManagedType default_instance;
            return (default_instance);
        }
        else
        {
            static char dummy_buffer[sizeof(ManagedType)] = {0};
            return (*reinterpret_cast<const ManagedType*>(dummy_buffer));
        }
    }
    const_cast<ft_sharedptr<ManagedType>*>(this)->set_error_unlocked(ER_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (*pointer);
}

template <typename ManagedType>
ManagedType* ft_sharedptr<ManagedType>::operator->()
{
    ManagedType* pointer;

    pointer = this->get();
    return (pointer);
}

template <typename ManagedType>
const ManagedType* ft_sharedptr<ManagedType>::operator->() const
{
    const ManagedType* pointer;

    pointer = this->get();
    return (pointer);
}

template <typename ManagedType>
ManagedType& ft_sharedptr<ManagedType>::operator[](size_t index)
{
    bool lock_acquired;
    ManagedType* pointer;
    ManagedType* element_pointer;

    lock_acquired = false;
    pointer = ft_nullptr;
    element_pointer = ft_nullptr;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error_unlocked(ft_errno);
        if constexpr (!std::is_abstract_v<ManagedType>)
        {
            static ManagedType default_instance;
            return (default_instance);
        }
        else
        {
            static char dummy_buffer[sizeof(ManagedType)] = {0};
            return (*reinterpret_cast<ManagedType*>(dummy_buffer));
        }
    }
    if (!this->_isArrayType)
    {
        this->set_error_unlocked(FT_ERR_INVALID_OPERATION);
        this->unlock_internal(lock_acquired);
        if constexpr (!std::is_abstract_v<ManagedType>)
        {
            static ManagedType default_instance;
            return (default_instance);
        }
        else
        {
            static char dummy_buffer[sizeof(ManagedType)] = {0};
            return (*reinterpret_cast<ManagedType*>(dummy_buffer));
        }
    }
    pointer = this->_managedPointer;
    if (!pointer)
    {
        this->set_error_unlocked(FT_ERR_INVALID_POINTER);
        this->unlock_internal(lock_acquired);
        if constexpr (!std::is_abstract_v<ManagedType>)
        {
            static ManagedType default_instance;
            return (default_instance);
        }
        else
        {
            static char dummy_buffer[sizeof(ManagedType)] = {0};
            return (*reinterpret_cast<ManagedType*>(dummy_buffer));
        }
    }
    if (index >= this->_arraySize)
    {
        this->set_error_unlocked(FT_ERR_OUT_OF_RANGE);
        this->unlock_internal(lock_acquired);
        if constexpr (!std::is_abstract_v<ManagedType>)
        {
            static ManagedType default_instance;
            return (default_instance);
        }
        else
        {
            static char dummy_buffer[sizeof(ManagedType)] = {0};
            return (*reinterpret_cast<ManagedType*>(dummy_buffer));
        }
    }
    element_pointer = &pointer[index];
    this->set_error_unlocked(ER_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (*element_pointer);
}

template <typename ManagedType>
const ManagedType& ft_sharedptr<ManagedType>::operator[](size_t index) const
{
    bool lock_acquired;
    const ManagedType* pointer;
    const ManagedType* element_pointer;

    lock_acquired = false;
    pointer = ft_nullptr;
    element_pointer = ft_nullptr;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        const_cast<ft_sharedptr<ManagedType>*>(this)->set_error_unlocked(ft_errno);
        if constexpr (!std::is_abstract_v<ManagedType>)
        {
            static ManagedType default_instance;
            return (default_instance);
        }
        else
        {
            static char dummy_buffer[sizeof(ManagedType)] = {0};
            return (*reinterpret_cast<const ManagedType*>(dummy_buffer));
        }
    }
    if (!this->_isArrayType)
    {
        const_cast<ft_sharedptr<ManagedType>*>(this)->set_error_unlocked(FT_ERR_INVALID_OPERATION);
        this->unlock_internal(lock_acquired);
        if constexpr (!std::is_abstract_v<ManagedType>)
        {
            static ManagedType default_instance;
            return (default_instance);
        }
        else
        {
            static char dummy_buffer[sizeof(ManagedType)] = {0};
            return (*reinterpret_cast<const ManagedType*>(dummy_buffer));
        }
    }
    pointer = this->_managedPointer;
    if (!pointer)
    {
        const_cast<ft_sharedptr<ManagedType>*>(this)->set_error_unlocked(FT_ERR_INVALID_POINTER);
        this->unlock_internal(lock_acquired);
        if constexpr (!std::is_abstract_v<ManagedType>)
        {
            static ManagedType default_instance;
            return (default_instance);
        }
        else
        {
            static char dummy_buffer[sizeof(ManagedType)] = {0};
            return (*reinterpret_cast<const ManagedType*>(dummy_buffer));
        }
    }
    if (index >= this->_arraySize)
    {
        const_cast<ft_sharedptr<ManagedType>*>(this)->set_error_unlocked(FT_ERR_OUT_OF_RANGE);
        this->unlock_internal(lock_acquired);
        if constexpr (!std::is_abstract_v<ManagedType>)
        {
            static ManagedType default_instance;
            return (default_instance);
        }
        else
        {
            static char dummy_buffer[sizeof(ManagedType)] = {0};
            return (*reinterpret_cast<const ManagedType*>(dummy_buffer));
        }
    }
    element_pointer = &pointer[index];
    const_cast<ft_sharedptr<ManagedType>*>(this)->set_error_unlocked(ER_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (*element_pointer);
}

template <typename ManagedType>
int ft_sharedptr<ManagedType>::use_count() const
{
    bool lock_acquired;
    int count;

    lock_acquired = false;
    count = 0;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        const_cast<ft_sharedptr<ManagedType>*>(this)->set_error_unlocked(ft_errno);
        return (0);
    }
    if (this->_referenceCount)
        count = *this->_referenceCount;
    else if (this->_managedPointer)
        count = 1;
    else
        count = 0;
    const_cast<ft_sharedptr<ManagedType>*>(this)->set_error_unlocked(ER_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (count);
}

template <typename ManagedType>
bool ft_sharedptr<ManagedType>::hasError() const
{
    bool lock_acquired;
    bool has_error;
    int entry_errno;

    lock_acquired = false;
    has_error = true;
    entry_errno = ft_errno;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        const_cast<ft_sharedptr<ManagedType>*>(this)->set_error_unlocked(ft_errno);
        return (true);
    }
    has_error = (this->_error_code != ER_SUCCESS);
    this->unlock_internal(lock_acquired);
    ft_errno = entry_errno;
    return (has_error);
}

template <typename ManagedType>
int ft_sharedptr<ManagedType>::get_error() const
{
    bool lock_acquired;
    int error_code;

    lock_acquired = false;
    error_code = ER_SUCCESS;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        const_cast<ft_sharedptr<ManagedType>*>(this)->set_error_unlocked(ft_errno);
        return (ft_errno);
    }
    error_code = this->_error_code;
    this->unlock_internal(lock_acquired);
    return (error_code);
}

template <typename ManagedType>
const char* ft_sharedptr<ManagedType>::get_error_str() const
{
    bool lock_acquired;
    int error_code;

    lock_acquired = false;
    error_code = ER_SUCCESS;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        const_cast<ft_sharedptr<ManagedType>*>(this)->set_error_unlocked(ft_errno);
        return (ft_strerror(ft_errno));
    }
    error_code = this->_error_code;
    this->unlock_internal(lock_acquired);
    return (ft_strerror(error_code));
}

template <typename ManagedType>
ManagedType* ft_sharedptr<ManagedType>::get()
{
    bool lock_acquired;
    ManagedType* pointer;

    lock_acquired = false;
    pointer = ft_nullptr;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error_unlocked(ft_errno);
        return (ft_nullptr);
    }
    pointer = this->_managedPointer;
    this->set_error_unlocked(ER_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (pointer);
}

template <typename ManagedType>
const ManagedType* ft_sharedptr<ManagedType>::get() const
{
    bool lock_acquired;
    const ManagedType* pointer;

    lock_acquired = false;
    pointer = ft_nullptr;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        const_cast<ft_sharedptr<ManagedType>*>(this)->set_error_unlocked(ft_errno);
        return (ft_nullptr);
    }
    pointer = this->_managedPointer;
    const_cast<ft_sharedptr<ManagedType>*>(this)->set_error_unlocked(ER_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (pointer);
}

template <typename ManagedType>
bool ft_sharedptr<ManagedType>::unique() const
{
    return (this->use_count() == 1);
}

template <typename ManagedType>
ft_sharedptr<ManagedType>::operator bool() const noexcept
{
    bool lock_acquired;
    bool result;
    int entry_errno;

    lock_acquired = false;
    result = false;
    entry_errno = ft_errno;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        const_cast<ft_sharedptr<ManagedType>*>(this)->set_error_unlocked(ft_errno);
        return (false);
    }
    result = (this->_managedPointer != ft_nullptr);
    this->unlock_internal(lock_acquired);
    ft_errno = entry_errno;
    return (result);
}

template <typename ManagedType>
void ft_sharedptr<ManagedType>::set_error(int error) const
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
        return ;
    this->set_error_unlocked(error);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename ManagedType>
void ft_sharedptr<ManagedType>::reset(ManagedType* pointer, size_t size, bool arrayType)
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error_unlocked(ft_errno);
        return ;
    }
    this->release_current_locked();
    this->_managedPointer = pointer;
    if (arrayType)
        this->_arraySize = size;
    else if (pointer)
        this->_arraySize = 1;
    else
        this->_arraySize = 0;
    this->_isArrayType = arrayType;
    if (!pointer)
    {
        this->_referenceCount = ft_nullptr;
        this->set_error_unlocked(ER_SUCCESS);
        this->unlock_internal(lock_acquired);
        return ;
    }
    this->_referenceCount = new (std::nothrow) int;
    if (!this->_referenceCount)
    {
        if (arrayType)
            delete[] pointer;
        else
            delete pointer;
        this->_managedPointer = ft_nullptr;
        this->_arraySize = 0;
        this->_isArrayType = false;
        this->set_error_unlocked(FT_ERR_NO_MEMORY);
        this->unlock_internal(lock_acquired);
        return ;
    }
    *this->_referenceCount = 1;
    this->set_error_unlocked(ER_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename ManagedType>
void ft_sharedptr<ManagedType>::swap(ft_sharedptr<ManagedType>& other)
{
    ft_sharedptr<ManagedType>* first;
    ft_sharedptr<ManagedType>* second;
    bool first_lock_acquired;
    bool second_lock_acquired;

    first = this;
    second = &other;
    if (first > second)
    {
        ft_sharedptr<ManagedType>* temp_pointer;

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
    ft_swap(this->_referenceCount, other._referenceCount);
    ft_swap(this->_arraySize, other._arraySize);
    ft_swap(this->_isArrayType, other._isArrayType);
    ft_swap(this->_error_code, other._error_code);
    this->set_error_unlocked(ER_SUCCESS);
    other.set_error_unlocked(ER_SUCCESS);
    second->unlock_internal(second_lock_acquired);
    first->unlock_internal(first_lock_acquired);
    return ;
}

template <typename ManagedType>
void ft_sharedptr<ManagedType>::add(const ManagedType& element)
{
    bool lock_acquired;
    ManagedType* previous_array;
    int* previous_reference;
    ManagedType* new_array;
    int* new_count;
    size_t index;
    bool detach_required;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error_unlocked(ft_errno);
        return ;
    }
    if (!this->_isArrayType)
    {
        this->set_error_unlocked(FT_ERR_INVALID_OPERATION);
        this->unlock_internal(lock_acquired);
        return ;
    }
    previous_array = this->_managedPointer;
    previous_reference = this->_referenceCount;
    new_array = new (std::nothrow) ManagedType[this->_arraySize + 1];
    if (!new_array)
    {
        this->set_error_unlocked(FT_ERR_NO_MEMORY);
        this->unlock_internal(lock_acquired);
        return ;
    }
    new_count = new (std::nothrow) int;
    if (!new_count)
    {
        delete[] new_array;
        this->set_error_unlocked(FT_ERR_NO_MEMORY);
        this->unlock_internal(lock_acquired);
        return ;
    }
    *new_count = 1;
    detach_required = false;
    if (previous_reference && *previous_reference > 1)
        detach_required = true;
    index = 0;
    while (index < this->_arraySize)
    {
        if (previous_array)
            new_array[index] = previous_array[index];
        else
            new_array[index] = ManagedType();
        ++index;
    }
    new_array[this->_arraySize] = element;
    if (detach_required)
        --(*previous_reference);
    else
    {
        if (previous_array)
            delete[] previous_array;
        if (previous_reference)
            delete previous_reference;
    }
    this->_managedPointer = new_array;
    this->_referenceCount = new_count;
    this->_arraySize = this->_arraySize + 1;
    this->_isArrayType = true;
    this->set_error_unlocked(ER_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename ManagedType>
void ft_sharedptr<ManagedType>::remove(int index)
{
    bool lock_acquired;
    ManagedType* previous_array;
    int* previous_reference;
    ManagedType* new_array;
    int* new_count;
    size_t copy_index;
    size_t new_size;
    bool detach_required;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error_unlocked(ft_errno);
        return ;
    }
    if (!this->_isArrayType)
    {
        this->set_error_unlocked(FT_ERR_INVALID_OPERATION);
        this->unlock_internal(lock_acquired);
        return ;
    }
    if (index < 0 || static_cast<size_t>(index) >= this->_arraySize)
    {
        this->set_error_unlocked(FT_ERR_OUT_OF_RANGE);
        this->unlock_internal(lock_acquired);
        return ;
    }
    new_size = this->_arraySize > 0 ? this->_arraySize - 1 : 0;
    previous_array = this->_managedPointer;
    previous_reference = this->_referenceCount;
    new_array = ft_nullptr;
    new_count = ft_nullptr;
    if (new_size > 0)
    {
        new_array = new (std::nothrow) ManagedType[new_size];
        if (!new_array)
        {
            this->set_error_unlocked(FT_ERR_NO_MEMORY);
            this->unlock_internal(lock_acquired);
            return ;
        }
        new_count = new (std::nothrow) int;
        if (!new_count)
        {
            delete[] new_array;
            this->set_error_unlocked(FT_ERR_NO_MEMORY);
            this->unlock_internal(lock_acquired);
            return ;
        }
        *new_count = 1;
    }
    detach_required = false;
    if (previous_reference && *previous_reference > 1)
        detach_required = true;
    copy_index = 0;
    while (copy_index < static_cast<size_t>(index))
    {
        if (new_array)
            new_array[copy_index] = previous_array ? previous_array[copy_index] : ManagedType();
        ++copy_index;
    }
    while (copy_index < new_size)
    {
        if (new_array)
            new_array[copy_index] = previous_array[copy_index + 1];
        ++copy_index;
    }
    if (detach_required)
        --(*previous_reference);
    else
    {
        if (previous_array)
            delete[] previous_array;
        if (previous_reference)
            delete previous_reference;
    }
    this->_managedPointer = new_array;
    this->_referenceCount = new_count;
    this->_arraySize = new_size;
    this->_isArrayType = true;
    this->set_error_unlocked(ER_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

template <typename ManagedType>
void ft_sharedptr<ManagedType>::set_error_unlocked(int error) const
{
    ft_sharedptr<ManagedType>* mutable_sharedptr;

    mutable_sharedptr = const_cast<ft_sharedptr<ManagedType>*>(this);
    mutable_sharedptr->_error_code = error;
    ft_errno = error;
    return ;
}

template <typename ManagedType>
int ft_sharedptr<ManagedType>::lock_internal(bool *lock_acquired) const
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
void ft_sharedptr<ManagedType>::unlock_internal(bool lock_acquired) const
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
int ft_sharedptr<ManagedType>::prepare_thread_safety()
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
void ft_sharedptr<ManagedType>::teardown_thread_safety()
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
int ft_sharedptr<ManagedType>::enable_thread_safety()
{
    return (this->prepare_thread_safety());
}

template <typename ManagedType>
void ft_sharedptr<ManagedType>::disable_thread_safety()
{
    this->teardown_thread_safety();
    this->set_error_unlocked(ER_SUCCESS);
    return ;
}

template <typename ManagedType>
bool ft_sharedptr<ManagedType>::is_thread_safe() const
{
    bool enabled;

    enabled = (this->_thread_safe_enabled && this->_mutex != ft_nullptr);
    const_cast<ft_sharedptr<ManagedType>*>(this)->set_error_unlocked(ER_SUCCESS);
    return (enabled);
}

template <typename ManagedType>
int ft_sharedptr<ManagedType>::lock(bool *lock_acquired) const
{
    int result;

    result = this->lock_internal(lock_acquired);
    if (result != 0)
        const_cast<ft_sharedptr<ManagedType>*>(this)->set_error_unlocked(ft_errno);
    else
        const_cast<ft_sharedptr<ManagedType>*>(this)->set_error_unlocked(ER_SUCCESS);
    return (result);
}

template <typename ManagedType>
void ft_sharedptr<ManagedType>::unlock(bool lock_acquired) const
{
    int entry_errno;

    entry_errno = ft_errno;
    this->unlock_internal(lock_acquired);
    if (this->_mutex != ft_nullptr && this->_mutex->get_error() != ER_SUCCESS)
        const_cast<ft_sharedptr<ManagedType>*>(this)->set_error_unlocked(this->_mutex->get_error());
    else
    {
        ft_errno = entry_errno;
        const_cast<ft_sharedptr<ManagedType>*>(this)->set_error_unlocked(ft_errno);
    }
    return ;
}

#endif
