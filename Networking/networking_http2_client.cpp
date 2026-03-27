#include "http2_client.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/basic.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include "openssl_support.hpp"

#if NETWORKING_HAS_OPENSSL
#include <openssl/ssl.h>
#include <cstdlib>

thread_local int32_t http2_frame::_last_error = FT_ERR_SUCCESS;

static void http2_append_raw_byte(ft_string &target, unsigned char value)
{
    target.append(static_cast<char>(value));
    return ;
}

http2_header_field::http2_header_field() noexcept
    : _initialised_state(FT_CLASS_STATE_UNINITIALISED), _name(), _value(), _mutex(ft_nullptr)
{
    return ;
}

http2_header_field::~http2_header_field() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    return ;
}

http2_header_field::http2_header_field(const http2_header_field &other) noexcept
    : http2_header_field()
{
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_lifecycle(other._initialised_state,
            "http2_header_field::http2_header_field(copy)",
            "source is uninitialised");
    if (this->initialize(other) != FT_ERR_SUCCESS)
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return ;
}

http2_header_field::http2_header_field(http2_header_field &&other) noexcept
    : http2_header_field()
{
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_lifecycle(other._initialised_state,
            "http2_header_field::http2_header_field(move)",
            "source is uninitialised");
    if (this->initialize(static_cast<http2_header_field &&>(other)) != FT_ERR_SUCCESS)
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return ;
}

int32_t http2_header_field::move(http2_header_field &other) noexcept
{
    return (this->initialize(static_cast<http2_header_field &&>(other)));
}

int32_t http2_header_field::initialize() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        errno_abort_lifecycle(this->_initialised_state, "http2_header_field::initialize", "initialize called on initialised instance");
    int32_t name_error = this->_name.initialize();
    if (name_error != FT_ERR_SUCCESS)
        return (name_error);
    int32_t value_error = this->_value.initialize();
    if (value_error != FT_ERR_SUCCESS)
    {
        this->_name.destroy();
        return (value_error);
    }
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (FT_ERR_SUCCESS);
}

int32_t http2_header_field::initialize(const http2_header_field &other) noexcept
{
    ft_bool lock_acquired;

    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_lifecycle(other._initialised_state,
            "http2_header_field::initialize(const http2_header_field &)",
            "source is uninitialised");
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (FT_ERR_SUCCESS);
    }
    if (this->initialize() != FT_ERR_SUCCESS)
        return (FT_ERR_INITIALIZATION_FAILED);
    lock_acquired = FT_FALSE;
    if (other.lock(&lock_acquired) != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (FT_ERR_INVALID_OPERATION);
    }
    this->_name = other._name;
    this->_value = other._value;
    const_cast<http2_header_field &>(other).unlock(lock_acquired);
    if (this->_name.get_error() != FT_ERR_SUCCESS
        || this->_value.get_error() != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (FT_ERR_INVALID_OPERATION);
    }
    return (FT_ERR_SUCCESS);
}

int32_t http2_header_field::initialize(http2_header_field &&other) noexcept
{
    int32_t initialize_error;

    initialize_error = this->initialize(other);
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    (void)other.destroy();
    return (FT_ERR_SUCCESS);
}

int32_t http2_header_field::destroy() noexcept
{
    int32_t first_error;
    int32_t disable_error;
    int32_t name_error;
    int32_t value_error;

    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED
        || this->_initialised_state == FT_CLASS_STATE_DESTROYED)
        return (FT_ERR_SUCCESS);
    first_error = FT_ERR_SUCCESS;
    disable_error = this->disable_thread_safety();
    if (disable_error != FT_ERR_SUCCESS)
        first_error = disable_error;
    name_error = this->_name.destroy();
    if (first_error == FT_ERR_SUCCESS && name_error != FT_ERR_SUCCESS)
        first_error = name_error;
    value_error = this->_value.destroy();
    if (first_error == FT_ERR_SUCCESS && value_error != FT_ERR_SUCCESS)
        first_error = value_error;
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return (first_error);
}

int32_t http2_header_field::enable_thread_safety() noexcept
{
    void *memory_pointer;
    pt_recursive_mutex *mutex_pointer;
    int32_t mutex_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "http2_header_field::enable_thread_safety");
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    memory_pointer = std::malloc(sizeof(pt_recursive_mutex));
    if (memory_pointer == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    mutex_pointer = new(memory_pointer) pt_recursive_mutex();
    mutex_error = mutex_pointer->initialize();
    if (mutex_error != FT_ERR_SUCCESS)
    {
        mutex_pointer->~pt_recursive_mutex();
        std::free(memory_pointer);
        return (mutex_error);
    }
    this->_mutex = mutex_pointer;
    return (FT_ERR_SUCCESS);
}

int32_t http2_header_field::disable_thread_safety() noexcept
{
    int32_t destroy_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "http2_header_field::disable_thread_safety");
    if (this->_mutex != ft_nullptr)
    {
        destroy_error = this->_mutex->destroy();
        this->_mutex->~pt_recursive_mutex();
        std::free(this->_mutex);
        this->_mutex = ft_nullptr;
        return (destroy_error);
    }
    return (FT_ERR_SUCCESS);
}

ft_bool http2_header_field::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int32_t http2_header_field::lock(ft_bool *lock_acquired) const noexcept
{
    http2_header_field *mutable_field;
    ft_bool has_mutex;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = FT_FALSE;
    mutable_field = const_cast<http2_header_field *>(this);
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "http2_header_field::lock");
    has_mutex = (mutable_field->_mutex != ft_nullptr);
    if (pt_recursive_mutex_lock_if_not_null(mutable_field->_mutex) != FT_ERR_SUCCESS)
        return (FT_ERR_INVALID_OPERATION);
    if (lock_acquired != ft_nullptr && has_mutex)
        *lock_acquired = FT_TRUE;
    return (FT_ERR_SUCCESS);
}

void http2_header_field::unlock(ft_bool lock_acquired) const noexcept
{
    http2_header_field *mutable_field;

    if (lock_acquired == FT_FALSE)
        return ;
    mutable_field = const_cast<http2_header_field *>(this);
    (void)pt_recursive_mutex_unlock_if_not_null(mutable_field->_mutex);
    return ;
}

ft_bool http2_header_field::set_name(const ft_string &name_value) noexcept
{
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    if (this->lock(&lock_acquired) != 0)
        return (FT_FALSE);
    this->_name = name_value;
    this->unlock(lock_acquired);
    return (this->_name.get_error() == FT_ERR_SUCCESS);
}

ft_bool http2_header_field::set_name_from_cstr(const char *name_value) noexcept
{
    ft_string temporary_name;

    if (name_value == ft_nullptr)
        return (FT_FALSE);
    if (temporary_name.initialize() != FT_ERR_SUCCESS)
        return (FT_FALSE);
    temporary_name = name_value;
    if (temporary_name.get_error() != FT_ERR_SUCCESS)
        return (FT_FALSE);
    return (this->set_name(temporary_name));
}

ft_bool http2_header_field::set_name_from_buffer(const char *buffer, ft_size_t length) noexcept
{
    ft_bool lock_acquired;

    if (buffer == ft_nullptr && length > 0)
        return (FT_FALSE);
    lock_acquired = FT_FALSE;
    if (this->lock(&lock_acquired) != 0)
        return (FT_FALSE);
    this->_name.assign(buffer, length);
    this->unlock(lock_acquired);
    return (this->_name.get_error() == FT_ERR_SUCCESS);
}

ft_bool http2_header_field::set_value(const ft_string &value_value) noexcept
{
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    if (this->lock(&lock_acquired) != 0)
        return (FT_FALSE);
    this->_value = value_value;
    this->unlock(lock_acquired);
    return (this->_value.get_error() == FT_ERR_SUCCESS);
}

ft_bool http2_header_field::set_value_from_cstr(const char *value_value) noexcept
{
    ft_string temporary_value;

    if (value_value == ft_nullptr)
        return (FT_FALSE);
    if (temporary_value.initialize() != FT_ERR_SUCCESS)
        return (FT_FALSE);
    temporary_value = value_value;
    if (temporary_value.get_error() != FT_ERR_SUCCESS)
        return (FT_FALSE);
    return (this->set_value(temporary_value));
}

ft_bool http2_header_field::set_value_from_buffer(const char *buffer, ft_size_t length) noexcept
{
    ft_bool lock_acquired;

    if (buffer == ft_nullptr && length > 0)
        return (FT_FALSE);
    lock_acquired = FT_FALSE;
    if (this->lock(&lock_acquired) != 0)
        return (FT_FALSE);
    this->_value.assign(buffer, length);
    this->unlock(lock_acquired);
    return (this->_value.get_error() == FT_ERR_SUCCESS);
}

ft_bool http2_header_field::assign(const ft_string &name_value, const ft_string &value_value) noexcept
{
    ft_bool lock_acquired;
    ft_bool success_state;

    success_state = FT_TRUE;
    lock_acquired = FT_FALSE;
    if (this->lock(&lock_acquired) != 0)
        return (FT_FALSE);
    this->_name = name_value;
    if (this->_name.get_error() != FT_ERR_SUCCESS)
        success_state = FT_FALSE;
    if (success_state)
    {
        this->_value = value_value;
        if (this->_value.get_error() != FT_ERR_SUCCESS)
            success_state = FT_FALSE;
    }
    this->unlock(lock_acquired);
    return (success_state);
}

