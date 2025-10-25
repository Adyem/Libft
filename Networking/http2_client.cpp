#include "http2_client.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"
#include <openssl/ssl.h>
#include <cstdlib>

static void http2_append_raw_byte(ft_string &target, unsigned char value)
{
    target.append(static_cast<char>(value));
    return ;
}

http2_header_field::http2_header_field() noexcept
    : _name(), _value(), _error_code(ER_SUCCESS),
      _thread_safe_enabled(false), _mutex(ft_nullptr)
{
    return ;
}

http2_header_field::~http2_header_field() noexcept
{
    this->clear();
    this->teardown_thread_safety();
    return ;
}

http2_header_field::http2_header_field(const http2_header_field &other) noexcept
    : _name(), _value(), _error_code(ER_SUCCESS),
      _thread_safe_enabled(false), _mutex(ft_nullptr)
{
    bool lock_acquired;
    bool success_state;
    ft_string other_name;
    ft_string other_value;

    success_state = true;
    lock_acquired = false;
    if (other.lock(&lock_acquired) != 0)
        success_state = false;
    if (success_state)
    {
        other_name = other._name;
        if (other_name.get_error() != ER_SUCCESS)
            success_state = false;
    }
    if (success_state)
    {
        other_value = other._value;
        if (other_value.get_error() != ER_SUCCESS)
            success_state = false;
    }
    if (lock_acquired)
        const_cast<http2_header_field &>(other).unlock(lock_acquired);
    if (success_state)
    {
        this->_name = other_name;
        this->_value = other_value;
        if (this->_name.get_error() != ER_SUCCESS)
            success_state = false;
        if (success_state && this->_value.get_error() != ER_SUCCESS)
            success_state = false;
    }
    if (success_state)
        this->set_error(ER_SUCCESS);
    else
        this->set_error(FT_ERR_NO_MEMORY);
    return ;
}

http2_header_field::http2_header_field(http2_header_field &&other) noexcept
    : _name(), _value(), _error_code(ER_SUCCESS),
      _thread_safe_enabled(false), _mutex(ft_nullptr)
{
    bool lock_acquired;
    bool success_state;

    success_state = true;
    lock_acquired = false;
    if (other.lock(&lock_acquired) != 0)
        success_state = false;
    if (success_state)
    {
        this->_name = other._name;
        this->_value = other._value;
        if (this->_name.get_error() != ER_SUCCESS)
            success_state = false;
        if (success_state && this->_value.get_error() != ER_SUCCESS)
            success_state = false;
        if (success_state)
        {
            other._name.clear();
            other._value.clear();
            if (other._name.get_error() != ER_SUCCESS)
                success_state = false;
            if (success_state && other._value.get_error() != ER_SUCCESS)
                success_state = false;
        }
    }
    if (lock_acquired)
        other.unlock(lock_acquired);
    if (success_state)
        this->set_error(ER_SUCCESS);
    else
        this->set_error(FT_ERR_NO_MEMORY);
    return ;
}

http2_header_field &http2_header_field::operator=(const http2_header_field &other) noexcept
{
    if (this != &other)
    {
        bool lock_acquired;
        bool success_state;
        ft_string other_name;
        ft_string other_value;

        success_state = true;
        lock_acquired = false;
        if (other.lock(&lock_acquired) != 0)
            success_state = false;
        if (success_state)
        {
            other_name = other._name;
            if (other_name.get_error() != ER_SUCCESS)
                success_state = false;
        }
        if (success_state)
        {
            other_value = other._value;
            if (other_value.get_error() != ER_SUCCESS)
                success_state = false;
        }
        if (lock_acquired)
            const_cast<http2_header_field &>(other).unlock(lock_acquired);
        if (success_state)
        {
            this->_name = other_name;
            this->_value = other_value;
            if (this->_name.get_error() != ER_SUCCESS)
                success_state = false;
            if (success_state && this->_value.get_error() != ER_SUCCESS)
                success_state = false;
        }
        if (success_state)
            this->set_error(ER_SUCCESS);
        else
            this->set_error(FT_ERR_NO_MEMORY);
    }
    return (*this);
}

http2_header_field &http2_header_field::operator=(http2_header_field &&other) noexcept
{
    if (this != &other)
    {
        bool lock_acquired;
        bool success_state;

        success_state = true;
        lock_acquired = false;
        if (other.lock(&lock_acquired) != 0)
            success_state = false;
        if (success_state)
        {
            this->_name = other._name;
            this->_value = other._value;
            if (this->_name.get_error() != ER_SUCCESS)
                success_state = false;
            if (success_state && this->_value.get_error() != ER_SUCCESS)
                success_state = false;
            if (success_state)
            {
                other._name.clear();
                other._value.clear();
                if (other._name.get_error() != ER_SUCCESS)
                    success_state = false;
                if (success_state && other._value.get_error() != ER_SUCCESS)
                    success_state = false;
            }
        }
        if (lock_acquired)
            other.unlock(lock_acquired);
        if (success_state)
            this->set_error(ER_SUCCESS);
        else
            this->set_error(FT_ERR_NO_MEMORY);
    }
    return (*this);
}

int http2_header_field::enable_thread_safety() noexcept
{
    void *memory_pointer;
    pt_mutex *mutex_pointer;

    if (this->_thread_safe_enabled && this->_mutex != ft_nullptr)
    {
        this->set_error(ER_SUCCESS);
        return (0);
    }
    memory_pointer = std::malloc(sizeof(pt_mutex));
    if (!memory_pointer)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return (-1);
    }
    mutex_pointer = new(memory_pointer) pt_mutex();
    if (mutex_pointer->get_error() != ER_SUCCESS)
    {
        int mutex_error;

        mutex_error = mutex_pointer->get_error();
        mutex_pointer->~pt_mutex();
        std::free(memory_pointer);
        this->set_error(mutex_error);
        return (-1);
    }
    this->_mutex = mutex_pointer;
    this->_thread_safe_enabled = true;
    this->set_error(ER_SUCCESS);
    return (0);
}

void http2_header_field::teardown_thread_safety() noexcept
{
    if (this->_mutex != ft_nullptr)
    {
        this->_mutex->~pt_mutex();
        std::free(this->_mutex);
        this->_mutex = ft_nullptr;
    }
    this->_thread_safe_enabled = false;
    this->set_error(ER_SUCCESS);
    return ;
}

void http2_header_field::disable_thread_safety() noexcept
{
    this->teardown_thread_safety();
    return ;
}

bool http2_header_field::is_thread_safe() const noexcept
{
    if (this->_thread_safe_enabled && this->_mutex != ft_nullptr)
        return (true);
    return (false);
}

int http2_header_field::lock(bool *lock_acquired) const noexcept
{
    http2_header_field *mutable_field;

    if (lock_acquired)
        *lock_acquired = false;
    if (!this->_thread_safe_enabled || this->_mutex == ft_nullptr)
    {
        ft_errno = ER_SUCCESS;
        return (0);
    }
    mutable_field = const_cast<http2_header_field *>(this);
    mutable_field->_mutex->lock(THREAD_ID);
    if (mutable_field->_mutex->get_error() != ER_SUCCESS)
    {
        mutable_field->set_error(mutable_field->_mutex->get_error());
        return (-1);
    }
    if (lock_acquired)
        *lock_acquired = true;
    mutable_field->set_error(ER_SUCCESS);
    return (0);
}

void http2_header_field::unlock(bool lock_acquired) const noexcept
{
    http2_header_field *mutable_field;
    int entry_errno;

    if (!lock_acquired)
        return ;
    mutable_field = const_cast<http2_header_field *>(this);
    if (mutable_field->_mutex == ft_nullptr)
        return ;
    entry_errno = ft_errno;
    mutable_field->_mutex->unlock(THREAD_ID);
    if (mutable_field->_mutex->get_error() != ER_SUCCESS)
    {
        mutable_field->set_error(mutable_field->_mutex->get_error());
        return ;
    }
    ft_errno = entry_errno;
    mutable_field->set_error(ER_SUCCESS);
    return ;
}

