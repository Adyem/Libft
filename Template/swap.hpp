#ifndef FT_SWAP_HPP
# define FT_SWAP_HPP

#include <utility>

template<typename T>
void ft_swap(T& a, T& b)
{
    T temp = std::move(a);
    a = std::move(b);
    b = std::move(temp);
}

#endif
