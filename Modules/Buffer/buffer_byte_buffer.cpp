#include "byte_buffer.hpp"
#include "../Basic/basic.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno_internal.hpp"
#include "../PThread/pthread_internal.hpp"
#include <new>
#include "../Basic/limits.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/recursive_mutex.hpp"

thread_local int32_t ft_byte_buffer::_last_error = FT_ERR_SUCCESS;

static ft_bool buffer_add_overflows(ft_size_t left, ft_size_t right) noexcept
{
    if (left > FT_SYSTEM_SIZE_MAX - right)
        return (FT_TRUE);
    return (FT_FALSE);
}

int32_t ft_byte_buffer::set_error(int32_t error_code) noexcept
{
    ft_byte_buffer::_last_error = error_code;
    return (error_code);
}

void ft_byte_buffer::reset_fields(void) noexcept
{
    this->_data = ft_nullptr;
    this->_size = 0;
    this->_capacity = 0;
    this->_read_position = 0;
    this->_fixed_capacity = FT_FALSE;
    this->_mutex = ft_nullptr;
    return ;
}

int32_t ft_byte_buffer::lock_internal(void) const noexcept
{
    return (pt_recursive_mutex_lock_if_not_null(this->_mutex));
}

void ft_byte_buffer::unlock_internal(void) const noexcept
{
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return ;
}

int32_t ft_byte_buffer::reserve_internal(ft_size_t required_capacity) noexcept
{
    uint8_t *new_data;
    ft_size_t new_capacity;

    if (required_capacity <= this->_capacity)
        return (FT_ERR_SUCCESS);
    if (this->_fixed_capacity == FT_TRUE)
        return (FT_ERR_FULL);
    new_capacity = this->_capacity;
    if (new_capacity == 0)
        new_capacity = 16;
    while (new_capacity < required_capacity)
    {
        if (new_capacity > FT_SYSTEM_SIZE_MAX / 2)
        {
            new_capacity = required_capacity;
            break ;
        }
        new_capacity = new_capacity * 2;
    }
    new_data = static_cast<uint8_t *>(cma_realloc(this->_data, new_capacity));
    if (new_data == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    this->_data = new_data;
    this->_capacity = new_capacity;
    return (FT_ERR_SUCCESS);
}

int32_t ft_byte_buffer::append_internal(const void *data, ft_size_t length) noexcept
{
    int32_t reserve_error;

    if (length == 0)
        return (FT_ERR_SUCCESS);
    if (data == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    if (buffer_add_overflows(this->_size, length) == FT_TRUE)
        return (FT_ERR_OUT_OF_RANGE);
    reserve_error = this->reserve_internal(this->_size + length);
    if (reserve_error != FT_ERR_SUCCESS)
        return (reserve_error);
    ft_memcpy(this->_data + this->_size, data, length);
    this->_size += length;
    return (FT_ERR_SUCCESS);
}

int32_t ft_byte_buffer::read_internal(void *data, ft_size_t length) noexcept
{
    if (length == 0)
        return (FT_ERR_SUCCESS);
    if (data == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    if (this->_read_position > this->_size)
        return (FT_ERR_INVALID_STATE);
    if (length > this->_size - this->_read_position)
        return (FT_ERR_OUT_OF_RANGE);
    ft_memcpy(data, this->_data + this->_read_position, length);
    this->_read_position += length;
    return (FT_ERR_SUCCESS);
}

ft_byte_buffer::ft_byte_buffer() noexcept
    : _data(ft_nullptr), _size(0), _capacity(0), _read_position(0),
      _fixed_capacity(FT_FALSE),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED), _mutex(ft_nullptr)
{
    return ;
}

ft_byte_buffer::~ft_byte_buffer() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    else
    {
        cma_free(this->_data);
        this->reset_fields();
        this->_initialised_state = FT_CLASS_STATE_UNINITIALISED;
    }
    return ;
}

int32_t ft_byte_buffer::initialize() noexcept
{
    return (this->initialize(0, FT_FALSE));
}

int32_t ft_byte_buffer::initialize(ft_size_t capacity,
    ft_bool fixed_capacity) noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        errno_abort_lifecycle(this->_initialised_state,
            "ft_byte_buffer::initialize",
            "called while object is already initialised");
    this->reset_fields();
    this->_fixed_capacity = fixed_capacity;
    if (capacity > 0)
    {
        this->_data = static_cast<uint8_t *>(cma_malloc(capacity));
        if (this->_data == ft_nullptr)
        {
            this->_initialised_state = FT_CLASS_STATE_DESTROYED;
            return (ft_byte_buffer::set_error(FT_ERR_NO_MEMORY));
        }
        this->_capacity = capacity;
    }
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (ft_byte_buffer::set_error(FT_ERR_SUCCESS));
}

