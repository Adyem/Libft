#include "class_istringstream.hpp"
#include "../Basic/basic.hpp"

ft_istringstream::ft_istringstream(const ft_string &string) noexcept
: ft_istream(), _buffer()
{
    (void)this->initialize();
    (void)this->_buffer.initialize(string);
    return ;
}

ft_istringstream::~ft_istringstream() noexcept
{
    return ;
}

ssize_t ft_istringstream::do_read(char *buffer, std::size_t count)
{
    ssize_t result;

    result = this->_buffer.read(buffer, count);
    return (result);
}

ft_string ft_istringstream::str() const
{
    ft_string value;

    if (this->_buffer.str(value) != FT_ERR_SUCCESS)
        return (value);
    return (value);
}

ft_istringstream &operator>>(ft_istringstream &input, int &value)
{
    ft_string content = input.str();
    value = ft_atoi(content.c_str());
    return (input);
}

ft_istringstream &operator>>(ft_istringstream &input, ft_string &value)
{
    value = input.str();
    return (input);
}