bool http2_header_field::set_name(const ft_string &name_value) noexcept
{
    bool lock_acquired;
    bool success_state;

    success_state = true;
    lock_acquired = false;
    if (this->lock(&lock_acquired) != 0)
        return (false);
    this->_name = name_value;
    if (this->_name.get_error() != ER_SUCCESS)
        success_state = false;
    if (success_state)
        this->set_error(ER_SUCCESS);
    else
        this->set_error(this->_name.get_error());
    this->unlock(lock_acquired);
    return (success_state);
}

bool http2_header_field::set_name_from_cstr(const char *name_value) noexcept
{
    if (name_value == ft_nullptr)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (false);
    }
    ft_string temporary_name;

    temporary_name = name_value;
    if (temporary_name.get_error() != ER_SUCCESS)
    {
        this->set_error(temporary_name.get_error());
        return (false);
    }
    return (this->set_name(temporary_name));
}

bool http2_header_field::set_name_from_buffer(const char *buffer, size_t length) noexcept
{
    bool lock_acquired;
    bool success_state;

    if (!buffer && length > 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (false);
    }
    success_state = true;
    lock_acquired = false;
    if (this->lock(&lock_acquired) != 0)
        return (false);
    this->_name.assign(buffer, length);
    if (this->_name.get_error() != ER_SUCCESS)
        success_state = false;
    if (success_state)
        this->set_error(ER_SUCCESS);
    else
        this->set_error(this->_name.get_error());
    this->unlock(lock_acquired);
    return (success_state);
}

bool http2_header_field::set_value(const ft_string &value_value) noexcept
{
    bool lock_acquired;
    bool success_state;

    success_state = true;
    lock_acquired = false;
    if (this->lock(&lock_acquired) != 0)
        return (false);
    this->_value = value_value;
    if (this->_value.get_error() != ER_SUCCESS)
        success_state = false;
    if (success_state)
        this->set_error(ER_SUCCESS);
    else
        this->set_error(this->_value.get_error());
    this->unlock(lock_acquired);
    return (success_state);
}

bool http2_header_field::set_value_from_cstr(const char *value_value) noexcept
{
    if (value_value == ft_nullptr)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (false);
    }
    ft_string temporary_value;

    temporary_value = value_value;
    if (temporary_value.get_error() != ER_SUCCESS)
    {
        this->set_error(temporary_value.get_error());
        return (false);
    }
    return (this->set_value(temporary_value));
}

bool http2_header_field::set_value_from_buffer(const char *buffer, size_t length) noexcept
{
    bool lock_acquired;
    bool success_state;

    if (!buffer && length > 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (false);
    }
    success_state = true;
    lock_acquired = false;
    if (this->lock(&lock_acquired) != 0)
        return (false);
    this->_value.assign(buffer, length);
    if (this->_value.get_error() != ER_SUCCESS)
        success_state = false;
    if (success_state)
        this->set_error(ER_SUCCESS);
    else
        this->set_error(this->_value.get_error());
    this->unlock(lock_acquired);
    return (success_state);
}

bool http2_header_field::assign(const ft_string &name_value, const ft_string &value_value) noexcept
{
    bool lock_acquired;
    bool success_state;

    success_state = true;
    lock_acquired = false;
    if (this->lock(&lock_acquired) != 0)
        return (false);
    this->_name = name_value;
    if (this->_name.get_error() != ER_SUCCESS)
        success_state = false;
    if (success_state)
    {
        this->_value = value_value;
        if (this->_value.get_error() != ER_SUCCESS)
            success_state = false;
    }
    if (success_state)
        this->set_error(ER_SUCCESS);
    else if (this->_name.get_error() != ER_SUCCESS)
        this->set_error(this->_name.get_error());
    else
        this->set_error(this->_value.get_error());
    this->unlock(lock_acquired);
    return (success_state);
}

bool http2_header_field::assign_from_cstr(const char *name_value, const char *value_value) noexcept
{
    ft_string name_string;
    ft_string value_string;

    if (name_value == ft_nullptr || value_value == ft_nullptr)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (false);
    }
    name_string = name_value;
    if (name_string.get_error() != ER_SUCCESS)
    {
        this->set_error(name_string.get_error());
        return (false);
    }
    value_string = value_value;
    if (value_string.get_error() != ER_SUCCESS)
    {
        this->set_error(value_string.get_error());
        return (false);
    }
    return (this->assign(name_string, value_string));
}

bool http2_header_field::assign_from_buffers(const char *name_buffer, size_t name_length,
    const char *value_buffer, size_t value_length) noexcept
{
    bool lock_acquired;
    bool success_state;

    if ((!name_buffer && name_length > 0) || (!value_buffer && value_length > 0))
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (false);
    }
    success_state = true;
    lock_acquired = false;
    if (this->lock(&lock_acquired) != 0)
        return (false);
    this->_name.assign(name_buffer, name_length);
    if (this->_name.get_error() != ER_SUCCESS)
        success_state = false;
    if (success_state)
    {
        this->_value.assign(value_buffer, value_length);
        if (this->_value.get_error() != ER_SUCCESS)
            success_state = false;
    }
    if (success_state)
        this->set_error(ER_SUCCESS);
    else if (this->_name.get_error() != ER_SUCCESS)
        this->set_error(this->_name.get_error());
    else
        this->set_error(this->_value.get_error());
    this->unlock(lock_acquired);
    return (success_state);
}

bool http2_header_field::copy_name(ft_string &out_name) const noexcept
{
    bool lock_acquired;
    bool success_state;

    success_state = true;
    lock_acquired = false;
    if (this->lock(&lock_acquired) != 0)
        return (false);
    out_name = this->_name;
    if (out_name.get_error() != ER_SUCCESS)
        success_state = false;
    if (success_state)
        const_cast<http2_header_field *>(this)->set_error(ER_SUCCESS);
    else
        const_cast<http2_header_field *>(this)->set_error(out_name.get_error());
    this->unlock(lock_acquired);
    return (success_state);
}

bool http2_header_field::copy_value(ft_string &out_value) const noexcept
{
    bool lock_acquired;
    bool success_state;

    success_state = true;
    lock_acquired = false;
    if (this->lock(&lock_acquired) != 0)
        return (false);
    out_value = this->_value;
    if (out_value.get_error() != ER_SUCCESS)
        success_state = false;
    if (success_state)
        const_cast<http2_header_field *>(this)->set_error(ER_SUCCESS);
    else
        const_cast<http2_header_field *>(this)->set_error(out_value.get_error());
    this->unlock(lock_acquired);
    return (success_state);
}

void http2_header_field::clear() noexcept
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock(&lock_acquired) != 0)
        return ;
    this->_name.clear();
    this->_value.clear();
    if (this->_name.get_error() != ER_SUCCESS)
        this->set_error(this->_name.get_error());
    else if (this->_value.get_error() != ER_SUCCESS)
        this->set_error(this->_value.get_error());
    else
        this->set_error(ER_SUCCESS);
    this->unlock(lock_acquired);
    return ;
}

int http2_header_field::get_error() const noexcept
{
    return (this->_error_code);
}

const char *http2_header_field::get_error_str() const noexcept
{
    return (ft_strerror(this->_error_code));
}

http2_frame::http2_frame() noexcept
    : _type(0), _flags(0), _stream_identifier(0), _payload(),
      _error_code(ER_SUCCESS), _thread_safe_enabled(false), _mutex(ft_nullptr)
{
    return ;
}