int32_t ft_byte_buffer::initialize(const ft_byte_buffer &other) noexcept
{
    int32_t lock_error;
    int32_t initialize_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_lifecycle(other._initialised_state,
            "ft_byte_buffer::initialize(const ft_byte_buffer &)",
            "called with uninitialised source object");
    if (&other == this)
        return (ft_byte_buffer::set_error(FT_ERR_SUCCESS));
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        initialize_error = this->destroy();
        if (initialize_error != FT_ERR_SUCCESS)
            return (ft_byte_buffer::set_error(initialize_error));
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->reset_fields();
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (ft_byte_buffer::set_error(FT_ERR_SUCCESS));
    }
    lock_error = other.lock_internal();
    if (lock_error != FT_ERR_SUCCESS)
        return (ft_byte_buffer::set_error(lock_error));
    initialize_error = this->initialize(other._capacity, other._fixed_capacity);
    if (initialize_error == FT_ERR_SUCCESS && other._size > 0)
        initialize_error = this->append_internal(other._data, other._size);
    if (initialize_error == FT_ERR_SUCCESS)
        this->_read_position = other._read_position;
    other.unlock_internal();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    }
    return (ft_byte_buffer::set_error(initialize_error));
}

int32_t ft_byte_buffer::initialize(ft_byte_buffer &&other) noexcept
{
    return (this->move(other));
}

int32_t ft_byte_buffer::destroy() noexcept
{
    int32_t destroy_error;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (ft_byte_buffer::set_error(FT_ERR_SUCCESS));
    }
    destroy_error = this->disable_thread_safety();
    cma_free(this->_data);
    this->reset_fields();
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    if (destroy_error != FT_ERR_SUCCESS)
        return (ft_byte_buffer::set_error(destroy_error));
    return (ft_byte_buffer::set_error(FT_ERR_SUCCESS));
}

int32_t ft_byte_buffer::move(ft_byte_buffer &other) noexcept
{
    int32_t lock_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_lifecycle(other._initialised_state,
            "ft_byte_buffer::move",
            "called with uninitialised source object");
    if (&other == this)
        return (ft_byte_buffer::set_error(FT_ERR_SUCCESS));
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        lock_error = this->destroy();
        if (lock_error != FT_ERR_SUCCESS)
            return (ft_byte_buffer::set_error(lock_error));
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->reset_fields();
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (ft_byte_buffer::set_error(FT_ERR_SUCCESS));
    }
    lock_error = other.lock_internal();
    if (lock_error != FT_ERR_SUCCESS)
        return (ft_byte_buffer::set_error(lock_error));
    this->_data = other._data;
    this->_size = other._size;
    this->_capacity = other._capacity;
    this->_read_position = other._read_position;
    this->_fixed_capacity = other._fixed_capacity;
    this->_mutex = other._mutex;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    other._data = ft_nullptr;
    other._size = 0;
    other._capacity = 0;
    other._read_position = 0;
    other._fixed_capacity = FT_FALSE;
    other._mutex = ft_nullptr;
    other._initialised_state = FT_CLASS_STATE_DESTROYED;
    this->unlock_internal();
    return (ft_byte_buffer::set_error(FT_ERR_SUCCESS));
}

