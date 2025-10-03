#include "class_stringbuf.hpp"

ft_stringbuf::ft_stringbuf(const ft_string &string) noexcept
: _storage(string), _position(0), _error_code(ER_SUCCESS)
{
    if (this->_storage.get_error() != ER_SUCCESS)
        this->set_error(this->_storage.get_error());
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
    bool failure_occurred = false;

    if (!buffer)
    {
        this->set_error(FT_EINVAL);
        return (0);
    }
    while (index < count && this->_position < this->_storage.size())
    {
        const char *current = this->_storage.at(this->_position);
        if (!current)
        {
            this->set_error(FT_EINVAL);
            failure_occurred = true;
            break ;
        }
        buffer[index] = *current;
        index++;
        this->_position++;
    }
    if (!failure_occurred)
        this->set_error(ER_SUCCESS);
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

ft_string ft_stringbuf::str() const
{
    const char *start = this->_storage.c_str();
    ft_string result(start + this->_position);
    if (result.get_error() != ER_SUCCESS)
        this->set_error(result.get_error());
    else
        this->set_error(ER_SUCCESS);
    return (result);
}
