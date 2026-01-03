#ifndef PTHREAD_ERRNO_GUARD_HPP
#define PTHREAD_ERRNO_GUARD_HPP

#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Errno/errno_internal.hpp"

template <typename GuardType>
class pt_errno_guard
{
    private:
        GuardType  *_first_guard;
        GuardType  *_second_guard;
        int         _previous_errno;

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
    , _previous_errno(ft_errno)
{
    return ;
}

template <typename GuardType>
pt_errno_guard<GuardType>::~pt_errno_guard() noexcept
{
    int unlock_error;

    unlock_error = FT_ERR_SUCCESSS;
    if (this->_second_guard && this->_second_guard->owns_lock())
    {
        this->_second_guard->unlock();
        if (this->_second_guard->get_error() != FT_ERR_SUCCESSS)
            unlock_error = this->_second_guard->get_error();
    }
    if (this->_first_guard && this->_first_guard->owns_lock())
    {
        this->_first_guard->unlock();
        if (this->_first_guard->get_error() != FT_ERR_SUCCESSS)
            unlock_error = this->_first_guard->get_error();
    }
    if (unlock_error != FT_ERR_SUCCESSS)
        ft_set_errno_locked(unlock_error);
    else
        ft_set_errno_locked(this->_previous_errno);
    ft_set_sys_errno_locked(FT_SYS_ERR_SUCCESS);
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
