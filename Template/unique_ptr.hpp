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
#include "../PThread/mutex.hpp"

template <typename ManagedType>
class ft_uniqueptr
{
    private:
        ManagedType* _managedPointer;
        size_t _arraySize;
        bool _isArrayType;
        mutable int _error_code;
        mutable pt_mutex _mutex;

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
    if (other._mutex.lock(THREAD_ID) != FT_SUCCESS)
        return ;
    _managedPointer = other._managedPointer;
    _arraySize = other._arraySize;
    _isArrayType = other._isArrayType;
    _error_code = other._error_code;
    other._managedPointer = ft_nullptr;
    other._arraySize = 0;
    other._isArrayType = false;
    other._error_code = ER_SUCCESS;
    other._mutex.unlock(THREAD_ID);
    return ;
}

template <typename ManagedType>
ft_uniqueptr<ManagedType>& ft_uniqueptr<ManagedType>::operator=(ft_uniqueptr&& other)
    noexcept
{
    if (this != &other)
    {
        destroy();
        if (other._mutex.lock(THREAD_ID) != FT_SUCCESS)
            return (*this);
        _managedPointer = other._managedPointer;
        _arraySize = other._arraySize;
        _isArrayType = other._isArrayType;
        _error_code = other._error_code;
        other._managedPointer = ft_nullptr;
        other._arraySize = 0;
        other._isArrayType = false;
        other._error_code = ER_SUCCESS;
        other._mutex.unlock(THREAD_ID);
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
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
        return ;
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
    this->_mutex.unlock(THREAD_ID);
    return ;
}

template <typename ManagedType>
ManagedType& ft_uniqueptr<ManagedType>::operator*()
{
    static ManagedType default_instance = ManagedType();
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        set_error(FT_ERR_MUTEX_NOT_OWNER);
        return (default_instance);
    }
    if (!_managedPointer)
    {
        this->set_error(FT_ERR_INVALID_POINTER);
        this->_mutex.unlock(THREAD_ID);
        if constexpr (!std::is_abstract_v<ManagedType>)
            return (default_instance);
        else
        {
            static char dummy_buffer[sizeof(ManagedType)] = {0};
            return (*reinterpret_cast<ManagedType*>(dummy_buffer));
        }
    }
    ManagedType& ref = *_managedPointer;
    this->set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
    return (ref);
}

template <typename ManagedType>
const ManagedType& ft_uniqueptr<ManagedType>::operator*() const
{
    static ManagedType default_instance = ManagedType();
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        const_cast<ft_uniqueptr<ManagedType>*>(this)->set_error(FT_ERR_MUTEX_NOT_OWNER);
        return (default_instance);
    }
    if (!_managedPointer)
    {
        const_cast<ft_uniqueptr<ManagedType>*>(this)->set_error(FT_ERR_INVALID_POINTER);
        this->_mutex.unlock(THREAD_ID);
        if constexpr (!std::is_abstract_v<ManagedType>)
            return (default_instance);
        else
        {
            static char dummy_buffer[sizeof(ManagedType)] = {0};
            return (*reinterpret_cast<const ManagedType*>(dummy_buffer));
        }
    }
    const ManagedType& ref = *_managedPointer;
    this->set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
    return (ref);
}

template <typename ManagedType>
ManagedType* ft_uniqueptr<ManagedType>::operator->()
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        set_error(FT_ERR_MUTEX_NOT_OWNER);
        return (ft_nullptr);
    }
    if (!_managedPointer)
    {
        this->set_error(FT_ERR_INVALID_POINTER);
        this->_mutex.unlock(THREAD_ID);
        return (ft_nullptr);
    }
    ManagedType* ptr = _managedPointer;
    this->set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
    return (ptr);
}

template <typename ManagedType>
const ManagedType* ft_uniqueptr<ManagedType>::operator->() const
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        const_cast<ft_uniqueptr<ManagedType>*>(this)->set_error(FT_ERR_MUTEX_NOT_OWNER);
        return (ft_nullptr);
    }
    if (!_managedPointer)
    {
        const_cast<ft_uniqueptr<ManagedType>*>(this)->set_error(FT_ERR_INVALID_POINTER);
        this->_mutex.unlock(THREAD_ID);
        return (ft_nullptr);
    }
    const ManagedType* ptr = _managedPointer;
    this->set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
    return (ptr);
}

template <typename ManagedType>
ManagedType& ft_uniqueptr<ManagedType>::operator[](size_t index)
{
    static ManagedType default_instance = ManagedType();
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        set_error(FT_ERR_MUTEX_NOT_OWNER);
        return (default_instance);
    }
    if (!_isArrayType)
    {
        this->set_error(FT_ERR_INVALID_OPERATION);
        this->_mutex.unlock(THREAD_ID);
        if constexpr (!std::is_abstract_v<ManagedType>)
            return (default_instance);
        else
        {
            static char dummy_buffer[sizeof(ManagedType)] = {0};
            return (*reinterpret_cast<ManagedType*>(dummy_buffer));
        }
    }
    if (!_managedPointer)
    {
        this->set_error(FT_ERR_INVALID_POINTER);
        this->_mutex.unlock(THREAD_ID);
        if constexpr (!std::is_abstract_v<ManagedType>)
            return (default_instance);
        else
        {
            static char dummy_buffer[sizeof(ManagedType)] = {0};
            return (*reinterpret_cast<ManagedType*>(dummy_buffer));
        }
    }
    if (index >= _arraySize)
    {
        this->set_error(FT_ERR_OUT_OF_RANGE);
        this->_mutex.unlock(THREAD_ID);
        if constexpr (!std::is_abstract_v<ManagedType>)
            return (default_instance);
        else
        {
            static char dummy_buffer[sizeof(ManagedType)] = {0};
            return (*reinterpret_cast<ManagedType*>(dummy_buffer));
        }
    }
    ManagedType& ref = _managedPointer[index];
    this->set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
    return (ref);
}

