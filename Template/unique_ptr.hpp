#ifndef UNIQUE_PTR_HPP
#define UNIQUE_PTR_HPP

#include "../Errno/errno.hpp"
#include "../CPP_class/nullptr.hpp"
#include "math.hpp"
#include <cstddef>
#include <utility>
#include <type_traits>
#include <new>

template <typename ManagedType>
class ft_uniqueptr
{
    private:
        ManagedType* _managedPointer;
        size_t _arraySize;
        bool _isArrayType;
        mutable int _errorCode;

        void release();

    public:
        template <typename... Args, typename = std::enable_if_t<
            !(is_single_convertible_to_size_t<Args...>::value) &&
            std::is_constructible_v<ManagedType, Args&&...>
            >>
        ft_uniqueptr(Args&&... args);

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
        ManagedType* release_ptr();
        void reset(ManagedType* pointer = ft_nullptr, size_t size = 1, bool arrayType = false);
        bool hasError() const;
        int get_error() const;
        const char* get_error_str() const;
        explicit operator bool() const noexcept;
        void swap(ft_uniqueptr& other);
        void set_error(int error) const;
};

template <typename ManagedType>
template <typename... Args, typename>
ft_uniqueptr<ManagedType>::ft_uniqueptr(Args&&... args)
    : _managedPointer(new (std::nothrow) ManagedType(std::forward<Args>(args)...)),
      _arraySize(1),
      _isArrayType(false),
      _errorCode(ER_SUCCESS)
{
	if (!_managedPointer)
        this->set_error(UNIQUE_PTR_ALLOCATION_FAILED);
	return ;
}

template <typename ManagedType>
ft_uniqueptr<ManagedType>::ft_uniqueptr(ManagedType* pointer, bool isArray, size_t arraySize)
    : _managedPointer(pointer),
      _arraySize(arraySize),
      _isArrayType(isArray),
      _errorCode(ER_SUCCESS)
{
	return ;
}

template <typename ManagedType>
ft_uniqueptr<ManagedType>::ft_uniqueptr()
    : _managedPointer(ft_nullptr),
      _arraySize(0),
      _isArrayType(false),
      _errorCode(ER_SUCCESS)
{
	return ;
}

template <typename ManagedType>
ft_uniqueptr<ManagedType>::ft_uniqueptr(size_t size)
    : _managedPointer(new (std::nothrow) ManagedType[size]),
      _arraySize(size),
      _isArrayType(true),
      _errorCode(ER_SUCCESS)
{
	if (size > 0 && !_managedPointer)
        this->set_error(UNIQUE_PTR_ALLOCATION_FAILED);
	return ;
}

template <typename ManagedType>
ft_uniqueptr<ManagedType>::ft_uniqueptr(ft_uniqueptr&& other) noexcept
    : _managedPointer(other._managedPointer),
      _arraySize(other._arraySize),
      _isArrayType(other._isArrayType),
      _errorCode(other._errorCode)
{
    other._managedPointer = ft_nullptr;
    other._arraySize = 0;
    other._isArrayType = false;
    other._errorCode = ER_SUCCESS;
	return ;
}

template <typename ManagedType>
ft_uniqueptr<ManagedType>& ft_uniqueptr<ManagedType>::operator=(ft_uniqueptr&& other) noexcept
{
    if (this != &other)
    {
        release();
        _managedPointer = other._managedPointer;
        _arraySize = other._arraySize;
        _isArrayType = other._isArrayType;
        _errorCode = other._errorCode;
        other._managedPointer = ft_nullptr;
        other._arraySize = 0;
        other._isArrayType = false;
        other._errorCode = ER_SUCCESS;
    }
    return (*this);
}

template <typename ManagedType>
ft_uniqueptr<ManagedType>::~ft_uniqueptr()
{
    release();
	return ;
}

