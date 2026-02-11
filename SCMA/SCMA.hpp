#ifndef SCMA_HPP
# define SCMA_HPP

#include <cstddef>
#include <type_traits>
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/limits.hpp"
#include "../PThread/recursive_mutex.hpp"


struct scma_handle
{
    ft_size_t    index;
    ft_size_t    generation;
};

int32_t     scma_initialize(ft_size_t initial_capacity);
void    scma_shutdown(void);
int32_t     scma_is_initialized(void);

scma_handle    scma_allocate(ft_size_t size);
int32_t     scma_free(scma_handle handle);
int32_t     scma_resize(scma_handle handle, ft_size_t new_size);
ft_size_t    scma_get_size(scma_handle handle);
int32_t     scma_handle_is_valid(scma_handle handle);

int32_t     scma_write(scma_handle handle, ft_size_t offset,
            const void *source, ft_size_t size);
int32_t     scma_read(scma_handle handle, ft_size_t offset,
            void *destination, ft_size_t size);

struct scma_stats
{
    ft_size_t    block_count;
    ft_size_t    used_size;
    ft_size_t    heap_capacity;
};

int32_t     scma_get_stats(scma_stats *out_stats);

void    scma_debug_dump(void);

pt_recursive_mutex    *scma_runtime_mutex(void);
int32_t     scma_enable_thread_safety(void);
int32_t     scma_disable_thread_safety(void);
bool    scma_is_thread_safe_enabled(void);
int32_t     scma_mutex_lock(void);
int32_t     scma_mutex_unlock(void);
int32_t     scma_mutex_close(void);
ft_size_t    scma_mutex_lock_count(void);

template <typename TValue>
class scma_handle_accessor_element_proxy;

template <typename TValue>
class scma_handle_accessor_const_element_proxy;

template <typename TValue>
class scma_handle_accessor
{
    private:
        scma_handle _handle;
        bool _initialized;
        static thread_local int32_t _last_error;


    public:
        scma_handle_accessor(void);
        explicit scma_handle_accessor(scma_handle handle);
        scma_handle_accessor(const scma_handle_accessor &other);
        scma_handle_accessor(scma_handle_accessor &&other);
        ~scma_handle_accessor(void);

        int32_t     initialize(void);
        int32_t     initialize(scma_handle handle);
        int32_t     destroy(void);
        int32_t     is_initialized(void) const;
        int32_t     bind(scma_handle handle);
        int32_t     is_bound(void) const;
        scma_handle    get_handle(void) const;

        scma_handle_accessor_element_proxy<TValue>    operator*(void);
        scma_handle_accessor_const_element_proxy<TValue>    operator*(void) const;
        scma_handle_accessor_element_proxy<TValue>    operator->(void);
        scma_handle_accessor_const_element_proxy<TValue>    operator->(void) const;
        scma_handle_accessor_element_proxy<TValue>    operator[](ft_size_t element_index);
        scma_handle_accessor_const_element_proxy<TValue>    operator[](ft_size_t element_index) const;

        int32_t     read_struct(TValue &destination) const;
        int32_t     write_struct(const TValue &source) const;
        int32_t     read_at(TValue &destination, ft_size_t element_index) const;
        int32_t     write_at(const TValue &source, ft_size_t element_index) const;
        ft_size_t    get_count(void) const;
        int32_t     get_error(void) const;

};

template <typename TValue>
thread_local int32_t scma_handle_accessor<TValue>::_last_error = FT_ERR_INVALID_STATE;

template <typename TValue>
inline scma_handle_accessor<TValue>::scma_handle_accessor(void)
{
    this->_handle.index = static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX);
    this->_handle.generation = static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX);
    this->_initialized = false;
    this->_last_error = FT_ERR_INVALID_STATE;
    return ;
}

template <typename TValue>
inline scma_handle_accessor<TValue>::scma_handle_accessor(scma_handle handle)
{
    this->_handle.index = static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX);
    this->_handle.generation = static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX);
    this->_initialized = false;
    this->_last_error = FT_ERR_INVALID_STATE;
    (void)this->initialize(handle);
    return ;
}

