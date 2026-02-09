#include "class_nullptr.hpp"
#include "../Basic/basic.hpp"

ft_nullptr_t::ft_nullptr_t() noexcept
{
    return ;
}

ft_nullptr_t::ft_nullptr_t(const ft_nullptr_t &other) noexcept
{
    (void)other;
    return ;
}

ft_nullptr_t &ft_nullptr_t::operator=(const ft_nullptr_t &other) noexcept
{
    (void)other;
    return (*this);
}

ft_nullptr_t::~ft_nullptr_t() noexcept
{
    return ;
}

void ft_nullptr_t::operator&() const
{
    return ;
}

const ft_nullptr_t ft_nullptr_instance = ft_nullptr_t();
