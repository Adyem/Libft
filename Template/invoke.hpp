#ifndef TEMPLATE_INVOKE_HPP
#define TEMPLATE_INVOKE_HPP

#include <type_traits>
#include <utility>

namespace ft_invoke_detail
{
    template <typename InstanceType>
    struct is_pointer
    {
        static const bool value = std::is_pointer<typename std::decay<InstanceType>::type>::value;
    };

    template <typename InstanceType>
    auto access(InstanceType &&instance)
        -> typename std::enable_if<is_pointer<InstanceType>::value,
            decltype(*std::forward<InstanceType>(instance))>::type
    {
        return (*instance);
    }

    template <typename InstanceType>
    auto access(InstanceType &&instance)
        -> typename std::enable_if<!is_pointer<InstanceType>::value,
            InstanceType &&>::type
    {
        return (std::forward<InstanceType>(instance));
    }
}

template <typename MemberFunction, typename InstanceType, typename... Args>
auto ft_invoke(MemberFunction &&member_function, InstanceType &&instance, Args&&... args)
    -> typename std::enable_if<
        std::is_member_function_pointer<typename std::decay<MemberFunction>::type>::value,
        decltype((ft_invoke_detail::access(std::forward<InstanceType>(instance)).*
            std::forward<MemberFunction>(member_function))(
                std::forward<Args>(args)...))>::type
{
    return ((ft_invoke_detail::access(std::forward<InstanceType>(instance)).*
        std::forward<MemberFunction>(member_function))(
            std::forward<Args>(args)...));
}

template <typename MemberObject, typename InstanceType>
auto ft_invoke(MemberObject &&member_object, InstanceType &&instance)
    -> typename std::enable_if<
        std::is_member_object_pointer<typename std::decay<MemberObject>::type>::value,
        decltype(ft_invoke_detail::access(std::forward<InstanceType>(instance)).*
            std::forward<MemberObject>(member_object))>::type
{
    return (ft_invoke_detail::access(std::forward<InstanceType>(instance)).*
        std::forward<MemberObject>(member_object));
}

template <typename FunctionType, typename... Args>
auto ft_invoke(FunctionType &&function, Args&&... args)
    -> typename std::enable_if<
        !std::is_member_pointer<typename std::decay<FunctionType>::type>::value,
        decltype(std::forward<FunctionType>(function)(std::forward<Args>(args)...))>::type
{
    return (std::forward<FunctionType>(function)(std::forward<Args>(args)...));
}

#endif
