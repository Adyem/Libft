#ifndef THREADING_ERRNO_GUARD_HPP
#define THREADING_ERRNO_GUARD_HPP

#include "../Basic/class_nullptr.hpp"
#include "../Errno/errno.hpp"

template <typename GuardType>
class pt_errno_guard
{
#ifdef LIBFT_TEST_BUILD
    public:
#else
    private:
#endif
        GuardType  *_first_guard;
        GuardType  *_second_guard;

    public:
        pt_errno_guard(GuardType &first_guard, GuardType &second_guard) noexcept;
        ~pt_errno_guard() noexcept;

        void dismiss() noexcept;
};

template <typename GuardType>
pt_errno_guard<GuardType>::pt_errno_guard(GuardType &first_guard,
        GuardType &second_guard) noexcept
    : _first_guard(&first_guard)
    , _second_guard(&second_guard)
{
    return ;
}

template <typename GuardType>
pt_errno_guard<GuardType>::~pt_errno_guard() noexcept
{
    if (this->_second_guard && this->_second_guard->owns_lock())
        this->_second_guard->unlock();
    if (this->_first_guard && this->_first_guard->owns_lock())
        this->_first_guard->unlock();
    return ;
}

template <typename GuardType>
void pt_errno_guard<GuardType>::dismiss() noexcept
{
    this->_first_guard = ft_nullptr;
    this->_second_guard = ft_nullptr;
    return ;
}

#endif
