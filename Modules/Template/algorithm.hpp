#ifndef FT_ALGORITHM_HPP
#define FT_ALGORITHM_HPP

#include <cstddef>
#include "swap.hpp"
#include "../RNG/rng.hpp"

template <typename RandomIt, typename Compare>
void ft_sort(RandomIt first, RandomIt last, Compare comp)
{
    RandomIt i = first;
    while (i != last)
    {
        RandomIt j = i + 1;
        while (j != last)
        {
            if (comp(*j, *i))
                ft_swap(*i, *j);
            ++j;
        }
        ++i;
    }
    return ;
}

template <typename RandomIt>
void ft_sort(RandomIt first, RandomIt last)
{
    ft_sort(first, last, [](const auto& a, const auto& b)
    {
        return (a < b);
    });
    return ;
}

template <typename RandomIt, typename T, typename Compare>
ft_bool ft_binary_search(RandomIt first, RandomIt last, const T& value, Compare comp)
{
    while (first < last)
    {
        RandomIt mid = first + (last - first) / 2;
        if (!comp(*mid, value) && !comp(value, *mid))
            return (FT_TRUE);
        if (comp(*mid, value))
            first = mid + 1;
        else
            last = mid;
    }
    return (FT_FALSE);
}

template <typename RandomIt, typename T>
ft_bool ft_binary_search(RandomIt first, RandomIt last, const T& value)
{
    return (ft_binary_search(first, last, value,
        [](const auto& a, const auto& b)
        {
            return (a < b);
        }));
}

template <typename RandomIt>
void ft_shuffle(RandomIt first, RandomIt last)
{
    if (first == last)
        return ;
    RandomIt iterator = last - 1;
    while (iterator > first)
    {
        ft_size_t random_index = static_cast<ft_size_t>(ft_random_int()) %
                    static_cast<ft_size_t>((iterator - first) + 1);
        ft_swap(*(first + random_index), *iterator);
        --iterator;
    }
    return ;
}

template <typename BidirectionalIt>
void ft_reverse(BidirectionalIt first, BidirectionalIt last)
{
    while (first != last && first != --last)
    {
        ft_swap(*first, *last);
        ++first;
    }
    return ;
}

#endif 