int32_t ft_byte_buffer::reserve(ft_size_t required_capacity) noexcept
{
    int32_t lock_error;
    int32_t reserve_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
        "ft_byte_buffer::reserve");
    lock_error = this->lock_internal();
    if (lock_error != FT_ERR_SUCCESS)
        return (ft_byte_buffer::set_error(lock_error));
    reserve_error = this->reserve_internal(required_capacity);
    this->unlock_internal();
    return (ft_byte_buffer::set_error(reserve_error));
}

int32_t ft_byte_buffer::clear() noexcept
{
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
        "ft_byte_buffer::clear");
    lock_error = this->lock_internal();
    if (lock_error != FT_ERR_SUCCESS)
        return (ft_byte_buffer::set_error(lock_error));
    this->_size = 0;
    this->_read_position = 0;
    this->unlock_internal();
    return (ft_byte_buffer::set_error(FT_ERR_SUCCESS));
}

int32_t ft_byte_buffer::reset_read_position() noexcept
{
    return (this->set_read_position(0));
}

int32_t ft_byte_buffer::set_read_position(ft_size_t position) noexcept
{
    int32_t lock_error;
    int32_t result;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
        "ft_byte_buffer::set_read_position");
    lock_error = this->lock_internal();
    if (lock_error != FT_ERR_SUCCESS)
        return (ft_byte_buffer::set_error(lock_error));
    result = FT_ERR_SUCCESS;
    if (position > this->_size)
        result = FT_ERR_OUT_OF_RANGE;
    else
        this->_read_position = position;
    this->unlock_internal();
    return (ft_byte_buffer::set_error(result));
}

int32_t ft_byte_buffer::consume(ft_size_t length) noexcept
{
    int32_t lock_error;
    int32_t result;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
        "ft_byte_buffer::consume");
    lock_error = this->lock_internal();
    if (lock_error != FT_ERR_SUCCESS)
        return (ft_byte_buffer::set_error(lock_error));
    result = FT_ERR_SUCCESS;
    if (this->_read_position > this->_size)
        result = FT_ERR_INVALID_STATE;
    else if (length > this->_size - this->_read_position)
        result = FT_ERR_OUT_OF_RANGE;
    else
        this->_read_position += length;
    this->unlock_internal();
    return (ft_byte_buffer::set_error(result));
}

int32_t ft_byte_buffer::append(const void *data, ft_size_t length) noexcept
{
    int32_t lock_error;
    int32_t append_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
        "ft_byte_buffer::append");
    lock_error = this->lock_internal();
    if (lock_error != FT_ERR_SUCCESS)
        return (ft_byte_buffer::set_error(lock_error));
    append_error = this->append_internal(data, length);
    this->unlock_internal();
    return (ft_byte_buffer::set_error(append_error));
}

int32_t ft_byte_buffer::read(void *data, ft_size_t length) noexcept
{
    int32_t lock_error;
    int32_t read_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
        "ft_byte_buffer::read");
    lock_error = this->lock_internal();
    if (lock_error != FT_ERR_SUCCESS)
        return (ft_byte_buffer::set_error(lock_error));
    read_error = this->read_internal(data, length);
    this->unlock_internal();
    return (ft_byte_buffer::set_error(read_error));
}

int32_t ft_byte_buffer::view(ft_size_t offset, ft_size_t length,
    const uint8_t **view_out) const noexcept
{
    int32_t lock_error;
    int32_t result;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
        "ft_byte_buffer::view");
    if (view_out == ft_nullptr)
        return (ft_byte_buffer::set_error(FT_ERR_INVALID_ARGUMENT));
    *view_out = ft_nullptr;
    lock_error = this->lock_internal();
    if (lock_error != FT_ERR_SUCCESS)
        return (ft_byte_buffer::set_error(lock_error));
    result = FT_ERR_SUCCESS;
    if (offset > this->_size)
        result = FT_ERR_OUT_OF_RANGE;
    else if (length > this->_size - offset)
        result = FT_ERR_OUT_OF_RANGE;
    else
        *view_out = this->_data + offset;
    this->unlock_internal();
    return (ft_byte_buffer::set_error(result));
}

