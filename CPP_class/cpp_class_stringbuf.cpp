#include "class_stringbuf.hpp"

ft_stringbuf::ft_stringbuf(const std::string &string) noexcept
: _storage(string), _position(0), _error_code(ER_SUCCESS)
{
    return ;
}

ft_stringbuf::~ft_stringbuf() noexcept
{
    return ;
}

void ft_stringbuf::set_error(int error_code) const
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}

std::size_t ft_stringbuf::read(char *buffer, std::size_t count)
{
    std::size_t index = 0;

    if (!buffer)
    {
        this->set_error(FT_EINVAL);
        return (0);
    }
    while (index < count && this->_position < this->_storage.size())
    {
        buffer[index] = this->_storage[this->_position];
        index++;
        this->_position++;
    }
    return (index);
}

bool ft_stringbuf::is_bad() const noexcept
{
    return (this->_error_code != ER_SUCCESS);
}

int ft_stringbuf::get_error() const noexcept
{
    return (this->_error_code);
}

const char *ft_stringbuf::get_error_str() const noexcept
{
    return (ft_strerror(this->_error_code));
}

std::string ft_stringbuf::str() const
{
    return (this->_storage.substr(this->_position));
}
