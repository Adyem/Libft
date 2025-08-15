#ifndef MATH_HPP
#define MATH_HPP

#include <cstddef>
#include <type_traits>

template <typename T, typename U>
constexpr std::common_type_t<T, U> ft_max(const T& a, const U& b) noexcept
{
    if (a < b)
        return b;
    return a;
}

template <typename T, typename U, typename Compare>
constexpr std::common_type_t<T, U> ft_max(const T& a, const U& b, Compare comp) noexcept
{
    if (comp(a, b))
        return b;
    return a;
}

template <typename T, typename U>
constexpr std::common_type_t<T, U> ft_min(const T& a, const U& b) noexcept
{
    if (b < a)
        return b;
    return a;
}

template <typename T, typename U, typename Compare>
constexpr std::common_type_t<T, U> ft_min(const T& a, const U& b, Compare comp) noexcept
{
    if (comp(b, a))
        return b;
    return a;
}

template <typename... Args>
struct is_single_convertible_to_size_t : std::false_type {};

template <typename Arg>
struct is_single_convertible_to_size_t<Arg> : std::is_convertible<std::decay_t<Arg>, size_t> {};

#endif