template <typename ManagedType>
void ft_uniqueptr<ManagedType>::release()
{
    if (_managedPointer)
    {
        if (_isArrayType)
            delete[] _managedPointer;
        else
            delete _managedPointer;
    }
    this->_managedPointer = ft_nullptr;
    this->_arraySize = 0;
    this->_isArrayType = false;
	return ;
}

template <typename ManagedType>
ManagedType& ft_uniqueptr<ManagedType>::operator*()
{
    if (!_managedPointer)
    {
        this->set_error(UNIQUE_PTR_NULL_PTR);
        if constexpr (!std::is_abstract_v<ManagedType>)
        {
            static ManagedType defaultInstance;
            return (defaultInstance);
        }
        else
        {
            this->set_error(UNIQUE_PTR_ALLOCATION_FAILED);
            static char dummy_buffer[sizeof(ManagedType)] = {0};
            return (*reinterpret_cast<ManagedType*>(dummy_buffer));
        }
    }
    return (*_managedPointer);
}

template <typename ManagedType>
const ManagedType& ft_uniqueptr<ManagedType>::operator*() const
{
    if (!_managedPointer)
    {
        const_cast<ft_uniqueptr<ManagedType>*>(this)->set_error(UNIQUE_PTR_NULL_PTR);
        if constexpr (!std::is_abstract_v<ManagedType>)
        {
            static ManagedType defaultInstance;
            return (defaultInstance);
        }
        else
        {
            this->set_error(UNIQUE_PTR_ALLOCATION_FAILED);
            static char dummy_buffer[sizeof(ManagedType)] = {0};
            return (*reinterpret_cast<const ManagedType*>(dummy_buffer));
        }
    }
    return (*_managedPointer);
}

template <typename ManagedType>
ManagedType* ft_uniqueptr<ManagedType>::operator->()
{
    if (!_managedPointer)
    {
        this->set_error(UNIQUE_PTR_NULL_PTR);
        return (ft_nullptr);
    }
    return (_managedPointer);
}

template <typename ManagedType>
const ManagedType* ft_uniqueptr<ManagedType>::operator->() const
{
    if (!_managedPointer)
    {
        const_cast<ft_uniqueptr<ManagedType>*>(this)->set_error(UNIQUE_PTR_NULL_PTR);
        return (ft_nullptr);
    }
    return (_managedPointer);
}

template <typename ManagedType>
ManagedType& ft_uniqueptr<ManagedType>::operator[](size_t index)
{
    if (!_isArrayType)
    {
        this->set_error(UNIQUE_PTR_INVALID_OPERATION);
        if constexpr (!std::is_abstract_v<ManagedType>)
        {
            static ManagedType defaultInstance;
            return (defaultInstance);
        }
        else
        {
            this->set_error(UNIQUE_PTR_ALLOCATION_FAILED);
            static char dummy_buffer[sizeof(ManagedType)] = {0};
            return (*reinterpret_cast<ManagedType*>(dummy_buffer));
        }
    }
    if (!_managedPointer)
    {
        this->set_error(UNIQUE_PTR_NULL_PTR);
        if constexpr (!std::is_abstract_v<ManagedType>)
        {
            static ManagedType defaultInstance;
            return (defaultInstance);
        }
        else
        {
            this->set_error(UNIQUE_PTR_ALLOCATION_FAILED);
            static char dummy_buffer[sizeof(ManagedType)] = {0};
            return (*reinterpret_cast<ManagedType*>(dummy_buffer));
        }
    }
    if (index >= _arraySize)
    {
        this->set_error(UNIQUE_PTR_OUT_OF_BOUNDS);
        if constexpr (!std::is_abstract_v<ManagedType>)
        {
            static ManagedType defaultInstance;
            return (defaultInstance);
        }
        else
        {
            this->set_error(UNIQUE_PTR_ALLOCATION_FAILED);
            static char dummy_buffer[sizeof(ManagedType)] = {0};
            return (*reinterpret_cast<ManagedType*>(dummy_buffer));
        }
    }
    return (_managedPointer[index]);
}

