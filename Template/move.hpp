#ifndef MOVE_HPP
# define MOVE_HPP

#include <type_traits>

template <typename Type>
constexpr typename std::remove_reference<Type>::type&& ft_move(Type&& value) noexcept
{
    return (static_cast<typename std::remove_reference<Type>::type&&>(value));
}

#endif
