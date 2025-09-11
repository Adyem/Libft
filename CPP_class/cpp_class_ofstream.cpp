#include "class_ofstream.hpp"
#include "class_nullptr.hpp"
#include <fcntl.h>

ft_ofstream::ft_ofstream() noexcept : _file(), _error_code(ER_SUCCESS)
{
    return ;
}

ft_ofstream::~ft_ofstream() noexcept
{
    return ;
}

void ft_ofstream::set_error(int error_code) const
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}

int ft_ofstream::open(const char *filename) noexcept
{
    if (filename == ft_nullptr)
    {
        this->set_error(FT_EINVAL);
        return (1);
    }
    if (this->_file.open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644) != 0)
    {
        this->set_error(this->_file.get_error());
        return (1);
    }
    return (0);
}

ssize_t ft_ofstream::write(const char *string) noexcept
{
    if (string == ft_nullptr)
    {
        this->set_error(FT_EINVAL);
        return (-1);
    }
    ssize_t result = this->_file.write(string);
    if (result < 0)
        this->set_error(this->_file.get_error());
    return (result);
}

void ft_ofstream::close() noexcept
{
    this->_file.close();
    return ;
}

int ft_ofstream::get_error() const noexcept
{
    return (this->_error_code);
}

const char *ft_ofstream::get_error_str() const noexcept
{
    return (ft_strerror(this->_error_code));
}