http2_frame::~http2_frame() noexcept
{
    this->clear_payload();
    this->teardown_thread_safety();
    return ;
}

http2_frame::http2_frame(const http2_frame &other) noexcept
    : _type(0), _flags(0), _stream_identifier(0), _payload(),
      _error_code(ER_SUCCESS), _thread_safe_enabled(false), _mutex(ft_nullptr)
{
    bool lock_acquired;
    bool success_state;

    success_state = true;
    lock_acquired = false;
    if (other.lock(&lock_acquired) != 0)
        success_state = false;
    if (success_state)
    {
        this->_type = other._type;
        this->_flags = other._flags;
        this->_stream_identifier = other._stream_identifier;
        this->_payload = other._payload;
        if (this->_payload.get_error() != ER_SUCCESS)
            success_state = false;
    }
    if (lock_acquired)
        const_cast<http2_frame &>(other).unlock(lock_acquired);
    if (success_state)
        this->set_error(ER_SUCCESS);
    else if (this->_payload.get_error() != ER_SUCCESS)
        this->set_error(this->_payload.get_error());
    else
        this->set_error(FT_ERR_NO_MEMORY);
    return ;
}

http2_frame::http2_frame(http2_frame &&other) noexcept
    : _type(0), _flags(0), _stream_identifier(0), _payload(),
      _error_code(ER_SUCCESS), _thread_safe_enabled(false), _mutex(ft_nullptr)
{
    bool lock_acquired;
    bool success_state;

    success_state = true;
    lock_acquired = false;
    if (other.lock(&lock_acquired) != 0)
        success_state = false;
    if (success_state)
    {
        this->_type = other._type;
        this->_flags = other._flags;
        this->_stream_identifier = other._stream_identifier;
        this->_payload = other._payload;
        if (this->_payload.get_error() != ER_SUCCESS)
            success_state = false;
        if (success_state)
        {
            other._payload.clear();
            if (other._payload.get_error() != ER_SUCCESS)
                success_state = false;
            other._type = 0;
            other._flags = 0;
            other._stream_identifier = 0;
        }
    }
    if (lock_acquired)
        other.unlock(lock_acquired);
    if (success_state)
        this->set_error(ER_SUCCESS);
    else if (this->_payload.get_error() != ER_SUCCESS)
        this->set_error(this->_payload.get_error());
    else
        this->set_error(FT_ERR_NO_MEMORY);
    return ;
}

http2_frame &http2_frame::operator=(const http2_frame &other) noexcept
{
    if (this != &other)
    {
        bool lock_acquired;
        bool success_state;

        success_state = true;
        lock_acquired = false;
        if (other.lock(&lock_acquired) != 0)
            success_state = false;
        if (success_state)
        {
            this->_type = other._type;
            this->_flags = other._flags;
            this->_stream_identifier = other._stream_identifier;
            this->_payload = other._payload;
            if (this->_payload.get_error() != ER_SUCCESS)
                success_state = false;
        }
        if (lock_acquired)
            const_cast<http2_frame &>(other).unlock(lock_acquired);
        if (success_state)
            this->set_error(ER_SUCCESS);
        else if (this->_payload.get_error() != ER_SUCCESS)
            this->set_error(this->_payload.get_error());
        else
            this->set_error(FT_ERR_NO_MEMORY);
    }
    return (*this);
}

http2_frame &http2_frame::operator=(http2_frame &&other) noexcept
{
    if (this != &other)
    {
        bool lock_acquired;
        bool success_state;

        success_state = true;
        lock_acquired = false;
        if (other.lock(&lock_acquired) != 0)
            success_state = false;
        if (success_state)
        {
            this->_type = other._type;
            this->_flags = other._flags;
            this->_stream_identifier = other._stream_identifier;
            this->_payload = other._payload;
            if (this->_payload.get_error() != ER_SUCCESS)
                success_state = false;
            if (success_state)
            {
                other._payload.clear();
                if (other._payload.get_error() != ER_SUCCESS)
                    success_state = false;
                other._type = 0;
                other._flags = 0;
                other._stream_identifier = 0;
            }
        }
        if (lock_acquired)
            other.unlock(lock_acquired);
        if (success_state)
            this->set_error(ER_SUCCESS);
        else if (this->_payload.get_error() != ER_SUCCESS)
            this->set_error(this->_payload.get_error());
        else
            this->set_error(FT_ERR_NO_MEMORY);
    }
    return (*this);
}

int http2_frame::enable_thread_safety() noexcept
{
    void *memory_pointer;
    pt_mutex *mutex_pointer;

    if (this->_thread_safe_enabled && this->_mutex != ft_nullptr)
    {
        this->set_error(ER_SUCCESS);
        return (0);
    }
    memory_pointer = std::malloc(sizeof(pt_mutex));
    if (!memory_pointer)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return (-1);
    }
    mutex_pointer = new(memory_pointer) pt_mutex();
    if (mutex_pointer->get_error() != ER_SUCCESS)
    {
        int mutex_error;

        mutex_error = mutex_pointer->get_error();
        mutex_pointer->~pt_mutex();
        std::free(memory_pointer);
        this->set_error(mutex_error);
        return (-1);
    }
    this->_mutex = mutex_pointer;
    this->_thread_safe_enabled = true;
    this->set_error(ER_SUCCESS);
    return (0);
}

void http2_frame::teardown_thread_safety() noexcept
{
    if (this->_mutex != ft_nullptr)
    {
        this->_mutex->~pt_mutex();
        std::free(this->_mutex);
        this->_mutex = ft_nullptr;
    }
    this->_thread_safe_enabled = false;
    this->set_error(ER_SUCCESS);
    return ;
}

void http2_frame::disable_thread_safety() noexcept
{
    this->teardown_thread_safety();
    return ;
}

bool http2_frame::is_thread_safe() const noexcept
{
    if (this->_thread_safe_enabled && this->_mutex != ft_nullptr)
        return (true);
    return (false);
}

int http2_frame::lock(bool *lock_acquired) const noexcept
{
    http2_frame *mutable_frame;

    if (lock_acquired)
        *lock_acquired = false;
    if (!this->_thread_safe_enabled || this->_mutex == ft_nullptr)
    {
        ft_errno = ER_SUCCESS;
        return (0);
    }
    mutable_frame = const_cast<http2_frame *>(this);
    mutable_frame->_mutex->lock(THREAD_ID);
    if (mutable_frame->_mutex->get_error() != ER_SUCCESS)
    {
        mutable_frame->set_error(mutable_frame->_mutex->get_error());
        return (-1);
    }
    if (lock_acquired)
        *lock_acquired = true;
    mutable_frame->set_error(ER_SUCCESS);
    return (0);
}

void http2_frame::unlock(bool lock_acquired) const noexcept
{
    http2_frame *mutable_frame;
    int entry_errno;

    if (!lock_acquired)
        return ;
    mutable_frame = const_cast<http2_frame *>(this);
    if (mutable_frame->_mutex == ft_nullptr)
        return ;
    entry_errno = ft_errno;
    mutable_frame->_mutex->unlock(THREAD_ID);
    if (mutable_frame->_mutex->get_error() != ER_SUCCESS)
    {
        mutable_frame->set_error(mutable_frame->_mutex->get_error());
        return ;
    }
    ft_errno = entry_errno;
    mutable_frame->set_error(ER_SUCCESS);
    return ;
}

bool http2_frame::set_type(uint8_t type_value) noexcept
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock(&lock_acquired) != 0)
        return (false);
    this->_type = type_value;
    this->set_error(ER_SUCCESS);
    this->unlock(lock_acquired);
    return (true);
}

