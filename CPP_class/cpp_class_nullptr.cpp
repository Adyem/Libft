#include "class_nullptr.hpp"
#include "../Libft/libft.hpp"

static const ft_nullptr_t *get_nullptr_address(const ft_nullptr_t &value) noexcept
{
    return (reinterpret_cast<const ft_nullptr_t*>(
        &const_cast<char&>(reinterpret_cast<const volatile char&>(value))));
}

void ft_nullptr_t::set_error_internal(int error_code) const noexcept
{
    this->_error_code = error_code;
    return ;
}

void ft_nullptr_t::set_error(int error_code) const noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    this->set_error_internal(error_code);
    ft_nullptr_t::restore_errno(entry_errno, error_code);
    return ;
}

void ft_nullptr_t::restore_errno(int entry_errno, int operation_errno) noexcept
{
    if (operation_errno != FT_ER_SUCCESSS)
    {
        ft_errno = operation_errno;
        return ;
    }
    ft_errno = entry_errno;
    return ;
}

ft_nullptr_t::ft_nullptr_t() noexcept
    : _error_code(FT_ER_SUCCESSS)
{
    this->set_error(FT_ER_SUCCESSS);
    return ;
}

ft_nullptr_t::ft_nullptr_t(const ft_nullptr_t &other) noexcept
    : _error_code(FT_ER_SUCCESSS)
{
    int entry_errno;
    int other_error;

    entry_errno = ft_errno;
    other_error = other.get_error();
    this->set_error_internal(other_error);
    ft_errno = entry_errno;
    return ;
}

ft_nullptr_t &ft_nullptr_t::operator=(const ft_nullptr_t &other) noexcept
{
    int entry_errno;
    int other_error;
    const ft_nullptr_t *other_address;

    other_address = get_nullptr_address(other);
    if (this == other_address)
        return (*this);
    entry_errno = ft_errno;
    other_error = other.get_error();
    this->set_error_internal(other_error);
    ft_nullptr_t::restore_errno(entry_errno, other_error);
    return (*this);
}

ft_nullptr_t::~ft_nullptr_t() noexcept
{
    this->set_error(FT_ER_SUCCESSS);
    return ;
}

int ft_nullptr_t::get_error() const noexcept
{
    int entry_errno;
    int error_code;

    entry_errno = ft_errno;
    error_code = this->_error_code;
    ft_nullptr_t::restore_errno(entry_errno, error_code);
    return (error_code);
}

const char *ft_nullptr_t::get_error_str() const noexcept
{
    int entry_errno;
    const char *error_string;

    entry_errno = ft_errno;
    error_string = ft_strerror(this->_error_code);
    ft_nullptr_t::restore_errno(entry_errno, this->_error_code);
    return (error_string);
}

void ft_nullptr_t::operator&() const
{
    return ;
}

const ft_nullptr_t ft_nullptr_instance = ft_nullptr_t();
