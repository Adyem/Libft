#include "class_istringstream.hpp"
#include "../Basic/basic.hpp"

ft_istringstream::ft_istringstream(const ft_string &string) noexcept
: ft_istream(), _buffer(string)
{
    return ;
}

ft_istringstream::~ft_istringstream() noexcept
{
    return ;
}

std::size_t ft_istringstream::do_read(char *buffer, std::size_t count)
{
    std::size_t result;

    result = this->_buffer.read(buffer, count);
    return (result);
}

ft_string ft_istringstream::str() const
{
    return (this->_buffer.str());
}

ft_istringstream &operator>>(ft_istringstream &input, int &value)
{
    ft_string content = input.str();
    value = ft_atoi(content.c_str(), ft_nullptr);
    return (input);
}

ft_istringstream &operator>>(ft_istringstream &input, ft_string &value)
{
    value = input.str();
    return (input);
}
