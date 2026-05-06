#ifndef TEMPLATE_TEMPLATE_CONCEPTS_HPP
#define TEMPLATE_TEMPLATE_CONCEPTS_HPP

#if defined(__cpp_concepts) && __cpp_concepts >= 201907L
#include <concepts>
#define FT_TEMPLATE_HAS_CONCEPTS 1
#else
#define FT_TEMPLATE_HAS_CONCEPTS 0
#endif

#if FT_TEMPLATE_HAS_CONCEPTS
#include <type_traits>

template <typename ConstructibleType, typename... ConstructArgs>
concept ft_constructible_from =
    std::is_constructible_v<ConstructibleType, ConstructArgs...>;

template <typename FromType, typename ToType>
concept ft_convertible_to = std::is_convertible_v<FromType, ToType>;
#endif

#endif
