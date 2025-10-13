#ifndef UNIQUE_PTR_HPP
#define UNIQUE_PTR_HPP

#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "math.hpp"
#include "swap.hpp"
#include <cstddef>
#include <utility>
#include <type_traits>
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

        void destroy();

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
        ManagedType* release();
        void reset(ManagedType* pointer = ft_nullptr, size_t size = 1,
                bool arrayType = false);
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
      _error_code(ER_SUCCESS)
{
    if (!_managedPointer)
        this->set_error(FT_ERR_NO_MEMORY);
    return ;
}

template <typename ManagedType>
ft_uniqueptr<ManagedType>::ft_uniqueptr(ManagedType* pointer, bool isArray,
        size_t arraySize)
    : _managedPointer(pointer),
      _arraySize(arraySize),
      _isArrayType(isArray),
      _error_code(ER_SUCCESS)
{
    return ;
}

template <typename ManagedType>
ft_uniqueptr<ManagedType>::ft_uniqueptr()
    : _managedPointer(ft_nullptr),
      _arraySize(0),
      _isArrayType(false),
      _error_code(ER_SUCCESS)
{
    return ;
}

template <typename ManagedType>
ft_uniqueptr<ManagedType>::ft_uniqueptr(size_t size)
    : _managedPointer(new (std::nothrow) ManagedType[size]),
      _arraySize(size),
      _isArrayType(true),
      _error_code(ER_SUCCESS)
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
      _error_code(ER_SUCCESS)
{
    _managedPointer = other._managedPointer;
    _arraySize = other._arraySize;
    _isArrayType = other._isArrayType;
    _error_code = other._error_code;
    other._managedPointer = ft_nullptr;
    other._arraySize = 0;
    other._isArrayType = false;
    other._error_code = ER_SUCCESS;
    return ;
}

template <typename ManagedType>
ft_uniqueptr<ManagedType>& ft_uniqueptr<ManagedType>::operator=(ft_uniqueptr&& other)
    noexcept
{
    if (this != &other)
    {
        destroy();
        _managedPointer = other._managedPointer;
        _arraySize = other._arraySize;
        _isArrayType = other._isArrayType;
        _error_code = other._error_code;
        other._managedPointer = ft_nullptr;
        other._arraySize = 0;
        other._isArrayType = false;
        other._error_code = ER_SUCCESS;
    }
    return (*this);
}

template <typename ManagedType>
ft_uniqueptr<ManagedType>::~ft_uniqueptr()
{
    destroy();
    return ;
}

template <typename ManagedType>
void ft_uniqueptr<ManagedType>::destroy()
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
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ManagedType>
ManagedType& ft_uniqueptr<ManagedType>::operator*()
{
    static ManagedType default_instance = ManagedType();
    if (this->_managedPointer == ft_nullptr)
    {
        this->set_error(FT_ERR_INVALID_POINTER);
        if constexpr (!std::is_abstract_v<ManagedType>)
            return (default_instance);
        else
        {
            static char dummy_buffer[sizeof(ManagedType)] = {0};
            return (*reinterpret_cast<ManagedType*>(dummy_buffer));
        }
    }
    ManagedType& reference = *this->_managedPointer;
    this->set_error(ER_SUCCESS);
    return (reference);
}

template <typename ManagedType>
const ManagedType& ft_uniqueptr<ManagedType>::operator*() const
{
    static ManagedType default_instance = ManagedType();
    if (this->_managedPointer == ft_nullptr)
    {
        const_cast<ft_uniqueptr<ManagedType>*>(this)->set_error(FT_ERR_INVALID_POINTER);
        if constexpr (!std::is_abstract_v<ManagedType>)
            return (default_instance);
        else
        {
            static char dummy_buffer[sizeof(ManagedType)] = {0};
            return (*reinterpret_cast<const ManagedType*>(dummy_buffer));
        }
    }
    const ManagedType& reference = *this->_managedPointer;
    this->set_error(ER_SUCCESS);
    return (reference);
}

template <typename ManagedType>
ManagedType* ft_uniqueptr<ManagedType>::operator->()
{
    if (this->_managedPointer == ft_nullptr)
    {
        this->set_error(FT_ERR_INVALID_POINTER);
        return (ft_nullptr);
    }
    ManagedType* pointer = this->_managedPointer;
    this->set_error(ER_SUCCESS);
    return (pointer);
}

template <typename ManagedType>
const ManagedType* ft_uniqueptr<ManagedType>::operator->() const
{
    if (this->_managedPointer == ft_nullptr)
    {
        const_cast<ft_uniqueptr<ManagedType>*>(this)->set_error(FT_ERR_INVALID_POINTER);
        return (ft_nullptr);
    }
    const ManagedType* pointer = this->_managedPointer;
    this->set_error(ER_SUCCESS);
    return (pointer);
}