template <typename TValue>
inline scma_handle_accessor<TValue>::scma_handle_accessor(const scma_handle_accessor &other)
{
    this->_handle.index = static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX);
    this->_handle.generation = static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX);
    this->_initialized = false;
    this->_last_error = FT_ERR_INVALID_STATE;
    if (!other.is_initialized())
        return ;
    if (this->initialize() != FT_ERR_SUCCESSS)
        return ;
    if (!other.is_bound())
    {
        this->_last_error = other.get_error();
        return ;
    }
    if (scma_mutex_lock() != 0)
    {
        this->_last_error = FT_ERR_SYS_MUTEX_LOCK_FAILED;
        return ;
    }
    if (!scma_handle_is_valid(other._handle))
    {
        this->_last_error = FT_ERR_INVALID_HANDLE;
        if (scma_mutex_unlock() != 0)
            this->_last_error = FT_ERR_SYS_MUTEX_UNLOCK_FAILED;
        return ;
    }
    this->_handle = other._handle;
    this->_last_error = other.get_error();
    if (scma_mutex_unlock() != 0)
        this->_last_error = FT_ERR_SYS_MUTEX_UNLOCK_FAILED;
    return ;
}

template <typename TValue>
inline scma_handle_accessor<TValue>::scma_handle_accessor(scma_handle_accessor &&other)
{
    this->_handle.index = static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX);
    this->_handle.generation = static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX);
    this->_initialized = false;
    this->_last_error = FT_ERR_INVALID_STATE;
    if (!other.is_initialized())
        return ;
    if (this->initialize() != FT_ERR_SUCCESSS)
        return ;
    if (scma_mutex_lock() != 0)
    {
        this->_last_error = FT_ERR_SYS_MUTEX_LOCK_FAILED;
        return ;
    }
    if (!other.is_bound())
    {
        other._handle.index = static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX);
        other._handle.generation = static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX);
        this->_last_error = other.get_error();
        if (scma_mutex_unlock() != 0)
            this->_last_error = FT_ERR_SYS_MUTEX_UNLOCK_FAILED;
        return ;
    }
    if (!scma_handle_is_valid(other._handle))
    {
        this->_last_error = FT_ERR_INVALID_HANDLE;
        other._handle.index = static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX);
        other._handle.generation = static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX);
        if (scma_mutex_unlock() != 0)
            this->_last_error = FT_ERR_SYS_MUTEX_UNLOCK_FAILED;
        return ;
    }
    this->_handle = other._handle;
    this->_last_error = other.get_error();
    other._handle.index = static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX);
    other._handle.generation = static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX);
    if (scma_mutex_unlock() != 0)
        this->_last_error = FT_ERR_SYS_MUTEX_UNLOCK_FAILED;
    return ;
}

template <typename TValue>
inline scma_handle_accessor<TValue>::~scma_handle_accessor(void)
{
    (void)this->destroy();
    return ;
}

template <typename TValue>
inline int32_t    scma_handle_accessor<TValue>::initialize(void)
{
    int32_t lock_result;

    lock_result = scma_mutex_lock();
    if (lock_result != 0)
    {
        this->_last_error = FT_ERR_SYS_MUTEX_LOCK_FAILED;
        return (FT_ERR_SYS_MUTEX_LOCK_FAILED);
    }
    if (this->_initialized)
    {
        this->_last_error = FT_ERR_SUCCESSS;
        if (scma_mutex_unlock() != 0)
            this->_last_error = FT_ERR_SYS_MUTEX_UNLOCK_FAILED;
        return (this->_last_error);
    }
    this->_handle.index = static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX);
    this->_handle.generation = static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX);
    this->_initialized = true;
    this->_last_error = FT_ERR_SUCCESSS;
    if (scma_mutex_unlock() != 0)
        this->_last_error = FT_ERR_SYS_MUTEX_UNLOCK_FAILED;
    return (this->_last_error);
}

template <typename TValue>
inline int32_t    scma_handle_accessor<TValue>::initialize(scma_handle handle)
{
    int32_t initialization_error;

    initialization_error = this->initialize();
    if (initialization_error != FT_ERR_SUCCESSS)
        return (initialization_error);
    if (this->bind(handle))
        return (FT_ERR_SUCCESSS);
    return (this->_last_error);
}

