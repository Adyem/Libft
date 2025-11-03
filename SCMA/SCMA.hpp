#ifndef SCMA_HPP
# define SCMA_HPP

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"
#include "../Libft/limits.hpp"
#include "../PThread/mutex.hpp"

struct scma_handle
{
    ft_size_t    index;
    ft_size_t    generation;
};

int     scma_initialize(ft_size_t initial_capacity);
void    scma_shutdown(void);
int     scma_is_initialized(void);

scma_handle    scma_allocate(ft_size_t size);
int     scma_free(scma_handle handle);
int     scma_resize(scma_handle handle, ft_size_t new_size);
ft_size_t    scma_get_size(scma_handle handle);
int     scma_handle_is_valid(scma_handle handle);

int     scma_write(scma_handle handle, ft_size_t offset,
            const void *source, ft_size_t size);
int     scma_read(scma_handle handle, ft_size_t offset,
            void *destination, ft_size_t size);
void    *scma_snapshot(scma_handle handle, ft_size_t *size);

void    scma_debug_dump(void);

class scma_mutex_lock_guard
{
    private:
        bool _owns_lock;
        bool _entered;
        mutable int _error_code;

        void    set_error(int error_code) const;

    public:
        scma_mutex_lock_guard(void);
        ~scma_mutex_lock_guard(void);

        bool    owns_lock(void) const;
        int     get_error(void) const;
        const char  *get_error_str(void) const;
};

pt_mutex    &scma_runtime_mutex(void);

template <typename TValue>
class scma_handle_accessor_element_proxy;

template <typename TValue>
class scma_handle_accessor_const_element_proxy;

template <typename TValue>
class scma_handle_accessor
{
    private:
        scma_handle _handle;
        mutable int _error_code;

        void    set_error(int error_code) const;

    public:
        scma_handle_accessor(void);
        explicit scma_handle_accessor(scma_handle handle);
        scma_handle_accessor(const scma_handle_accessor &other);
        scma_handle_accessor(scma_handle_accessor &&other);
        ~scma_handle_accessor(void);

        int     bind(scma_handle handle);
        int     is_bound(void) const;
        scma_handle    get_handle(void) const;

        scma_handle_accessor_element_proxy<TValue>    operator*(void);
        scma_handle_accessor_const_element_proxy<TValue>    operator*(void) const;
        scma_handle_accessor_element_proxy<TValue>    operator->(void);
        scma_handle_accessor_const_element_proxy<TValue>    operator->(void) const;
        scma_handle_accessor_element_proxy<TValue>    operator[](ft_size_t element_index);
        scma_handle_accessor_const_element_proxy<TValue>    operator[](ft_size_t element_index) const;

        int     read_struct(TValue &destination) const;
        int     write_struct(const TValue &source) const;
        int     read_at(TValue &destination, ft_size_t element_index) const;
        int     write_at(const TValue &source, ft_size_t element_index) const;
        ft_size_t    get_count(void) const;

        int     get_error(void) const;
        const char  *get_error_str(void) const;
};

template <typename TValue>
inline scma_handle_accessor<TValue>::scma_handle_accessor(void)
{
    this->_handle.index = static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX);
    this->_handle.generation = static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX);
    this->_error_code = ER_SUCCESS;
    return ;
}

template <typename TValue>
inline scma_handle_accessor<TValue>::scma_handle_accessor(scma_handle handle)
{
    this->_handle.index = static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX);
    this->_handle.generation = static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX);
    this->_error_code = ER_SUCCESS;
    this->bind(handle);
    return ;
}

template <typename TValue>
inline scma_handle_accessor<TValue>::scma_handle_accessor(const scma_handle_accessor &other)
{
    this->_handle.index = static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX);
    this->_handle.generation = static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX);
    this->_error_code = ER_SUCCESS;
    if (!other.is_bound())
    {
        this->set_error(other._error_code);
        return ;
    }
    scma_mutex_lock_guard mutex_guard;
    if (mutex_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(mutex_guard.get_error());
        return ;
    }
    if (!scma_handle_is_valid(other._handle))
    {
        this->set_error(ft_errno);
        return ;
    }
    this->_handle = other._handle;
    this->set_error(other._error_code);
    return ;
}

