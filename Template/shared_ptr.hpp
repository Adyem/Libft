#ifndef SHARED_PTR_HPP
#define SHARED_PTR_HPP

#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "math.hpp"
#include "swap.hpp"
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

        void release();

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
            : _managedPointer(other._managedPointer),
              _referenceCount(other._referenceCount),
              _arraySize(other._arraySize),
              _isArrayType(other._isArrayType),
              _error_code(other._error_code)
        {
            if (_referenceCount && _error_code == ER_SUCCESS)
                ++(*_referenceCount);
        }

        template <typename Other, typename = std::enable_if_t<std::is_convertible_v<Other*,
                 ManagedType*>>>
        ft_sharedptr(ft_sharedptr<Other>&& other) noexcept
            : _managedPointer(other._managedPointer),
              _referenceCount(other._referenceCount),
              _arraySize(other._arraySize),
              _isArrayType(other._isArrayType),
              _error_code(other._error_code)
        {
            other._managedPointer = ft_nullptr;
            other._referenceCount = ft_nullptr;
            other._arraySize = 0;
            other._isArrayType = false;
            other._error_code = ER_SUCCESS;
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
      _referenceCount(new (std::nothrow) int),
      _arraySize(0),
      _isArrayType(false),
      _error_code(ER_SUCCESS)
{
    if (!_referenceCount)
    {
        this->set_error(SHARED_PTR_ALLOCATION_FAILED);
        return ;
    }
    *_referenceCount = 1;
    _managedPointer = new (std::nothrow) ManagedType(std::forward<Args>(args)...);
    if (!_managedPointer)
    {
        this->set_error(SHARED_PTR_ALLOCATION_FAILED);
        delete _referenceCount;
        _referenceCount = ft_nullptr;
        return ;
    }
}

template <typename ManagedType>
ft_sharedptr<ManagedType>::ft_sharedptr(ManagedType* pointer, bool isArray, size_t arraySize)
    : _managedPointer(pointer),
      _referenceCount(ft_nullptr),
      _arraySize(arraySize),
      _isArrayType(isArray),
      _error_code(ER_SUCCESS)
{
    if (pointer)
        _referenceCount = new (std::nothrow) int;
    else
        _referenceCount = ft_nullptr;
    if (_referenceCount)
        *_referenceCount = 1;
    else if (pointer)
        this->set_error(SHARED_PTR_ALLOCATION_FAILED);
}

template <typename ManagedType>
ft_sharedptr<ManagedType>::ft_sharedptr()
    : _managedPointer(ft_nullptr),
      _referenceCount(ft_nullptr),
      _arraySize(0),
      _isArrayType(false),
      _error_code(ER_SUCCESS)
{
}

template <typename ManagedType>
ft_sharedptr<ManagedType>::ft_sharedptr(size_t size)
    : _managedPointer(ft_nullptr),
      _referenceCount(new (std::nothrow) int),
      _arraySize(size),
      _isArrayType(true),
      _error_code(ER_SUCCESS)
{
    if (_referenceCount)
    {
        *_referenceCount = 1;
        _managedPointer = new (std::nothrow) ManagedType[size];
        if (!_managedPointer)
        {
            this->set_error(SHARED_PTR_ALLOCATION_FAILED);
            delete _referenceCount;
            _referenceCount = ft_nullptr;
        }
    }
    else
    {
        this->set_error(SHARED_PTR_ALLOCATION_FAILED);
    }
}

template <typename ManagedType>
ft_sharedptr<ManagedType>::ft_sharedptr(const ft_sharedptr<ManagedType>& other)
    : _managedPointer(other._managedPointer),
      _referenceCount(other._referenceCount),
      _arraySize(other._arraySize),
      _isArrayType(other._isArrayType),
      _error_code(other._error_code)
{
    if (_referenceCount && _error_code == ER_SUCCESS)
        ++(*_referenceCount);
}

template <typename ManagedType>
ft_sharedptr<ManagedType>::ft_sharedptr(ft_sharedptr<ManagedType>&& other) noexcept
    : _managedPointer(other._managedPointer),
      _referenceCount(other._referenceCount),
      _arraySize(other._arraySize),
      _isArrayType(other._isArrayType),
      _error_code(other._error_code)
{
    other._managedPointer = ft_nullptr;
    other._referenceCount = ft_nullptr;
    other._arraySize = 0;
    other._isArrayType = false;
    other._error_code = ER_SUCCESS;
}

template <typename ManagedType>
ft_sharedptr<ManagedType>::~ft_sharedptr()
{
    release();
}

template <typename ManagedType>
void ft_sharedptr<ManagedType>::release()
{
    if (_referenceCount)
    {
        --(*_referenceCount);
        if (*_referenceCount == 0)
        {
            if (_managedPointer)
            {
                if (_isArrayType)
                    delete[] _managedPointer;
                else
                    delete _managedPointer;
            }
            delete _referenceCount;
        }
    }
    _managedPointer = ft_nullptr;
    _referenceCount = ft_nullptr;
    return ;
}

template <typename ManagedType>
ft_sharedptr<ManagedType>& ft_sharedptr<ManagedType>::operator=(ft_sharedptr<ManagedType>&& other) noexcept
{
    if (this != &other)
    {
        release();
        _managedPointer = other._managedPointer;
        _referenceCount = other._referenceCount;
        _arraySize = other._arraySize;
        _isArrayType = other._isArrayType;
        _error_code = other._error_code;
        other._managedPointer = ft_nullptr;
        other._referenceCount = ft_nullptr;
        other._arraySize = 0;
        other._isArrayType = false;
        other._error_code = ER_SUCCESS;
    }
    return (*this);
}

template <typename ManagedType>
ft_sharedptr<ManagedType>& ft_sharedptr<ManagedType>::operator=(const ft_sharedptr<ManagedType>& other)
{
    if (this != &other)
    {
        release();
        _managedPointer = other._managedPointer;
        _referenceCount = other._referenceCount;
        _arraySize = other._arraySize;
        _isArrayType = other._isArrayType;
        _error_code = other._error_code;
        if (_referenceCount && _error_code == ER_SUCCESS)
            ++(*_referenceCount);
    }
    return (*this);
}

template <typename ManagedType>
ManagedType& ft_sharedptr<ManagedType>::operator*()
{
    if (!_managedPointer)
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
    return (*_managedPointer);
}

template <typename ManagedType>
const ManagedType& ft_sharedptr<ManagedType>::operator*() const
{
    if (!_managedPointer)
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
    return (*_managedPointer);
}

template <typename ManagedType>
ManagedType* ft_sharedptr<ManagedType>::operator->()
{
    if (!_managedPointer)
    {
        this->set_error(SHARED_PTR_NULL_PTR);
        return (ft_nullptr);
    }
    this->set_error(ER_SUCCESS);
    return (_managedPointer);
}

template <typename ManagedType>
const ManagedType* ft_sharedptr<ManagedType>::operator->() const
{
    if (!_managedPointer)
    {
        const_cast<ft_sharedptr<ManagedType>*>(this)->set_error(SHARED_PTR_NULL_PTR);
        return (ft_nullptr);
    }
    this->set_error(ER_SUCCESS);
    return (_managedPointer);
}

template <typename ManagedType>
ManagedType& ft_sharedptr<ManagedType>::operator[](size_t index)
{
    if (!_isArrayType)
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
    if (!_managedPointer)
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
    if (index >= _arraySize)
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
    return (_managedPointer[index]);
}

template <typename ManagedType>
const ManagedType& ft_sharedptr<ManagedType>::operator[](size_t index) const
{
    if (!_isArrayType)
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
    if (!_managedPointer)
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
    if (index >= _arraySize)
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
    return (_managedPointer[index]);
}

template <typename ManagedType>
int ft_sharedptr<ManagedType>::use_count() const
{
    if (_referenceCount != ft_nullptr)
    {
        this->set_error(ER_SUCCESS);
        return (*_referenceCount);
    }
    this->set_error(ER_SUCCESS);
    return (0);
}

template <typename ManagedType>
bool ft_sharedptr<ManagedType>::hasError() const
{
    return (_error_code != ER_SUCCESS);
}

template <typename ManagedType>
int ft_sharedptr<ManagedType>::get_error() const
{
    return (_error_code);
}

template <typename ManagedType>
const char* ft_sharedptr<ManagedType>::get_error_str() const
{
    return (ft_strerror(_error_code));
}

template <typename ManagedType>
ManagedType* ft_sharedptr<ManagedType>::get()
{
    this->set_error(ER_SUCCESS);
    return (_managedPointer);
}

template <typename ManagedType>
const ManagedType* ft_sharedptr<ManagedType>::get() const
{
    this->set_error(ER_SUCCESS);
    return (_managedPointer);
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
    _error_code = error;
    return ;
}

template <typename ManagedType>
ft_sharedptr<ManagedType>::operator bool() const noexcept
{
    return (_managedPointer != ft_nullptr);
}

template <typename ManagedType>
void ft_sharedptr<ManagedType>::reset(ManagedType* pointer, size_t size, bool arrayType)
{
    release();
    _managedPointer = pointer;
    _arraySize = size;
    _isArrayType = arrayType;
    _error_code = ER_SUCCESS;
    _referenceCount = new (std::nothrow) int;
    if (_referenceCount)
    {
        *_referenceCount = 1;
    }
    else
    {
        this->set_error(SHARED_PTR_ALLOCATION_FAILED);
        if (_managedPointer)
        {
            if (_isArrayType)
                delete[] _managedPointer;
            else
                delete _managedPointer;
        }
        _managedPointer = ft_nullptr;
    }
    if (this->_referenceCount != ft_nullptr)
        this->set_error(ER_SUCCESS);
    return ;
}

template <typename ManagedType>
void ft_sharedptr<ManagedType>::swap(ft_sharedptr<ManagedType>& other)
{
    ft_swap(_managedPointer, other._managedPointer);
    ft_swap(_referenceCount, other._referenceCount);
    ft_swap(_arraySize, other._arraySize);
    ft_swap(_isArrayType, other._isArrayType);
    ft_swap(_error_code, other._error_code);
    this->set_error(ER_SUCCESS);
    other.set_error(ER_SUCCESS);
    return ;
}

template <typename ManagedType>
void ft_sharedptr<ManagedType>::add(const ManagedType& element)
{
    if (!_isArrayType)
    {
        this->set_error(SHARED_PTR_INVALID_OPERATION);
        return ;
    }
    size_t newSize = _arraySize + 1;
    ManagedType* newArray = new (std::nothrow) ManagedType[newSize];
    if (!newArray)
    {
        this->set_error(SHARED_PTR_ALLOCATION_FAILED);
        return ;
    }
    size_t array_index = 0;
    while (array_index < _arraySize)
    {
        newArray[array_index] = _managedPointer[array_index];
        ++array_index;
    }
    newArray[_arraySize] = element;
    delete[] _managedPointer;
    _managedPointer = newArray;
    _arraySize = newSize;
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ManagedType>
void ft_sharedptr<ManagedType>::remove(int index)
{
    if (!_isArrayType)
    {
        this->set_error(SHARED_PTR_INVALID_OPERATION);
        return ;
    }
    if (!_managedPointer || static_cast<size_t>(index) >= _arraySize || index < 0)
    {
        this->set_error(SHARED_PTR_OUT_OF_BOUNDS);
        return ;
    }
    size_t newSize = _arraySize - 1;
    ManagedType* newArray;
    if (newSize > 0)
        newArray = new (std::nothrow) ManagedType[newSize];
    else
        newArray = ft_nullptr;
    if (newSize > 0 && !newArray)
    {
        this->set_error(SHARED_PTR_ALLOCATION_FAILED);
        return ;
    }
    size_t array_index = 0;
    while (array_index < static_cast<size_t>(index))
    {
        newArray[array_index] = _managedPointer[array_index];
        ++array_index;
    }
    size_t copy_index = index;
    while (copy_index < newSize)
    {
        newArray[copy_index] = _managedPointer[copy_index+1];
        ++copy_index;
    }
    delete[] _managedPointer;
    _managedPointer = newArray;
    _arraySize = newSize;
    this->set_error(ER_SUCCESS);
    return ;
}

#endif
