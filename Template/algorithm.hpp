#ifndef FT_ALGORITHM_HPP
#define FT_ALGORITHM_HPP

#include <cstddef>
#include "swap.hpp"
#include "../RNG/RNG.hpp"

// Simple sort using comparator
// Defaults to ascending order using operator<
template <typename RandomIt, typename Compare>
void ft_sort(RandomIt first, RandomIt last, Compare comp)
{
    for (RandomIt i = first; i != last; ++i)
    {
        for (RandomIt j = i + 1; j != last; ++j)
        {
            if (comp(*j, *i))
                ft_swap(*i, *j);
        }
    }
    return ;
}

template <typename RandomIt>
void ft_sort(RandomIt first, RandomIt last)
{
    ft_sort(first, last, [](const auto& a, const auto& b){ return a < b; });
    return ;
}

// Binary search in sorted range
// Returns true if value is found
template <typename RandomIt, typename T, typename Compare>
bool ft_binary_search(RandomIt first, RandomIt last, const T& value, Compare comp)
{
    while (first < last)
    {
        RandomIt mid = first + (last - first) / 2;
        if (!comp(*mid, value) && !comp(value, *mid))
            return true;
        if (comp(*mid, value))
            first = mid + 1;
        else
            last = mid;
    }
    return false;
}

template <typename RandomIt, typename T>
bool ft_binary_search(RandomIt first, RandomIt last, const T& value)
{
    return ft_binary_search(first, last, value,
        [](const auto& a, const auto& b){ return a < b; });
}

// Fisher-Yates shuffle
template <typename RandomIt>
void ft_shuffle(RandomIt first, RandomIt last)
{
    if (first == last)
        return ;
    for (RandomIt i = last - 1; i > first; --i)
    {
        size_t j = static_cast<size_t>(ft_random_int()) %
                    static_cast<size_t>((i - first) + 1);
        ft_swap(*(first + j), *i);
    }
    return ;
}

// Reverse elements in range
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

#endif // FT_ALGORITHM_HPP