int32_t ft_byte_buffer::append_u8(uint8_t value) noexcept
{
    return (this->append(&value, 1));
}

int32_t ft_byte_buffer::append_u16_be(uint16_t value) noexcept
{
    uint8_t bytes[2];

    bytes[0] = static_cast<uint8_t>((value >> 8) & 0xFFU);
    bytes[1] = static_cast<uint8_t>(value & 0xFFU);
    return (this->append(bytes, 2));
}

int32_t ft_byte_buffer::append_u16_le(uint16_t value) noexcept
{
    uint8_t bytes[2];

    bytes[0] = static_cast<uint8_t>(value & 0xFFU);
    bytes[1] = static_cast<uint8_t>((value >> 8) & 0xFFU);
    return (this->append(bytes, 2));
}

int32_t ft_byte_buffer::append_u32_be(uint32_t value) noexcept
{
    uint8_t bytes[4];

    bytes[0] = static_cast<uint8_t>((value >> 24) & 0xFFU);
    bytes[1] = static_cast<uint8_t>((value >> 16) & 0xFFU);
    bytes[2] = static_cast<uint8_t>((value >> 8) & 0xFFU);
    bytes[3] = static_cast<uint8_t>(value & 0xFFU);
    return (this->append(bytes, 4));
}

int32_t ft_byte_buffer::append_u32_le(uint32_t value) noexcept
{
    uint8_t bytes[4];

    bytes[0] = static_cast<uint8_t>(value & 0xFFU);
    bytes[1] = static_cast<uint8_t>((value >> 8) & 0xFFU);
    bytes[2] = static_cast<uint8_t>((value >> 16) & 0xFFU);
    bytes[3] = static_cast<uint8_t>((value >> 24) & 0xFFU);
    return (this->append(bytes, 4));
}

int32_t ft_byte_buffer::append_u64_be(uint64_t value) noexcept
{
    uint8_t bytes[8];

    bytes[0] = static_cast<uint8_t>((value >> 56) & 0xFFU);
    bytes[1] = static_cast<uint8_t>((value >> 48) & 0xFFU);
    bytes[2] = static_cast<uint8_t>((value >> 40) & 0xFFU);
    bytes[3] = static_cast<uint8_t>((value >> 32) & 0xFFU);
    bytes[4] = static_cast<uint8_t>((value >> 24) & 0xFFU);
    bytes[5] = static_cast<uint8_t>((value >> 16) & 0xFFU);
    bytes[6] = static_cast<uint8_t>((value >> 8) & 0xFFU);
    bytes[7] = static_cast<uint8_t>(value & 0xFFU);
    return (this->append(bytes, 8));
}

int32_t ft_byte_buffer::append_u64_le(uint64_t value) noexcept
{
    uint8_t bytes[8];

    bytes[0] = static_cast<uint8_t>(value & 0xFFU);
    bytes[1] = static_cast<uint8_t>((value >> 8) & 0xFFU);
    bytes[2] = static_cast<uint8_t>((value >> 16) & 0xFFU);
    bytes[3] = static_cast<uint8_t>((value >> 24) & 0xFFU);
    bytes[4] = static_cast<uint8_t>((value >> 32) & 0xFFU);
    bytes[5] = static_cast<uint8_t>((value >> 40) & 0xFFU);
    bytes[6] = static_cast<uint8_t>((value >> 48) & 0xFFU);
    bytes[7] = static_cast<uint8_t>((value >> 56) & 0xFFU);
    return (this->append(bytes, 8));
}

int32_t ft_byte_buffer::read_u8(uint8_t *value_out) noexcept
{
    if (value_out == ft_nullptr)
        return (ft_byte_buffer::set_error(FT_ERR_INVALID_ARGUMENT));
    return (this->read(value_out, 1));
}

