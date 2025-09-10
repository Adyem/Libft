#include "class_istream.hpp"

ft_istream::ft_istream() noexcept
: _gcount(0), _bad(false), _error_code(ER_SUCCESS)
{
    return ;
}

ft_istream::~ft_istream() noexcept
{
    return ;
}

void ft_istream::set_error(int error_code) const
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}

void ft_istream::read(char *buffer, std::size_t count)
{
    std::size_t readed;

    this->_error_code = ER_SUCCESS;
    this->_gcount = 0;
    this->_bad = false;
    readed = this->do_read(buffer, count);
    this->_gcount = readed;
    if (this->_error_code != ER_SUCCESS)
        this->_bad = true;
    return ;
}

std::size_t ft_istream::gcount() const noexcept
{
    return (this->_gcount);
}

bool ft_istream::bad() const noexcept
{
    return (this->_bad);
}

int ft_istream::get_error() const noexcept
{
    return (this->_error_code);
}

const char *ft_istream::get_error_str() const noexcept
{
    return (ft_strerror(this->_error_code));
}