template <typename TValue>
inline int32_t    scma_handle_accessor<TValue>::destroy(void)
{
    if (scma_mutex_lock() != 0)
    {
        this->_last_error = FT_ERR_SYS_MUTEX_LOCK_FAILED;
        return (FT_ERR_SYS_MUTEX_LOCK_FAILED);
    }
    if (!this->_initialized)
    {
        this->_last_error = FT_ERR_SUCCESSS;
        if (scma_mutex_unlock() != 0)
            this->_last_error = FT_ERR_SYS_MUTEX_UNLOCK_FAILED;
        return (this->_last_error);
    }
    this->_handle.index = static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX);
    this->_handle.generation = static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX);
    this->_initialized = false;
    this->_last_error = FT_ERR_SUCCESSS;
    if (scma_mutex_unlock() != 0)
        this->_last_error = FT_ERR_SYS_MUTEX_UNLOCK_FAILED;
    return (this->_last_error);
}

template <typename TValue>
inline int32_t    scma_handle_accessor<TValue>::is_initialized(void) const
{
    int32_t initialized;

    initialized = 0;
    if (scma_mutex_lock() != 0)
        return (0);
    if (this->_initialized)
        initialized = 1;
    if (scma_mutex_unlock() != 0)
        return (0);
    return (initialized);
}

template <typename TValue>
inline int32_t    scma_handle_accessor<TValue>::bind(scma_handle handle)
{
    if (scma_mutex_lock() != 0)
    {
        this->_last_error = FT_ERR_SYS_MUTEX_LOCK_FAILED;
        return (0);
    }
    if (!this->_initialized)
    {
        this->_last_error = FT_ERR_INVALID_STATE;
        if (scma_mutex_unlock() != 0)
            this->_last_error = FT_ERR_SYS_MUTEX_UNLOCK_FAILED;
        return (0);
    }
    if (!scma_handle_is_valid(handle))
    {
        this->_last_error = FT_ERR_INVALID_HANDLE;
        if (scma_mutex_unlock() != 0)
            this->_last_error = FT_ERR_SYS_MUTEX_UNLOCK_FAILED;
        return (0);
    }
    this->_handle = handle;
    this->_last_error = FT_ERR_SUCCESSS;
    if (scma_mutex_unlock() != 0)
    {
        this->_last_error = FT_ERR_SYS_MUTEX_UNLOCK_FAILED;
        return (0);
    }
    return (1);
}

template <typename TValue>
inline int32_t    scma_handle_accessor<TValue>::is_bound(void) const
{
    int32_t is_bound_result;

    is_bound_result = 0;
    if (scma_mutex_lock() != 0)
        return (0);
    if (!this->_initialized)
        is_bound_result = 0;
    else if (this->_handle.index == static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX))
        is_bound_result = 0;
    else if (this->_handle.generation == static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX))
        is_bound_result = 0;
    else
        is_bound_result = 1;
    if (scma_mutex_unlock() != 0)
        return (0);
    return (is_bound_result);
}

template <typename TValue>
inline scma_handle    scma_handle_accessor<TValue>::get_handle(void) const
{
    scma_handle handle;

    handle.index = static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX);
    handle.generation = static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX);
    if (scma_mutex_lock() != 0)
        return (handle);
    handle = this->_handle;
    if (scma_mutex_unlock() != 0)
    {
        handle.index = static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX);
        handle.generation = static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX);
        return (handle);
    }
    return (handle);
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
inline int32_t    scma_handle_accessor<TValue>::read_struct(TValue &destination) const
{
    int32_t read_result;
    ft_size_t required_size;
    ft_size_t host_size;
    ft_size_t block_size;

    read_result = 0;
    if (!this->is_initialized())
    {
        this->_last_error = FT_ERR_INVALID_STATE;
        return (0);
    }
    if (scma_mutex_lock() != 0)
    {
        this->_last_error = FT_ERR_SYS_MUTEX_LOCK_FAILED;
        return (0);
    }
    if (!std::is_trivially_copyable<TValue>::value)
    {
        this->_last_error = FT_ERR_UNSUPPORTED_TYPE;
        goto cleanup;
    }
    if (!this->is_bound())
    {
        this->_last_error = FT_ERR_INVALID_HANDLE;
        goto cleanup;
    }
    host_size = sizeof(TValue);
    if (host_size > static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX))
    {
        this->_last_error = FT_ERR_OUT_OF_RANGE;
        goto cleanup;
    }
    required_size = host_size;
    block_size = scma_get_size(this->_handle);
    if (block_size < required_size)
    {
        this->_last_error = FT_ERR_OUT_OF_RANGE;
        goto cleanup;
    }
    if (!scma_read(this->_handle, 0, &destination, required_size))
    {
        this->_last_error = FT_ERR_INVALID_OPERATION;
        goto cleanup;
    }
    this->_last_error = FT_ERR_SUCCESSS;
    read_result = 1;