template <typename ManagedType>
ManagedType& ft_uniqueptr<ManagedType>::operator[](size_t index)
{
    static ManagedType default_instance = ManagedType();
    if (!this->_isArrayType)
    {
        this->set_error(FT_ERR_INVALID_OPERATION);
        if constexpr (!std::is_abstract_v<ManagedType>)
            return (default_instance);
        else
        {
            static char dummy_buffer[sizeof(ManagedType)] = {0};
            return (*reinterpret_cast<ManagedType*>(dummy_buffer));
        }
    }
    if (this->_managedPointer == ft_nullptr)
    {
        this->set_error(FT_ERR_INVALID_POINTER);
        if constexpr (!std::is_abstract_v<ManagedType>)
            return (default_instance);
        else
        {
            static char dummy_buffer[sizeof(ManagedType)] = {0};
            return (*reinterpret_cast<ManagedType*>(dummy_buffer));
        }
    }
    if (index >= this->_arraySize)
    {
        this->set_error(FT_ERR_OUT_OF_RANGE);
        if constexpr (!std::is_abstract_v<ManagedType>)
            return (default_instance);
        else
        {
            static char dummy_buffer[sizeof(ManagedType)] = {0};
            return (*reinterpret_cast<ManagedType*>(dummy_buffer));
        }
    }
    ManagedType& reference = this->_managedPointer[index];
    this->set_error(ER_SUCCESS);
    return (reference);
}

template <typename ManagedType>
const ManagedType& ft_uniqueptr<ManagedType>::operator[](size_t index) const
{
    static ManagedType default_instance = ManagedType();
    if (!this->_isArrayType)
    {
        const_cast<ft_uniqueptr<ManagedType>*>(this)->set_error
            (FT_ERR_INVALID_OPERATION);
        if constexpr (!std::is_abstract_v<ManagedType>)
            return (default_instance);
        else
        {
            static char dummy_buffer[sizeof(ManagedType)] = {0};
            return (*reinterpret_cast<const ManagedType*>(dummy_buffer));
        }
    }
    if (this->_managedPointer == ft_nullptr)
    {
        const_cast<ft_uniqueptr<ManagedType>*>(this)->set_error(FT_ERR_INVALID_POINTER);
        if constexpr (!std::is_abstract_v<ManagedType>)
            return (default_instance);
        else
        {
            static char dummy_buffer[sizeof(ManagedType)] = {0};
            return (*reinterpret_cast<const ManagedType*>(dummy_buffer));
        }
    }
    if (index >= this->_arraySize)
    {
        const_cast<ft_uniqueptr<ManagedType>*>(this)->set_error
            (FT_ERR_OUT_OF_RANGE);
        if constexpr (!std::is_abstract_v<ManagedType>)
            return (default_instance);
        else
        {
            static char dummy_buffer[sizeof(ManagedType)] = {0};
            return (*reinterpret_cast<const ManagedType*>(dummy_buffer));
        }
    }
    const ManagedType& reference = this->_managedPointer[index];
    this->set_error(ER_SUCCESS);
    return (reference);
}

template <typename ManagedType>
ManagedType* ft_uniqueptr<ManagedType>::get()
{
    ManagedType* pointer = this->_managedPointer;
    this->set_error(ER_SUCCESS);
    return (pointer);
}

template <typename ManagedType>
const ManagedType* ft_uniqueptr<ManagedType>::get() const
{
    const ManagedType* pointer = this->_managedPointer;
    const_cast<ft_uniqueptr<ManagedType>*>(this)->set_error(ER_SUCCESS);
    return (pointer);
}

template <typename ManagedType>
ManagedType* ft_uniqueptr<ManagedType>::release()
{
    ManagedType* released_pointer = this->_managedPointer;
    this->_managedPointer = ft_nullptr;
    this->_arraySize = 0;
    this->_isArrayType = false;
    this->_error_code = ER_SUCCESS;
    this->set_error(ER_SUCCESS);
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
    return (this->_error_code != ER_SUCCESS);
}

template <typename ManagedType>
int ft_uniqueptr<ManagedType>::get_error() const
{
    this->set_error(this->_error_code);
    return (this->_error_code);
}

template <typename ManagedType>
const char* ft_uniqueptr<ManagedType>::get_error_str() const
{
    this->set_error(this->_error_code);
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
    ft_swap(this->_managedPointer, other._managedPointer);
    ft_swap(this->_arraySize, other._arraySize);
    ft_swap(this->_isArrayType, other._isArrayType);
    ft_swap(this->_error_code, other._error_code);
    this->set_error(ER_SUCCESS);
    other.set_error(ER_SUCCESS);
    return ;
}

template <typename ManagedType>
void ft_uniqueptr<ManagedType>::set_error(int error) const
{
    ft_errno = error;
    this->_error_code = error;
}

#endif
