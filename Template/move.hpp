#ifndef FT_MOVE_HPP
# define FT_MOVE_HPP

#include <type_traits>

template <typename ValueType>
constexpr typename std::remove_reference<ValueType>::type &&ft_move(ValueType &&value) noexcept
{
    typedef typename std::remove_reference<ValueType>::type moved_type;

    return (static_cast<moved_type &&>(value));
}

#endif