template <typename TValue>
inline scma_handle_accessor<TValue>::scma_handle_accessor(scma_handle_accessor &&other)
{
    this->_handle.index = static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX);
    this->_handle.generation = static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX);
    this->_error_code = ER_SUCCESS;
    scma_mutex_lock_guard mutex_guard;
    if (mutex_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(mutex_guard.get_error());
        return ;
    }
    if (!other.is_bound())
    {
        this->set_error(other._error_code);
        other._handle.index = static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX);
        other._handle.generation = static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX);
        return ;
    }
    if (!scma_handle_is_valid(other._handle))
    {
        this->set_error(ft_errno);
        other._handle.index = static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX);
        other._handle.generation = static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX);
        return ;
    }
    this->_handle = other._handle;
    this->set_error(other._error_code);
    other._handle.index = static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX);
    other._handle.generation = static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX);
    other._error_code = ER_SUCCESS;
    return ;
}

template <typename TValue>
inline scma_handle_accessor<TValue>::~scma_handle_accessor(void)
{
    return ;
}

template <typename TValue>
inline void    scma_handle_accessor<TValue>::set_error(int error_code) const
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}

template <typename TValue>
inline int    scma_handle_accessor<TValue>::bind(scma_handle handle)
{
    scma_mutex_lock_guard mutex_guard;
    if (mutex_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(mutex_guard.get_error());
        return (0);
    }
    if (!scma_handle_is_valid(handle))
    {
        this->set_error(ft_errno);
        return (0);
    }
    this->_handle = handle;
    this->set_error(ER_SUCCESS);
    return (1);
}

template <typename TValue>
inline int    scma_handle_accessor<TValue>::is_bound(void) const
{
    if (this->_handle.index == static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX))
        return (0);
    if (this->_handle.generation == static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX))
        return (0);
    return (1);
}

template <typename TValue>
inline scma_handle    scma_handle_accessor<TValue>::get_handle(void) const
{
    return (this->_handle);
}

template <typename TValue>
inline scma_handle_accessor_element_proxy<TValue>    scma_handle_accessor<TValue>::operator*(void)
{
    return (scma_handle_accessor_element_proxy<TValue>(this, 0));
}

template <typename TValue>
inline scma_handle_accessor_const_element_proxy<TValue>    scma_handle_accessor<TValue>::operator*(void) const
{
    return (scma_handle_accessor_const_element_proxy<TValue>(this, 0));
}

template <typename TValue>
inline scma_handle_accessor_element_proxy<TValue>    scma_handle_accessor<TValue>::operator->(void)
{
    return (scma_handle_accessor_element_proxy<TValue>(this, 0));
}

template <typename TValue>
inline scma_handle_accessor_const_element_proxy<TValue>    scma_handle_accessor<TValue>::operator->(void) const
{
    return (scma_handle_accessor_const_element_proxy<TValue>(this, 0));
}

template <typename TValue>
inline scma_handle_accessor_element_proxy<TValue>    scma_handle_accessor<TValue>::operator[](ft_size_t element_index)
{
    return (scma_handle_accessor_element_proxy<TValue>(this, element_index));
}

template <typename TValue>
inline scma_handle_accessor_const_element_proxy<TValue>    scma_handle_accessor<TValue>::operator[](ft_size_t element_index) const
{
    return (scma_handle_accessor_const_element_proxy<TValue>(this, element_index));
}

template <typename TValue>
inline ft_size_t    scma_handle_accessor_calculate_offset(ft_size_t element_index)
{
    size_t host_size;
    ft_size_t element_size;
    ft_size_t offset;

    host_size = sizeof(TValue);
    if (host_size == 0)
        return (0);
    if (host_size > static_cast<size_t>(FT_SYSTEM_SIZE_MAX))
        return (0);
    element_size = static_cast<ft_size_t>(host_size);
    if (element_index > static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX) / element_size)
        return (0);
    offset = element_index * element_size;
    return (offset);
}