bool http2_frame::get_type(uint8_t &out_type) const noexcept
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock(&lock_acquired) != 0)
        return (false);
    out_type = this->_type;
    const_cast<http2_frame *>(this)->set_error(ER_SUCCESS);
    this->unlock(lock_acquired);
    return (true);
}

bool http2_frame::set_flags(uint8_t flags_value) noexcept
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock(&lock_acquired) != 0)
        return (false);
    this->_flags = flags_value;
    this->set_error(ER_SUCCESS);
    this->unlock(lock_acquired);
    return (true);
}

bool http2_frame::get_flags(uint8_t &out_flags) const noexcept
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock(&lock_acquired) != 0)
        return (false);
    out_flags = this->_flags;
    const_cast<http2_frame *>(this)->set_error(ER_SUCCESS);
    this->unlock(lock_acquired);
    return (true);
}

bool http2_frame::set_stream_identifier(uint32_t stream_identifier_value) noexcept
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock(&lock_acquired) != 0)
        return (false);
    this->_stream_identifier = stream_identifier_value;
    this->set_error(ER_SUCCESS);
    this->unlock(lock_acquired);
    return (true);
}

bool http2_frame::get_stream_identifier(uint32_t &out_stream_identifier) const noexcept
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock(&lock_acquired) != 0)
        return (false);
    out_stream_identifier = this->_stream_identifier;
    const_cast<http2_frame *>(this)->set_error(ER_SUCCESS);
    this->unlock(lock_acquired);
    return (true);
}

bool http2_frame::set_payload(const ft_string &payload_value) noexcept
{
    bool lock_acquired;
    bool success_state;

    success_state = true;
    lock_acquired = false;
    if (this->lock(&lock_acquired) != 0)
        return (false);
    this->_payload = payload_value;
    if (this->_payload.get_error() != ER_SUCCESS)
        success_state = false;
    if (success_state)
        this->set_error(ER_SUCCESS);
    else
        this->set_error(this->_payload.get_error());
    this->unlock(lock_acquired);
    return (success_state);
}

bool http2_frame::set_payload_from_buffer(const char *buffer, size_t length) noexcept
{
    bool lock_acquired;
    bool success_state;

    if (!buffer && length > 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (false);
    }
    success_state = true;
    lock_acquired = false;
    if (this->lock(&lock_acquired) != 0)
        return (false);
    this->_payload.assign(buffer, length);
    if (this->_payload.get_error() != ER_SUCCESS)
        success_state = false;
    if (success_state)
        this->set_error(ER_SUCCESS);
    else
        this->set_error(this->_payload.get_error());
    this->unlock(lock_acquired);
    return (success_state);
}

bool http2_frame::copy_payload(ft_string &out_payload) const noexcept
{
    bool lock_acquired;
    bool success_state;

    success_state = true;
    lock_acquired = false;
    if (this->lock(&lock_acquired) != 0)
        return (false);
    out_payload = this->_payload;
    if (out_payload.get_error() != ER_SUCCESS)
        success_state = false;
    if (success_state)
        const_cast<http2_frame *>(this)->set_error(ER_SUCCESS);
    else
        const_cast<http2_frame *>(this)->set_error(out_payload.get_error());
    this->unlock(lock_acquired);
    return (success_state);
}

void http2_frame::clear_payload() noexcept
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock(&lock_acquired) != 0)
        return ;
    this->_payload.clear();
    if (this->_payload.get_error() != ER_SUCCESS)
        this->set_error(this->_payload.get_error());
    else
        this->set_error(ER_SUCCESS);
    this->unlock(lock_acquired);
    return ;
}

int http2_frame::get_error() const noexcept
{
    return (this->_error_code);
}

const char *http2_frame::get_error_str() const noexcept
{
    return (ft_strerror(this->_error_code));
}

void http2_header_field::set_error(int error_code) const noexcept
{
    http2_header_field *mutable_field;

    mutable_field = const_cast<http2_header_field *>(this);
    mutable_field->_error_code = error_code;
    ft_errno = error_code;
    return ;
}

void http2_frame::set_error(int error_code) const noexcept
{
    http2_frame *mutable_frame;

    mutable_frame = const_cast<http2_frame *>(this);
    mutable_frame->_error_code = error_code;
    ft_errno = error_code;
    return ;
}

http2_stream_state::http2_stream_state() noexcept
    : buffer(), dependency_identifier(0), weight(16), exclusive_dependency(false),
      remote_window(65535), local_window(65535)
{
    return ;
}

http2_stream_state::~http2_stream_state() noexcept
{
    this->buffer.clear();
    return ;
}

http2_stream_manager::http2_stream_manager() noexcept
    : _streams(), _stream_identifiers(), _initial_remote_window(65535),
      _initial_local_window(65535), _connection_remote_window(65535),
      _connection_local_window(65535), _error_code(ER_SUCCESS)
{
    return ;
}

http2_stream_manager::~http2_stream_manager() noexcept
{
    this->_streams.clear();
    this->_stream_identifiers.clear();
    this->_initial_remote_window = 65535;
    this->_initial_local_window = 65535;
    this->_connection_remote_window = 65535;
    this->_connection_local_window = 65535;
    this->set_error(ER_SUCCESS);
    return ;
}

void http2_stream_manager::set_error(int error_code) const noexcept
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}

bool http2_stream_manager::validate_receive_window(uint32_t stream_identifier,
    uint32_t length) noexcept
{
    Pair<uint32_t, http2_stream_state> *stream_entry;

    stream_entry = this->_streams.find(stream_identifier);
    if (stream_entry == ft_nullptr)
    {
        this->set_error(FT_ERR_NOT_FOUND);
        return (false);
    }
    if (stream_entry->value.local_window < length)
    {
        this->set_error(FT_ERR_OUT_OF_RANGE);
        return (false);
    }
    if (this->_connection_local_window < length)
    {
        this->set_error(FT_ERR_OUT_OF_RANGE);
        return (false);
    }
    this->set_error(ER_SUCCESS);
    return (true);
}

bool http2_stream_manager::record_received_data(uint32_t stream_identifier,
    uint32_t length) noexcept
{
    Pair<uint32_t, http2_stream_state> *stream_entry;

    stream_entry = this->_streams.find(stream_identifier);
    if (stream_entry == ft_nullptr)
    {
        this->set_error(FT_ERR_NOT_FOUND);
        return (false);
    }
    if (stream_entry->value.local_window < length)
    {
        this->set_error(FT_ERR_OUT_OF_RANGE);
        return (false);
    }
    if (this->_connection_local_window < length)
    {
        this->set_error(FT_ERR_OUT_OF_RANGE);
        return (false);
    }
    stream_entry->value.local_window -= length;
    this->_connection_local_window -= length;
    this->set_error(ER_SUCCESS);
    return (true);
}

bool http2_stream_manager::reserve_remote_connection_window(uint32_t length) noexcept
{
    if (this->_connection_remote_window < length)
    {
        this->set_error(FT_ERR_OUT_OF_RANGE);
        return (false);
    }
    this->set_error(ER_SUCCESS);
    return (true);
}

bool http2_stream_manager::record_connection_send(uint32_t length) noexcept
{
    if (this->_connection_remote_window < length)
    {
        this->set_error(FT_ERR_OUT_OF_RANGE);
        return (false);
    }
    this->_connection_remote_window -= length;
    this->set_error(ER_SUCCESS);
    return (true);
}

