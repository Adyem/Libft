#include "class_nullptr.hpp"

ft_nullptr_t::ft_nullptr_t() noexcept
{
    return ;
}

ft_nullptr_t::ft_nullptr_t(const ft_nullptr_t &) noexcept
{
    return ;
}

ft_nullptr_t::ft_nullptr_t(ft_nullptr_t &&) noexcept
{
    return ;
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