template <typename TValue>
inline int    scma_handle_accessor_validate_target(scma_handle handle, ft_size_t required_size, ft_size_t *out_size)
{
    scma_mutex_lock_guard mutex_guard;
    if (mutex_guard.get_error() != ER_SUCCESS)
    {
        ft_errno = mutex_guard.get_error();
        return (0);
    }
    if (!scma_handle_is_valid(handle))
        return (0);
    ft_size_t block_size;

    block_size = scma_get_size(handle);
    if (block_size < required_size)
    {
        ft_errno = FT_ERR_OUT_OF_RANGE;
        return (0);
    }
    if (out_size)
        *out_size = block_size;
    return (1);
}

template <typename TValue>
inline int    scma_handle_accessor<TValue>::read_struct(TValue &destination) const
{
    scma_mutex_lock_guard mutex_guard;
    if (mutex_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(mutex_guard.get_error());
        return (0);
    }
    if (!std::is_trivially_copyable<TValue>::value)
    {
        this->set_error(FT_ERR_UNSUPPORTED_TYPE);
        return (0);
    }
    if (!this->is_bound())
    {
        this->set_error(FT_ERR_INVALID_HANDLE);
        return (0);
    }
    ft_size_t required_size;

    size_t host_size;

    host_size = sizeof(TValue);
    if (host_size > static_cast<size_t>(FT_SYSTEM_SIZE_MAX))
    {
        this->set_error(FT_ERR_OUT_OF_RANGE);
        return (0);
    }
    required_size = static_cast<ft_size_t>(host_size);
    if (!scma_handle_accessor_validate_target<TValue>(this->_handle, required_size, ft_nullptr))
    {
        this->set_error(ft_errno);
        return (0);
    }
    if (!scma_read(this->_handle, 0, &destination, required_size))
    {
        this->set_error(ft_errno);
        return (0);
    }
    this->set_error(ER_SUCCESS);
    return (1);
}

template <typename TValue>
inline int    scma_handle_accessor<TValue>::write_struct(const TValue &source) const
{
    scma_mutex_lock_guard mutex_guard;
    if (mutex_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(mutex_guard.get_error());
        return (0);
    }
    if (!std::is_trivially_copyable<TValue>::value)
    {
        this->set_error(FT_ERR_UNSUPPORTED_TYPE);
        return (0);
    }
    if (!this->is_bound())
    {
        this->set_error(FT_ERR_INVALID_HANDLE);
        return (0);
    }
    ft_size_t required_size;

    size_t host_size;

    host_size = sizeof(TValue);
    if (host_size > static_cast<size_t>(FT_SYSTEM_SIZE_MAX))
    {
        this->set_error(FT_ERR_OUT_OF_RANGE);
        return (0);
    }
    required_size = static_cast<ft_size_t>(host_size);
    if (!scma_handle_accessor_validate_target<TValue>(this->_handle, required_size, ft_nullptr))
    {
        this->set_error(ft_errno);
        return (0);
    }
    if (!scma_write(this->_handle, 0, &source, required_size))
    {
        this->set_error(ft_errno);
        return (0);
    }
    this->set_error(ER_SUCCESS);
    return (1);
}