ft_bool http2_header_field::assign_from_cstr(const char *name_value, const char *value_value) noexcept
{
    ft_string name_string;
    ft_string value_string;
    int32_t initialize_error;

    if (name_value == ft_nullptr || value_value == ft_nullptr)
        return (FT_FALSE);
    initialize_error = name_string.initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        return (FT_FALSE);
    initialize_error = value_string.initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        return (FT_FALSE);
    name_string = name_value;
    if (name_string.get_error() != FT_ERR_SUCCESS)
        return (FT_FALSE);
    value_string = value_value;
    if (value_string.get_error() != FT_ERR_SUCCESS)
        return (FT_FALSE);
    return (this->assign(name_string, value_string));
}

ft_bool http2_header_field::assign_from_buffers(const char *name_buffer, ft_size_t name_length,
    const char *value_buffer, ft_size_t value_length) noexcept
{
    ft_bool lock_acquired;
    ft_bool success_state;

    if ((name_buffer == ft_nullptr && name_length > 0) || (value_buffer == ft_nullptr && value_length > 0))
        return (FT_FALSE);
    success_state = FT_TRUE;
    lock_acquired = FT_FALSE;
    if (this->lock(&lock_acquired) != 0)
        return (FT_FALSE);
    this->_name.assign(name_buffer, name_length);
    if (this->_name.get_error() != FT_ERR_SUCCESS)
        success_state = FT_FALSE;
    if (success_state)
    {
        this->_value.assign(value_buffer, value_length);
        if (this->_value.get_error() != FT_ERR_SUCCESS)
            success_state = FT_FALSE;
    }
    this->unlock(lock_acquired);
    return (success_state);
}

ft_bool http2_header_field::copy_name(ft_string &out_name) const noexcept
{
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    if (this->lock(&lock_acquired) != 0)
        return (FT_FALSE);
    out_name = this->_name;
    this->unlock(lock_acquired);
    return (out_name.get_error() == FT_ERR_SUCCESS);
}

ft_bool http2_header_field::copy_value(ft_string &out_value) const noexcept
{
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    if (this->lock(&lock_acquired) != 0)
        return (FT_FALSE);
    out_value = this->_value;
    this->unlock(lock_acquired);
    return (out_value.get_error() == FT_ERR_SUCCESS);
}

void http2_header_field::clear() noexcept
{
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    if (this->lock(&lock_acquired) != 0)
        return ;
    this->_name.clear();
    this->_value.clear();
    this->unlock(lock_acquired);
    return ;
}

http2_frame::http2_frame() noexcept
    : _initialised_state(FT_CLASS_STATE_UNINITIALISED), _type(0), _flags(0),
      _stream_identifier(0), _payload(), _mutex(ft_nullptr)
{
    return ;
}

http2_frame::~http2_frame() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    return ;
}

int32_t http2_frame::initialize() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        errno_abort_lifecycle(this->_initialised_state, "http2_frame::initialize", "initialize called on initialised instance");
    this->_type = 0;
    this->_flags = 0;
    this->_stream_identifier = 0;
    int32_t payload_error = this->_payload.initialize();
    if (payload_error != FT_ERR_SUCCESS)
    {
        this->set_error(payload_error);
        return (payload_error);
    }
    this->_mutex = ft_nullptr;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t http2_frame::destroy() noexcept
{
    int32_t disable_error;
    int32_t payload_error;
    int32_t first_error;
    int32_t previous_error;

    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED
        || this->_initialised_state == FT_CLASS_STATE_DESTROYED)
        return (FT_ERR_SUCCESS);
    previous_error = http2_frame::_last_error;
    first_error = FT_ERR_SUCCESS;
    disable_error = this->disable_thread_safety();
    if (disable_error != FT_ERR_SUCCESS)
        first_error = disable_error;
    payload_error = this->_payload.destroy();
    if (first_error == FT_ERR_SUCCESS && payload_error != FT_ERR_SUCCESS)
        first_error = payload_error;
    this->_type = 0;
    this->_flags = 0;
    this->_stream_identifier = 0;
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    if (first_error != FT_ERR_SUCCESS)
        return (this->set_error(first_error));
    this->set_error(previous_error);
    return (FT_ERR_SUCCESS);
}

http2_frame::http2_frame(const http2_frame &other) noexcept
    : http2_frame()
{
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_lifecycle(other._initialised_state,
            "http2_frame::http2_frame(copy)",
            "source is uninitialised");
    if (this->initialize(other) != FT_ERR_SUCCESS)
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return ;
}

http2_frame::http2_frame(http2_frame &&other) noexcept
    : http2_frame()
{
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_lifecycle(other._initialised_state,
            "http2_frame::http2_frame(move)",
            "source is uninitialised");
    if (this->initialize(static_cast<http2_frame &&>(other)) != FT_ERR_SUCCESS)
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return ;
}

int32_t http2_frame::move(http2_frame &other) noexcept
{
    return (this->initialize(static_cast<http2_frame &&>(other)));
}

int32_t http2_frame::initialize(const http2_frame &other) noexcept
{
    ft_bool lock_acquired;
    int32_t destroy_error;
    int32_t initialize_error;
    int32_t source_error;
    int32_t lock_error;
    int32_t enable_error;
    int32_t payload_error;

    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_lifecycle(other._initialised_state,
            "http2_frame::initialize(const http2_frame &)",
            "source is uninitialised");
    source_error = http2_frame::_last_error;
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        destroy_error = this->destroy();
        if (destroy_error != FT_ERR_SUCCESS)
            return (this->set_error(destroy_error));
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_type = 0;
        this->_flags = 0;
        this->_stream_identifier = 0;
        this->_mutex = ft_nullptr;
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(source_error);
        return (FT_ERR_SUCCESS);
    }
    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (this->set_error(initialize_error));
    }
    lock_acquired = FT_FALSE;
    lock_error = other.lock(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (this->set_error(lock_error));
    }
    this->_type = other._type;
    this->_flags = other._flags;
    this->_stream_identifier = other._stream_identifier;
    this->_payload = other._payload;
    const_cast<http2_frame &>(other).unlock(lock_acquired);
    payload_error = this->_payload.get_error();
    if (payload_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (this->set_error(payload_error));
    }
    if (other.is_thread_safe() == FT_TRUE)
    {
        enable_error = this->enable_thread_safety();
        if (enable_error != FT_ERR_SUCCESS)
        {
            (void)this->destroy();
            return (this->set_error(enable_error));
        }
    }
    this->set_error(source_error);
    return (FT_ERR_SUCCESS);
}

int32_t http2_frame::initialize(http2_frame &&other) noexcept
{
    int32_t initialize_error;
    int32_t destroy_error;
    int32_t source_error;

    initialize_error = this->initialize(other);
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    source_error = http2_frame::_last_error;
    destroy_error = other.destroy();
    if (destroy_error != FT_ERR_SUCCESS)
        return (this->set_error(destroy_error));
    this->set_error(source_error);
    return (FT_ERR_SUCCESS);
}

int32_t http2_frame::enable_thread_safety() noexcept
{
    void *memory_pointer;
    pt_recursive_mutex *mutex_pointer;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "http2_frame::enable_thread_safety");
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    memory_pointer = std::malloc(sizeof(pt_recursive_mutex));
    if (memory_pointer == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    mutex_pointer = new(memory_pointer) pt_recursive_mutex();
    if (mutex_pointer->initialize() != FT_ERR_SUCCESS)
    {
        (void)mutex_pointer->destroy();
        mutex_pointer->~pt_recursive_mutex();
        std::free(memory_pointer);
        return (FT_ERR_INVALID_OPERATION);
    }
    this->_mutex = mutex_pointer;
    return (FT_ERR_SUCCESS);
}

int32_t http2_frame::disable_thread_safety() noexcept
{
    int32_t destroy_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "http2_frame::disable_thread_safety");
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    this->_mutex->~pt_recursive_mutex();
    std::free(this->_mutex);
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

ft_bool http2_frame::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int32_t http2_frame::lock(ft_bool *lock_acquired) const noexcept
{
    http2_frame *mutable_frame;
    ft_bool has_mutex;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = FT_FALSE;
    mutable_frame = const_cast<http2_frame *>(this);
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "http2_frame::lock");
    has_mutex = (mutable_frame->_mutex != ft_nullptr);
    if (pt_recursive_mutex_lock_if_not_null(mutable_frame->_mutex) != FT_ERR_SUCCESS)
        return (FT_ERR_INVALID_OPERATION);
    if (lock_acquired != ft_nullptr && has_mutex)
        *lock_acquired = FT_TRUE;
    return (FT_ERR_SUCCESS);
}

void http2_frame::unlock(ft_bool lock_acquired) const noexcept
{
    http2_frame *mutable_frame;

    if (lock_acquired == FT_FALSE)
        return ;
    mutable_frame = const_cast<http2_frame *>(this);
    (void)pt_recursive_mutex_unlock_if_not_null(mutable_frame->_mutex);
    return ;
}

ft_bool http2_frame::set_type(uint8_t type_value) noexcept
{
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    if (this->lock(&lock_acquired) != 0)
        return (FT_FALSE);
    this->_type = type_value;
    this->unlock(lock_acquired);
    return (FT_TRUE);
}

ft_bool http2_frame::get_type(uint8_t &out_type) const noexcept
{
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    if (this->lock(&lock_acquired) != 0)
        return (FT_FALSE);
    out_type = this->_type;
    this->unlock(lock_acquired);
    return (FT_TRUE);
}

ft_bool http2_frame::set_flags(uint8_t flags_value) noexcept
{
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    if (this->lock(&lock_acquired) != 0)
        return (FT_FALSE);
    this->_flags = flags_value;
    this->unlock(lock_acquired);
    return (FT_TRUE);
}

ft_bool http2_frame::get_flags(uint8_t &out_flags) const noexcept
{
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    if (this->lock(&lock_acquired) != 0)
        return (FT_FALSE);
    out_flags = this->_flags;
    this->unlock(lock_acquired);
    return (FT_TRUE);
}

