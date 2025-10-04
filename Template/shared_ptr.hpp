#ifndef SHARED_PTR_HPP
#define SHARED_PTR_HPP

#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "math.hpp"
#include "swap.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"
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
        pt_mutex* _referenceMutex;
        size_t _arraySize;
        bool _isArrayType;
        mutable pt_mutex _mutex;
        mutable int _error_code;

        void release_locked();

    public:
        template <typename U> friend class ft_sharedptr;

        template <typename... Args, typename = std::enable_if_t<
            !(is_single_convertible_to_size_t<Args...>::value) &&
            std::is_constructible_v<ManagedType, Args&&...>
            >>
        ft_sharedptr(Args&&... args);

        ft_sharedptr(ManagedType* pointer, bool isArray = false, size_t arraySize = 1);
        ft_sharedptr();
        ft_sharedptr(size_t size);
        ft_sharedptr(const ft_sharedptr<ManagedType>& other);
        ft_sharedptr(ft_sharedptr<ManagedType>&& other) noexcept;
        ~ft_sharedptr();

        ft_sharedptr<ManagedType>& operator=(ft_sharedptr<ManagedType>&& other) noexcept;
        ft_sharedptr<ManagedType>& operator=(const ft_sharedptr<ManagedType>& other);

        template <typename Other, typename = std::enable_if_t<std::is_convertible_v<Other*,
                 ManagedType*>>>
        ft_sharedptr(const ft_sharedptr<Other>& other)
            : _managedPointer(ft_nullptr),
              _referenceCount(ft_nullptr),
              _referenceMutex(ft_nullptr),
              _arraySize(0),
              _isArrayType(false),
              _mutex(),
              _error_code(ER_SUCCESS)
        {
            ft_unique_lock<pt_mutex> other_guard(other._mutex);
            if (other_guard.get_error() != ER_SUCCESS)
            {
                this->set_error(other_guard.get_error());
                return ;
            }
            this->_managedPointer = other._managedPointer;
            this->_referenceCount = other._referenceCount;
            this->_referenceMutex = other._referenceMutex;
            this->_arraySize = other._arraySize;
            this->_isArrayType = other._isArrayType;
            this->set_error(other._error_code);
            if (this->_referenceCount && this->_referenceMutex && this->_error_code == ER_SUCCESS)
            {
                ft_unique_lock<pt_mutex> shared_guard(*this->_referenceMutex);
                if (shared_guard.get_error() != ER_SUCCESS)
                {
                    this->set_error(shared_guard.get_error());
                    return ;
                }
                ++(*this->_referenceCount);
                this->set_error(ER_SUCCESS);
                return ;
            }
            if (this->_referenceCount && !this->_referenceMutex)
            {
                this->set_error(SHARED_PTR_INVALID_OPERATION);
                return ;
            }
            return ;
        }

        template <typename Other, typename = std::enable_if_t<std::is_convertible_v<Other*,
                 ManagedType*>>>
        ft_sharedptr(ft_sharedptr<Other>&& other) noexcept
            : _managedPointer(ft_nullptr),
              _referenceCount(ft_nullptr),
              _referenceMutex(ft_nullptr),
              _arraySize(0),
              _isArrayType(false),
              _mutex(),
              _error_code(ER_SUCCESS)
        {
            ft_unique_lock<pt_mutex> other_guard(other._mutex);
            if (other_guard.get_error() != ER_SUCCESS)
            {
                this->set_error(other_guard.get_error());
                return ;
            }
            this->_managedPointer = other._managedPointer;
            this->_referenceCount = other._referenceCount;
            this->_referenceMutex = other._referenceMutex;
            this->_arraySize = other._arraySize;
            this->_isArrayType = other._isArrayType;
            this->set_error(other._error_code);
            other._managedPointer = ft_nullptr;
            other._referenceCount = ft_nullptr;
            other._referenceMutex = ft_nullptr;
            other._arraySize = 0;
            other._isArrayType = false;
            other.set_error(ER_SUCCESS);
            this->set_error(ER_SUCCESS);
            return ;
        }

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

        void reset(ManagedType* pointer = ft_nullptr, size_t size = 1, bool arrayType = false);
        void swap(ft_sharedptr<ManagedType>& other);
        void remove(int index);
        void add(const ManagedType& element);
};

