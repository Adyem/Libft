#include "class_istringstream.hpp"

ft_istringstream::ft_istringstream(const std::string &string) noexcept
: std::istream(ft_nullptr), _buffer(string), _error_code(ER_SUCCESS)
{
    this->init(&_buffer);
    return ;
}

ft_istringstream::~ft_istringstream() noexcept
{
    return ;
}

void ft_istringstream::set_error(int error_code) const
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}

int ft_istringstream::get_error() const noexcept
{
    return (this->_error_code);
}

const char *ft_istringstream::get_error_str() const noexcept
{
    return (ft_strerror(this->_error_code));
}