ft_bool http2_frame::set_stream_identifier(uint32_t stream_identifier_value) noexcept
{
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    if (this->lock(&lock_acquired) != 0)
        return (FT_FALSE);
    this->_stream_identifier = stream_identifier_value;
    this->unlock(lock_acquired);
    return (FT_TRUE);
}

ft_bool http2_frame::get_stream_identifier(uint32_t &out_stream_identifier) const noexcept
{
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    if (this->lock(&lock_acquired) != 0)
        return (FT_FALSE);
    out_stream_identifier = this->_stream_identifier;
    this->unlock(lock_acquired);
    return (FT_TRUE);
}

ft_bool http2_frame::set_payload(const ft_string &payload_value) noexcept
{
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    if (this->lock(&lock_acquired) != 0)
        return (FT_FALSE);
    this->_payload = payload_value;
    this->unlock(lock_acquired);
    return (this->_payload.get_error() == FT_ERR_SUCCESS);
}

ft_bool http2_frame::set_payload_from_buffer(const char *buffer, ft_size_t length) noexcept
{
    ft_bool lock_acquired;

    if (buffer == ft_nullptr && length > 0)
        return (FT_FALSE);
    lock_acquired = FT_FALSE;
    if (this->lock(&lock_acquired) != 0)
        return (FT_FALSE);
    this->_payload.assign(buffer, length);
    this->unlock(lock_acquired);
    return (this->_payload.get_error() == FT_ERR_SUCCESS);
}

ft_bool http2_frame::copy_payload(ft_string &out_payload) const noexcept
{
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    if (this->lock(&lock_acquired) != 0)
        return (FT_FALSE);
    out_payload = this->_payload;
    this->unlock(lock_acquired);
    return (out_payload.get_error() == FT_ERR_SUCCESS);
}

int32_t http2_frame::set_error(int32_t error_code) const noexcept
{
    http2_frame::_last_error = error_code;
    return (error_code);
}

int32_t http2_frame::get_error(void) const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "http2_frame::get_error");
    return (http2_frame::_last_error);
}

const char *http2_frame::get_error_str(void) const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "http2_frame::get_error_str");
    return (ft_strerror(http2_frame::_last_error));
}

void http2_frame::clear_payload() noexcept
{
    ft_bool lock_acquired;

    lock_acquired = FT_FALSE;
    if (this->lock(&lock_acquired) != 0)
        return ;
    this->_payload.clear();
    this->unlock(lock_acquired);
    return ;
}

http2_stream_state::http2_stream_state() noexcept
    : buffer(), dependency_identifier(0), weight(16), exclusive_dependency(FT_FALSE),
      remote_window(65535), local_window(65535)
{
    if (this->buffer.initialize() != FT_ERR_SUCCESS)
        (void)this->buffer.destroy();
    return ;
}

http2_stream_state::~http2_stream_state() noexcept
{
    (void)this->buffer.destroy();
    return ;
}

http2_stream_manager::http2_stream_manager() noexcept
    : _initialised_state(FT_CLASS_STATE_UNINITIALISED), _streams(), _stream_identifiers(),
      _initial_remote_window(65535),
      _initial_local_window(65535), _connection_remote_window(65535),
      _connection_local_window(65535), _mutex(ft_nullptr)
{
    return ;
}

thread_local int32_t http2_stream_manager::_last_error = FT_ERR_SUCCESS;

http2_stream_manager::http2_stream_manager(const http2_stream_manager &other) noexcept
    : http2_stream_manager()
{
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_lifecycle(other._initialised_state,
            "http2_stream_manager::http2_stream_manager(copy)",
            "source is uninitialised");
    if (this->initialize(other) != FT_ERR_SUCCESS)
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return ;
}

http2_stream_manager::http2_stream_manager(http2_stream_manager &&other) noexcept
    : http2_stream_manager()
{
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_lifecycle(other._initialised_state,
            "http2_stream_manager::http2_stream_manager(move)",
            "source is uninitialised");
    if (this->initialize(static_cast<http2_stream_manager &&>(other)) != FT_ERR_SUCCESS)
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return ;
}

http2_stream_manager::~http2_stream_manager() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    return ;
}

int32_t http2_stream_manager::initialize() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        errno_abort_lifecycle(this->_initialised_state, "http2_stream_manager::initialize", "initialize called on initialised instance");
    int32_t streams_error = this->_streams.initialize();
    if (streams_error != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_UNINITIALISED;
        return (this->set_error(streams_error));
    }
    int32_t identifiers_error = this->_stream_identifiers.initialize();
    if (identifiers_error != FT_ERR_SUCCESS)
    {
        this->_streams.destroy();
        this->_initialised_state = FT_CLASS_STATE_UNINITIALISED;
        return (this->set_error(identifiers_error));
    }
    this->_initial_remote_window = 65535;
    this->_initial_local_window = 65535;
    this->_connection_remote_window = 65535;
    this->_connection_local_window = 65535;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (this->set_error(FT_ERR_SUCCESS));
}

int32_t http2_stream_manager::move(http2_stream_manager &other) noexcept
{
    return (this->initialize(static_cast<http2_stream_manager &&>(other)));
}

int32_t http2_stream_manager::initialize(const http2_stream_manager &other) noexcept
{
    ft_bool lock_acquired;
    int32_t destroy_error;
    int32_t initialize_error;
    int32_t source_error;
    int32_t lock_error;
    int32_t streams_error;
    int32_t identifiers_error;

    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_lifecycle(other._initialised_state,
            "http2_stream_manager::initialize(const http2_stream_manager &)",
            "source is uninitialised");
    source_error = http2_stream_manager::_last_error;
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        destroy_error = this->destroy();
        if (destroy_error != FT_ERR_SUCCESS)
            return (this->set_error(destroy_error));
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initial_remote_window = 65535;
        this->_initial_local_window = 65535;
        this->_connection_remote_window = 65535;
        this->_connection_local_window = 65535;
        this->_mutex = ft_nullptr;
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(source_error);
        return (FT_ERR_SUCCESS);
    }
    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (this->set_error(initialize_error));
    }
    lock_acquired = FT_FALSE;
    lock_error = other.lock(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (this->set_error(lock_error));
    }
    this->_initial_remote_window = other._initial_remote_window;
    this->_initial_local_window = other._initial_local_window;
    this->_connection_remote_window = other._connection_remote_window;
    this->_connection_local_window = other._connection_local_window;
    streams_error = this->_streams.copy_from(other._streams);
    if (streams_error != FT_ERR_SUCCESS)
    {
        const_cast<http2_stream_manager &>(other).unlock(lock_acquired);
        (void)this->destroy();
        return (this->set_error(streams_error));
    }
    identifiers_error = this->_stream_identifiers.copy_from(
            other._stream_identifiers);
    const_cast<http2_stream_manager &>(other).unlock(lock_acquired);
    if (identifiers_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (this->set_error(identifiers_error));
    }
    this->set_error(source_error);
    return (FT_ERR_SUCCESS);
}

int32_t http2_stream_manager::initialize(http2_stream_manager &&other) noexcept
{
    int32_t initialize_error;
    int32_t destroy_error;
    int32_t source_error;

    initialize_error = this->initialize(other);
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    source_error = http2_stream_manager::_last_error;
    destroy_error = other.destroy();
    if (destroy_error != FT_ERR_SUCCESS)
        return (this->set_error(destroy_error));
    this->set_error(source_error);
    return (FT_ERR_SUCCESS);
}

int32_t http2_stream_manager::destroy() noexcept
{
    int32_t first_error;
    int32_t disable_error;
    int32_t stream_error;
    int32_t identifiers_error;
    int32_t previous_error;

    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED
        || this->_initialised_state == FT_CLASS_STATE_DESTROYED)
        return (FT_ERR_SUCCESS);
    previous_error = http2_stream_manager::_last_error;
    first_error = FT_ERR_SUCCESS;
    disable_error = this->disable_thread_safety();
    if (disable_error != FT_ERR_SUCCESS)
        first_error = disable_error;
    stream_error = this->_streams.destroy();
    if (first_error == FT_ERR_SUCCESS && stream_error != FT_ERR_SUCCESS)
        first_error = stream_error;
    identifiers_error = this->_stream_identifiers.destroy();
    if (first_error == FT_ERR_SUCCESS && identifiers_error != FT_ERR_SUCCESS)
        first_error = identifiers_error;
    this->_initial_remote_window = 65535;
    this->_initial_local_window = 65535;
    this->_connection_remote_window = 65535;
    this->_connection_local_window = 65535;
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    if (first_error != FT_ERR_SUCCESS)
        return (this->set_error(first_error));
    this->set_error(previous_error);
    return (FT_ERR_SUCCESS);
}

int32_t http2_stream_manager::enable_thread_safety() noexcept
{
    void *memory_pointer;
    pt_recursive_mutex *mutex_pointer;
    int32_t mutex_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "http2_stream_manager::enable_thread_safety");
    if (this->_mutex != ft_nullptr)
    {
        return (this->set_error(FT_ERR_SUCCESS));
    }
    memory_pointer = std::malloc(sizeof(pt_recursive_mutex));
    if (!memory_pointer)
    {
        return (this->set_error(FT_ERR_NO_MEMORY));
    }
    mutex_pointer = new(memory_pointer) pt_recursive_mutex();
    mutex_error = mutex_pointer->initialize();
    if (mutex_error != FT_ERR_SUCCESS)
    {
        mutex_pointer->~pt_recursive_mutex();
        std::free(memory_pointer);
        return (this->set_error(mutex_error));
    }
    this->_mutex = mutex_pointer;
    return (this->set_error(FT_ERR_SUCCESS));
}