template <typename ManagedType>
template <typename... Args, typename>
ft_sharedptr<ManagedType>::ft_sharedptr(Args&&... args)
    : _managedPointer(ft_nullptr),
      _referenceCount(ft_nullptr),
      _referenceMutex(ft_nullptr),
      _arraySize(0),
      _isArrayType(false),
      _mutex(),
      _error_code(ER_SUCCESS)
{
    this->set_error(ER_SUCCESS);
    this->_referenceCount = new (std::nothrow) int;
    if (!this->_referenceCount)
    {
        this->set_error(SHARED_PTR_ALLOCATION_FAILED);
        return ;
    }
    this->_referenceMutex = new (std::nothrow) pt_mutex;
    if (!this->_referenceMutex)
    {
        delete this->_referenceCount;
        this->_referenceCount = ft_nullptr;
        this->set_error(SHARED_PTR_ALLOCATION_FAILED);
        return ;
    }
    *this->_referenceCount = 1;
    this->_managedPointer = new (std::nothrow) ManagedType(std::forward<Args>(args)...);
    if (!this->_managedPointer)
    {
        delete this->_referenceCount;
        this->_referenceCount = ft_nullptr;
        delete this->_referenceMutex;
        this->_referenceMutex = ft_nullptr;
        this->set_error(SHARED_PTR_ALLOCATION_FAILED);
        return ;
    }
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ManagedType>
ft_sharedptr<ManagedType>::ft_sharedptr(ManagedType* pointer, bool isArray, size_t arraySize)
    : _managedPointer(pointer),
      _referenceCount(ft_nullptr),
      _referenceMutex(ft_nullptr),
      _arraySize(arraySize),
      _isArrayType(isArray),
      _mutex(),
      _error_code(ER_SUCCESS)
{
    this->set_error(ER_SUCCESS);
    if (pointer)
    {
        this->_referenceCount = new (std::nothrow) int;
        if (!this->_referenceCount)
        {
            this->set_error(SHARED_PTR_ALLOCATION_FAILED);
            return ;
        }
        this->_referenceMutex = new (std::nothrow) pt_mutex;
        if (!this->_referenceMutex)
        {
            delete this->_referenceCount;
            this->_referenceCount = ft_nullptr;
            this->set_error(SHARED_PTR_ALLOCATION_FAILED);
            return ;
        }
        *this->_referenceCount = 1;
    }
    return ;
}

template <typename ManagedType>
ft_sharedptr<ManagedType>::ft_sharedptr()
    : _managedPointer(ft_nullptr),
      _referenceCount(ft_nullptr),
      _referenceMutex(ft_nullptr),
      _arraySize(0),
      _isArrayType(false),
      _mutex(),
      _error_code(ER_SUCCESS)
{
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ManagedType>
ft_sharedptr<ManagedType>::ft_sharedptr(size_t size)
    : _managedPointer(ft_nullptr),
      _referenceCount(ft_nullptr),
      _referenceMutex(ft_nullptr),
      _arraySize(size),
      _isArrayType(true),
      _mutex(),
      _error_code(ER_SUCCESS)
{
    this->set_error(ER_SUCCESS);
    this->_referenceCount = new (std::nothrow) int;
    if (!this->_referenceCount)
    {
        this->set_error(SHARED_PTR_ALLOCATION_FAILED);
        return ;
    }
    this->_referenceMutex = new (std::nothrow) pt_mutex;
    if (!this->_referenceMutex)
    {
        delete this->_referenceCount;
        this->_referenceCount = ft_nullptr;
        this->set_error(SHARED_PTR_ALLOCATION_FAILED);
        return ;
    }
    *this->_referenceCount = 1;
    this->_managedPointer = new (std::nothrow) ManagedType[size];
    if (!this->_managedPointer)
    {
        delete this->_referenceCount;
        this->_referenceCount = ft_nullptr;
        delete this->_referenceMutex;
        this->_referenceMutex = ft_nullptr;
        this->set_error(SHARED_PTR_ALLOCATION_FAILED);
        return ;
    }
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ManagedType>
ft_sharedptr<ManagedType>::ft_sharedptr(const ft_sharedptr<ManagedType>& other)
    : _managedPointer(ft_nullptr),
      _referenceCount(ft_nullptr),
      _referenceMutex(ft_nullptr),
      _arraySize(0),
      _isArrayType(false),
      _mutex(),
      _error_code(ER_SUCCESS)
{
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(other_guard.get_error());
        return ;
    }
    this->_managedPointer = other._managedPointer;
    this->_referenceCount = other._referenceCount;
    this->_referenceMutex = other._referenceMutex;
    this->_arraySize = other._arraySize;
    this->_isArrayType = other._isArrayType;
    this->set_error(other._error_code);
    if (this->_referenceCount && this->_referenceMutex && this->_error_code == ER_SUCCESS)
    {
        ft_unique_lock<pt_mutex> shared_guard(*this->_referenceMutex);
        if (shared_guard.get_error() != ER_SUCCESS)
        {
            this->set_error(shared_guard.get_error());
            return ;
        }
        ++(*this->_referenceCount);
        this->set_error(ER_SUCCESS);
        return ;
    }
    if (this->_referenceCount && !this->_referenceMutex)
    {
        this->set_error(SHARED_PTR_INVALID_OPERATION);
        return ;
    }
    return ;
}

template <typename ManagedType>
ft_sharedptr<ManagedType>::ft_sharedptr(ft_sharedptr<ManagedType>&& other) noexcept
    : _managedPointer(ft_nullptr),
      _referenceCount(ft_nullptr),
      _referenceMutex(ft_nullptr),
      _arraySize(0),
      _isArrayType(false),
      _mutex(),
      _error_code(ER_SUCCESS)
{
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(other_guard.get_error());
        return ;
    }
    this->_managedPointer = other._managedPointer;
    this->_referenceCount = other._referenceCount;
    this->_referenceMutex = other._referenceMutex;
    this->_arraySize = other._arraySize;
    this->_isArrayType = other._isArrayType;
    this->set_error(other._error_code);
    other._managedPointer = ft_nullptr;
    other._referenceCount = ft_nullptr;
    other._referenceMutex = ft_nullptr;
    other._arraySize = 0;
    other._isArrayType = false;
    other.set_error(ER_SUCCESS);
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ManagedType>
ft_sharedptr<ManagedType>::~ft_sharedptr()
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->release_locked();
    return ;
}

template <typename ManagedType>
void ft_sharedptr<ManagedType>::release_locked()
{
    ManagedType* pointer_to_delete;
    int* reference_to_delete;
    pt_mutex* mutex_to_delete;
    bool delete_as_array;

    pointer_to_delete = ft_nullptr;
    reference_to_delete = ft_nullptr;
    mutex_to_delete = ft_nullptr;
    delete_as_array = false;
    if (this->_referenceCount && this->_referenceMutex)
    {
        ft_unique_lock<pt_mutex> shared_guard(*this->_referenceMutex);
        if (shared_guard.get_error() != ER_SUCCESS)
        {
            this->set_error(shared_guard.get_error());
            return ;
        }
        --(*this->_referenceCount);
        if (*this->_referenceCount == 0)
        {
            pointer_to_delete = this->_managedPointer;
            reference_to_delete = this->_referenceCount;
            mutex_to_delete = this->_referenceMutex;
            delete_as_array = this->_isArrayType;
        }
        this->_managedPointer = ft_nullptr;
        this->_referenceCount = ft_nullptr;
        this->_referenceMutex = ft_nullptr;
        this->_arraySize = 0;
        this->_isArrayType = false;
        this->set_error(ER_SUCCESS);
    }
    else
    {
        if (this->_managedPointer)
        {
            pointer_to_delete = this->_managedPointer;
            delete_as_array = this->_isArrayType;
        }
        if (this->_referenceCount)
            reference_to_delete = this->_referenceCount;
        if (this->_referenceMutex)
            mutex_to_delete = this->_referenceMutex;
        this->_managedPointer = ft_nullptr;
        this->_referenceCount = ft_nullptr;
        this->_referenceMutex = ft_nullptr;
        this->_arraySize = 0;
        this->_isArrayType = false;
        this->set_error(ER_SUCCESS);
    }
    if (pointer_to_delete)
    {
        if (delete_as_array)
            delete[] pointer_to_delete;
        else
            delete pointer_to_delete;
    }
    if (reference_to_delete)
        delete reference_to_delete;
    if (mutex_to_delete)
        delete mutex_to_delete;
    return ;
}

template <typename ManagedType>
ft_sharedptr<ManagedType>& ft_sharedptr<ManagedType>::operator=(ft_sharedptr<ManagedType>&& other) noexcept
{
    uintptr_t this_address;
    uintptr_t other_address;
    ft_sharedptr<ManagedType>* first_sharedptr;
    ft_sharedptr<ManagedType>* second_sharedptr;

    if (this == &other)
    {
        ft_unique_lock<pt_mutex> self_guard(this->_mutex);
        if (self_guard.get_error() != ER_SUCCESS)
            this->set_error(self_guard.get_error());
        else
            this->set_error(ER_SUCCESS);
        return (*this);
    }
    this_address = reinterpret_cast<uintptr_t>(this);
    other_address = reinterpret_cast<uintptr_t>(&other);
    if (this_address < other_address)
    {
        first_sharedptr = this;
        second_sharedptr = &other;
    }
    else
    {
        first_sharedptr = &other;
        second_sharedptr = this;
    }
    ft_unique_lock<pt_mutex> first_guard(first_sharedptr->_mutex);
    if (first_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(first_guard.get_error());
        return (*this);
    }
    ft_unique_lock<pt_mutex> second_guard(second_sharedptr->_mutex);
    if (second_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(second_guard.get_error());
        return (*this);
    }
    this->release_locked();
    this->_managedPointer = other._managedPointer;
    this->_referenceCount = other._referenceCount;
    this->_referenceMutex = other._referenceMutex;
    this->_arraySize = other._arraySize;
    this->_isArrayType = other._isArrayType;
    other._managedPointer = ft_nullptr;
    other._referenceCount = ft_nullptr;
    other._referenceMutex = ft_nullptr;
    other._arraySize = 0;
    other._isArrayType = false;
    other.set_error(ER_SUCCESS);
    this->set_error(ER_SUCCESS);
    return (*this);
}

template <typename ManagedType>
ft_sharedptr<ManagedType>& ft_sharedptr<ManagedType>::operator=(const ft_sharedptr<ManagedType>& other)
{
    uintptr_t this_address;
    uintptr_t other_address;
    const ft_sharedptr<ManagedType>* first_sharedptr;
    const ft_sharedptr<ManagedType>* second_sharedptr;

    if (this == &other)
    {
        ft_unique_lock<pt_mutex> self_guard(this->_mutex);
        if (self_guard.get_error() != ER_SUCCESS)
            this->set_error(self_guard.get_error());
        else
            this->set_error(ER_SUCCESS);
        return (*this);
    }
    this_address = reinterpret_cast<uintptr_t>(this);
    other_address = reinterpret_cast<uintptr_t>(&other);
    if (this_address < other_address)
    {
        first_sharedptr = this;
        second_sharedptr = &other;
    }
    else
    {
        first_sharedptr = &other;
        second_sharedptr = this;
    }
    ft_unique_lock<pt_mutex> first_guard(first_sharedptr->_mutex);
    if (first_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(first_guard.get_error());
        return (*this);
    }
    ft_unique_lock<pt_mutex> second_guard(second_sharedptr->_mutex);
    if (second_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(second_guard.get_error());
        return (*this);
    }
    this->release_locked();
    this->_managedPointer = other._managedPointer;
    this->_referenceCount = other._referenceCount;
    this->_referenceMutex = other._referenceMutex;
    this->_arraySize = other._arraySize;
    this->_isArrayType = other._isArrayType;
    this->set_error(other._error_code);
    if (this->_referenceCount && this->_referenceMutex && this->_error_code == ER_SUCCESS)
    {
        ft_unique_lock<pt_mutex> shared_guard(*this->_referenceMutex);
        if (shared_guard.get_error() != ER_SUCCESS)
        {
            this->set_error(shared_guard.get_error());
            return (*this);
        }
        ++(*this->_referenceCount);
        this->set_error(ER_SUCCESS);
        return (*this);
    }
    if (this->_referenceCount && !this->_referenceMutex)
    {
        this->set_error(SHARED_PTR_INVALID_OPERATION);
        return (*this);
    }
    this->set_error(ER_SUCCESS);
    return (*this);
}

template <typename ManagedType>
ManagedType& ft_sharedptr<ManagedType>::operator*()
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
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
    if (!this->_managedPointer)
    {
        this->set_error(SHARED_PTR_NULL_PTR);
        if constexpr (!std::is_abstract_v<ManagedType>)
        {
            static ManagedType default_instance;
            return (default_instance);
        }
        else
        {
            this->set_error(SHARED_PTR_ALLOCATION_FAILED);
            static char dummy_buffer[sizeof(ManagedType)] = {0};
            return (*reinterpret_cast<ManagedType*>(dummy_buffer));
        }
    }
    this->set_error(ER_SUCCESS);
    return (*this->_managedPointer);
}