template <typename TValue>
inline int    scma_handle_accessor<TValue>::read_at(TValue &destination, ft_size_t element_index) const
{
    scma_mutex_lock_guard mutex_guard;
    if (mutex_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(mutex_guard.get_error());
        return (0);
    }
    if (!std::is_trivially_copyable<TValue>::value)
    {
        this->set_error(FT_ERR_UNSUPPORTED_TYPE);
        return (0);
    }
    if (!this->is_bound())
    {
        this->set_error(FT_ERR_INVALID_HANDLE);
        return (0);
    }
    ft_size_t offset;
    ft_size_t element_size;

    size_t host_size;

    host_size = sizeof(TValue);
    if (host_size > static_cast<size_t>(FT_SYSTEM_SIZE_MAX))
    {
        this->set_error(FT_ERR_OUT_OF_RANGE);
        return (0);
    }
    element_size = static_cast<ft_size_t>(host_size);
    offset = scma_handle_accessor_calculate_offset<TValue>(element_index);
    if (element_size == 0 || (offset == 0 && element_index != 0))
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (0);
    }
    ft_size_t required_size;

    if (offset > static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX) - element_size)
    {
        this->set_error(FT_ERR_OUT_OF_RANGE);
        return (0);
    }
    required_size = offset + element_size;
    if (!scma_handle_accessor_validate_target<TValue>(this->_handle, required_size, ft_nullptr))
    {
        this->set_error(ft_errno);
        return (0);
    }
    if (!scma_read(this->_handle, offset, &destination, element_size))
    {
        this->set_error(ft_errno);
        return (0);
    }
    this->set_error(ER_SUCCESS);
    return (1);
}

template <typename TValue>
inline int    scma_handle_accessor<TValue>::write_at(const TValue &source, ft_size_t element_index) const
{
    scma_mutex_lock_guard mutex_guard;
    if (mutex_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(mutex_guard.get_error());
        return (0);
    }
    if (!std::is_trivially_copyable<TValue>::value)
    {
        this->set_error(FT_ERR_UNSUPPORTED_TYPE);
        return (0);
    }
    if (!this->is_bound())
    {
        this->set_error(FT_ERR_INVALID_HANDLE);
        return (0);
    }
    ft_size_t offset;
    ft_size_t element_size;

    size_t host_size;

    host_size = sizeof(TValue);
    if (host_size > static_cast<size_t>(FT_SYSTEM_SIZE_MAX))
    {
        this->set_error(FT_ERR_OUT_OF_RANGE);
        return (0);
    }
    element_size = static_cast<ft_size_t>(host_size);
    offset = scma_handle_accessor_calculate_offset<TValue>(element_index);
    if (element_size == 0 || (offset == 0 && element_index != 0))
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (0);
    }
    ft_size_t required_size;

    if (offset > static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX) - element_size)
    {
        this->set_error(FT_ERR_OUT_OF_RANGE);
        return (0);
    }
    required_size = offset + element_size;
    if (!scma_handle_accessor_validate_target<TValue>(this->_handle, required_size, ft_nullptr))
    {
        this->set_error(ft_errno);
        return (0);
    }
    if (!scma_write(this->_handle, offset, &source, element_size))
    {
        this->set_error(ft_errno);
        return (0);
    }
    this->set_error(ER_SUCCESS);
    return (1);
}

template <typename TValue>
inline ft_size_t    scma_handle_accessor<TValue>::get_count(void) const
{
    scma_mutex_lock_guard mutex_guard;
    if (mutex_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(mutex_guard.get_error());
        return (0);
    }
    if (!std::is_trivially_copyable<TValue>::value)
    {
        this->set_error(FT_ERR_UNSUPPORTED_TYPE);
        return (0);
    }
    if (!this->is_bound())
    {
        this->set_error(FT_ERR_INVALID_HANDLE);
        return (0);
    }
    ft_size_t block_size;
    ft_size_t element_size;

    size_t host_size;

    host_size = sizeof(TValue);
    if (host_size > static_cast<size_t>(FT_SYSTEM_SIZE_MAX))
    {
        this->set_error(FT_ERR_OUT_OF_RANGE);
        return (0);
    }
    element_size = static_cast<ft_size_t>(host_size);
    if (!scma_handle_accessor_validate_target<TValue>(this->_handle, 0, &block_size))
    {
        this->set_error(ft_errno);
        return (0);
    }
    if (element_size == 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (0);
    }
    this->set_error(ER_SUCCESS);
    return (block_size / element_size);
}

template <typename TValue>
inline int    scma_handle_accessor<TValue>::get_error(void) const
{
    return (this->_error_code);
}

template <typename TValue>
inline const char  *scma_handle_accessor<TValue>::get_error_str(void) const
{
    return (ft_strerror(this->_error_code));
}