void http2_stream_manager::remove_stream_identifier(uint32_t stream_identifier) noexcept
{
    size_t identifier_count;
    ft_vector<uint32_t>::iterator iterator_value;
    size_t index;

    identifier_count = this->_stream_identifiers.size();
    if (this->_stream_identifiers.get_error() != ER_SUCCESS)
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

bool http2_stream_manager::open_stream(uint32_t stream_identifier) noexcept
{
    Pair<uint32_t, http2_stream_state> *existing_entry;
    http2_stream_state new_state;

    existing_entry = this->_streams.find(stream_identifier);
    if (existing_entry != ft_nullptr)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (false);
    }
    new_state.remote_window = this->_initial_remote_window;
    new_state.local_window = this->_initial_local_window;
    this->_streams.insert(stream_identifier, new_state);
    if (this->_streams.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_streams.get_error());
        return (false);
    }
    this->_stream_identifiers.push_back(stream_identifier);
    if (this->_stream_identifiers.get_error() != ER_SUCCESS)
    {
        this->_streams.remove(stream_identifier);
        this->set_error(this->_stream_identifiers.get_error());
        return (false);
    }
    this->set_error(ER_SUCCESS);
    return (true);
}

bool http2_stream_manager::append_data(uint32_t stream_identifier, const char *data,
    size_t length) noexcept
{
    Pair<uint32_t, http2_stream_state> *stream_entry;
    size_t index;
    uint32_t length_32;

    if (!data && length > 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (false);
    }
    stream_entry = this->_streams.find(stream_identifier);
    if (stream_entry == ft_nullptr)
    {
        this->set_error(FT_ERR_NOT_FOUND);
        return (false);
    }
    if (length > 0xFFFFFFFFu)
    {
        this->set_error(FT_ERR_OUT_OF_RANGE);
        return (false);
    }
    length_32 = static_cast<uint32_t>(length);
    if (!this->validate_receive_window(stream_identifier, length_32))
        return (false);
    index = 0;
    while (index < length)
    {
        stream_entry->value.buffer.append(data[index]);
        if (stream_entry->value.buffer.get_error() != ER_SUCCESS)
        {
            this->set_error(stream_entry->value.buffer.get_error());
            return (false);
        }
        index++;
    }
    if (!this->record_received_data(stream_identifier, length_32))
        return (false);
    this->set_error(ER_SUCCESS);
    return (true);
}

bool http2_stream_manager::close_stream(uint32_t stream_identifier) noexcept
{
    Pair<uint32_t, http2_stream_state> *stream_entry;

    stream_entry = this->_streams.find(stream_identifier);
    if (stream_entry == ft_nullptr)
    {
        this->set_error(FT_ERR_NOT_FOUND);
        return (false);
    }
    this->_streams.remove(stream_identifier);
    if (this->_streams.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_streams.get_error());
        return (false);
    }
    this->remove_stream_identifier(stream_identifier);
    this->set_error(ER_SUCCESS);
    return (true);
}

bool http2_stream_manager::get_stream_buffer(uint32_t stream_identifier,
    ft_string &out_buffer) const noexcept
{
    const Pair<uint32_t, http2_stream_state> *stream_entry;

    stream_entry = this->_streams.find(stream_identifier);
    if (stream_entry == ft_nullptr)
    {
        this->set_error(FT_ERR_NOT_FOUND);
        return (false);
    }
    out_buffer = stream_entry->value.buffer;
    if (out_buffer.get_error() != ER_SUCCESS)
    {
        this->set_error(out_buffer.get_error());
        return (false);
    }
    this->set_error(ER_SUCCESS);
    return (true);
}

bool http2_stream_manager::update_priority(uint32_t stream_identifier,
    uint32_t dependency_identifier, uint8_t weight, bool exclusive) noexcept
{
    Pair<uint32_t, http2_stream_state> *stream_entry;
    size_t identifier_count;
    ft_vector<uint32_t>::iterator iterator_value;
    size_t index;

    if (stream_identifier == dependency_identifier)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (false);
    }
    if (weight == 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (false);
    }
    stream_entry = this->_streams.find(stream_identifier);
    if (stream_entry == ft_nullptr)
    {
        this->set_error(FT_ERR_NOT_FOUND);
        return (false);
    }
    if (exclusive)
    {
        identifier_count = this->_stream_identifiers.size();
        if (this->_stream_identifiers.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_stream_identifiers.get_error());
            return (false);
        }
        iterator_value = this->_stream_identifiers.begin();
        index = 0;
        while (index < identifier_count)
        {
            uint32_t child_identifier;
            Pair<uint32_t, http2_stream_state> *child_entry;

            child_identifier = *iterator_value;
            child_entry = this->_streams.find(child_identifier);
            if (child_entry == ft_nullptr)
            {
                this->set_error(this->_streams.get_error());
                return (false);
            }
            if (child_identifier != stream_identifier
                && child_entry->value.dependency_identifier == dependency_identifier)
                child_entry->value.dependency_identifier = stream_identifier;
            ++iterator_value;
            ++index;
        }
    }
    stream_entry->value.dependency_identifier = dependency_identifier;
    stream_entry->value.weight = weight;
    stream_entry->value.exclusive_dependency = exclusive;
    this->set_error(ER_SUCCESS);
    return (true);
}

bool http2_stream_manager::get_priority(uint32_t stream_identifier,
    uint32_t &dependency_identifier, uint8_t &weight, bool &exclusive) const noexcept
{
    const Pair<uint32_t, http2_stream_state> *stream_entry;

    stream_entry = this->_streams.find(stream_identifier);
    if (stream_entry == ft_nullptr)
    {
        this->set_error(FT_ERR_NOT_FOUND);
        return (false);
    }
    dependency_identifier = stream_entry->value.dependency_identifier;
    weight = stream_entry->value.weight;
    exclusive = stream_entry->value.exclusive_dependency;
    this->set_error(ER_SUCCESS);
    return (true);
}