template <typename ManagedType>
const ManagedType& ft_sharedptr<ManagedType>::operator*() const
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_sharedptr<ManagedType>*>(this)->set_error(guard.get_error());
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
    if (!this->_managedPointer)
    {
        const_cast<ft_sharedptr<ManagedType>*>(this)->set_error(SHARED_PTR_NULL_PTR);
        if constexpr (!std::is_abstract_v<ManagedType>)
        {
            static ManagedType default_instance;
            return (default_instance);
        }
        else
        {
            this->set_error(SHARED_PTR_ALLOCATION_FAILED);
            static char dummy_buffer[sizeof(ManagedType)] = {0};
            return (*reinterpret_cast<const ManagedType*>(dummy_buffer));
        }
    }
    this->set_error(ER_SUCCESS);
    return (*this->_managedPointer);
}

template <typename ManagedType>
ManagedType* ft_sharedptr<ManagedType>::operator->()
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return (ft_nullptr);
    }
    if (!this->_managedPointer)
    {
        this->set_error(SHARED_PTR_NULL_PTR);
        return (ft_nullptr);
    }
    this->set_error(ER_SUCCESS);
    return (this->_managedPointer);
}

template <typename ManagedType>
const ManagedType* ft_sharedptr<ManagedType>::operator->() const
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_sharedptr<ManagedType>*>(this)->set_error(guard.get_error());
        return (ft_nullptr);
    }
    if (!this->_managedPointer)
    {
        const_cast<ft_sharedptr<ManagedType>*>(this)->set_error(SHARED_PTR_NULL_PTR);
        return (ft_nullptr);
    }
    this->set_error(ER_SUCCESS);
    return (this->_managedPointer);
}