template <typename ManagedType>
const ManagedType& ft_uniqueptr<ManagedType>::operator[](size_t index) const
{
    if (!_isArrayType)
    {
        const_cast<ft_uniqueptr<ManagedType>*>(this)->set_error(UNIQUE_PTR_INVALID_OPERATION);
        if constexpr (!std::is_abstract_v<ManagedType>)
        {
            static ManagedType defaultInstance;
            return (defaultInstance);
        }
        else
        {
            this->set_error(UNIQUE_PTR_ALLOCATION_FAILED);
            static char dummy_buffer[sizeof(ManagedType)] = {0};
            return (*reinterpret_cast<const ManagedType*>(dummy_buffer));
        }
    }
    if (!_managedPointer)
    {
        const_cast<ft_uniqueptr<ManagedType>*>(this)->set_error(UNIQUE_PTR_NULL_PTR);
        if constexpr (!std::is_abstract_v<ManagedType>)
        {
            static ManagedType defaultInstance;
            return (defaultInstance);
        }
        else
        {
            this->set_error(UNIQUE_PTR_ALLOCATION_FAILED);
            static char dummy_buffer[sizeof(ManagedType)] = {0};
            return (*reinterpret_cast<const ManagedType*>(dummy_buffer));
        }
    }
    if (index >= _arraySize)
    {
        const_cast<ft_uniqueptr<ManagedType>*>(this)->set_error(UNIQUE_PTR_OUT_OF_BOUNDS);
        if constexpr (!std::is_abstract_v<ManagedType>)
        {
            static ManagedType defaultInstance;
            return (defaultInstance);
        }
        else
        {
            this->set_error(UNIQUE_PTR_ALLOCATION_FAILED);
            static char dummy_buffer[sizeof(ManagedType)] = {0};
            return (*reinterpret_cast<const ManagedType*>(dummy_buffer));
        }
    }
    return (_managedPointer[index]);
}

template <typename ManagedType>
ManagedType* ft_uniqueptr<ManagedType>::get()
{
    return (_managedPointer);
}

template <typename ManagedType>
const ManagedType* ft_uniqueptr<ManagedType>::get() const
{
    return (_managedPointer);
}

template <typename ManagedType>
ManagedType* ft_uniqueptr<ManagedType>::release_ptr()
{
    ManagedType* tmp = _managedPointer;
    _managedPointer = ft_nullptr;
    _arraySize = 0;
    _isArrayType = false;
    _errorCode = ER_SUCCESS;
    return (tmp);
}

template <typename ManagedType>
void ft_uniqueptr<ManagedType>::reset(ManagedType* pointer, size_t size, bool arrayType)
{
    release();
    _managedPointer = pointer;
    _arraySize = size;
    _isArrayType = arrayType;
    _errorCode = ER_SUCCESS;
}

template <typename ManagedType>
bool ft_uniqueptr<ManagedType>::hasError() const
{
    return (_errorCode != ER_SUCCESS);
}

template <typename ManagedType>
int ft_uniqueptr<ManagedType>::get_error() const
{
    return (_errorCode);
}

template <typename ManagedType>
const char* ft_uniqueptr<ManagedType>::get_error_str() const
{
    return (ft_strerror(_errorCode));
}

template <typename ManagedType>
ft_uniqueptr<ManagedType>::operator bool() const noexcept
{
    return (_managedPointer != ft_nullptr);
}

template <typename ManagedType>
void ft_uniqueptr<ManagedType>::swap(ft_uniqueptr& other)
{
    std::swap(_managedPointer, other._managedPointer);
    std::swap(_arraySize, other._arraySize);
    std::swap(_isArrayType, other._isArrayType);
    std::swap(_errorCode, other._errorCode);
}

template <typename ManagedType>
void ft_uniqueptr<ManagedType>::set_error(int error) const
{
    ft_errno = error;
    _errorCode = error;
}

#endif