template <typename TValue>
class scma_handle_accessor_element_proxy
{
    private:
        scma_handle_accessor<TValue> *_parent;
        ft_size_t _index;
        TValue _value;
        int _should_write_back;
        mutable int _error_code;

        void    set_error(int error_code) const;

    public:
        scma_handle_accessor_element_proxy(scma_handle_accessor<TValue> *parent, ft_size_t element_index);
        scma_handle_accessor_element_proxy(const scma_handle_accessor_element_proxy &other) = delete;
        scma_handle_accessor_element_proxy    &operator=(const scma_handle_accessor_element_proxy &other) = delete;
        scma_handle_accessor_element_proxy(scma_handle_accessor_element_proxy &&other);
        scma_handle_accessor_element_proxy    &operator=(scma_handle_accessor_element_proxy &&other);
        ~scma_handle_accessor_element_proxy(void);

        TValue    *operator->(void);
        TValue    &operator*(void);
        scma_handle_accessor_element_proxy    &operator=(const TValue &source);
        operator TValue(void) const;

        int     get_error(void) const;
        const char  *get_error_str(void) const;
};

template <typename TValue>
class scma_handle_accessor_const_element_proxy
{
    private:
        const scma_handle_accessor<TValue> *_parent;
        ft_size_t _index;
        mutable TValue _value;
        mutable int _error_code;

        void    set_error(int error_code) const;

    public:
        scma_handle_accessor_const_element_proxy(const scma_handle_accessor<TValue> *parent, ft_size_t element_index);
        scma_handle_accessor_const_element_proxy(const scma_handle_accessor_const_element_proxy &other) = delete;
        scma_handle_accessor_const_element_proxy    &operator=(const scma_handle_accessor_const_element_proxy &other) = delete;
        scma_handle_accessor_const_element_proxy(scma_handle_accessor_const_element_proxy &&other);
        scma_handle_accessor_const_element_proxy    &operator=(scma_handle_accessor_const_element_proxy &&other);
        ~scma_handle_accessor_const_element_proxy(void);

        const TValue    *operator->(void) const;
        const TValue    &operator*(void) const;
        operator TValue(void) const;

        int     get_error(void) const;
        const char  *get_error_str(void) const;
};

template <typename TValue>
inline void    scma_handle_accessor_element_proxy<TValue>::set_error(int error_code) const
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}

