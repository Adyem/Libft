#ifndef CONSTRUCTOR_HPP
#define CONSTRUCTOR_HPP

#include <type_traits>
#include <utility>
#include <new>

constexpr auto* placement_new_ref = static_cast<void* (*)(std::size_t, void*)>(&::operator new);

template <typename T, typename... Args>
constexpr T* construct_at(T* destination, Args&&... args)
{
    ::new (static_cast<void*>(destination)) T(std::forward<Args>(args)...);
    return (destination);
}

template <typename T>
constexpr T* construct_default_at(T* destination)
{
    ::new (static_cast<void*>(destination)) T();
    return (destination);
}

template <typename T>
constexpr void destroy_at(T* object)
{
    if constexpr (!std::is_trivially_destructible_v<T>)
        object->~T();
    return ;
}

#endif
