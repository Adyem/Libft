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
        int         _previous_error;

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
    , _previous_error(ft_global_error_stack_peek_last_error())
{
    return ;
}

static int pt_errno_guard_capture_error() noexcept
{
    int error_value;

    error_value = ft_global_error_stack_peek_last_error();
    ft_global_error_stack_drop_last_error();
    if (error_value != FT_ERR_SUCCESSS)
        ft_global_error_stack_push(error_value);
    return (error_value);
}

template <typename GuardType>
pt_errno_guard<GuardType>::~pt_errno_guard() noexcept
{
    int unlock_error;

    unlock_error = FT_ERR_SUCCESSS;
    if (this->_second_guard && this->_second_guard->owns_lock())
    {
        this->_second_guard->unlock();
        unlock_error = pt_errno_guard_capture_error();
    }
    if (this->_first_guard && this->_first_guard->owns_lock())
    {
        this->_first_guard->unlock();
        unlock_error = pt_errno_guard_capture_error();
    }
    if (unlock_error != FT_ERR_SUCCESSS)
        ft_global_error_stack_push(unlock_error);
    else
        ft_global_error_stack_push(this->_previous_error);
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