cleanup:
    if (scma_mutex_unlock() != 0)
    {
        this->_last_error = FT_ERR_SYS_MUTEX_UNLOCK_FAILED;
        read_result = 0;
    }
    return (read_result);
}

template <typename TValue>
inline int32_t    scma_handle_accessor<TValue>::write_struct(const TValue &source) const
{
    int32_t write_result;
    ft_size_t required_size;
    ft_size_t host_size;
    ft_size_t block_size;

    write_result = 0;
    if (!this->is_initialized())
    {
        this->_last_error = FT_ERR_INVALID_STATE;
        return (0);
    }
    if (scma_mutex_lock() != 0)
    {
        this->_last_error = FT_ERR_SYS_MUTEX_LOCK_FAILED;
        return (0);
    }
    if (!std::is_trivially_copyable<TValue>::value)
    {
        this->_last_error = FT_ERR_UNSUPPORTED_TYPE;
        goto cleanup;
    }
    if (!this->is_bound())
    {
        this->_last_error = FT_ERR_INVALID_HANDLE;
        goto cleanup;
    }
    host_size = sizeof(TValue);
    if (host_size > static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX))
    {
        this->_last_error = FT_ERR_OUT_OF_RANGE;
        goto cleanup;
    }
    required_size = host_size;
    block_size = scma_get_size(this->_handle);
    if (block_size < required_size)
    {
        this->_last_error = FT_ERR_OUT_OF_RANGE;
        goto cleanup;
    }
    if (!scma_write(this->_handle, 0, &source, required_size))
    {
        this->_last_error = FT_ERR_INVALID_OPERATION;
        goto cleanup;
    }
    this->_last_error = FT_ERR_SUCCESSS;
    write_result = 1;

cleanup:
    if (scma_mutex_unlock() != 0)
    {
        this->_last_error = FT_ERR_SYS_MUTEX_UNLOCK_FAILED;
        write_result = 0;
    }
    return (write_result);
}

template <typename TValue>
inline int32_t    scma_handle_accessor<TValue>::read_at(TValue &destination, ft_size_t element_index) const
{
    int32_t read_result;
    ft_size_t offset;
    ft_size_t element_size;
    ft_size_t required_size;
    ft_size_t host_size;
    ft_size_t block_size;

    read_result = 0;
    if (!this->is_initialized())
    {
        this->_last_error = FT_ERR_INVALID_STATE;
        return (0);
    }
    if (scma_mutex_lock() != 0)
    {
        this->_last_error = FT_ERR_SYS_MUTEX_LOCK_FAILED;
        return (0);
    }
    if (!std::is_trivially_copyable<TValue>::value)
    {
        this->_last_error = FT_ERR_UNSUPPORTED_TYPE;
        goto cleanup;
    }
    if (!this->is_bound())
    {
        this->_last_error = FT_ERR_INVALID_HANDLE;
        goto cleanup;
    }
    host_size = sizeof(TValue);
    if (host_size > static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX))
    {
        this->_last_error = FT_ERR_OUT_OF_RANGE;
        goto cleanup;
    }
    element_size = host_size;
    if (element_size == 0 || element_index > static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX) / element_size)
    {
        this->_last_error = FT_ERR_INVALID_ARGUMENT;
        goto cleanup;
    }
    offset = element_index * element_size;
    if (offset == 0 && element_index != 0)
    {
        this->_last_error = FT_ERR_INVALID_ARGUMENT;
        goto cleanup;
    }
    if (offset > static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX) - element_size)
    {
        this->_last_error = FT_ERR_OUT_OF_RANGE;
        goto cleanup;
    }
    required_size = offset + element_size;
    block_size = scma_get_size(this->_handle);
    if (block_size < required_size)
    {
        this->_last_error = FT_ERR_OUT_OF_RANGE;
        goto cleanup;
    }
    if (!scma_read(this->_handle, offset, &destination, element_size))
    {
        this->_last_error = FT_ERR_INVALID_OPERATION;
        goto cleanup;
    }
    this->_last_error = FT_ERR_SUCCESSS;
    read_result = 1;