template <typename ManagedType>
ManagedType& ft_sharedptr<ManagedType>::operator[](size_t index)
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
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
        this->set_error(SHARED_PTR_INVALID_OPERATION);
        if constexpr (!std::is_abstract_v<ManagedType>)
        {
            static ManagedType default_instance;
            return (default_instance);
        }
        else
        {
            this->set_error(SHARED_PTR_ALLOCATION_FAILED);
            return (ft_nullptr);
        }
    }
    if (!this->_managedPointer)
    {
        this->set_error(SHARED_PTR_NULL_PTR);
        if constexpr (!std::is_abstract_v<ManagedType>)
        {
            static ManagedType default_instance;
            return (default_instance);
        }
        else
        {
            this->set_error(SHARED_PTR_ALLOCATION_FAILED);
            return (ft_nullptr);
        }
    }
    if (index >= this->_arraySize)
    {
        this->set_error(SHARED_PTR_OUT_OF_BOUNDS);
        if constexpr (!std::is_abstract_v<ManagedType>)
        {
            static ManagedType default_instance;
            return (default_instance);
        }
        else
        {
            this->set_error(SHARED_PTR_ALLOCATION_FAILED);
            return (ft_nullptr);
        }
    }
    this->set_error(ER_SUCCESS);
    return (this->_managedPointer[index]);
}

