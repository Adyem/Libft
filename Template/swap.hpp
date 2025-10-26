#ifndef FT_SWAP_HPP
# define FT_SWAP_HPP

#include <utility>
#include "move.hpp"

template<typename ValueType>
constexpr void ft_swap(ValueType &left_value, ValueType &right_value)
{
    ValueType temporary_value = ft_move(left_value);
    left_value = ft_move(right_value);
    right_value = ft_move(temporary_value);
    return ;
}

#endif
