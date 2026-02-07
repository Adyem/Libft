#ifndef TEMPLATE_INVOKE_HPP
#define TEMPLATE_INVOKE_HPP

#include <type_traits>
#include <utility>
#include "template_concepts.hpp"

template <typename InstanceType>
struct ft_invoke_detail_is_pointer
{
    static const bool value = std::is_pointer<typename std::decay<InstanceType>::type>::value;
};

#if FT_TEMPLATE_HAS_CONCEPTS
template <typename InstanceType>
constexpr auto ft_invoke_detail_access(InstanceType &&instance)
    requires std::is_pointer_v<std::decay_t<InstanceType>>
{
    return (*instance);
}

template <typename InstanceType>
constexpr auto ft_invoke_detail_access(InstanceType &&instance)
    requires (!std::is_pointer_v<std::decay_t<InstanceType>>)
{
    return (std::forward<InstanceType>(instance));
}
#else
template <typename InstanceType>
constexpr auto ft_invoke_detail_access(InstanceType &&instance)
    -> typename std::enable_if<ft_invoke_detail_is_pointer<InstanceType>::value,
        decltype(*std::forward<InstanceType>(instance))>::type
{
    return (*instance);
}

template <typename InstanceType>
constexpr auto ft_invoke_detail_access(InstanceType &&instance)
    -> typename std::enable_if<!ft_invoke_detail_is_pointer<InstanceType>::value,
        InstanceType &&>::type
{
    return (std::forward<InstanceType>(instance));
}
#endif

template <typename MemberFunction, typename InstanceType, typename... Args>
constexpr auto ft_invoke(MemberFunction &&member_function, InstanceType &&instance, Args&&... args)
#if FT_TEMPLATE_HAS_CONCEPTS
    requires std::is_member_function_pointer_v<std::decay_t<MemberFunction>>
#else
    -> typename std::enable_if<
        std::is_member_function_pointer<typename std::decay<MemberFunction>::type>::value,
        decltype((ft_invoke_detail_access(std::forward<InstanceType>(instance)).*
            std::forward<MemberFunction>(member_function))(
                std::forward<Args>(args)...))>::type
#endif
{
    return ((ft_invoke_detail_access(std::forward<InstanceType>(instance)).*
        std::forward<MemberFunction>(member_function))(
            std::forward<Args>(args)...));
}

template <typename MemberObject, typename InstanceType>
constexpr auto ft_invoke(MemberObject &&member_object, InstanceType &&instance)
#if FT_TEMPLATE_HAS_CONCEPTS
    requires std::is_member_object_pointer_v<std::decay_t<MemberObject>>
#else
    -> typename std::enable_if<
        std::is_member_object_pointer<typename std::decay<MemberObject>::type>::value,
        decltype(ft_invoke_detail_access(std::forward<InstanceType>(instance)).*
            std::forward<MemberObject>(member_object))>::type
#endif
{
    return (ft_invoke_detail_access(std::forward<InstanceType>(instance)).*
        std::forward<MemberObject>(member_object));
}

template <typename FunctionType, typename... Args>
constexpr auto ft_invoke(FunctionType &&function, Args&&... args)
#if FT_TEMPLATE_HAS_CONCEPTS
    requires (!std::is_member_pointer_v<std::decay_t<FunctionType>>)
#else
    -> typename std::enable_if<
        !std::is_member_pointer<typename std::decay<FunctionType>::type>::value,
        decltype(std::forward<FunctionType>(function)(std::forward<Args>(args)...))>::type
#endif
{
    return (std::forward<FunctionType>(function)(std::forward<Args>(args)...));
}

#endif