bool http2_stream_manager::update_remote_initial_window(uint32_t new_window) noexcept
{
    uint32_t previous_window;
    size_t identifier_count;
    size_t index;

    if (new_window > 0x7FFFFFFFu)
    {
        this->set_error(FT_ERR_OUT_OF_RANGE);
        return (false);
    }
    previous_window = this->_initial_remote_window;
    this->_initial_remote_window = new_window;
    identifier_count = this->_stream_identifiers.size();
    if (this->_stream_identifiers.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_stream_identifiers.get_error());
        return (false);
    }
    index = 0;
    while (index < identifier_count)
    {
        uint32_t identifier_value;
        Pair<uint32_t, http2_stream_state> *stream_entry;

        identifier_value = this->_stream_identifiers[index];
        stream_entry = this->_streams.find(identifier_value);
        if (stream_entry == ft_nullptr)
        {
            this->set_error(this->_streams.get_error());
            return (false);
        }
        if (new_window >= previous_window)
        {
            uint32_t delta;
            unsigned long long updated_window;

            delta = new_window - previous_window;
            updated_window = static_cast<unsigned long long>(stream_entry->value.remote_window)
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
    this->set_error(ER_SUCCESS);
    return (true);
}

bool http2_stream_manager::update_local_initial_window(uint32_t new_window) noexcept
{
    uint32_t previous_window;
    size_t identifier_count;
    size_t index;

    if (new_window > 0x7FFFFFFFu)
    {
        this->set_error(FT_ERR_OUT_OF_RANGE);
        return (false);
    }
    previous_window = this->_initial_local_window;
    this->_initial_local_window = new_window;
    identifier_count = this->_stream_identifiers.size();
    if (this->_stream_identifiers.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_stream_identifiers.get_error());
        return (false);
    }
    index = 0;
    while (index < identifier_count)
    {
        uint32_t identifier_value;
        Pair<uint32_t, http2_stream_state> *stream_entry;

        identifier_value = this->_stream_identifiers[index];
        stream_entry = this->_streams.find(identifier_value);
        if (stream_entry == ft_nullptr)
        {
            this->set_error(this->_streams.get_error());
            return (false);
        }
        if (new_window >= previous_window)
        {
            uint32_t delta;
            unsigned long long updated_window;

            delta = new_window - previous_window;
            updated_window = static_cast<unsigned long long>(stream_entry->value.local_window)
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
    this->set_error(ER_SUCCESS);
    return (true);
}

uint32_t http2_stream_manager::get_local_window(uint32_t stream_identifier) const noexcept
{
    const Pair<uint32_t, http2_stream_state> *stream_entry;

    stream_entry = this->_streams.find(stream_identifier);
    if (stream_entry == ft_nullptr)
    {
        this->set_error(FT_ERR_NOT_FOUND);
        return (0);
    }
    this->set_error(ER_SUCCESS);
    return (stream_entry->value.local_window);
}

uint32_t http2_stream_manager::get_remote_window(uint32_t stream_identifier) const noexcept
{
    const Pair<uint32_t, http2_stream_state> *stream_entry;

    stream_entry = this->_streams.find(stream_identifier);
    if (stream_entry == ft_nullptr)
    {
        this->set_error(FT_ERR_NOT_FOUND);
        return (0);
    }
    this->set_error(ER_SUCCESS);
    return (stream_entry->value.remote_window);
}

bool http2_stream_manager::increase_local_window(uint32_t stream_identifier,
    uint32_t increment) noexcept
{
    Pair<uint32_t, http2_stream_state> *stream_entry;
    unsigned long long updated_window;

    if (increment > 0x7FFFFFFFu)
    {
        this->set_error(FT_ERR_OUT_OF_RANGE);
        return (false);
    }
    stream_entry = this->_streams.find(stream_identifier);
    if (stream_entry == ft_nullptr)
    {
        this->set_error(FT_ERR_NOT_FOUND);
        return (false);
    }
    updated_window = static_cast<unsigned long long>(stream_entry->value.local_window)
        + increment;
    if (updated_window > 0x7FFFFFFFul)
    {
        this->set_error(FT_ERR_OUT_OF_RANGE);
        return (false);
    }
    stream_entry->value.local_window = static_cast<uint32_t>(updated_window);
    this->set_error(ER_SUCCESS);
    return (true);
}

bool http2_stream_manager::increase_remote_window(uint32_t stream_identifier,
    uint32_t increment) noexcept
{
    Pair<uint32_t, http2_stream_state> *stream_entry;
    unsigned long long updated_window;

    if (increment > 0x7FFFFFFFu)
    {
        this->set_error(FT_ERR_OUT_OF_RANGE);
        return (false);
    }
    stream_entry = this->_streams.find(stream_identifier);
    if (stream_entry == ft_nullptr)
    {
        this->set_error(FT_ERR_NOT_FOUND);
        return (false);
    }
    updated_window = static_cast<unsigned long long>(stream_entry->value.remote_window)
        + increment;
    if (updated_window > 0x7FFFFFFFul)
    {
        this->set_error(FT_ERR_OUT_OF_RANGE);
        return (false);
    }
    stream_entry->value.remote_window = static_cast<uint32_t>(updated_window);
    this->set_error(ER_SUCCESS);
    return (true);
}

bool http2_stream_manager::reserve_send_window(uint32_t stream_identifier,
    uint32_t length) noexcept
{
    Pair<uint32_t, http2_stream_state> *stream_entry;

    stream_entry = this->_streams.find(stream_identifier);
    if (stream_entry == ft_nullptr)
    {
        this->set_error(FT_ERR_NOT_FOUND);
        return (false);
    }
    if (!this->reserve_remote_connection_window(length))
        return (false);
    if (stream_entry->value.remote_window < length)
    {
        this->set_error(FT_ERR_OUT_OF_RANGE);
        return (false);
    }
    stream_entry->value.remote_window -= length;
    if (!this->record_connection_send(length))
        return (false);
    this->set_error(ER_SUCCESS);
    return (true);
}

bool http2_stream_manager::update_connection_local_window(uint32_t increment) noexcept
{
    unsigned long long updated_window;

    if (increment > 0x7FFFFFFFu)
    {
        this->set_error(FT_ERR_OUT_OF_RANGE);
        return (false);
    }
    updated_window = static_cast<unsigned long long>(this->_connection_local_window)
        + increment;
    if (updated_window > 0x7FFFFFFFul)
    {
        this->set_error(FT_ERR_OUT_OF_RANGE);
        return (false);
    }
    this->_connection_local_window = static_cast<uint32_t>(updated_window);
    this->set_error(ER_SUCCESS);
    return (true);
}

bool http2_stream_manager::update_connection_remote_window(uint32_t increment) noexcept
{
    unsigned long long updated_window;

    if (increment > 0x7FFFFFFFu)
    {
        this->set_error(FT_ERR_OUT_OF_RANGE);
        return (false);
    }
    updated_window = static_cast<unsigned long long>(this->_connection_remote_window)
        + increment;
    if (updated_window > 0x7FFFFFFFul)
    {
        this->set_error(FT_ERR_OUT_OF_RANGE);
        return (false);
    }
    this->_connection_remote_window = static_cast<uint32_t>(updated_window);
    this->set_error(ER_SUCCESS);
    return (true);
}

uint32_t http2_stream_manager::get_connection_local_window() const noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_connection_local_window);
}

uint32_t http2_stream_manager::get_connection_remote_window() const noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_connection_remote_window);
}

int http2_stream_manager::get_error() const noexcept
{
    return (this->_error_code);
}

const char *http2_stream_manager::get_error_str() const noexcept
{
    return (ft_strerror(this->_error_code));
}

void http2_settings_state::set_error(int error_code) const noexcept
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}

bool http2_settings_state::apply_single_setting(uint16_t identifier, uint32_t value,
    http2_stream_manager &streams) noexcept
{
    if (identifier == 0x1)
    {
        this->_header_table_size = value;
        this->set_error(ER_SUCCESS);
        return (true);
    }
    if (identifier == 0x2)
    {
        if (value > 1)
        {
            this->set_error(FT_ERR_INVALID_ARGUMENT);
            return (false);
        }
        this->_enable_push = (value != 0);
        this->set_error(ER_SUCCESS);
        return (true);
    }
    if (identifier == 0x3)
    {
        this->_max_concurrent_streams = value;
        this->set_error(ER_SUCCESS);
        return (true);
    }
    if (identifier == 0x4)
    {
        if (value > 0x7FFFFFFFu)
        {
            this->set_error(FT_ERR_OUT_OF_RANGE);
            return (false);
        }
        if (!streams.update_remote_initial_window(value))
        {
            this->set_error(streams.get_error());
            return (false);
        }
        this->_initial_remote_window = value;
        this->set_error(ER_SUCCESS);
        return (true);
    }
    if (identifier == 0x5)
    {
        if (value < 16384 || value > 16777215)
        {
            this->set_error(FT_ERR_OUT_OF_RANGE);
            return (false);
        }
        this->_max_frame_size = value;
        this->set_error(ER_SUCCESS);
        return (true);
    }
    if (identifier == 0x6)
    {
        this->_max_header_list_size = value;
        this->set_error(ER_SUCCESS);
        return (true);
    }
    this->set_error(ER_SUCCESS);
    return (true);
}

http2_settings_state::http2_settings_state() noexcept
    : _header_table_size(4096), _enable_push(true), _max_concurrent_streams(0xFFFFFFFFu),
      _initial_local_window(65535), _initial_remote_window(65535),
      _max_frame_size(16384), _max_header_list_size(0), _error_code(ER_SUCCESS)
{
    return ;
}

http2_settings_state::~http2_settings_state() noexcept
{
    this->set_error(ER_SUCCESS);
    return ;
}