int32_t http2_stream_manager::lock(ft_bool *lock_acquired) const noexcept
{
    http2_stream_manager *mutable_manager;
    ft_bool has_mutex;
    int32_t lock_error;

    if (lock_acquired)
        *lock_acquired = FT_FALSE;
    mutable_manager = const_cast<http2_stream_manager *>(this);
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "http2_stream_manager::lock");
    has_mutex = (this->_mutex != ft_nullptr);
    lock_error = pt_recursive_mutex_lock_if_not_null(mutable_manager->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (lock_acquired && has_mutex)
        *lock_acquired = FT_TRUE;
    return (FT_ERR_SUCCESS);
}

void http2_stream_manager::unlock(ft_bool lock_acquired) const noexcept
{
    http2_stream_manager *mutable_manager;

    if (!lock_acquired)
        return ;
    mutable_manager = const_cast<http2_stream_manager *>(this);
    (void)pt_recursive_mutex_unlock_if_not_null(mutable_manager->_mutex);
    return ;
}

int32_t http2_stream_manager::disable_thread_safety() noexcept
{
    int32_t destroy_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "http2_stream_manager::disable_thread_safety");
    if (this->_mutex == ft_nullptr)
        return (this->set_error(FT_ERR_SUCCESS));
    destroy_error = this->_mutex->destroy();
    this->_mutex->~pt_recursive_mutex();
    std::free(this->_mutex);
    this->_mutex = ft_nullptr;
    return (this->set_error(destroy_error));
}

ft_bool http2_stream_manager::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int32_t http2_stream_manager::set_error(int32_t error_code) const noexcept
{
    http2_stream_manager::_last_error = error_code;
    return (error_code);
}

int32_t http2_stream_manager::get_error(void) const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "http2_stream_manager::get_error");
    return (http2_stream_manager::_last_error);
}

const char *http2_stream_manager::get_error_str(void) const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "http2_stream_manager::get_error_str");
    return (ft_strerror(http2_stream_manager::_last_error));
}

ft_bool http2_stream_manager::validate_receive_window(uint32_t stream_identifier,
    uint32_t length) noexcept
{
    Pair<uint32_t, http2_stream_state> *stream_entry;

    stream_entry = this->_streams.find(stream_identifier);
    if (stream_entry == this->_streams.end())
    {
        return (FT_FALSE);
    }
    if (stream_entry->value.local_window < length)
    {
        return (FT_FALSE);
    }
    if (this->_connection_local_window < length)
    {
        return (FT_FALSE);
    }
    return (FT_TRUE);
}

ft_bool http2_stream_manager::record_received_data(uint32_t stream_identifier,
    uint32_t length) noexcept
{
    Pair<uint32_t, http2_stream_state> *stream_entry;

    stream_entry = this->_streams.find(stream_identifier);
    if (stream_entry == this->_streams.end())
    {
        return (FT_FALSE);
    }
    if (stream_entry->value.local_window < length)
    {
        return (FT_FALSE);
    }
    if (this->_connection_local_window < length)
    {
        return (FT_FALSE);
    }
    stream_entry->value.local_window -= length;
    this->_connection_local_window -= length;
    return (FT_TRUE);
}

ft_bool http2_stream_manager::reserve_remote_connection_window(uint32_t length) noexcept
{
    if (this->_connection_remote_window < length)
    {
        return (FT_FALSE);
    }
    return (FT_TRUE);
}

ft_bool http2_stream_manager::record_connection_send(uint32_t length) noexcept
{
    if (this->_connection_remote_window < length)
    {
        return (FT_FALSE);
    }
    this->_connection_remote_window -= length;
    return (FT_TRUE);
}

void http2_stream_manager::remove_stream_identifier(uint32_t stream_identifier) noexcept
{
    ft_size_t identifier_count;
    ft_vector<uint32_t>::iterator iterator_value;
    ft_size_t index;

    identifier_count = this->_stream_identifiers.size();
    if (FT_ERR_INVALID_STATE != FT_ERR_SUCCESS)
        return ;
    iterator_value = this->_stream_identifiers.begin();
    index = 0;
    while (index < identifier_count)
    {
        if (*iterator_value == stream_identifier)
        {
            this->_stream_identifiers.erase(iterator_value);
            return ;
        }
        ++iterator_value;
        ++index;
    }
    return ;
}