cleanup:
    if (scma_mutex_unlock() != 0)
    {
        this->_last_error = FT_ERR_SYS_MUTEX_UNLOCK_FAILED;
        read_result = 0;
    }
    return (read_result);
}

template <typename TValue>
inline int32_t    scma_handle_accessor<TValue>::write_at(const TValue &source, ft_size_t element_index) const
{
    int32_t write_result;
    ft_size_t offset;
    ft_size_t element_size;
    ft_size_t required_size;
    ft_size_t host_size;
    ft_size_t block_size;

    write_result = 0;
    if (!this->is_initialized())
    {
        this->_last_error = FT_ERR_INVALID_STATE;
        return (0);
    }
    if (scma_mutex_lock() != 0)
    {
        this->_last_error = FT_ERR_SYS_MUTEX_LOCK_FAILED;
        return (0);
    }
    if (!std::is_trivially_copyable<TValue>::value)
    {
        this->_last_error = FT_ERR_UNSUPPORTED_TYPE;
        goto cleanup;
    }
    if (!this->is_bound())
    {
        this->_last_error = FT_ERR_INVALID_HANDLE;
        goto cleanup;
    }
    host_size = sizeof(TValue);
    if (host_size > static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX))
    {
        this->_last_error = FT_ERR_OUT_OF_RANGE;
        goto cleanup;
    }
    element_size = host_size;
    if (element_size == 0 || element_index > static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX) / element_size)
    {
        this->_last_error = FT_ERR_INVALID_ARGUMENT;
        goto cleanup;
    }
    offset = element_index * element_size;
    if (offset == 0 && element_index != 0)
    {
        this->_last_error = FT_ERR_INVALID_ARGUMENT;
        goto cleanup;
    }
    if (offset > static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX) - element_size)
    {
        this->_last_error = FT_ERR_OUT_OF_RANGE;
        goto cleanup;
    }
    required_size = offset + element_size;
    block_size = scma_get_size(this->_handle);
    if (block_size < required_size)
    {
        this->_last_error = FT_ERR_OUT_OF_RANGE;
        goto cleanup;
    }
    if (!scma_write(this->_handle, offset, &source, element_size))
    {
        this->_last_error = FT_ERR_INVALID_OPERATION;
        goto cleanup;
    }
    this->_last_error = FT_ERR_SUCCESSS;
    write_result = 1;

cleanup:
    if (scma_mutex_unlock() != 0)
    {
        this->_last_error = FT_ERR_SYS_MUTEX_UNLOCK_FAILED;
        write_result = 0;
    }
    return (write_result);
}