int32_t ft_byte_buffer::read_u16_be(uint16_t *value_out) noexcept
{
    uint8_t bytes[2];
    int32_t read_error;

    if (value_out == ft_nullptr)
        return (ft_byte_buffer::set_error(FT_ERR_INVALID_ARGUMENT));
    read_error = this->read(bytes, 2);
    if (read_error != FT_ERR_SUCCESS)
        return (read_error);
    *value_out = static_cast<uint16_t>(
        (static_cast<uint16_t>(bytes[0]) << 8)
        | static_cast<uint16_t>(bytes[1]));
    return (ft_byte_buffer::set_error(FT_ERR_SUCCESS));
}

int32_t ft_byte_buffer::read_u16_le(uint16_t *value_out) noexcept
{
    uint8_t bytes[2];
    int32_t read_error;

    if (value_out == ft_nullptr)
        return (ft_byte_buffer::set_error(FT_ERR_INVALID_ARGUMENT));
    read_error = this->read(bytes, 2);
    if (read_error != FT_ERR_SUCCESS)
        return (read_error);
    *value_out = static_cast<uint16_t>(
        static_cast<uint16_t>(bytes[0])
        | (static_cast<uint16_t>(bytes[1]) << 8));
    return (ft_byte_buffer::set_error(FT_ERR_SUCCESS));
}

int32_t ft_byte_buffer::read_u32_be(uint32_t *value_out) noexcept
{
    uint8_t bytes[4];
    int32_t read_error;

    if (value_out == ft_nullptr)
        return (ft_byte_buffer::set_error(FT_ERR_INVALID_ARGUMENT));
    read_error = this->read(bytes, 4);
    if (read_error != FT_ERR_SUCCESS)
        return (read_error);
    *value_out = (static_cast<uint32_t>(bytes[0]) << 24)
        | (static_cast<uint32_t>(bytes[1]) << 16)
        | (static_cast<uint32_t>(bytes[2]) << 8)
        | static_cast<uint32_t>(bytes[3]);
    return (ft_byte_buffer::set_error(FT_ERR_SUCCESS));
}

int32_t ft_byte_buffer::read_u32_le(uint32_t *value_out) noexcept
{
    uint8_t bytes[4];
    int32_t read_error;

    if (value_out == ft_nullptr)
        return (ft_byte_buffer::set_error(FT_ERR_INVALID_ARGUMENT));
    read_error = this->read(bytes, 4);
    if (read_error != FT_ERR_SUCCESS)
        return (read_error);
    *value_out = static_cast<uint32_t>(bytes[0])
        | (static_cast<uint32_t>(bytes[1]) << 8)
        | (static_cast<uint32_t>(bytes[2]) << 16)
        | (static_cast<uint32_t>(bytes[3]) << 24);
    return (ft_byte_buffer::set_error(FT_ERR_SUCCESS));
}

int32_t ft_byte_buffer::read_u64_be(uint64_t *value_out) noexcept
{
    uint8_t bytes[8];
    int32_t read_error;

    if (value_out == ft_nullptr)
        return (ft_byte_buffer::set_error(FT_ERR_INVALID_ARGUMENT));
    read_error = this->read(bytes, 8);
    if (read_error != FT_ERR_SUCCESS)
        return (read_error);
    *value_out = (static_cast<uint64_t>(bytes[0]) << 56)
        | (static_cast<uint64_t>(bytes[1]) << 48)
        | (static_cast<uint64_t>(bytes[2]) << 40)
        | (static_cast<uint64_t>(bytes[3]) << 32)
        | (static_cast<uint64_t>(bytes[4]) << 24)
        | (static_cast<uint64_t>(bytes[5]) << 16)
        | (static_cast<uint64_t>(bytes[6]) << 8)
        | static_cast<uint64_t>(bytes[7]);
    return (ft_byte_buffer::set_error(FT_ERR_SUCCESS));
}