template <typename ManagedType>
const ManagedType& ft_sharedptr<ManagedType>::operator[](size_t index) const
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_sharedptr<ManagedType>*>(this)->set_error(guard.get_error());
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
        const_cast<ft_sharedptr<ManagedType>*>(this)->set_error(SHARED_PTR_INVALID_OPERATION);
        if constexpr (!std::is_abstract_v<ManagedType>)
        {
            static ManagedType default_instance;
            return (default_instance);
        }
        else
        {
            this->set_error(SHARED_PTR_ALLOCATION_FAILED);
            return (ft_nullptr);
        }
    }
    if (!this->_managedPointer)
    {
        const_cast<ft_sharedptr<ManagedType>*>(this)->set_error(SHARED_PTR_NULL_PTR);
        if constexpr (!std::is_abstract_v<ManagedType>)
        {
            static ManagedType default_instance;
            return (default_instance);
        }
        else
        {
            this->set_error(SHARED_PTR_ALLOCATION_FAILED);
            return (ft_nullptr);
        }
    }
    if (index >= this->_arraySize)
    {
        const_cast<ft_sharedptr<ManagedType>*>(this)->set_error(SHARED_PTR_OUT_OF_BOUNDS);
        if constexpr (!std::is_abstract_v<ManagedType>)
        {
            static ManagedType default_instance;
            return (default_instance);
        }
        else
        {
            this->set_error(SHARED_PTR_ALLOCATION_FAILED);
            return (ft_nullptr);
        }
    }
    this->set_error(ER_SUCCESS);
    return (this->_managedPointer[index]);
}

