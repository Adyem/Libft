#ifndef NULLPTR_HPP
#define NULLPTR_HPP

#include "../Errno/errno.hpp"

class ft_nullptr_t
{
    private:
        void operator&() const;

    public:
        ft_nullptr_t() noexcept;
        ft_nullptr_t(const ft_nullptr_t &other) noexcept;
        ft_nullptr_t &operator=(const ft_nullptr_t &other) noexcept;
        ~ft_nullptr_t() noexcept;

        template <typename PointerType>
        operator PointerType*() const noexcept;

        template <typename ClassType, typename MemberType>
        operator MemberType ClassType::*() const noexcept;
};

extern const ft_nullptr_t ft_nullptr_instance;

#define ft_nullptr (ft_nullptr_instance)

template <typename PointerType>
inline ft_nullptr_t::operator PointerType*() const noexcept
{
    return (static_cast<PointerType*>(nullptr));
}

template <typename ClassType, typename MemberType>
inline ft_nullptr_t::operator MemberType ClassType::*() const noexcept
{
    return (static_cast<MemberType ClassType::*>(nullptr));
}

#endif