int32_t ft_byte_buffer::read_u64_le(uint64_t *value_out) noexcept
{
    uint8_t bytes[8];
    int32_t read_error;

    if (value_out == ft_nullptr)
        return (ft_byte_buffer::set_error(FT_ERR_INVALID_ARGUMENT));
    read_error = this->read(bytes, 8);
    if (read_error != FT_ERR_SUCCESS)
        return (read_error);
    *value_out = static_cast<uint64_t>(bytes[0])
        | (static_cast<uint64_t>(bytes[1]) << 8)
        | (static_cast<uint64_t>(bytes[2]) << 16)
        | (static_cast<uint64_t>(bytes[3]) << 24)
        | (static_cast<uint64_t>(bytes[4]) << 32)
        | (static_cast<uint64_t>(bytes[5]) << 40)
        | (static_cast<uint64_t>(bytes[6]) << 48)
        | (static_cast<uint64_t>(bytes[7]) << 56);
    return (ft_byte_buffer::set_error(FT_ERR_SUCCESS));
}

const uint8_t *ft_byte_buffer::data() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
        "ft_byte_buffer::data");
    ft_byte_buffer::set_error(FT_ERR_SUCCESS);
    return (this->_data);
}

ft_size_t ft_byte_buffer::size() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
        "ft_byte_buffer::size");
    ft_byte_buffer::set_error(FT_ERR_SUCCESS);
    return (this->_size);
}

ft_size_t ft_byte_buffer::capacity() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
        "ft_byte_buffer::capacity");
    ft_byte_buffer::set_error(FT_ERR_SUCCESS);
    return (this->_capacity);
}

ft_size_t ft_byte_buffer::read_position() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
        "ft_byte_buffer::read_position");
    ft_byte_buffer::set_error(FT_ERR_SUCCESS);
    return (this->_read_position);
}

ft_size_t ft_byte_buffer::remaining() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
        "ft_byte_buffer::remaining");
    ft_byte_buffer::set_error(FT_ERR_SUCCESS);
    if (this->_read_position > this->_size)
        return (0);
    return (this->_size - this->_read_position);
}

ft_bool ft_byte_buffer::is_fixed_capacity() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
        "ft_byte_buffer::is_fixed_capacity");
    ft_byte_buffer::set_error(FT_ERR_SUCCESS);
    return (this->_fixed_capacity);
}

ft_bool ft_byte_buffer::is_initialised() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        return (FT_TRUE);
    return (FT_FALSE);
}

int32_t ft_byte_buffer::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int32_t initialization_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
        "ft_byte_buffer::enable_thread_safety");
    if (this->_mutex != ft_nullptr)
        return (ft_byte_buffer::set_error(FT_ERR_SUCCESS));
    mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    if (mutex_pointer == ft_nullptr)
        return (ft_byte_buffer::set_error(FT_ERR_NO_MEMORY));
    initialization_error = mutex_pointer->initialize();
    if (initialization_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        return (ft_byte_buffer::set_error(initialization_error));
    }
    this->_mutex = mutex_pointer;
    return (ft_byte_buffer::set_error(FT_ERR_SUCCESS));
}

int32_t ft_byte_buffer::disable_thread_safety() noexcept
{
    int32_t destroy_error;

    if (this->_mutex == ft_nullptr)
        return (ft_byte_buffer::set_error(FT_ERR_SUCCESS));
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    if (destroy_error != FT_ERR_SUCCESS)
        return (ft_byte_buffer::set_error(destroy_error));
    return (ft_byte_buffer::set_error(FT_ERR_SUCCESS));
}

ft_bool ft_byte_buffer::is_thread_safe() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
        "ft_byte_buffer::is_thread_safe");
    ft_byte_buffer::set_error(FT_ERR_SUCCESS);
    if (this->_mutex != ft_nullptr)
        return (FT_TRUE);
    return (FT_FALSE);
}

int32_t ft_byte_buffer::get_error() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_lifecycle(this->_initialised_state,
            "ft_byte_buffer::get_error",
            "called on uninitialised object");
    return (ft_byte_buffer::_last_error);
}

const char *ft_byte_buffer::get_error_str() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_lifecycle(this->_initialised_state,
            "ft_byte_buffer::get_error_str",
            "called on uninitialised object");
    return (ft_strerror(ft_byte_buffer::_last_error));
}