ft_bool http2_stream_manager::open_stream(uint32_t stream_identifier) noexcept
{
    Pair<uint32_t, http2_stream_state> *existing_entry;
    http2_stream_state new_state;
    ft_bool lock_acquired;
    ft_bool success_state;

    lock_acquired = FT_FALSE;
    success_state = FT_TRUE;
    if (this->lock(&lock_acquired) != 0)
        return (FT_FALSE);
    existing_entry = this->_streams.find(stream_identifier);
    if (existing_entry != this->_streams.end())
    {
        this->unlock(lock_acquired);
        this->set_error(FT_ERR_ALREADY_EXISTS);
        success_state = FT_FALSE;
        return (FT_FALSE);
    }
    if (success_state)
    {
        new_state.remote_window = this->_initial_remote_window;
        new_state.local_window = this->_initial_local_window;
        this->_streams.insert(stream_identifier, new_state);
    }
    if (success_state)
    {
        this->_stream_identifiers.push_back(stream_identifier);
        if (this->_stream_identifiers.get_error() != FT_ERR_SUCCESS)
        {
            this->unlock(lock_acquired);
            this->set_error(this->_stream_identifiers.get_error());
            return (FT_FALSE);
        }
    }
    this->unlock(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return (success_state);
}

ft_bool http2_stream_manager::append_data(uint32_t stream_identifier, const char *data,
    ft_size_t length) noexcept
{
    Pair<uint32_t, http2_stream_state> *stream_entry;
    ft_size_t index;
    uint32_t length_32;
    ft_bool lock_acquired;
    ft_bool success_state;

    lock_acquired = FT_FALSE;
    success_state = FT_TRUE;
    if (this->lock(&lock_acquired) != 0)
    {
        this->set_error(FT_ERR_INVALID_OPERATION);
        return (FT_FALSE);
    }
    if (!data && length > 0)
    {
        this->unlock(lock_acquired);
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        success_state = FT_FALSE;
        return (FT_FALSE);
    }
    stream_entry = ft_nullptr;
    if (success_state)
    {
        stream_entry = this->_streams.find(stream_identifier);
        if (stream_entry == this->_streams.end())
        {
            this->unlock(lock_acquired);
            this->set_error(FT_ERR_NOT_FOUND);
            success_state = FT_FALSE;
            return (FT_FALSE);
        }
    }
    if (success_state && length > 0xFFFFFFFFu)
    {
        this->unlock(lock_acquired);
        this->set_error(FT_ERR_OUT_OF_RANGE);
        success_state = FT_FALSE;
        return (FT_FALSE);
    }
    if (success_state)
    {
        length_32 = static_cast<uint32_t>(length);
        if (!this->validate_receive_window(stream_identifier, length_32))
        {
            this->unlock(lock_acquired);
            this->set_error(FT_ERR_INVALID_OPERATION);
            success_state = FT_FALSE;
            return (FT_FALSE);
        }
        if (success_state)
        {
            index = 0;
            while (index < length)
            {
                stream_entry->value.buffer.append(data[index]);
                if (stream_entry->value.buffer.get_error() != FT_ERR_SUCCESS)
                {
                    this->unlock(lock_acquired);
                    this->set_error(stream_entry->value.buffer.get_error());
                    success_state = FT_FALSE;
                    return (FT_FALSE);
                }
                index++;
            }
            if (success_state)
            {
                if (!this->record_received_data(stream_identifier, length_32))
                {
                    this->unlock(lock_acquired);
                    this->set_error(FT_ERR_INVALID_OPERATION);
                    success_state = FT_FALSE;
                    return (FT_FALSE);
                }
            }
        }
    }
    this->unlock(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return (success_state);
}

ft_bool http2_stream_manager::close_stream(uint32_t stream_identifier) noexcept
{
    Pair<uint32_t, http2_stream_state> *stream_entry;
    ft_bool lock_acquired;
    ft_bool success_state;

    lock_acquired = FT_FALSE;
    success_state = FT_TRUE;
    if (this->lock(&lock_acquired) != 0)
    {
        this->set_error(FT_ERR_INVALID_OPERATION);
        return (FT_FALSE);
    }
    stream_entry = this->_streams.find(stream_identifier);
    if (stream_entry == this->_streams.end())
    {
        this->unlock(lock_acquired);
        this->set_error(FT_ERR_NOT_FOUND);
        success_state = FT_FALSE;
        return (FT_FALSE);
    }
    if (success_state)
    {
        this->_streams.remove(stream_identifier);
        this->remove_stream_identifier(stream_identifier);
    }
    this->unlock(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return (success_state);
}

ft_bool http2_stream_manager::get_stream_buffer(uint32_t stream_identifier,
    ft_string &out_buffer) const noexcept
{
    const Pair<uint32_t, http2_stream_state> *stream_entry;
    ft_bool lock_acquired;
    ft_bool success_state;

    lock_acquired = FT_FALSE;
    success_state = FT_TRUE;
    if (this->lock(&lock_acquired) != 0)
    {
        this->set_error(FT_ERR_INVALID_OPERATION);
        return (FT_FALSE);
    }
    stream_entry = this->_streams.find(stream_identifier);
    if (stream_entry == this->_streams.end())
    {
        this->unlock(lock_acquired);
        this->set_error(FT_ERR_NOT_FOUND);
        success_state = FT_FALSE;
        return (FT_FALSE);
    }
    if (success_state)
    {
        out_buffer = stream_entry->value.buffer;
        if (out_buffer.get_error() != FT_ERR_SUCCESS)
        {
            this->unlock(lock_acquired);
            this->set_error(out_buffer.get_error());
            success_state = FT_FALSE;
            return (FT_FALSE);
        }
    }
    this->unlock(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return (success_state);
}

ft_bool http2_stream_manager::update_priority(uint32_t stream_identifier,
    uint32_t dependency_identifier, uint8_t weight, ft_bool exclusive) noexcept
{
    Pair<uint32_t, http2_stream_state> *stream_entry;
    ft_size_t identifier_count;
    ft_vector<uint32_t>::iterator iterator_value;
    ft_size_t index;
    ft_bool lock_acquired;
    ft_bool success_state;

    lock_acquired = FT_FALSE;
    success_state = FT_TRUE;
    if (this->lock(&lock_acquired) != 0)
        return (FT_FALSE);
    stream_entry = ft_nullptr;
    if (stream_identifier == dependency_identifier)
    {
        success_state = FT_FALSE;
    }
    if (success_state && weight == 0)
    {
        success_state = FT_FALSE;
    }
    if (success_state)
    {
        stream_entry = this->_streams.find(stream_identifier);
        if (stream_entry == this->_streams.end())
        {
            success_state = FT_FALSE;
        }
    }
    if (success_state && exclusive)
    {
        identifier_count = this->_stream_identifiers.size();
        iterator_value = this->_stream_identifiers.begin();
        index = 0;
        while (index < identifier_count && success_state)
        {
            uint32_t child_identifier;
            Pair<uint32_t, http2_stream_state> *child_entry;

            child_identifier = *iterator_value;
            child_entry = this->_streams.find(child_identifier);
            if (child_entry == this->_streams.end())
            {
                success_state = FT_FALSE;
            }
            else if (child_identifier != stream_identifier
                && child_entry->value.dependency_identifier == dependency_identifier)
                child_entry->value.dependency_identifier = stream_identifier;
            ++iterator_value;
            ++index;
        }
    }
    if (success_state && stream_entry != this->_streams.end())
    {
        stream_entry->value.dependency_identifier = dependency_identifier;
        stream_entry->value.weight = weight;
        stream_entry->value.exclusive_dependency = exclusive;
    }
    this->unlock(lock_acquired);
    return (success_state);
}

ft_bool http2_stream_manager::get_priority(uint32_t stream_identifier,
    uint32_t &dependency_identifier, uint8_t &weight, ft_bool &exclusive) const noexcept
{
    const Pair<uint32_t, http2_stream_state> *stream_entry;
    ft_bool lock_acquired;
    ft_bool success_state;

    lock_acquired = FT_FALSE;
    success_state = FT_TRUE;
    if (this->lock(&lock_acquired) != 0)
        return (FT_FALSE);
    stream_entry = this->_streams.find(stream_identifier);
    if (stream_entry == this->_streams.end())
    {
        success_state = FT_FALSE;
    }
    if (success_state)
    {
        dependency_identifier = stream_entry->value.dependency_identifier;
        weight = stream_entry->value.weight;
        exclusive = stream_entry->value.exclusive_dependency;
    }
    this->unlock(lock_acquired);
    return (success_state);
}

ft_bool http2_stream_manager::update_remote_initial_window(uint32_t new_window) noexcept
{
    uint32_t previous_window;
    ft_size_t identifier_count;
    ft_size_t index;
    ft_bool lock_acquired;
    ft_bool success_state;

    lock_acquired = FT_FALSE;
    success_state = FT_TRUE;
    if (this->lock(&lock_acquired) != 0)
    {
        this->set_error(FT_ERR_INVALID_OPERATION);
        return (FT_FALSE);
    }
    previous_window = this->_initial_remote_window;
    if (new_window > 0x7FFFFFFFu)
    {
        this->unlock(lock_acquired);
        this->set_error(FT_ERR_OUT_OF_RANGE);
        return (FT_FALSE);
    }
    if (success_state)
    {
        this->_initial_remote_window = new_window;
        identifier_count = this->_stream_identifiers.size();
        index = 0;
        while (index < identifier_count && success_state)
        {
            uint32_t identifier_value;
            Pair<uint32_t, http2_stream_state> *stream_entry;

            identifier_value = this->_stream_identifiers[index];
            stream_entry = this->_streams.find(identifier_value);
            if (stream_entry == this->_streams.end())
            {
                this->unlock(lock_acquired);
                this->set_error(FT_ERR_NOT_FOUND);
                return (FT_FALSE);
            }
            else if (new_window >= previous_window)
            {
                uint32_t delta;
                uint64_t updated_window;

                delta = new_window - previous_window;
                updated_window = static_cast<uint64_t>(stream_entry->value.remote_window)
                    + delta;
                if (updated_window > 0x7FFFFFFFul)
                    stream_entry->value.remote_window = 0x7FFFFFFF;
                else
                    stream_entry->value.remote_window = static_cast<uint32_t>(updated_window);
            }
            else
            {
                uint32_t delta;

                delta = previous_window - new_window;
                if (stream_entry->value.remote_window > delta)
                    stream_entry->value.remote_window -= delta;
                else
                    stream_entry->value.remote_window = 0;
            }
            index++;
        }
    }
    this->unlock(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return (success_state);
}

ft_bool http2_stream_manager::update_local_initial_window(uint32_t new_window) noexcept
{
    uint32_t previous_window;
    ft_size_t identifier_count;
    ft_size_t index;
    ft_bool lock_acquired;
    ft_bool success_state;

    lock_acquired = FT_FALSE;
    success_state = FT_TRUE;
    if (this->lock(&lock_acquired) != 0)
    {
        this->set_error(FT_ERR_INVALID_OPERATION);
        return (FT_FALSE);
    }
    previous_window = this->_initial_local_window;
    if (new_window > 0x7FFFFFFFu)
    {
        this->unlock(lock_acquired);
        this->set_error(FT_ERR_OUT_OF_RANGE);
        return (FT_FALSE);
    }
    if (success_state)
    {
        this->_initial_local_window = new_window;
        identifier_count = this->_stream_identifiers.size();
        index = 0;
        while (index < identifier_count && success_state)
        {
            uint32_t identifier_value;
            Pair<uint32_t, http2_stream_state> *stream_entry;

            identifier_value = this->_stream_identifiers[index];
            stream_entry = this->_streams.find(identifier_value);
            if (stream_entry == this->_streams.end())
            {
                this->unlock(lock_acquired);
                this->set_error(FT_ERR_NOT_FOUND);
                return (FT_FALSE);
            }
            else if (new_window >= previous_window)
            {
                uint32_t delta;
                uint64_t updated_window;

                delta = new_window - previous_window;
                updated_window = static_cast<uint64_t>(stream_entry->value.local_window)
                    + delta;
                if (updated_window > 0x7FFFFFFFul)
                    stream_entry->value.local_window = 0x7FFFFFFF;
                else
                    stream_entry->value.local_window = static_cast<uint32_t>(updated_window);
            }
            else
            {
                uint32_t delta;

                delta = previous_window - new_window;
                if (stream_entry->value.local_window > delta)
                    stream_entry->value.local_window -= delta;
                else
                    stream_entry->value.local_window = 0;
            }
            index++;
        }
    }
    this->unlock(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return (success_state);
}

uint32_t http2_stream_manager::get_local_window(uint32_t stream_identifier) const noexcept
{
    const Pair<uint32_t, http2_stream_state> *stream_entry;
    ft_bool lock_acquired;
    ft_bool success_state;
    uint32_t window_value;

    lock_acquired = FT_FALSE;
    success_state = FT_TRUE;
    window_value = 0;
    if (this->lock(&lock_acquired) != 0)
    {
        this->set_error(FT_ERR_INVALID_OPERATION);
        return (0);
    }
    stream_entry = this->_streams.find(stream_identifier);
    if (stream_entry == this->_streams.end())
    {
        this->unlock(lock_acquired);
        this->set_error(FT_ERR_NOT_FOUND);
        return (0);
    }
    if (success_state)
    {
        window_value = stream_entry->value.local_window;
    }
    this->unlock(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return (window_value);
}

uint32_t http2_stream_manager::get_remote_window(uint32_t stream_identifier) const noexcept
{
    const Pair<uint32_t, http2_stream_state> *stream_entry;
    ft_bool lock_acquired;
    ft_bool success_state;
    uint32_t window_value;

    lock_acquired = FT_FALSE;
    success_state = FT_TRUE;
    window_value = 0;
    if (this->lock(&lock_acquired) != 0)
    {
        this->set_error(FT_ERR_INVALID_OPERATION);
        return (0);
    }
    stream_entry = this->_streams.find(stream_identifier);
    if (stream_entry == this->_streams.end())
    {
        this->unlock(lock_acquired);
        this->set_error(FT_ERR_NOT_FOUND);
        return (0);
    }
    if (success_state)
    {
        window_value = stream_entry->value.remote_window;
    }
    this->unlock(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return (window_value);
}

ft_bool http2_stream_manager::increase_local_window(uint32_t stream_identifier,
    uint32_t increment) noexcept
{
    Pair<uint32_t, http2_stream_state> *stream_entry;
    uint64_t updated_window;
    ft_bool lock_acquired;
    ft_bool success_state;

    lock_acquired = FT_FALSE;
    success_state = FT_TRUE;
    stream_entry = ft_nullptr;
    if (this->lock(&lock_acquired) != 0)
    {
        this->set_error(FT_ERR_INVALID_OPERATION);
        return (FT_FALSE);
    }
    if (increment > 0x7FFFFFFFu)
    {
        this->unlock(lock_acquired);
        this->set_error(FT_ERR_OUT_OF_RANGE);
        return (FT_FALSE);
    }
    if (success_state)
    {
        stream_entry = this->_streams.find(stream_identifier);
        if (stream_entry == this->_streams.end())
        {
            this->unlock(lock_acquired);
            this->set_error(FT_ERR_NOT_FOUND);
            return (FT_FALSE);
        }
    }
    if (success_state)
    {
        updated_window = static_cast<uint64_t>(stream_entry->value.local_window)
            + increment;
        if (updated_window > 0x7FFFFFFFul)
        {
            this->unlock(lock_acquired);
            this->set_error(FT_ERR_OUT_OF_RANGE);
            return (FT_FALSE);
        }
        else
        {
            stream_entry->value.local_window = static_cast<uint32_t>(updated_window);
        }
    }
    this->unlock(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return (success_state);
}

ft_bool http2_stream_manager::increase_remote_window(uint32_t stream_identifier,
    uint32_t increment) noexcept
{
    Pair<uint32_t, http2_stream_state> *stream_entry;
    uint64_t updated_window;
    ft_bool lock_acquired;
    ft_bool success_state;

    lock_acquired = FT_FALSE;
    success_state = FT_TRUE;
    stream_entry = ft_nullptr;
    if (this->lock(&lock_acquired) != 0)
    {
        this->set_error(FT_ERR_INVALID_OPERATION);
        return (FT_FALSE);
    }
    if (increment > 0x7FFFFFFFu)
    {
        this->unlock(lock_acquired);
        this->set_error(FT_ERR_OUT_OF_RANGE);
        return (FT_FALSE);
    }
    if (success_state)
    {
        stream_entry = this->_streams.find(stream_identifier);
        if (stream_entry == this->_streams.end())
        {
            this->unlock(lock_acquired);
            this->set_error(FT_ERR_NOT_FOUND);
            return (FT_FALSE);
        }
    }
    if (success_state)
    {
        updated_window = static_cast<uint64_t>(stream_entry->value.remote_window)
            + increment;
        if (updated_window > 0x7FFFFFFFul)
        {
            this->unlock(lock_acquired);
            this->set_error(FT_ERR_OUT_OF_RANGE);
            return (FT_FALSE);
        }
        else
        {
            stream_entry->value.remote_window = static_cast<uint32_t>(updated_window);
        }
    }
    this->unlock(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return (success_state);
}

ft_bool http2_stream_manager::reserve_send_window(uint32_t stream_identifier,
    uint32_t length) noexcept
{
    Pair<uint32_t, http2_stream_state> *stream_entry;
    ft_bool lock_acquired;
    ft_bool success_state;

    lock_acquired = FT_FALSE;
    success_state = FT_TRUE;
    if (this->lock(&lock_acquired) != 0)
    {
        this->set_error(FT_ERR_INVALID_OPERATION);
        return (FT_FALSE);
    }
    stream_entry = this->_streams.find(stream_identifier);
    if (stream_entry == this->_streams.end())
    {
        this->unlock(lock_acquired);
        this->set_error(FT_ERR_NOT_FOUND);
        return (FT_FALSE);
    }
    if (success_state)
    {
        if (!this->reserve_remote_connection_window(length))
        {
            this->unlock(lock_acquired);
            this->set_error(FT_ERR_INVALID_OPERATION);
            return (FT_FALSE);
        }
    }
    if (success_state && stream_entry->value.remote_window < length)
    {
        this->unlock(lock_acquired);
        this->set_error(FT_ERR_OUT_OF_RANGE);
        return (FT_FALSE);
    }
    if (success_state)
    {
        stream_entry->value.remote_window -= length;
        if (!this->record_connection_send(length))
        {
            this->unlock(lock_acquired);
            this->set_error(FT_ERR_INVALID_OPERATION);
            return (FT_FALSE);
        }
    }
    this->unlock(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return (success_state);
}

ft_bool http2_stream_manager::update_connection_local_window(uint32_t increment) noexcept
{
    uint64_t updated_window;
    ft_bool lock_acquired;
    ft_bool success_state;

    lock_acquired = FT_FALSE;
    success_state = FT_TRUE;
    if (this->lock(&lock_acquired) != 0)
    {
        this->set_error(FT_ERR_INVALID_OPERATION);
        return (FT_FALSE);
    }
    if (increment > 0x7FFFFFFFu)
    {
        this->unlock(lock_acquired);
        this->set_error(FT_ERR_OUT_OF_RANGE);
        return (FT_FALSE);
    }
    if (success_state)
    {
        updated_window = static_cast<uint64_t>(this->_connection_local_window)
            + increment;
        if (updated_window > 0x7FFFFFFFul)
        {
            this->unlock(lock_acquired);
            this->set_error(FT_ERR_OUT_OF_RANGE);
            return (FT_FALSE);
        }
        else
        {
            this->_connection_local_window = static_cast<uint32_t>(updated_window);
        }
    }
    this->unlock(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return (success_state);
}

ft_bool http2_stream_manager::update_connection_remote_window(uint32_t increment) noexcept
{
    uint64_t updated_window;
    ft_bool lock_acquired;
    ft_bool success_state;

    lock_acquired = FT_FALSE;
    success_state = FT_TRUE;
    if (this->lock(&lock_acquired) != 0)
    {
        this->set_error(FT_ERR_INVALID_OPERATION);
        return (FT_FALSE);
    }
    if (increment > 0x7FFFFFFFu)
    {
        this->unlock(lock_acquired);
        this->set_error(FT_ERR_OUT_OF_RANGE);
        return (FT_FALSE);
    }
    if (success_state)
    {
        updated_window = static_cast<uint64_t>(this->_connection_remote_window)
            + increment;
        if (updated_window > 0x7FFFFFFFul)
        {
            this->unlock(lock_acquired);
            this->set_error(FT_ERR_OUT_OF_RANGE);
            return (FT_FALSE);
        }
        else
        {
            this->_connection_remote_window = static_cast<uint32_t>(updated_window);
        }
    }
    this->unlock(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return (success_state);
}

uint32_t http2_stream_manager::get_connection_local_window() const noexcept
{
    ft_bool lock_acquired;
    uint32_t window_value;

    lock_acquired = FT_FALSE;
    window_value = 0;
    if (this->lock(&lock_acquired) != 0)
    {
        this->set_error(FT_ERR_INVALID_OPERATION);
        return (0);
    }
    window_value = this->_connection_local_window;
    this->unlock(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return (window_value);
}

uint32_t http2_stream_manager::get_connection_remote_window() const noexcept
{
    ft_bool lock_acquired;
    uint32_t window_value;

    lock_acquired = FT_FALSE;
    window_value = 0;
    if (this->lock(&lock_acquired) != 0)
    {
        this->set_error(FT_ERR_INVALID_OPERATION);
        return (0);
    }
    window_value = this->_connection_remote_window;
    this->unlock(lock_acquired);
    this->set_error(FT_ERR_SUCCESS);
    return (window_value);
}

ft_bool http2_settings_state::apply_single_setting(uint16_t identifier, uint32_t value,
    http2_stream_manager &streams) noexcept
{
    if (identifier == 0x1)
    {
        this->_header_table_size = value;
        return (FT_TRUE);
    }
    if (identifier == 0x2)
    {
        if (value > 1)
        {
            return (FT_FALSE);
        }
        this->_enable_push = (value != 0);
        return (FT_TRUE);
    }
    if (identifier == 0x3)
    {
        this->_max_concurrent_streams = value;
        return (FT_TRUE);
    }
    if (identifier == 0x4)
    {
        if (value > 0x7FFFFFFFu)
        {
            return (FT_FALSE);
        }
        if (!streams.update_remote_initial_window(value))
        {
            return (FT_FALSE);
        }
        this->_initial_remote_window = value;
        return (FT_TRUE);
    }
    if (identifier == 0x5)
    {
        if (value < 16384 || value > 16777215)
        {
            return (FT_FALSE);
        }
        this->_max_frame_size = value;
        return (FT_TRUE);
    }
    if (identifier == 0x6)
    {
        this->_max_header_list_size = value;
        return (FT_TRUE);
    }
    return (FT_TRUE);
}

http2_settings_state::http2_settings_state() noexcept
    : _initialised_state(FT_CLASS_STATE_UNINITIALISED), _header_table_size(4096),
      _enable_push(FT_TRUE), _max_concurrent_streams(0xFFFFFFFFu),
      _initial_local_window(65535), _initial_remote_window(65535),
      _max_frame_size(16384), _max_header_list_size(0)
{
    return ;
}

http2_settings_state::http2_settings_state(const http2_settings_state &other) noexcept
    : http2_settings_state()
{
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_lifecycle(other._initialised_state,
            "http2_settings_state::http2_settings_state(copy)",
            "source is uninitialised");
    if (this->initialize(other) != FT_ERR_SUCCESS)
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return ;
}

http2_settings_state::http2_settings_state(http2_settings_state &&other) noexcept
    : http2_settings_state()
{
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_lifecycle(other._initialised_state,
            "http2_settings_state::http2_settings_state(move)",
            "source is uninitialised");
    if (this->initialize(static_cast<http2_settings_state &&>(other)) != FT_ERR_SUCCESS)
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return ;
}

http2_settings_state::~http2_settings_state() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    return ;
}

int32_t http2_settings_state::initialize() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        errno_abort_lifecycle(this->_initialised_state, "http2_settings_state::initialize", "initialize called on initialised instance");
    this->_header_table_size = 4096;
    this->_enable_push = FT_TRUE;
    this->_max_concurrent_streams = 0xFFFFFFFFu;
    this->_initial_local_window = 65535;
    this->_initial_remote_window = 65535;
    this->_max_frame_size = 16384;
    this->_max_header_list_size = 0;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (FT_ERR_SUCCESS);
}

int32_t http2_settings_state::move(http2_settings_state &other) noexcept
{
    return (this->initialize(static_cast<http2_settings_state &&>(other)));
}

int32_t http2_settings_state::initialize(const http2_settings_state &other) noexcept
{
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_lifecycle(other._initialised_state,
            "http2_settings_state::initialize(const http2_settings_state &)",
            "source is uninitialised");
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (FT_ERR_SUCCESS);
    }
    this->_header_table_size = other._header_table_size;
    this->_enable_push = other._enable_push;
    this->_max_concurrent_streams = other._max_concurrent_streams;
    this->_initial_local_window = other._initial_local_window;
    this->_initial_remote_window = other._initial_remote_window;
    this->_max_frame_size = other._max_frame_size;
    this->_max_header_list_size = other._max_header_list_size;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (FT_ERR_SUCCESS);
}

int32_t http2_settings_state::initialize(http2_settings_state &&other) noexcept
{
    int32_t initialize_error;

    initialize_error = this->initialize(other);
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    (void)other.destroy();
    return (FT_ERR_SUCCESS);
}

int32_t http2_settings_state::destroy() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED
        || this->_initialised_state == FT_CLASS_STATE_DESTROYED)
        return (FT_ERR_SUCCESS);
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return (FT_ERR_SUCCESS);
}

ft_bool http2_settings_state::apply_remote_settings(const http2_frame &frame,
    http2_stream_manager &streams) noexcept
{
    ft_size_t payload_length;
    const unsigned char *payload_bytes;
    ft_size_t offset;
    uint8_t frame_type;
    uint8_t frame_flags;
    ft_string payload_copy;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "http2_settings_state::apply_remote_settings");
    if (!frame.get_type(frame_type))
    {
        return (FT_FALSE);
    }
    if (frame_type != 0x4)
    {
        return (FT_FALSE);
    }
    if (!frame.copy_payload(payload_copy))
    {
        return (FT_FALSE);
    }
    payload_length = payload_copy.size();
    if (payload_copy.get_error() != FT_ERR_SUCCESS)
    {
        return (FT_FALSE);
    }
    if (!frame.get_flags(frame_flags))
    {
        return (FT_FALSE);
    }
    if ((frame_flags & 0x1) != 0)
    {
        if (payload_length != 0)
        {
            return (FT_FALSE);
        }
        return (FT_TRUE);
    }
    if ((payload_length % 6) != 0)
    {
        return (FT_FALSE);
    }
    payload_bytes = reinterpret_cast<const unsigned char*>(payload_copy.c_str());
    if (payload_copy.get_error() != FT_ERR_SUCCESS)
    {
        return (FT_FALSE);
    }
    offset = 0;
    while (offset < payload_length)
    {
        uint16_t identifier;
        uint32_t value;

        identifier = static_cast<uint16_t>(static_cast<uint32_t>(payload_bytes[offset]) << 8);
        identifier |= static_cast<uint16_t>(payload_bytes[offset + 1]);
        value = (static_cast<uint32_t>(payload_bytes[offset + 2]) << 24)
            | (static_cast<uint32_t>(payload_bytes[offset + 3]) << 16)
            | (static_cast<uint32_t>(payload_bytes[offset + 4]) << 8)
            | static_cast<uint32_t>(payload_bytes[offset + 5]);
        if (!this->apply_single_setting(identifier, value, streams))
            return (FT_FALSE);
        offset += 6;
    }
    return (FT_TRUE);
}

ft_bool http2_settings_state::update_local_initial_window(uint32_t new_window,
    http2_stream_manager &streams) noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "http2_settings_state::update_local_initial_window");
    if (new_window > 0x7FFFFFFFu)
    {
        return (FT_FALSE);
    }
    if (!streams.update_local_initial_window(new_window))
    {
        return (FT_FALSE);
    }
    this->_initial_local_window = new_window;
    return (FT_TRUE);
}

ft_bool http2_settings_state::update_remote_initial_window(uint32_t new_window,
    http2_stream_manager &streams) noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "http2_settings_state::update_remote_initial_window");
    if (new_window > 0x7FFFFFFFu)
    {
        return (FT_FALSE);
    }
    if (!streams.update_remote_initial_window(new_window))
    {
        return (FT_FALSE);
    }
    this->_initial_remote_window = new_window;
    return (FT_TRUE);
}

uint32_t http2_settings_state::get_header_table_size() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "http2_settings_state::get_header_table_size");
    return (this->_header_table_size);
}

ft_bool http2_settings_state::get_enable_push() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "http2_settings_state::get_enable_push");
    return (this->_enable_push);
}

