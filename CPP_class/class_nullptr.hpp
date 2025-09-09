#ifndef NULLPTR_HPP
#define NULLPTR_HPP

namespace ft
{
    struct nullptr_t
    {
        template <typename PointerType>
        operator PointerType*() const { return (nullptr); }

        template <typename ClassType, typename MemberType>
        operator MemberType ClassType::*() const { return (nullptr); }

    private:
        void operator&() const;
    };

    extern const nullptr_t ft_nullptr_instance;
}

#define ft_nullptr (ft::ft_nullptr_instance)

#endif
