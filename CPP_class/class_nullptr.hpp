#ifndef NULLPTR_HPP
#define NULLPTR_HPP

#include "../Errno/errno.hpp"

class ft_nullptr_t
{
    private:
        mutable int _error_code;

        void set_error_internal(int error_code) const noexcept;
        void set_error(int error_code) const noexcept;
        static void restore_errno(int entry_errno, int operation_errno) noexcept;

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

        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

extern const ft_nullptr_t ft_nullptr_instance;

#define ft_nullptr (ft_nullptr_instance)

template <typename PointerType>
inline ft_nullptr_t::operator PointerType*() const noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    this->set_error_internal(FT_ER_SUCCESSS);
    ft_nullptr_t::restore_errno(entry_errno, FT_ER_SUCCESSS);
    return (static_cast<PointerType*>(nullptr));
}

template <typename ClassType, typename MemberType>
inline ft_nullptr_t::operator MemberType ClassType::*() const noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    this->set_error_internal(FT_ER_SUCCESSS);
    ft_nullptr_t::restore_errno(entry_errno, FT_ER_SUCCESSS);
    return (static_cast<MemberType ClassType::*>(nullptr));
}

#endif