bool http2_settings_state::apply_remote_settings(const http2_frame &frame,
    http2_stream_manager &streams) noexcept
{
    size_t payload_length;
    const unsigned char *payload_bytes;
    size_t offset;
    uint8_t frame_type;
    uint8_t frame_flags;
    ft_string payload_copy;

    if (!frame.get_type(frame_type))
    {
        this->set_error(frame.get_error());
        return (false);
    }
    if (frame_type != 0x4)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (false);
    }
    if (!frame.copy_payload(payload_copy))
    {
        this->set_error(frame.get_error());
        return (false);
    }
    payload_length = payload_copy.size();
    if (payload_copy.get_error() != ER_SUCCESS)
    {
        this->set_error(payload_copy.get_error());
        return (false);
    }
    if (!frame.get_flags(frame_flags))
    {
        this->set_error(frame.get_error());
        return (false);
    }
    if ((frame_flags & 0x1) != 0)
    {
        if (payload_length != 0)
        {
            this->set_error(FT_ERR_INVALID_ARGUMENT);
            return (false);
        }
        this->set_error(ER_SUCCESS);
        return (true);
    }
    if ((payload_length % 6) != 0)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return (false);
    }
    payload_bytes = reinterpret_cast<const unsigned char*>(payload_copy.c_str());
    if (payload_copy.get_error() != ER_SUCCESS)
    {
        this->set_error(payload_copy.get_error());
        return (false);
    }
    offset = 0;
    while (offset < payload_length)
    {
        uint16_t identifier;
        uint32_t value;

        identifier = static_cast<uint16_t>(static_cast<unsigned int>(payload_bytes[offset]) << 8);
        identifier |= static_cast<uint16_t>(payload_bytes[offset + 1]);
        value = (static_cast<uint32_t>(payload_bytes[offset + 2]) << 24)
            | (static_cast<uint32_t>(payload_bytes[offset + 3]) << 16)
            | (static_cast<uint32_t>(payload_bytes[offset + 4]) << 8)
            | static_cast<uint32_t>(payload_bytes[offset + 5]);
        if (!this->apply_single_setting(identifier, value, streams))
            return (false);
        offset += 6;
    }
    this->set_error(ER_SUCCESS);
    return (true);
}

bool http2_settings_state::update_local_initial_window(uint32_t new_window,
    http2_stream_manager &streams) noexcept
{
    if (new_window > 0x7FFFFFFFu)
    {
        this->set_error(FT_ERR_OUT_OF_RANGE);
        return (false);
    }
    if (!streams.update_local_initial_window(new_window))
    {
        this->set_error(streams.get_error());
        return (false);
    }
    this->_initial_local_window = new_window;
    this->set_error(ER_SUCCESS);
    return (true);
}

bool http2_settings_state::update_remote_initial_window(uint32_t new_window,
    http2_stream_manager &streams) noexcept
{
    if (new_window > 0x7FFFFFFFu)
    {
        this->set_error(FT_ERR_OUT_OF_RANGE);
        return (false);
    }
    if (!streams.update_remote_initial_window(new_window))
    {
        this->set_error(streams.get_error());
        return (false);
    }
    this->_initial_remote_window = new_window;
    this->set_error(ER_SUCCESS);
    return (true);
}

uint32_t http2_settings_state::get_header_table_size() const noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_header_table_size);
}

bool http2_settings_state::get_enable_push() const noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_enable_push);
}

uint32_t http2_settings_state::get_max_concurrent_streams() const noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_max_concurrent_streams);
}

uint32_t http2_settings_state::get_initial_local_window() const noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_initial_local_window);
}

uint32_t http2_settings_state::get_initial_remote_window() const noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_initial_remote_window);
}

uint32_t http2_settings_state::get_max_frame_size() const noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_max_frame_size);
}

uint32_t http2_settings_state::get_max_header_list_size() const noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_max_header_list_size);
}

int http2_settings_state::get_error() const noexcept
{
    return (this->_error_code);
}

const char *http2_settings_state::get_error_str() const noexcept
{
    return (ft_strerror(this->_error_code));
}

bool http2_encode_frame(const http2_frame &frame, ft_string &out_buffer,
    int &error_code) noexcept
{
    size_t payload_length;
    unsigned char header[9];
    size_t index;
    ft_string payload_copy;
    const char *payload_data;
    uint8_t frame_type;
    uint8_t frame_flags;
    uint32_t frame_stream_identifier;

    out_buffer.clear();
    if (out_buffer.get_error() != ER_SUCCESS)
    {
        error_code = out_buffer.get_error();
        return (false);
    }
    if (!frame.copy_payload(payload_copy))
    {
        error_code = frame.get_error();
        return (false);
    }
    payload_length = payload_copy.size();
    if (payload_copy.get_error() != ER_SUCCESS)
    {
        error_code = payload_copy.get_error();
        return (false);
    }
    if (payload_length > 0xFFFFFF)
    {
        error_code = FT_ERR_OUT_OF_RANGE;
        return (false);
    }
    if (!frame.get_type(frame_type))
    {
        error_code = frame.get_error();
        return (false);
    }
    if (!frame.get_flags(frame_flags))
    {
        error_code = frame.get_error();
        return (false);
    }
    if (!frame.get_stream_identifier(frame_stream_identifier))
    {
        error_code = frame.get_error();
        return (false);
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
        http2_append_raw_byte(out_buffer, header[index]);
        if (out_buffer.get_error() != ER_SUCCESS)
        {
            error_code = out_buffer.get_error();
            return (false);
        }
        index++;
    }
    payload_data = payload_copy.c_str();
    if (payload_copy.get_error() != ER_SUCCESS)
    {
        error_code = payload_copy.get_error();
        return (false);
    }
    index = 0;
    while (index < payload_length)
    {
        http2_append_raw_byte(out_buffer,
            static_cast<unsigned char>(payload_data[index]));
        if (out_buffer.get_error() != ER_SUCCESS)
        {
            error_code = out_buffer.get_error();
            return (false);
        }
        index++;
    }
    error_code = ER_SUCCESS;
    return (true);
}

bool http2_decode_frame(const unsigned char *buffer, size_t buffer_size,
    size_t &offset, http2_frame &out_frame, int &error_code) noexcept
{
    size_t remaining;
    size_t payload_length;
    size_t index;
    uint32_t stream_identifier_value;
    ft_string payload_copy;

    if (!buffer)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        return (false);
    }
    if (buffer_size < offset)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        return (false);
    }
    remaining = buffer_size - offset;
    if (remaining < 9)
    {
        error_code = FT_ERR_OUT_OF_RANGE;
        return (false);
    }
    payload_length = (static_cast<size_t>(buffer[offset]) << 16);
    payload_length |= (static_cast<size_t>(buffer[offset + 1]) << 8);
    payload_length |= static_cast<size_t>(buffer[offset + 2]);
    if (!out_frame.set_type(buffer[offset + 3]))
    {
        error_code = out_frame.get_error();
        return (false);
    }
    if (!out_frame.set_flags(buffer[offset + 4]))
    {
        error_code = out_frame.get_error();
        return (false);
    }
    stream_identifier_value = (static_cast<uint32_t>(buffer[offset + 5] & 0x7F) << 24);
    stream_identifier_value |= (static_cast<uint32_t>(buffer[offset + 6]) << 16);
    stream_identifier_value |= (static_cast<uint32_t>(buffer[offset + 7]) << 8);
    stream_identifier_value |= static_cast<uint32_t>(buffer[offset + 8]);
    if (!out_frame.set_stream_identifier(stream_identifier_value))
    {
        error_code = out_frame.get_error();
        return (false);
    }
    if (remaining < 9 + payload_length)
    {
        error_code = FT_ERR_OUT_OF_RANGE;
        return (false);
    }
    payload_copy.clear();
    if (payload_copy.get_error() != ER_SUCCESS)
    {
        error_code = payload_copy.get_error();
        return (false);
    }
    index = 0;
    while (index < payload_length)
    {
        http2_append_raw_byte(payload_copy,
            static_cast<unsigned char>(buffer[offset + 9 + index]));
        if (payload_copy.get_error() != ER_SUCCESS)
        {
            error_code = payload_copy.get_error();
            return (false);
        }
        index++;
    }
    if (!out_frame.set_payload(payload_copy))
    {
        error_code = out_frame.get_error();
        return (false);
    }
    offset += 9 + payload_length;
    error_code = ER_SUCCESS;
    return (true);
}