uint32_t http2_settings_state::get_max_concurrent_streams() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "http2_settings_state::get_max_concurrent_streams");
    return (this->_max_concurrent_streams);
}

uint32_t http2_settings_state::get_initial_local_window() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "http2_settings_state::get_initial_local_window");
    return (this->_initial_local_window);
}

uint32_t http2_settings_state::get_initial_remote_window() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "http2_settings_state::get_initial_remote_window");
    return (this->_initial_remote_window);
}

uint32_t http2_settings_state::get_max_frame_size() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "http2_settings_state::get_max_frame_size");
    return (this->_max_frame_size);
}

uint32_t http2_settings_state::get_max_header_list_size() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "http2_settings_state::get_max_header_list_size");
    return (this->_max_header_list_size);
}

ft_bool http2_encode_frame(const http2_frame &frame, ft_string &out_buffer,
    int32_t &error_code) noexcept
{
    ft_size_t payload_length;
    unsigned char header[9];
    ft_size_t index;
    ft_string payload_copy;
    const char *payload_data;
    uint8_t frame_type;
    uint8_t frame_flags;
    uint32_t frame_stream_identifier;

    if (out_buffer.clear() != FT_ERR_SUCCESS)
    {
        error_code = FT_ERR_NO_MEMORY;
        return (FT_FALSE);
    }
    if (!frame.copy_payload(payload_copy))
    {
        error_code = FT_ERR_INVALID_STATE;
        return (FT_FALSE);
    }
    payload_length = payload_copy.size();
    if (payload_length > 0xFFFFFF)
    {
        error_code = FT_ERR_OUT_OF_RANGE;
        return (FT_FALSE);
    }
    if (!frame.get_type(frame_type))
    {
        error_code = FT_ERR_INVALID_STATE;
        return (FT_FALSE);
    }
    if (!frame.get_flags(frame_flags))
    {
        error_code = FT_ERR_INVALID_STATE;
        return (FT_FALSE);
    }
    if (!frame.get_stream_identifier(frame_stream_identifier))
    {
        error_code = FT_ERR_INVALID_STATE;
        return (FT_FALSE);
    }
    header[0] = static_cast<unsigned char>((payload_length >> 16) & 0xFF);
    header[1] = static_cast<unsigned char>((payload_length >> 8) & 0xFF);
    header[2] = static_cast<unsigned char>(payload_length & 0xFF);
    header[3] = frame_type;
    header[4] = frame_flags;
    header[5] = static_cast<unsigned char>((frame_stream_identifier >> 24) & 0x7F);
    header[6] = static_cast<unsigned char>((frame_stream_identifier >> 16) & 0xFF);
    header[7] = static_cast<unsigned char>((frame_stream_identifier >> 8) & 0xFF);
    header[8] = static_cast<unsigned char>(frame_stream_identifier & 0xFF);
    index = 0;
    while (index < sizeof(header))
    {
        if (out_buffer.append(static_cast<char>(header[index])) != FT_ERR_SUCCESS)
        {
            error_code = FT_ERR_NO_MEMORY;
            return (FT_FALSE);
        }
        index++;
    }
    payload_data = payload_copy.c_str();
    index = 0;
    while (index < payload_length)
    {
        if (out_buffer.append(static_cast<char>(payload_data[index])) != FT_ERR_SUCCESS)
        {
            error_code = FT_ERR_NO_MEMORY;
            return (FT_FALSE);
        }
        index++;
    }
    error_code = FT_ERR_SUCCESS;
    return (FT_TRUE);
}

