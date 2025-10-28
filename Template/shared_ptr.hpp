#ifndef SHARED_PTR_HPP
#define SHARED_PTR_HPP

#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "math.hpp"
#include "swap.hpp"
#include "template_concepts.hpp"
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

        void release_current();

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
      _error_code(ER_SUCCESS)
{
    int* reference_count;

    reference_count = new (std::nothrow) int;
    if (!reference_count)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return ;
    }
    ManagedType* pointer = new (std::nothrow) ManagedType(std::forward<Args>(args)...);
    if (!pointer)
    {
        delete reference_count;
        this->set_error(FT_ERR_NO_MEMORY);
        return ;
    }
    *reference_count = 1;
    this->_managedPointer = pointer;
    this->_referenceCount = reference_count;
    this->_arraySize = 1;
    this->_isArrayType = false;
    this->set_error(ER_SUCCESS);
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
      _error_code(ER_SUCCESS)
{
    int* reference_count;

    reference_count = new (std::nothrow) int;
    if (!reference_count)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return ;
    }
    ManagedType* pointer = new (std::nothrow) ManagedType(std::forward<Args>(args)...);
    if (!pointer)
    {
        delete reference_count;
        this->set_error(FT_ERR_NO_MEMORY);
        return ;
    }
    *reference_count = 1;
    this->_managedPointer = pointer;
    this->_referenceCount = reference_count;
    this->_arraySize = 1;
    this->_isArrayType = false;
    this->set_error(ER_SUCCESS);
    return ;
}
#endif

template <typename ManagedType>
ft_sharedptr<ManagedType>::ft_sharedptr(ManagedType* pointer, bool isArray, size_t arraySize)
    : _managedPointer(pointer),
      _referenceCount(ft_nullptr),
      _arraySize(isArray ? arraySize : (pointer ? 1 : 0)),
      _isArrayType(isArray),
      _error_code(ER_SUCCESS)
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
            this->set_error(FT_ERR_NO_MEMORY);
            return ;
        }
        *this->_referenceCount = 1;
    }
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ManagedType>
ft_sharedptr<ManagedType>::ft_sharedptr()
    : _managedPointer(ft_nullptr),
      _referenceCount(ft_nullptr),
      _arraySize(0),
      _isArrayType(false),
      _error_code(ER_SUCCESS)
{
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ManagedType>
ft_sharedptr<ManagedType>::ft_sharedptr(size_t size)
    : _managedPointer(ft_nullptr),
      _referenceCount(ft_nullptr),
      _arraySize(size),
      _isArrayType(true),
      _error_code(ER_SUCCESS)
{
    if (size == 0)
    {
        this->set_error(ER_SUCCESS);
        return ;
    }
    this->_referenceCount = new (std::nothrow) int;
    if (!this->_referenceCount)
    {
        this->_arraySize = 0;
        this->_isArrayType = false;
        this->set_error(FT_ERR_NO_MEMORY);
        return ;
    }
    this->_managedPointer = new (std::nothrow) ManagedType[size];
    if (!this->_managedPointer)
    {
        delete this->_referenceCount;
        this->_referenceCount = ft_nullptr;
        this->_arraySize = 0;
        this->_isArrayType = false;
        this->set_error(FT_ERR_NO_MEMORY);
        return ;
    }
    *this->_referenceCount = 1;
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ManagedType>
ft_sharedptr<ManagedType>::ft_sharedptr(const ft_sharedptr<ManagedType>& other)
    : _managedPointer(other._managedPointer),
      _referenceCount(other._referenceCount),
      _arraySize(other._arraySize),
      _isArrayType(other._isArrayType),
      _error_code(other._error_code)
{
    if (this->_referenceCount)
    {
        ++(*this->_referenceCount);
        this->set_error(ER_SUCCESS);
    }
    else
        this->set_error(other._error_code);
    return ;
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
    return ;
}

template <typename ManagedType>
ft_sharedptr<ManagedType>::~ft_sharedptr()
{
    this->release_current();
    return ;
}

template <typename ManagedType>
void ft_sharedptr<ManagedType>::release_current()
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
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ManagedType>
ft_sharedptr<ManagedType>& ft_sharedptr<ManagedType>::operator=(ft_sharedptr<ManagedType>&& other) noexcept
{
    if (this == &other)
    {
        this->set_error(ER_SUCCESS);
        return (*this);
    }
    this->release_current();
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
    return (*this);
}

template <typename ManagedType>
ft_sharedptr<ManagedType>& ft_sharedptr<ManagedType>::operator=(const ft_sharedptr<ManagedType>& other)
{
    if (this == &other)
    {
        this->set_error(ER_SUCCESS);
        return (*this);
    }
    this->release_current();
    this->_managedPointer = other._managedPointer;
    this->_referenceCount = other._referenceCount;
    this->_arraySize = other._arraySize;
    this->_isArrayType = other._isArrayType;
    this->_error_code = other._error_code;
    if (this->_referenceCount)
    {
        ++(*this->_referenceCount);
        this->set_error(ER_SUCCESS);
    }
    return (*this);
}

#if FT_TEMPLATE_HAS_CONCEPTS
template <typename ManagedType>
template <typename Other>
ft_sharedptr<ManagedType>::ft_sharedptr(const ft_sharedptr<Other>& other)
    requires ft_convertible_to<Other*, ManagedType*>
    : _managedPointer(other._managedPointer),
      _referenceCount(other._referenceCount),
      _arraySize(other._arraySize),
      _isArrayType(other._isArrayType),
      _error_code(other._error_code)
{
    if (this->_referenceCount)
    {
        ++(*this->_referenceCount);
        this->set_error(ER_SUCCESS);
    }
    else
        this->set_error(other._error_code);
    return ;
}
#else
template <typename ManagedType>
template <typename Other, typename>
ft_sharedptr<ManagedType>::ft_sharedptr(const ft_sharedptr<Other>& other)
    : _managedPointer(other._managedPointer),
      _referenceCount(other._referenceCount),
      _arraySize(other._arraySize),
      _isArrayType(other._isArrayType),
      _error_code(other._error_code)
{
    if (this->_referenceCount)
    {
        ++(*this->_referenceCount);
        this->set_error(ER_SUCCESS);
    }
    else
        this->set_error(other._error_code);
    return ;
}
#endif

#if FT_TEMPLATE_HAS_CONCEPTS
template <typename ManagedType>
template <typename Other>
ft_sharedptr<ManagedType>::ft_sharedptr(ft_sharedptr<Other>&& other) noexcept
    requires ft_convertible_to<Other*, ManagedType*>
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
    this->set_error(ER_SUCCESS);
    return ;
}
#else
template <typename ManagedType>
template <typename Other, typename>
ft_sharedptr<ManagedType>::ft_sharedptr(ft_sharedptr<Other>&& other) noexcept
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
    return ;
}
#endif

