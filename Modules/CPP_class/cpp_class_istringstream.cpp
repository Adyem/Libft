#include "class_istringstream.hpp"
#include "../Basic/basic.hpp"

ft_istringstream::ft_istringstream() noexcept
: ft_istream(), _buffer()
{
    return ;
}

ft_istringstream::ft_istringstream(const ft_istringstream &other) noexcept
: ft_istream(), _buffer()
{
    uint32_t initialize_error;
    ft_string string_value;

    string_value = other.get_string();
    initialize_error = this->initialize(string_value);
    if (initialize_error != FT_ERR_SUCCESS)
        (void)this->destroy();
    return ;
}

ft_istringstream::ft_istringstream(ft_istringstream &&other) noexcept
: ft_istream(), _buffer()
{
    uint32_t initialize_error;

    initialize_error = this->move(other);
    if (initialize_error != FT_ERR_SUCCESS)
        (void)this->destroy();
    return ;
}

ft_istringstream::~ft_istringstream() noexcept
{
    (void)this->destroy();
    return ;
}

int32_t ft_istringstream::initialize(const ft_string &string) noexcept
{
    uint32_t stream_initialize_error;
    uint32_t buffer_initialize_error;

    stream_initialize_error = ft_istream::initialize();
    if (stream_initialize_error != FT_ERR_SUCCESS)
        return (stream_initialize_error);
    buffer_initialize_error = this->_buffer.initialize(string);
    if (buffer_initialize_error != FT_ERR_SUCCESS)
    {
        (void)ft_istream::destroy();
        return (buffer_initialize_error);
    }
    return (FT_ERR_SUCCESS);
}

int32_t ft_istringstream::destroy() noexcept
{
    int32_t buffer_destroy_error;
    int32_t stream_destroy_error;

    buffer_destroy_error = this->_buffer.destroy();
    stream_destroy_error = ft_istream::destroy();
    if (buffer_destroy_error != FT_ERR_SUCCESS)
        return (buffer_destroy_error);
    return (stream_destroy_error);
}

int32_t ft_istringstream::move(ft_istringstream &other) noexcept
{
    if (&other == this)
        return (FT_ERR_SUCCESS);
    int32_t destroy_error = this->destroy();
    if (destroy_error != FT_ERR_SUCCESS)
        return (destroy_error);
    int32_t stream_move_error = ft_istream::move(other);
    if (stream_move_error != FT_ERR_SUCCESS)
        return (stream_move_error);
    int32_t buffer_move_error = this->_buffer.move(other._buffer);
    if (buffer_move_error != FT_ERR_SUCCESS)
    {
        (void)this->destroy();
        return (buffer_move_error);
    }
    return (FT_ERR_SUCCESS);
}

ssize_t ft_istringstream::do_read(char *buffer, ft_size_t count)
{
    ssize_t result;

    result = this->_buffer.read(buffer, count);
    return (result);
}

ft_string ft_istringstream::get_string() const
{
    ft_string value;
    int32_t initialize_error;

    initialize_error = value.initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        return (value);
    if (this->_buffer.get_string(value) != FT_ERR_SUCCESS)
        return (value);
    return (value);
}

ft_istringstream &operator>>(ft_istringstream &input, int32_t &value)
{
    ft_string content = input.get_string();
    value = ft_atoi(content.c_str());
    return (input);
}

ft_istringstream &operator>>(ft_istringstream &input, ft_string &value)
{
    value = input.get_string();
    return (input);
}