template <typename ManagedType>
int ft_sharedptr<ManagedType>::use_count() const
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_sharedptr<ManagedType>*>(this)->set_error(guard.get_error());
        return (0);
    }
    if (!this->_referenceCount)
    {
        this->set_error(ER_SUCCESS);
        return (0);
    }
    if (!this->_referenceMutex)
    {
        this->set_error(SHARED_PTR_INVALID_OPERATION);
        return (0);
    }
    ft_unique_lock<pt_mutex> shared_guard(*this->_referenceMutex);
    if (shared_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(shared_guard.get_error());
        return (0);
    }
    this->set_error(ER_SUCCESS);
    return (*this->_referenceCount);
}

template <typename ManagedType>
bool ft_sharedptr<ManagedType>::hasError() const
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_sharedptr<ManagedType>*>(this)->set_error(guard.get_error());
        return (true);
    }
    return (this->_error_code != ER_SUCCESS);
}

template <typename ManagedType>
int ft_sharedptr<ManagedType>::get_error() const
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_sharedptr<ManagedType>*>(this)->set_error(guard.get_error());
        return (guard.get_error());
    }
    return (this->_error_code);
}

template <typename ManagedType>
const char* ft_sharedptr<ManagedType>::get_error_str() const
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_sharedptr<ManagedType>*>(this)->set_error(guard.get_error());
        return (ft_strerror(guard.get_error()));
    }
    return (ft_strerror(this->_error_code));
}

