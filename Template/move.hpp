#ifndef FT_MOVE_HPP
# define FT_MOVE_HPP

#ifndef FT_REMOVE_REFERENCE_HPP
# define FT_REMOVE_REFERENCE_HPP

template <typename Type>
struct ft_remove_reference
{
    typedef Type type;
};

template <typename Type>
struct ft_remove_reference<Type &>
{
    typedef Type type;
};

template <typename Type>
struct ft_remove_reference<Type &&>
{
    typedef Type type;
};

#endif

template <typename Type>
constexpr typename ft_remove_reference<Type>::type &&ft_move(Type &&value) noexcept
{
    typedef typename ft_remove_reference<Type>::type &&result_type;

    return (static_cast<result_type>(value));
}

#endif