ft_bool http2_decode_frame(const unsigned char *buffer, ft_size_t buffer_size,
    ft_size_t &offset, http2_frame &out_frame, int32_t &error_code) noexcept
{
    ft_size_t remaining;
    ft_size_t payload_length;
    ft_size_t index;
    uint32_t stream_identifier_value;
    ft_string payload_copy;

    if (!buffer)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        return (FT_FALSE);
    }
    if (buffer_size < offset)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        return (FT_FALSE);
    }
    remaining = buffer_size - offset;
    if (remaining < 9)
    {
        error_code = FT_ERR_OUT_OF_RANGE;
        return (FT_FALSE);
    }
    payload_length = (static_cast<ft_size_t>(buffer[offset]) << 16);
    payload_length |= (static_cast<ft_size_t>(buffer[offset + 1]) << 8);
    payload_length |= static_cast<ft_size_t>(buffer[offset + 2]);
    if (!out_frame.set_type(buffer[offset + 3]))
    {
        error_code = FT_ERR_INVALID_STATE;
        return (FT_FALSE);
    }
    if (!out_frame.set_flags(buffer[offset + 4]))
    {
        error_code = FT_ERR_INVALID_STATE;
        return (FT_FALSE);
    }
    stream_identifier_value = (static_cast<uint32_t>(buffer[offset + 5] & 0x7F) << 24);
    stream_identifier_value |= (static_cast<uint32_t>(buffer[offset + 6]) << 16);
    stream_identifier_value |= (static_cast<uint32_t>(buffer[offset + 7]) << 8);
    stream_identifier_value |= static_cast<uint32_t>(buffer[offset + 8]);
    if (!out_frame.set_stream_identifier(stream_identifier_value))
    {
        error_code = FT_ERR_INVALID_STATE;
        return (FT_FALSE);
    }
    if (remaining < 9 + payload_length)
    {
        error_code = FT_ERR_OUT_OF_RANGE;
        return (FT_FALSE);
    }
    if (payload_copy.initialize() != FT_ERR_SUCCESS)
    {
        error_code = payload_copy.get_error();
        return (FT_FALSE);
    }
    if (payload_copy.get_error() != FT_ERR_SUCCESS)
    {
        error_code = payload_copy.get_error();
        return (FT_FALSE);
    }
    index = 0;
    while (index < payload_length)
    {
        http2_append_raw_byte(payload_copy,
            static_cast<unsigned char>(buffer[offset + 9 + index]));
        if (payload_copy.get_error() != FT_ERR_SUCCESS)
        {
            error_code = payload_copy.get_error();
            return (FT_FALSE);
        }
        index++;
    }
    if (!out_frame.set_payload(payload_copy))
    {
        error_code = FT_ERR_INVALID_STATE;
        return (FT_FALSE);
    }
    offset += 9 + payload_length;
    error_code = FT_ERR_SUCCESS;
    return (FT_TRUE);
}