template <typename ManagedType>
ManagedType* ft_sharedptr<ManagedType>::get()
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return (ft_nullptr);
    }
    this->set_error(ER_SUCCESS);
    return (this->_managedPointer);
}

template <typename ManagedType>
const ManagedType* ft_sharedptr<ManagedType>::get() const
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_sharedptr<ManagedType>*>(this)->set_error(guard.get_error());
        return (ft_nullptr);
    }
    this->set_error(ER_SUCCESS);
    return (this->_managedPointer);
}

template <typename ManagedType>
bool ft_sharedptr<ManagedType>::unique() const
{
    return (this->use_count() == 1);
}

template <typename ManagedType>
void ft_sharedptr<ManagedType>::set_error(int error) const
{
    ft_errno = error;
    this->_error_code = error;
    return ;
}

template <typename ManagedType>
ft_sharedptr<ManagedType>::operator bool() const noexcept
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_sharedptr<ManagedType>*>(this)->set_error(guard.get_error());
        return (false);
    }
    return (this->_managedPointer != ft_nullptr);
}

template <typename ManagedType>
void ft_sharedptr<ManagedType>::reset(ManagedType* pointer, size_t size, bool arrayType)
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->release_locked();
    this->_managedPointer = pointer;
    this->_arraySize = size;
    this->_isArrayType = arrayType;
    this->_referenceCount = ft_nullptr;
    this->_referenceMutex = ft_nullptr;
    this->set_error(ER_SUCCESS);
    if (!this->_managedPointer)
    {
        this->set_error(ER_SUCCESS);
        return ;
    }
    this->_referenceCount = new (std::nothrow) int;
    if (!this->_referenceCount)
    {
        this->set_error(SHARED_PTR_ALLOCATION_FAILED);
        if (this->_managedPointer)
        {
            if (this->_isArrayType)
                delete[] this->_managedPointer;
            else
                delete this->_managedPointer;
        }
        this->_managedPointer = ft_nullptr;
        return ;
    }
    this->_referenceMutex = new (std::nothrow) pt_mutex;
    if (!this->_referenceMutex)
    {
        delete this->_referenceCount;
        this->_referenceCount = ft_nullptr;
        this->set_error(SHARED_PTR_ALLOCATION_FAILED);
        if (this->_managedPointer)
        {
            if (this->_isArrayType)
                delete[] this->_managedPointer;
            else
                delete this->_managedPointer;
        }
        this->_managedPointer = ft_nullptr;
        return ;
    }
    *this->_referenceCount = 1;
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ManagedType>
void ft_sharedptr<ManagedType>::swap(ft_sharedptr<ManagedType>& other)
{
    uintptr_t this_address;
    uintptr_t other_address;
    ft_sharedptr<ManagedType>* first_sharedptr;
    ft_sharedptr<ManagedType>* second_sharedptr;

    if (this == &other)
    {
        ft_unique_lock<pt_mutex> self_guard(this->_mutex);
        if (self_guard.get_error() != ER_SUCCESS)
            this->set_error(self_guard.get_error());
        else
            this->set_error(ER_SUCCESS);
        return ;
    }
    this_address = reinterpret_cast<uintptr_t>(this);
    other_address = reinterpret_cast<uintptr_t>(&other);
    if (this_address < other_address)
    {
        first_sharedptr = this;
        second_sharedptr = &other;
    }
    else
    {
        first_sharedptr = &other;
        second_sharedptr = this;
    }
    ft_unique_lock<pt_mutex> first_guard(first_sharedptr->_mutex);
    if (first_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(first_guard.get_error());
        return ;
    }
    ft_unique_lock<pt_mutex> second_guard(second_sharedptr->_mutex);
    if (second_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(second_guard.get_error());
        return ;
    }
    ft_swap(this->_managedPointer, other._managedPointer);
    ft_swap(this->_referenceCount, other._referenceCount);
    ft_swap(this->_referenceMutex, other._referenceMutex);
    ft_swap(this->_arraySize, other._arraySize);
    ft_swap(this->_isArrayType, other._isArrayType);
    ft_swap(this->_error_code, other._error_code);
    this->set_error(ER_SUCCESS);
    other.set_error(ER_SUCCESS);
    return ;
}