template <typename ManagedType>
const ManagedType& ft_uniqueptr<ManagedType>::operator[](size_t index) const
{
    static ManagedType default_instance = ManagedType();
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        const_cast<ft_uniqueptr<ManagedType>*>(this)->set_error(FT_ERR_MUTEX_NOT_OWNER);
        return (default_instance);
    }
    if (!_isArrayType)
    {
        const_cast<ft_uniqueptr<ManagedType>*>(this)->set_error
            (FT_ERR_INVALID_OPERATION);
        this->_mutex.unlock(THREAD_ID);
        if constexpr (!std::is_abstract_v<ManagedType>)
            return (default_instance);
        else
        {
            static char dummy_buffer[sizeof(ManagedType)] = {0};
            return (*reinterpret_cast<const ManagedType*>(dummy_buffer));
        }
    }
    if (!_managedPointer)
    {
        const_cast<ft_uniqueptr<ManagedType>*>(this)->set_error(FT_ERR_INVALID_POINTER);
        this->_mutex.unlock(THREAD_ID);
        if constexpr (!std::is_abstract_v<ManagedType>)
            return (default_instance);
        else
        {
            static char dummy_buffer[sizeof(ManagedType)] = {0};
            return (*reinterpret_cast<const ManagedType*>(dummy_buffer));
        }
    }
    if (index >= _arraySize)
    {
        const_cast<ft_uniqueptr<ManagedType>*>(this)->set_error
            (FT_ERR_OUT_OF_RANGE);
        this->_mutex.unlock(THREAD_ID);
        if constexpr (!std::is_abstract_v<ManagedType>)
            return (default_instance);
        else
        {
            static char dummy_buffer[sizeof(ManagedType)] = {0};
            return (*reinterpret_cast<const ManagedType*>(dummy_buffer));
        }
    }
    const ManagedType& ref = _managedPointer[index];
    this->set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
    return (ref);
}

template <typename ManagedType>
ManagedType* ft_uniqueptr<ManagedType>::get()
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        set_error(FT_ERR_MUTEX_NOT_OWNER);
        return (ft_nullptr);
    }
    ManagedType* ptr = _managedPointer;
    this->_mutex.unlock(THREAD_ID);
    this->set_error(ER_SUCCESS);
    return (ptr);
}

template <typename ManagedType>
const ManagedType* ft_uniqueptr<ManagedType>::get() const
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        const_cast<ft_uniqueptr<ManagedType>*>(this)->set_error(FT_ERR_MUTEX_NOT_OWNER);
        return (ft_nullptr);
    }
    const ManagedType* ptr = _managedPointer;
    this->_mutex.unlock(THREAD_ID);
    this->set_error(ER_SUCCESS);
    return (ptr);
}

template <typename ManagedType>
ManagedType* ft_uniqueptr<ManagedType>::release()
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        set_error(FT_ERR_MUTEX_NOT_OWNER);
        return (ft_nullptr);
    }
    ManagedType* tmp = _managedPointer;
    _managedPointer = ft_nullptr;
    _arraySize = 0;
    _isArrayType = false;
    _error_code = ER_SUCCESS;
    this->set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
    return (tmp);
}

template <typename ManagedType>
void ft_uniqueptr<ManagedType>::reset(ManagedType* pointer, size_t size, bool arrayType)
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        set_error(FT_ERR_MUTEX_NOT_OWNER);
        return ;
    }
    if (_managedPointer)
    {
        if (_isArrayType)
            delete[] _managedPointer;
        else
            delete _managedPointer;
    }
    _managedPointer = pointer;
    _arraySize = size;
    _isArrayType = arrayType;
    _error_code = ER_SUCCESS;
    this->set_error(ER_SUCCESS);
    this->_mutex.unlock(THREAD_ID);
}

template <typename ManagedType>
bool ft_uniqueptr<ManagedType>::hasError() const
{
    return (_error_code != ER_SUCCESS);
}

template <typename ManagedType>
int ft_uniqueptr<ManagedType>::get_error() const
{
    return (_error_code);
}

template <typename ManagedType>
const char* ft_uniqueptr<ManagedType>::get_error_str() const
{
    return (ft_strerror(_error_code));
}

template <typename ManagedType>
ft_uniqueptr<ManagedType>::operator bool() const noexcept
{
    return (_managedPointer != ft_nullptr);
}

template <typename ManagedType>
void ft_uniqueptr<ManagedType>::swap(ft_uniqueptr& other)
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
        return ;
    if (other._mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        this->_mutex.unlock(THREAD_ID);
        return ;
    }
    ft_swap(_managedPointer, other._managedPointer);
    ft_swap(_arraySize, other._arraySize);
    ft_swap(_isArrayType, other._isArrayType);
    ft_swap(_error_code, other._error_code);
    this->set_error(ER_SUCCESS);
    other.set_error(ER_SUCCESS);
    other._mutex.unlock(THREAD_ID);
    this->_mutex.unlock(THREAD_ID);
}

template <typename ManagedType>
void ft_uniqueptr<ManagedType>::set_error(int error) const
{
    ft_errno = error;
    _error_code = error;
}

#endif
