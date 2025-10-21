#ifndef FT_SWAP_HPP
# define FT_SWAP_HPP

#include <utility>
#include "move.hpp"

template<typename T>
void ft_swap(T& a, T& b)
{
    T temp = ft_move(a);
    a = ft_move(b);
    b = ft_move(temp);
}

#endif
