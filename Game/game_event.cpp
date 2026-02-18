#include "game_event.hpp"
#include "../Template/move.hpp"
#include <climits>

static void game_event_unlock_guard(ft_unique_lock<pt_mutex> &guard)
{
    if (guard.owns_lock())
        guard.unlock();
    return ;
}

int ft_event::lock_pair(const ft_event &first, const ft_event &second,
    ft_unique_lock<pt_mutex> &first_guard,
    ft_unique_lock<pt_mutex> &second_guard)
{
    const ft_event *ordered_first;
    const ft_event *ordered_second;
    bool swapped;

    if (&first == &second)
    {
        first_guard = ft_unique_lock<pt_mutex>(first._mutex);
        second_guard = ft_unique_lock<pt_mutex>();
        if (first_guard.owns_lock() == false)
            return (FT_ERR_INVALID_STATE);
        return (FT_ERR_SUCCESS);
    }
    ordered_first = &first;
    ordered_second = &second;
    swapped = false;
    if (ordered_first > ordered_second)
    {
        const ft_event *temporary;

        temporary = ordered_first;
        ordered_first = ordered_second;
        ordered_second = temporary;
        swapped = true;
    }
    first_guard = ft_unique_lock<pt_mutex>(ordered_first->_mutex);
    if (first_guard.owns_lock() == false)
        return (FT_ERR_INVALID_STATE);
    second_guard = ft_unique_lock<pt_mutex>(ordered_second->_mutex);
    if (second_guard.owns_lock() == false)
    {
        game_event_unlock_guard(first_guard);
        return (FT_ERR_INVALID_STATE);
    }
    if (swapped)
    {
        ft_unique_lock<pt_mutex> temporary_guard;

        temporary_guard = ft_move(first_guard);
        first_guard = ft_move(second_guard);
        second_guard = ft_move(temporary_guard);
    }
    return (FT_ERR_SUCCESS);
}

ft_event::ft_event() noexcept
    : _id(0), _duration(0), _modifier1(0), _modifier2(0), _modifier3(0),
      _modifier4(0), _callback(), _error(FT_ERR_SUCCESS), _mutex()
{
    return ;
}

ft_event::~ft_event() noexcept
{
    return ;
}

ft_event::ft_event(const ft_event &other) noexcept
    : _id(0), _duration(0), _modifier1(0), _modifier2(0), _modifier3(0),
      _modifier4(0), _callback(), _error(FT_ERR_SUCCESS), _mutex()
{
    *this = other;
    return ;
}

ft_event &ft_event::operator=(const ft_event &other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;

    if (this == &other)
        return (*this);
    lock_error = ft_event::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_id = other._id;
    this->_duration = other._duration;
    this->_modifier1 = other._modifier1;
    this->_modifier2 = other._modifier2;
    this->_modifier3 = other._modifier3;
    this->_modifier4 = other._modifier4;
    this->_callback = other._callback;
    this->_error = other._error;
    game_event_unlock_guard(this_guard);
    game_event_unlock_guard(other_guard);
    return (*this);
}

ft_event::ft_event(ft_event &&other) noexcept
    : _id(0), _duration(0), _modifier1(0), _modifier2(0), _modifier3(0),
      _modifier4(0), _callback(), _error(FT_ERR_SUCCESS), _mutex()
{
    *this = other;
    return ;
}

ft_event &ft_event::operator=(ft_event &&other) noexcept
{
    *this = other;
    return (*this);
}

int ft_event::get_id() const noexcept { return (this->_id); }
void ft_event::set_id(int id) noexcept { if (id < 0) { this->set_error(FT_ERR_INVALID_ARGUMENT); return ; } this->_id = id; this->set_error(FT_ERR_SUCCESS); return ; }
int ft_event::get_duration() const noexcept { return (this->_duration); }
void ft_event::set_duration(int duration) noexcept { if (duration < 0) { this->set_error(FT_ERR_INVALID_ARGUMENT); return ; } this->_duration = duration; this->set_error(FT_ERR_SUCCESS); return ; }
int ft_event::add_duration(int duration) noexcept { if (duration < 0) return (FT_ERR_INVALID_ARGUMENT); if (duration > 0 && this->_duration > INT_MAX - duration) return (FT_ERR_OUT_OF_RANGE); this->_duration += duration; this->set_error(FT_ERR_SUCCESS); return (FT_ERR_SUCCESS); }
void ft_event::sub_duration(int duration) noexcept { if (duration < 0 || duration > this->_duration) { this->set_error(FT_ERR_INVALID_ARGUMENT); return ; } this->_duration -= duration; this->set_error(FT_ERR_SUCCESS); return ; }
int ft_event::get_modifier1() const noexcept { return (this->_modifier1); }
void ft_event::set_modifier1(int mod) noexcept { this->_modifier1 = mod; this->set_error(FT_ERR_SUCCESS); return ; }
void ft_event::add_modifier1(int mod) noexcept { this->_modifier1 += mod; this->set_error(FT_ERR_SUCCESS); return ; }
void ft_event::sub_modifier1(int mod) noexcept { this->_modifier1 -= mod; this->set_error(FT_ERR_SUCCESS); return ; }
int ft_event::get_modifier2() const noexcept { return (this->_modifier2); }
void ft_event::set_modifier2(int mod) noexcept { this->_modifier2 = mod; this->set_error(FT_ERR_SUCCESS); return ; }
void ft_event::add_modifier2(int mod) noexcept { this->_modifier2 += mod; this->set_error(FT_ERR_SUCCESS); return ; }
void ft_event::sub_modifier2(int mod) noexcept { this->_modifier2 -= mod; this->set_error(FT_ERR_SUCCESS); return ; }
int ft_event::get_modifier3() const noexcept { return (this->_modifier3); }
void ft_event::set_modifier3(int mod) noexcept { this->_modifier3 = mod; this->set_error(FT_ERR_SUCCESS); return ; }
void ft_event::add_modifier3(int mod) noexcept { this->_modifier3 += mod; this->set_error(FT_ERR_SUCCESS); return ; }
void ft_event::sub_modifier3(int mod) noexcept { this->_modifier3 -= mod; this->set_error(FT_ERR_SUCCESS); return ; }
int ft_event::get_modifier4() const noexcept { return (this->_modifier4); }
void ft_event::set_modifier4(int mod) noexcept { this->_modifier4 = mod; this->set_error(FT_ERR_SUCCESS); return ; }
void ft_event::add_modifier4(int mod) noexcept { this->_modifier4 += mod; this->set_error(FT_ERR_SUCCESS); return ; }
void ft_event::sub_modifier4(int mod) noexcept { this->_modifier4 -= mod; this->set_error(FT_ERR_SUCCESS); return ; }
const ft_function<void(ft_world&, ft_event&)> &ft_event::get_callback() const noexcept { return (this->_callback); }
void ft_event::set_callback(ft_function<void(ft_world&, ft_event&)> &&callback) noexcept { this->_callback = ft_move(callback); this->set_error(FT_ERR_SUCCESS); return ; }
int ft_event::get_error() const noexcept { return (this->_error); }
const char *ft_event::get_error_str() const noexcept { return (ft_strerror(this->_error)); }
void ft_event::set_error(int err) const noexcept { this->_error = err; return ; }