template <typename ManagedType>
void ft_sharedptr<ManagedType>::add(const ManagedType& element)
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    if (!this->_isArrayType)
    {
        this->set_error(SHARED_PTR_INVALID_OPERATION);
        return ;
    }
    if (!this->_managedPointer)
    {
        this->set_error(SHARED_PTR_NULL_PTR);
        return ;
    }
    ft_unique_lock<pt_mutex> shared_guard_placeholder;
    if (this->_referenceMutex)
    {
        shared_guard_placeholder = ft_unique_lock<pt_mutex>(*this->_referenceMutex);
        if (shared_guard_placeholder.get_error() != ER_SUCCESS)
        {
            this->set_error(shared_guard_placeholder.get_error());
            return ;
        }
    }
    size_t new_size;
    ManagedType* new_array;
    ManagedType* old_array;

    new_size = this->_arraySize + 1;
    new_array = new (std::nothrow) ManagedType[new_size];
    if (!new_array)
    {
        this->set_error(SHARED_PTR_ALLOCATION_FAILED);
        return ;
    }
    size_t array_index;

    array_index = 0;
    while (array_index < this->_arraySize)
    {
        new_array[array_index] = this->_managedPointer[array_index];
        ++array_index;
    }
    new_array[this->_arraySize] = element;
    old_array = this->_managedPointer;
    this->_managedPointer = new_array;
    this->_arraySize = new_size;
    if (old_array)
        delete[] old_array;
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ManagedType>
void ft_sharedptr<ManagedType>::remove(int index)
{
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    if (!this->_isArrayType)
    {
        this->set_error(SHARED_PTR_INVALID_OPERATION);
        return ;
    }
    if (!this->_managedPointer || static_cast<size_t>(index) >= this->_arraySize || index < 0)
    {
        this->set_error(SHARED_PTR_OUT_OF_BOUNDS);
        return ;
    }
    ft_unique_lock<pt_mutex> shared_guard_placeholder;
    if (this->_referenceMutex)
    {
        shared_guard_placeholder = ft_unique_lock<pt_mutex>(*this->_referenceMutex);
        if (shared_guard_placeholder.get_error() != ER_SUCCESS)
        {
            this->set_error(shared_guard_placeholder.get_error());
            return ;
        }
    }
    size_t new_size;
    ManagedType* new_array;
    ManagedType* old_array;

    new_size = this->_arraySize - 1;
    new_array = ft_nullptr;
    if (new_size > 0)
    {
        new_array = new (std::nothrow) ManagedType[new_size];
        if (!new_array)
        {
            this->set_error(SHARED_PTR_ALLOCATION_FAILED);
            return ;
        }
    }
    if (new_array)
    {
        size_t array_index;
        size_t copy_index;

        array_index = 0;
        while (array_index < static_cast<size_t>(index))
        {
            new_array[array_index] = this->_managedPointer[array_index];
            ++array_index;
        }
        copy_index = static_cast<size_t>(index);
        while (copy_index < new_size)
        {
            new_array[copy_index] = this->_managedPointer[copy_index + 1];
            ++copy_index;
        }
    }
    old_array = this->_managedPointer;
    this->_managedPointer = new_array;
    this->_arraySize = new_size;
    if (old_array)
        delete[] old_array;
    this->set_error(ER_SUCCESS);
    return ;
}

#endif