ft_bool http2_compress_headers(const ft_vector<http2_header_field> &headers,
    ft_string &out_block, int32_t &error_code) noexcept
{
    ft_size_t header_count;
    ft_size_t index;

    out_block.clear();
    if (out_block.get_error() != FT_ERR_SUCCESS)
    {
        error_code = out_block.get_error();
        return (FT_FALSE);
    }
    header_count = headers.size();
        if (header_count > 0xFFFF)
    {
        error_code = FT_ERR_OUT_OF_RANGE;
        return (FT_FALSE);
    }
    http2_append_raw_byte(out_block,
        static_cast<unsigned char>((header_count >> 8) & 0xFF));
    if (out_block.get_error() != FT_ERR_SUCCESS)
    {
        error_code = out_block.get_error();
        return (FT_FALSE);
    }
    http2_append_raw_byte(out_block,
        static_cast<unsigned char>(header_count & 0xFF));
    if (out_block.get_error() != FT_ERR_SUCCESS)
    {
        error_code = out_block.get_error();
        return (FT_FALSE);
    }
    index = 0;
    while (index < header_count)
    {
        const http2_header_field &field = headers[index];
        ft_string field_name;
        ft_string field_value;
        ft_size_t name_length;
        ft_size_t value_length;
        ft_size_t name_index;
        ft_size_t value_index;
        const char *name_data;
        const char *value_data;

        if (!field.copy_name(field_name))
        {
            error_code = FT_ERR_INVALID_STATE;
            return (FT_FALSE);
        }
        if (!field.copy_value(field_value))
        {
            error_code = FT_ERR_INVALID_STATE;
            return (FT_FALSE);
        }
        name_length = field_name.size();
        if (field_name.get_error() != FT_ERR_SUCCESS)
        {
            error_code = field_name.get_error();
            return (FT_FALSE);
        }
        value_length = field_value.size();
        if (field_value.get_error() != FT_ERR_SUCCESS)
        {
            error_code = field_value.get_error();
            return (FT_FALSE);
        }
        if (name_length > 0xFFFF || value_length > 0xFFFF)
        {
            error_code = FT_ERR_OUT_OF_RANGE;
            return (FT_FALSE);
        }
        http2_append_raw_byte(out_block,
            static_cast<unsigned char>((name_length >> 8) & 0xFF));
        if (out_block.get_error() != FT_ERR_SUCCESS)
        {
            error_code = out_block.get_error();
            return (FT_FALSE);
        }
        http2_append_raw_byte(out_block,
            static_cast<unsigned char>(name_length & 0xFF));
        if (out_block.get_error() != FT_ERR_SUCCESS)
        {
            error_code = out_block.get_error();
            return (FT_FALSE);
        }
        name_data = field_name.c_str();
        if (field_name.get_error() != FT_ERR_SUCCESS)
        {
            error_code = field_name.get_error();
            return (FT_FALSE);
        }
        name_index = 0;
        while (name_index < name_length)
        {
            http2_append_raw_byte(out_block,
                static_cast<unsigned char>(name_data[name_index]));
            if (out_block.get_error() != FT_ERR_SUCCESS)
            {
                error_code = out_block.get_error();
                return (FT_FALSE);
            }
            name_index++;
        }
        http2_append_raw_byte(out_block,
            static_cast<unsigned char>((value_length >> 8) & 0xFF));
        if (out_block.get_error() != FT_ERR_SUCCESS)
        {
            error_code = out_block.get_error();
            return (FT_FALSE);
        }
        http2_append_raw_byte(out_block,
            static_cast<unsigned char>(value_length & 0xFF));
        if (out_block.get_error() != FT_ERR_SUCCESS)
        {
            error_code = out_block.get_error();
            return (FT_FALSE);
        }
        value_data = field_value.c_str();
        if (field_value.get_error() != FT_ERR_SUCCESS)
        {
            error_code = field_value.get_error();
            return (FT_FALSE);
        }
        value_index = 0;
        while (value_index < value_length)
        {
            http2_append_raw_byte(out_block,
                static_cast<unsigned char>(value_data[value_index]));
            if (out_block.get_error() != FT_ERR_SUCCESS)
            {
                error_code = out_block.get_error();
                return (FT_FALSE);
            }
            value_index++;
        }
        index++;
    }
    error_code = FT_ERR_SUCCESS;
    return (FT_TRUE);
}

ft_bool http2_decompress_headers(const ft_string &block,
    ft_vector<http2_header_field> &out_headers, int32_t &error_code) noexcept
{
    const unsigned char *buffer;
    const unsigned char *cursor;
    const unsigned char *end;
    ft_size_t buffer_length;
    ft_size_t header_count;
    ft_size_t index;

    out_headers.clear();
        buffer_length = block.size();
    if (block.get_error() != FT_ERR_SUCCESS)
    {
        error_code = block.get_error();
        return (FT_FALSE);
    }
    buffer = reinterpret_cast<const unsigned char*>(block.c_str());
    if (block.get_error() != FT_ERR_SUCCESS)
    {
        error_code = block.get_error();
        return (FT_FALSE);
    }
    if (buffer_length < 2)
    {
        error_code = FT_ERR_OUT_OF_RANGE;
        return (FT_FALSE);
    }
    header_count = (static_cast<ft_size_t>(buffer[0]) << 8)
        | static_cast<ft_size_t>(buffer[1]);
    cursor = buffer + 2;
    end = buffer + buffer_length;
    index = 0;
    while (index < header_count)
    {
        http2_header_field entry;
        ft_size_t name_length;
        ft_size_t value_length;

        if (entry.initialize() != FT_ERR_SUCCESS)
        {
            error_code = FT_ERR_INVALID_STATE;
            return (FT_FALSE);
        }

        if (cursor + 4 > end)
        {
            error_code = FT_ERR_OUT_OF_RANGE;
            return (FT_FALSE);
        }
        name_length = (static_cast<ft_size_t>(cursor[0]) << 8)
            | static_cast<ft_size_t>(cursor[1]);
        cursor += 2;
        if (cursor + name_length + 2 > end)
        {
            error_code = FT_ERR_OUT_OF_RANGE;
            return (FT_FALSE);
        }
        if (!entry.set_name_from_buffer(reinterpret_cast<const char*>(cursor),
                name_length))
        {
            error_code = FT_ERR_INVALID_STATE;
            return (FT_FALSE);
        }
        cursor += name_length;
        value_length = (static_cast<ft_size_t>(cursor[0]) << 8)
            | static_cast<ft_size_t>(cursor[1]);
        cursor += 2;
        if (cursor + value_length > end)
        {
            error_code = FT_ERR_OUT_OF_RANGE;
            return (FT_FALSE);
        }
        if (!entry.set_value_from_buffer(reinterpret_cast<const char*>(cursor),
                value_length))
        {
            error_code = FT_ERR_INVALID_STATE;
            return (FT_FALSE);
        }
        cursor += value_length;
        out_headers.push_back(entry);
                index++;
    }
    error_code = FT_ERR_SUCCESS;
    return (FT_TRUE);
}

ft_bool http2_select_alpn_protocol(SSL *ssl_session, ft_bool &selected_http2,
    int32_t &error_code) noexcept
{
    const unsigned char *selected_protocol;
    uint32_t selected_length;
    unsigned char protocols[13];
    int32_t result;

    selected_http2 = FT_FALSE;
    if (!ssl_session)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        return (FT_FALSE);
    }
    protocols[0] = 2;
    protocols[1] = 'h';
    protocols[2] = '2';
    protocols[3] = 8;
    protocols[4] = 'h';
    protocols[5] = 't';
    protocols[6] = 't';
    protocols[7] = 'p';
    protocols[8] = '/';
    protocols[9] = '1';
    protocols[10] = '.';
    protocols[11] = '1';
    protocols[12] = '1';
    result = SSL_set_alpn_protos(ssl_session, protocols, sizeof(protocols));
    if (result != 0)
    {
        error_code = FT_ERR_IO;
        return (FT_FALSE);
    }
    selected_protocol = ft_nullptr;
    selected_length = 0;
    SSL_get0_alpn_selected(ssl_session, &selected_protocol, &selected_length);
    if (selected_protocol && selected_length == 2)
    {
        if (selected_protocol[0] == 'h' && selected_protocol[1] == '2')
            selected_http2 = FT_TRUE;
    }
    error_code = FT_ERR_SUCCESS;
    return (FT_TRUE);
}

#endif