template <typename ManagedType>
ManagedType& ft_sharedptr<ManagedType>::operator*()
{
    if (!this->_managedPointer)
    {
        this->set_error(FT_ERR_INVALID_POINTER);
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
    this->set_error(ER_SUCCESS);
    return (*this->_managedPointer);
}

template <typename ManagedType>
const ManagedType& ft_sharedptr<ManagedType>::operator*() const
{
    if (!this->_managedPointer)
    {
        const_cast<ft_sharedptr<ManagedType>*>(this)->set_error(FT_ERR_INVALID_POINTER);
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
    const_cast<ft_sharedptr<ManagedType>*>(this)->set_error(ER_SUCCESS);
    return (*this->_managedPointer);
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
    if (!this->_isArrayType)
    {
        this->set_error(FT_ERR_INVALID_OPERATION);
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
        this->set_error(FT_ERR_INVALID_POINTER);
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
        this->set_error(FT_ERR_OUT_OF_RANGE);
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
    this->set_error(ER_SUCCESS);
    return (this->_managedPointer[index]);
}

template <typename ManagedType>
const ManagedType& ft_sharedptr<ManagedType>::operator[](size_t index) const
{
    if (!this->_isArrayType)
    {
        const_cast<ft_sharedptr<ManagedType>*>(this)->set_error(FT_ERR_INVALID_OPERATION);
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
        const_cast<ft_sharedptr<ManagedType>*>(this)->set_error(FT_ERR_INVALID_POINTER);
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
        const_cast<ft_sharedptr<ManagedType>*>(this)->set_error(FT_ERR_OUT_OF_RANGE);
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
    const_cast<ft_sharedptr<ManagedType>*>(this)->set_error(ER_SUCCESS);
    return (this->_managedPointer[index]);
}

template <typename ManagedType>
int ft_sharedptr<ManagedType>::use_count() const
{
    if (!this->_referenceCount)
        return (0);
    return (*this->_referenceCount);
}

template <typename ManagedType>
bool ft_sharedptr<ManagedType>::hasError() const
{
    return (this->_error_code != ER_SUCCESS);
}

template <typename ManagedType>
int ft_sharedptr<ManagedType>::get_error() const
{
    return (this->_error_code);
}

template <typename ManagedType>
const char* ft_sharedptr<ManagedType>::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}

template <typename ManagedType>
ManagedType* ft_sharedptr<ManagedType>::get()
{
    this->set_error(ER_SUCCESS);
    return (this->_managedPointer);
}

template <typename ManagedType>
const ManagedType* ft_sharedptr<ManagedType>::get() const
{
    const_cast<ft_sharedptr<ManagedType>*>(this)->set_error(ER_SUCCESS);
    return (this->_managedPointer);
}

template <typename ManagedType>
bool ft_sharedptr<ManagedType>::unique() const
{
    return (this->use_count() == 1);
}

template <typename ManagedType>
ft_sharedptr<ManagedType>::operator bool() const noexcept
{
    return (this->_managedPointer != ft_nullptr);
}

template <typename ManagedType>
void ft_sharedptr<ManagedType>::set_error(int error) const
{
    ft_errno = error;
    this->_error_code = error;
    return ;
}

template <typename ManagedType>
void ft_sharedptr<ManagedType>::reset(ManagedType* pointer, size_t size, bool arrayType)
{
    this->release_current();
    this->_managedPointer = pointer;
    this->_arraySize = arrayType ? size : (pointer ? 1 : 0);
    this->_isArrayType = arrayType;
    if (!pointer)
    {
        this->_referenceCount = ft_nullptr;
        this->set_error(ER_SUCCESS);
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
        this->set_error(FT_ERR_NO_MEMORY);
        return ;
    }
    *this->_referenceCount = 1;
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ManagedType>
void ft_sharedptr<ManagedType>::swap(ft_sharedptr<ManagedType>& other)
{
    ft_swap(this->_managedPointer, other._managedPointer);
    ft_swap(this->_referenceCount, other._referenceCount);
    ft_swap(this->_arraySize, other._arraySize);
    ft_swap(this->_isArrayType, other._isArrayType);
    ft_swap(this->_error_code, other._error_code);
    return ;
}

template <typename ManagedType>
void ft_sharedptr<ManagedType>::add(const ManagedType& element)
{
    ManagedType* previous_array;
    int* previous_reference;
    ManagedType* new_array;
    int* new_count;
    size_t index;
    bool detach_required;

    if (!this->_isArrayType)
    {
        this->set_error(FT_ERR_INVALID_OPERATION);
        return ;
    }
    previous_array = this->_managedPointer;
    previous_reference = this->_referenceCount;
    new_array = new (std::nothrow) ManagedType[this->_arraySize + 1];
    if (!new_array)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return ;
    }
    new_count = new (std::nothrow) int;
    if (!new_count)
    {
        delete[] new_array;
        this->set_error(FT_ERR_NO_MEMORY);
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
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ManagedType>
void ft_sharedptr<ManagedType>::remove(int index)
{
    ManagedType* previous_array;
    int* previous_reference;
    ManagedType* new_array;
    int* new_count;
    size_t copy_index;
    size_t new_size;
    bool detach_required;

    if (!this->_isArrayType)
    {
        this->set_error(FT_ERR_INVALID_OPERATION);
        return ;
    }
    if (index < 0 || static_cast<size_t>(index) >= this->_arraySize)
    {
        this->set_error(FT_ERR_OUT_OF_RANGE);
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
            this->set_error(FT_ERR_NO_MEMORY);
            return ;
        }
        new_count = new (std::nothrow) int;
        if (!new_count)
        {
            delete[] new_array;
            this->set_error(FT_ERR_NO_MEMORY);
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
    this->set_error(ER_SUCCESS);
    return ;
}

#endif