bool http2_compress_headers(const ft_vector<http2_header_field> &headers,
    ft_string &out_block, int &error_code) noexcept
{
    size_t header_count;
    size_t index;

    out_block.clear();
    if (out_block.get_error() != ER_SUCCESS)
    {
        error_code = out_block.get_error();
        return (false);
    }
    header_count = headers.size();
    if (headers.get_error() != ER_SUCCESS)
    {
        error_code = headers.get_error();
        return (false);
    }
    if (header_count > 0xFFFF)
    {
        error_code = FT_ERR_OUT_OF_RANGE;
        return (false);
    }
    http2_append_raw_byte(out_block,
        static_cast<unsigned char>((header_count >> 8) & 0xFF));
    if (out_block.get_error() != ER_SUCCESS)
    {
        error_code = out_block.get_error();
        return (false);
    }
    http2_append_raw_byte(out_block,
        static_cast<unsigned char>(header_count & 0xFF));
    if (out_block.get_error() != ER_SUCCESS)
    {
        error_code = out_block.get_error();
        return (false);
    }
    index = 0;
    while (index < header_count)
    {
        const http2_header_field &field = headers[index];
        ft_string field_name;
        ft_string field_value;
        size_t name_length;
        size_t value_length;
        size_t name_index;
        size_t value_index;
        const char *name_data;
        const char *value_data;

        if (!field.copy_name(field_name))
        {
            error_code = field.get_error();
            return (false);
        }
        if (!field.copy_value(field_value))
        {
            error_code = field.get_error();
            return (false);
        }
        name_length = field_name.size();
        if (field_name.get_error() != ER_SUCCESS)
        {
            error_code = field_name.get_error();
            return (false);
        }
        value_length = field_value.size();
        if (field_value.get_error() != ER_SUCCESS)
        {
            error_code = field_value.get_error();
            return (false);
        }
        if (name_length > 0xFFFF || value_length > 0xFFFF)
        {
            error_code = FT_ERR_OUT_OF_RANGE;
            return (false);
        }
        http2_append_raw_byte(out_block,
            static_cast<unsigned char>((name_length >> 8) & 0xFF));
        if (out_block.get_error() != ER_SUCCESS)
        {
            error_code = out_block.get_error();
            return (false);
        }
        http2_append_raw_byte(out_block,
            static_cast<unsigned char>(name_length & 0xFF));
        if (out_block.get_error() != ER_SUCCESS)
        {
            error_code = out_block.get_error();
            return (false);
        }
        name_data = field_name.c_str();
        if (field_name.get_error() != ER_SUCCESS)
        {
            error_code = field_name.get_error();
            return (false);
        }
        name_index = 0;
        while (name_index < name_length)
        {
            http2_append_raw_byte(out_block,
                static_cast<unsigned char>(name_data[name_index]));
            if (out_block.get_error() != ER_SUCCESS)
            {
                error_code = out_block.get_error();
                return (false);
            }
            name_index++;
        }
        http2_append_raw_byte(out_block,
            static_cast<unsigned char>((value_length >> 8) & 0xFF));
        if (out_block.get_error() != ER_SUCCESS)
        {
            error_code = out_block.get_error();
            return (false);
        }
        http2_append_raw_byte(out_block,
            static_cast<unsigned char>(value_length & 0xFF));
        if (out_block.get_error() != ER_SUCCESS)
        {
            error_code = out_block.get_error();
            return (false);
        }
        value_data = field_value.c_str();
        if (field_value.get_error() != ER_SUCCESS)
        {
            error_code = field_value.get_error();
            return (false);
        }
        value_index = 0;
        while (value_index < value_length)
        {
            http2_append_raw_byte(out_block,
                static_cast<unsigned char>(value_data[value_index]));
            if (out_block.get_error() != ER_SUCCESS)
            {
                error_code = out_block.get_error();
                return (false);
            }
            value_index++;
        }
        index++;
    }
    error_code = ER_SUCCESS;
    return (true);
}

bool http2_decompress_headers(const ft_string &block,
    ft_vector<http2_header_field> &out_headers, int &error_code) noexcept
{
    const unsigned char *buffer;
    const unsigned char *cursor;
    const unsigned char *end;
    size_t buffer_length;
    size_t header_count;
    size_t index;

    out_headers.clear();
    if (out_headers.get_error() != ER_SUCCESS)
    {
        error_code = out_headers.get_error();
        return (false);
    }
    buffer_length = block.size();
    if (block.get_error() != ER_SUCCESS)
    {
        error_code = block.get_error();
        return (false);
    }
    buffer = reinterpret_cast<const unsigned char*>(block.c_str());
    if (block.get_error() != ER_SUCCESS)
    {
        error_code = block.get_error();
        return (false);
    }
    if (buffer_length < 2)
    {
        error_code = FT_ERR_OUT_OF_RANGE;
        return (false);
    }
    header_count = (static_cast<size_t>(buffer[0]) << 8)
        | static_cast<size_t>(buffer[1]);
    cursor = buffer + 2;
    end = buffer + buffer_length;
    index = 0;
    while (index < header_count)
    {
        http2_header_field entry;
        size_t name_length;
        size_t value_length;

        if (cursor + 4 > end)
        {
            error_code = FT_ERR_OUT_OF_RANGE;
            return (false);
        }
        name_length = (static_cast<size_t>(cursor[0]) << 8)
            | static_cast<size_t>(cursor[1]);
        cursor += 2;
        if (cursor + name_length + 2 > end)
        {
            error_code = FT_ERR_OUT_OF_RANGE;
            return (false);
        }
        if (!entry.set_name_from_buffer(reinterpret_cast<const char*>(cursor),
                name_length))
        {
            error_code = entry.get_error();
            return (false);
        }
        cursor += name_length;
        value_length = (static_cast<size_t>(cursor[0]) << 8)
            | static_cast<size_t>(cursor[1]);
        cursor += 2;
        if (cursor + value_length > end)
        {
            error_code = FT_ERR_OUT_OF_RANGE;
            return (false);
        }
        if (!entry.set_value_from_buffer(reinterpret_cast<const char*>(cursor),
                value_length))
        {
            error_code = entry.get_error();
            return (false);
        }
        cursor += value_length;
        out_headers.push_back(entry);
        if (out_headers.get_error() != ER_SUCCESS)
        {
            error_code = out_headers.get_error();
            return (false);
        }
        index++;
    }
    error_code = ER_SUCCESS;
    return (true);
}

bool http2_select_alpn_protocol(SSL *ssl_session, bool &selected_http2,
    int &error_code) noexcept
{
    const unsigned char *selected_protocol;
    unsigned int selected_length;
    unsigned char protocols[13];
    int result;

    selected_http2 = false;
    if (!ssl_session)
    {
        error_code = FT_ERR_INVALID_ARGUMENT;
        return (false);
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
        return (false);
    }
    selected_protocol = ft_nullptr;
    selected_length = 0;
    SSL_get0_alpn_selected(ssl_session, &selected_protocol, &selected_length);
    if (selected_protocol && selected_length == 2)
    {
        if (selected_protocol[0] == 'h' && selected_protocol[1] == '2')
            selected_http2 = true;
    }
    error_code = ER_SUCCESS;
    return (true);
}