template <typename TValue>
inline ft_size_t    scma_handle_accessor<TValue>::get_count(void) const
{
    ft_size_t block_size;
    ft_size_t element_size;
    ft_size_t element_count;
    ft_size_t host_size;

    element_count = 0;
    if (!this->is_initialized())
    {
        this->_last_error = FT_ERR_INVALID_STATE;
        return (0);
    }
    if (scma_mutex_lock() != 0)
    {
        this->_last_error = FT_ERR_SYS_MUTEX_LOCK_FAILED;
        return (0);
    }
    if (!std::is_trivially_copyable<TValue>::value)
    {
        this->_last_error = FT_ERR_UNSUPPORTED_TYPE;
        goto cleanup;
    }
    if (!this->is_bound())
    {
        this->_last_error = FT_ERR_INVALID_HANDLE;
        goto cleanup;
    }
    host_size = sizeof(TValue);
    if (host_size > static_cast<ft_size_t>(FT_SYSTEM_SIZE_MAX))
    {
        this->_last_error = FT_ERR_OUT_OF_RANGE;
        goto cleanup;
    }
    element_size = host_size;
    block_size = scma_get_size(this->_handle);
    if (element_size == 0)
    {
        this->_last_error = FT_ERR_INVALID_ARGUMENT;
        goto cleanup;
    }
    this->_last_error = FT_ERR_SUCCESSS;
    element_count = block_size / element_size;

cleanup:
    if (scma_mutex_unlock() != 0)
    {
        this->_last_error = FT_ERR_SYS_MUTEX_UNLOCK_FAILED;
        element_count = 0;
    }
    return (element_count);
}

template <typename TValue>
inline int32_t    scma_handle_accessor<TValue>::get_error(void) const
{
    return (this->_last_error);
}

template <typename TValue>
class scma_handle_accessor_element_proxy
{
    private:
        scma_handle_accessor<TValue> *_parent;
        ft_size_t _index;
        TValue _value;
        int32_t _should_write_back;


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

};

template <typename TValue>
class scma_handle_accessor_const_element_proxy
{
    private:
        const scma_handle_accessor<TValue> *_parent;
        ft_size_t _index;
        mutable TValue _value;


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
};

template <typename TValue>
inline scma_handle_accessor_element_proxy<TValue>::scma_handle_accessor_element_proxy(scma_handle_accessor<TValue> *parent, ft_size_t element_index)
{
    this->_parent = parent;
    this->_index = element_index;
    this->_should_write_back = 0;
    if (!this->_parent)
        return ;
    if (!this->_parent->read_at(this->_value, this->_index))
    {
        this->_parent = ft_nullptr;
        return ;
    }
    return ;
}

template <typename TValue>
inline scma_handle_accessor_element_proxy<TValue>::scma_handle_accessor_element_proxy(scma_handle_accessor_element_proxy &&other)
{
    this->_parent = other._parent;
    this->_index = other._index;
    this->_value = other._value;
    this->_should_write_back = other._should_write_back;
    other._parent = ft_nullptr;
    other._should_write_back = 0;
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
    other._parent = ft_nullptr;
    other._should_write_back = 0;
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
        return ;
    }
    return ;
}

template <typename TValue>
inline TValue    *scma_handle_accessor_element_proxy<TValue>::operator->(void)
{
    this->_should_write_back = 1;
    return (&this->_value);
}

template <typename TValue>
inline TValue    &scma_handle_accessor_element_proxy<TValue>::operator*(void)
{
    this->_should_write_back = 1;
    return (this->_value);
}

template <typename TValue>
inline scma_handle_accessor_element_proxy<TValue>    &scma_handle_accessor_element_proxy<TValue>::operator=(const TValue &source)
{
    if (!this->_parent)
    {
        return (*this);
    }
    this->_value = source;
    if (!this->_parent->write_at(this->_value, this->_index))
    {
        return (*this);
    }
    this->_should_write_back = 0;
    return (*this);
}

template <typename TValue>
inline scma_handle_accessor_element_proxy<TValue>::operator TValue(void) const
{
    return (this->_value);
}

template <typename TValue>
inline scma_handle_accessor_const_element_proxy<TValue>::scma_handle_accessor_const_element_proxy(const scma_handle_accessor<TValue> *parent, ft_size_t element_index)
{
    this->_parent = parent;
    this->_index = element_index;
    if (!this->_parent)
        return ;
    if (!this->_parent->read_at(this->_value, this->_index))
    {
        this->_parent = ft_nullptr;
        return ;
    }
    return ;
}

template <typename TValue>
inline scma_handle_accessor_const_element_proxy<TValue>::scma_handle_accessor_const_element_proxy(scma_handle_accessor_const_element_proxy &&other)
{
    this->_parent = other._parent;
    this->_index = other._index;
    this->_value = other._value;
    other._parent = ft_nullptr;
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
    other._parent = ft_nullptr;
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

#endif