template <typename TValue>
inline scma_handle_accessor_element_proxy<TValue>::scma_handle_accessor_element_proxy(scma_handle_accessor<TValue> *parent, ft_size_t element_index)
{
    this->_parent = parent;
    this->_index = element_index;
    this->_should_write_back = 0;
    this->_error_code = ER_SUCCESS;
    if (!this->_parent)
        return ;
    if (!this->_parent->read_at(this->_value, this->_index))
    {
        this->set_error(this->_parent->get_error());
        this->_parent = ft_nullptr;
        return ;
    }
    this->_should_write_back = 1;
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename TValue>
inline scma_handle_accessor_element_proxy<TValue>::scma_handle_accessor_element_proxy(scma_handle_accessor_element_proxy &&other)
{
    this->_parent = other._parent;
    this->_index = other._index;
    this->_value = other._value;
    this->_should_write_back = other._should_write_back;
    this->_error_code = other._error_code;
    other._parent = ft_nullptr;
    other._should_write_back = 0;
    other._error_code = ER_SUCCESS;
    return ;
}

template <typename TValue>
inline scma_handle_accessor_element_proxy<TValue>    &scma_handle_accessor_element_proxy<TValue>::operator=(scma_handle_accessor_element_proxy &&other)
{
    if (this == &other)
        return (*this);
    this->_parent = other._parent;
    this->_index = other._index;
    this->_value = other._value;
    this->_should_write_back = other._should_write_back;
    this->_error_code = other._error_code;
    other._parent = ft_nullptr;
    other._should_write_back = 0;
    other._error_code = ER_SUCCESS;
    return (*this);
}

template <typename TValue>
inline scma_handle_accessor_element_proxy<TValue>::~scma_handle_accessor_element_proxy(void)
{
    if (!this->_parent)
        return ;
    if (!this->_should_write_back)
        return ;
    if (!this->_parent->write_at(this->_value, this->_index))
    {
        this->set_error(this->_parent->get_error());
        return ;
    }
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename TValue>
inline TValue    *scma_handle_accessor_element_proxy<TValue>::operator->(void)
{
    return (&this->_value);
}

template <typename TValue>
inline TValue    &scma_handle_accessor_element_proxy<TValue>::operator*(void)
{
    return (this->_value);
}

template <typename TValue>
inline scma_handle_accessor_element_proxy<TValue>    &scma_handle_accessor_element_proxy<TValue>::operator=(const TValue &source)
{
    if (!this->_parent)
    {
        this->set_error(FT_ERR_INVALID_HANDLE);
        return (*this);
    }
    this->_value = source;
    if (!this->_parent->write_at(this->_value, this->_index))
    {
        this->set_error(this->_parent->get_error());
        return (*this);
    }
    this->_should_write_back = 1;
    this->set_error(ER_SUCCESS);
    return (*this);
}

template <typename TValue>
inline scma_handle_accessor_element_proxy<TValue>::operator TValue(void) const
{
    return (this->_value);
}

template <typename TValue>
inline int    scma_handle_accessor_element_proxy<TValue>::get_error(void) const
{
    return (this->_error_code);
}

template <typename TValue>
inline const char  *scma_handle_accessor_element_proxy<TValue>::get_error_str(void) const
{
    return (ft_strerror(this->_error_code));
}

template <typename TValue>
inline void    scma_handle_accessor_const_element_proxy<TValue>::set_error(int error_code) const
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}

template <typename TValue>
inline scma_handle_accessor_const_element_proxy<TValue>::scma_handle_accessor_const_element_proxy(const scma_handle_accessor<TValue> *parent, ft_size_t element_index)
{
    this->_parent = parent;
    this->_index = element_index;
    this->_error_code = ER_SUCCESS;
    if (!this->_parent)
        return ;
    if (!this->_parent->read_at(this->_value, this->_index))
    {
        this->set_error(this->_parent->get_error());
        this->_parent = ft_nullptr;
        return ;
    }
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename TValue>
inline scma_handle_accessor_const_element_proxy<TValue>::scma_handle_accessor_const_element_proxy(scma_handle_accessor_const_element_proxy &&other)
{
    this->_parent = other._parent;
    this->_index = other._index;
    this->_value = other._value;
    this->_error_code = other._error_code;
    other._parent = ft_nullptr;
    other._error_code = ER_SUCCESS;
    return ;
}

template <typename TValue>
inline scma_handle_accessor_const_element_proxy<TValue>    &scma_handle_accessor_const_element_proxy<TValue>::operator=(scma_handle_accessor_const_element_proxy &&other)
{
    if (this == &other)
        return (*this);
    this->_parent = other._parent;
    this->_index = other._index;
    this->_value = other._value;
    this->_error_code = other._error_code;
    other._parent = ft_nullptr;
    other._error_code = ER_SUCCESS;
    return (*this);
}

template <typename TValue>
inline scma_handle_accessor_const_element_proxy<TValue>::~scma_handle_accessor_const_element_proxy(void)
{
    return ;
}

template <typename TValue>
inline const TValue    *scma_handle_accessor_const_element_proxy<TValue>::operator->(void) const
{
    return (&this->_value);
}

template <typename TValue>
inline const TValue    &scma_handle_accessor_const_element_proxy<TValue>::operator*(void) const
{
    return (this->_value);
}

template <typename TValue>
inline scma_handle_accessor_const_element_proxy<TValue>::operator TValue(void) const
{
    return (this->_value);
}

template <typename TValue>
inline int    scma_handle_accessor_const_element_proxy<TValue>::get_error(void) const
{
    return (this->_error_code);
}

template <typename TValue>
inline const char  *scma_handle_accessor_const_element_proxy<TValue>::get_error_str(void) const
{
    return (ft_strerror(this->_error_code));
}


#endif
