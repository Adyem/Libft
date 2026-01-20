#include "game_pathfinding.hpp"
#include <utility>
#include "../Template/move.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/pthread.hpp"
#include "../Errno/errno_internal.hpp"

static size_t distance_component(size_t a, size_t b)
{
    if (a > b)
        return (a - b);
    return (b - a);
}

static void game_pathfinding_sleep_backoff() noexcept
{
    pt_thread_sleep(1);
    return ;
}

static void game_pathfinding_finalize_lock(ft_unique_lock<pt_mutex> &guard) noexcept
{
    if (guard.owns_lock())
        guard.unlock();
    return ;
}

static void game_pathfinding_finalize_lock(ft_unique_lock<pt_recursive_mutex> &guard) noexcept
{
    if (guard.owns_lock())
        guard.unlock();
    return ;
}

ft_path_step::ft_path_step() noexcept
    : _x(0), _y(0), _z(0), _error_code(FT_ERR_SUCCESSS),
    _system_error_code(FT_SYS_ERR_SUCCESS), _mutex()
{
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

ft_path_step::~ft_path_step() noexcept
{
    return ;
}

int ft_path_step::lock_pair(const ft_path_step &first,
        const ft_path_step &second,
        ft_unique_lock<pt_recursive_mutex> &first_guard,
        ft_unique_lock<pt_recursive_mutex> &second_guard) noexcept
{
    const ft_path_step *ordered_first;
    const ft_path_step *ordered_second;
    bool swapped;

    if (&first == &second)
    {
        ft_unique_lock<pt_recursive_mutex> single_guard(first._mutex);

        if (single_guard.get_error() != FT_ERR_SUCCESSS)
        {
            first.set_system_error(single_guard.get_error());
            return (single_guard.get_error());
        }
        first_guard = ft_move(single_guard);
        second_guard = ft_unique_lock<pt_recursive_mutex>();
        first.set_system_error(FT_SYS_ERR_SUCCESS);
        return (FT_ERR_SUCCESSS);
    }
    ordered_first = &first;
    ordered_second = &second;
    swapped = false;
    if (ordered_first > ordered_second)
    {
        const ft_path_step *temporary;

        temporary = ordered_first;
        ordered_first = ordered_second;
        ordered_second = temporary;
        swapped = true;
    }
    ft_unique_lock<pt_recursive_mutex> lower_guard(ordered_first->_mutex);
    if (lower_guard.get_error() != FT_ERR_SUCCESSS)
    {
        ordered_first->set_system_error(lower_guard.get_error());
        return (lower_guard.get_error());
    }
    ft_unique_lock<pt_recursive_mutex> upper_guard(ordered_second->_mutex);
    if (upper_guard.get_error() != FT_ERR_SUCCESSS)
    {
        if (lower_guard.owns_lock())
            lower_guard.unlock();
        ordered_second->set_system_error(upper_guard.get_error());
        return (upper_guard.get_error());
    }
    if (!swapped)
    {
        first_guard = ft_move(lower_guard);
        second_guard = ft_move(upper_guard);
    }
    else
    {
        first_guard = ft_move(upper_guard);
        second_guard = ft_move(lower_guard);
    }
    ordered_first->set_system_error(FT_SYS_ERR_SUCCESS);
    ordered_second->set_system_error(FT_SYS_ERR_SUCCESS);
    return (FT_ERR_SUCCESSS);
}

ft_path_step::ft_path_step(const ft_path_step &other) noexcept
    : _x(0), _y(0), _z(0), _error_code(FT_ERR_SUCCESSS),
    _system_error_code(FT_SYS_ERR_SUCCESS), _mutex()
{
    int other_error;
    int other_system_error;

    other_error = other.get_error();
    ft_unique_lock<pt_recursive_mutex> other_guard(other._mutex);
    ft_unique_lock<pt_recursive_mutex> this_guard(this->_mutex);

    if (other_guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_system_error(other_guard.get_error());
        game_pathfinding_finalize_lock(other_guard);
        game_pathfinding_finalize_lock(this_guard);
        return ;
    }
    if (this_guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_system_error(this_guard.get_error());
        game_pathfinding_finalize_lock(other_guard);
        game_pathfinding_finalize_lock(this_guard);
        return ;
    }
    this->_x = other._x;
    this->_y = other._y;
    this->_z = other._z;
    other_system_error = other._system_error_code;
    this->set_error(other_error);
    this->set_system_error(other_system_error);
    game_pathfinding_finalize_lock(other_guard);
    game_pathfinding_finalize_lock(this_guard);
    return ;
}

ft_path_step &ft_path_step::operator=(const ft_path_step &other) noexcept
{
    ft_unique_lock<pt_recursive_mutex> this_guard;
    ft_unique_lock<pt_recursive_mutex> other_guard;
    int lock_error;
    int other_error;
    int other_system_error;

    if (this == &other)
        return (*this);
    other_error = other.get_error();
    lock_error = ft_path_step::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_system_error(lock_error);
        return (*this);
    }
    this->_x = other._x;
    this->_y = other._y;
    this->_z = other._z;
    other_system_error = other._system_error_code;
    this->set_error(other_error);
    this->set_system_error(other_system_error);
    game_pathfinding_finalize_lock(this_guard);
    game_pathfinding_finalize_lock(other_guard);
    return (*this);
}

ft_path_step::ft_path_step(ft_path_step &&other) noexcept
    : _x(0), _y(0), _z(0), _error_code(FT_ERR_SUCCESSS),
    _system_error_code(FT_SYS_ERR_SUCCESS), _mutex()
{
    int other_error;
    int other_system_error;

    other_error = other.get_error();
    ft_unique_lock<pt_recursive_mutex> other_guard(other._mutex);
    ft_unique_lock<pt_recursive_mutex> this_guard(this->_mutex);

    if (other_guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_system_error(other_guard.get_error());
        game_pathfinding_finalize_lock(other_guard);
        game_pathfinding_finalize_lock(this_guard);
        return ;
    }
    if (this_guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_system_error(this_guard.get_error());
        game_pathfinding_finalize_lock(other_guard);
        game_pathfinding_finalize_lock(this_guard);
        return ;
    }
    this->_x = other._x;
    this->_y = other._y;
    this->_z = other._z;
    other_system_error = other._system_error_code;
    this->set_error(other_error);
    this->set_system_error(other_system_error);
    other._x = 0;
    other._y = 0;
    other._z = 0;
    other.set_error(FT_ERR_SUCCESSS);
    other.set_system_error(FT_SYS_ERR_SUCCESS);
    game_pathfinding_finalize_lock(other_guard);
    game_pathfinding_finalize_lock(this_guard);
    return ;
}

ft_path_step &ft_path_step::operator=(ft_path_step &&other) noexcept
{
    ft_unique_lock<pt_recursive_mutex> this_guard;
    ft_unique_lock<pt_recursive_mutex> other_guard;
    int lock_error;
    int other_error;
    int other_system_error;

    if (this == &other)
        return (*this);
    other_error = other.get_error();
    lock_error = ft_path_step::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_system_error(lock_error);
        return (*this);
    }
    this->_x = other._x;
    this->_y = other._y;
    this->_z = other._z;
    other_system_error = other._system_error_code;
    this->set_error(other_error);
    this->set_system_error(other_system_error);
    other._x = 0;
    other._y = 0;
    other._z = 0;
    other.set_error(FT_ERR_SUCCESSS);
    other.set_system_error(FT_SYS_ERR_SUCCESS);
    game_pathfinding_finalize_lock(this_guard);
    game_pathfinding_finalize_lock(other_guard);
    return (*this);
}

int ft_path_step::set_coordinates(size_t x, size_t y, size_t z) noexcept
{
    ft_unique_lock<pt_recursive_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_system_error(guard.get_error());
        game_pathfinding_finalize_lock(guard);
        return (guard.get_error());
    }
    this->_x = x;
    this->_y = y;
    this->_z = z;
    this->set_error(FT_ERR_SUCCESSS);
    game_pathfinding_finalize_lock(guard);
    return (FT_ERR_SUCCESSS);
}

int ft_path_step::set_x(size_t x) noexcept
{
    ft_unique_lock<pt_recursive_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_system_error(guard.get_error());
        game_pathfinding_finalize_lock(guard);
        return (guard.get_error());
    }
    this->_x = x;
    this->set_error(FT_ERR_SUCCESSS);
    game_pathfinding_finalize_lock(guard);
    return (FT_ERR_SUCCESSS);
}

int ft_path_step::set_y(size_t y) noexcept
{
    ft_unique_lock<pt_recursive_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_system_error(guard.get_error());
        game_pathfinding_finalize_lock(guard);
        return (guard.get_error());
    }
    this->_y = y;
    this->set_error(FT_ERR_SUCCESSS);
    game_pathfinding_finalize_lock(guard);
    return (FT_ERR_SUCCESSS);
}

int ft_path_step::set_z(size_t z) noexcept
{
    ft_unique_lock<pt_recursive_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_system_error(guard.get_error());
        game_pathfinding_finalize_lock(guard);
        return (guard.get_error());
    }
    this->_z = z;
    this->set_error(FT_ERR_SUCCESSS);
    game_pathfinding_finalize_lock(guard);
    return (FT_ERR_SUCCESSS);
}

size_t ft_path_step::get_x() const noexcept
{
    size_t value;

    ft_unique_lock<pt_recursive_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_path_step *>(this)->set_system_error(guard.get_error());
        game_pathfinding_finalize_lock(guard);
        return (0);
    }
    value = this->_x;
    const_cast<ft_path_step *>(this)->set_error(FT_ERR_SUCCESSS);
    game_pathfinding_finalize_lock(guard);
    return (value);
}

size_t ft_path_step::get_y() const noexcept
{
    size_t value;

    ft_unique_lock<pt_recursive_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_path_step *>(this)->set_system_error(guard.get_error());
        game_pathfinding_finalize_lock(guard);
        return (0);
    }
    value = this->_y;
    const_cast<ft_path_step *>(this)->set_error(FT_ERR_SUCCESSS);
    game_pathfinding_finalize_lock(guard);
    return (value);
}

size_t ft_path_step::get_z() const noexcept
{
    size_t value;

    ft_unique_lock<pt_recursive_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_path_step *>(this)->set_system_error(guard.get_error());
        game_pathfinding_finalize_lock(guard);
        return (0);
    }
    value = this->_z;
    const_cast<ft_path_step *>(this)->set_error(FT_ERR_SUCCESSS);
    game_pathfinding_finalize_lock(guard);
    return (value);
}

int ft_path_step::get_error() const noexcept
{
    int error_code;
    int system_error;

    ft_unique_lock<pt_recursive_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_path_step *>(this)->set_system_error(guard.get_error());
        game_pathfinding_finalize_lock(guard);
        return (guard.get_error());
    }
    error_code = this->_error_code;
    system_error = this->_system_error_code;
    game_pathfinding_finalize_lock(guard);
    if (error_code != FT_ERR_SUCCESSS)
    {
        ft_set_errno_locked(error_code);
        return (error_code);
    }
    if (system_error != FT_SYS_ERR_SUCCESS)
    {
        ft_set_errno_locked(system_error);
        return (system_error);
    }
    ft_set_errno_locked(FT_ERR_SUCCESSS);
    return (FT_ERR_SUCCESSS);
}

const char *ft_path_step::get_error_str() const noexcept
{
    int error_code;
    int system_error;
    const char *error_string;

    {
        ft_unique_lock<pt_recursive_mutex> guard(this->_mutex);
        if (guard.get_error() != FT_ERR_SUCCESSS)
        {
            const_cast<ft_path_step *>(this)->set_system_error(guard.get_error());
            game_pathfinding_finalize_lock(guard);
            return (ft_strerror(guard.get_error()));
        }
        error_code = this->_error_code;
        system_error = this->_system_error_code;
        game_pathfinding_finalize_lock(guard);
    }
    if (error_code != FT_ERR_SUCCESSS)
        error_string = ft_strerror(error_code);
    else if (system_error != FT_SYS_ERR_SUCCESS)
        error_string = ft_strerror(system_error);
    else
        error_string = ft_strerror(FT_ERR_SUCCESSS);
    if (error_code != FT_ERR_SUCCESSS)
        ft_set_errno_locked(error_code);
    else
        ft_set_errno_locked(system_error);
    return (error_string);
}

void ft_path_step::set_error(int error) const noexcept
{
    this->_error_code = error;
    ft_set_errno_locked(error);
    return ;
}

void ft_path_step::set_system_error(int error) const noexcept
{
    this->_system_error_code = error;
    ft_set_sys_errno_locked(error);
    return ;
}

void ft_path_step::reset_system_error() const noexcept
{
    ft_unique_lock<pt_recursive_mutex> guard(this->_mutex);

    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_system_error(guard.get_error());
        game_pathfinding_finalize_lock(guard);
        return ;
    }
    this->_system_error_code = FT_SYS_ERR_SUCCESS;
    ft_set_sys_errno_locked(FT_SYS_ERR_SUCCESS);
    game_pathfinding_finalize_lock(guard);
    return ;
}

ft_pathfinding::ft_pathfinding() noexcept
    : _error_code(FT_ERR_SUCCESSS), _current_path(), _needs_replan(false), _mutex()
{
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

ft_pathfinding::~ft_pathfinding()
{
    return ;
}

void ft_pathfinding::sleep_backoff() noexcept
{
    game_pathfinding_sleep_backoff();
    return ;
}

void ft_pathfinding::finalize_lock(ft_unique_lock<pt_mutex> &guard) noexcept
{
    game_pathfinding_finalize_lock(guard);
    return ;
}

int ft_pathfinding::lock_pair(const ft_pathfinding &first,
        const ft_pathfinding &second,
        ft_unique_lock<pt_mutex> &first_guard,
        ft_unique_lock<pt_mutex> &second_guard) noexcept
{
    const ft_pathfinding *ordered_first;
    const ft_pathfinding *ordered_second;
    bool swapped;

    if (&first == &second)
    {
        ft_unique_lock<pt_mutex> single_guard(first._mutex);

        if (single_guard.get_error() != FT_ERR_SUCCESSS)
        {
            ft_errno = single_guard.get_error();
            return (single_guard.get_error());
        }
        first_guard = ft_move(single_guard);
        second_guard = ft_unique_lock<pt_mutex>();
        ft_errno = FT_ERR_SUCCESSS;
        return (FT_ERR_SUCCESSS);
    }
    ordered_first = &first;
    ordered_second = &second;
    swapped = false;
    if (ordered_first > ordered_second)
    {
        const ft_pathfinding *temporary;

        temporary = ordered_first;
        ordered_first = ordered_second;
        ordered_second = temporary;
        swapped = true;
    }
    ft_unique_lock<pt_mutex> lower_guard(ordered_first->_mutex);
    if (lower_guard.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = lower_guard.get_error();
        return (lower_guard.get_error());
    }
    ft_unique_lock<pt_mutex> upper_guard(ordered_second->_mutex);
    if (upper_guard.get_error() != FT_ERR_SUCCESSS)
    {
        if (lower_guard.owns_lock())
            lower_guard.unlock();
        ft_errno = upper_guard.get_error();
        return (upper_guard.get_error());
    }
    if (!swapped)
    {
        first_guard = ft_move(lower_guard);
        second_guard = ft_move(upper_guard);
    }
    else
    {
        first_guard = ft_move(upper_guard);
        second_guard = ft_move(lower_guard);
    }
    ft_errno = FT_ERR_SUCCESSS;
    return (FT_ERR_SUCCESSS);
}

ft_pathfinding::ft_pathfinding(const ft_pathfinding &other) noexcept
    : _error_code(FT_ERR_SUCCESSS), _current_path(), _needs_replan(false), _mutex()
{
    int other_error;
    size_t index;

    other_error = other.get_error();
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    ft_unique_lock<pt_mutex> this_guard(this->_mutex);

    if (other_guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(other_guard.get_error());
        ft_pathfinding::finalize_lock(other_guard);
        ft_pathfinding::finalize_lock(this_guard);
        return ;
    }
    if (this_guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this_guard.get_error());
        ft_pathfinding::finalize_lock(other_guard);
        ft_pathfinding::finalize_lock(this_guard);
        return ;
    }
    index = 0;
    while (index < other._current_path.size())
    {
        this->_current_path.push_back(other._current_path[index]);
        if (this->_current_path.get_error() != FT_ERR_SUCCESSS)
        {
            this->set_error(this->_current_path.get_error());
            ft_pathfinding::finalize_lock(other_guard);
            ft_pathfinding::finalize_lock(this_guard);
            return ;
        }
        index += 1;
    }
    this->_needs_replan = other._needs_replan;
    this->set_error(other_error);
    ft_pathfinding::finalize_lock(other_guard);
    ft_pathfinding::finalize_lock(this_guard);
    return ;
}

ft_pathfinding &ft_pathfinding::operator=(const ft_pathfinding &other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;
    int other_error;
    size_t index;

    if (this == &other)
        return (*this);
    other_error = other.get_error();
    lock_error = ft_pathfinding::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_current_path.clear();
    index = 0;
    while (index < other._current_path.size())
    {
        this->_current_path.push_back(other._current_path[index]);
        if (this->_current_path.get_error() != FT_ERR_SUCCESSS)
        {
            this->set_error(this->_current_path.get_error());
            ft_pathfinding::finalize_lock(this_guard);
            ft_pathfinding::finalize_lock(other_guard);
            return (*this);
        }
        index += 1;
    }
    this->_needs_replan = other._needs_replan;
    this->set_error(other_error);
    ft_pathfinding::finalize_lock(this_guard);
    ft_pathfinding::finalize_lock(other_guard);
    return (*this);
}

ft_pathfinding::ft_pathfinding(ft_pathfinding &&other) noexcept
    : _error_code(FT_ERR_SUCCESSS), _current_path(), _needs_replan(false), _mutex()
{
    int other_error;

    other_error = other.get_error();
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    ft_unique_lock<pt_mutex> this_guard(this->_mutex);

    if (other_guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(other_guard.get_error());
        ft_pathfinding::finalize_lock(other_guard);
        ft_pathfinding::finalize_lock(this_guard);
        return ;
    }
    if (this_guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this_guard.get_error());
        ft_pathfinding::finalize_lock(other_guard);
        ft_pathfinding::finalize_lock(this_guard);
        return ;
    }
    this->_current_path = ft_move(other._current_path);
    if (this->_current_path.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_current_path.get_error());
        other.set_error(this->_current_path.get_error());
        ft_pathfinding::finalize_lock(other_guard);
        ft_pathfinding::finalize_lock(this_guard);
        return ;
    }
    this->set_error(other_error);
    this->_needs_replan = other._needs_replan;
    other.set_error(FT_ERR_SUCCESSS);
    other._needs_replan = false;
    other.set_error(FT_ERR_SUCCESSS);
    ft_pathfinding::finalize_lock(other_guard);
    ft_pathfinding::finalize_lock(this_guard);
    return ;
}

ft_pathfinding &ft_pathfinding::operator=(ft_pathfinding &&other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int lock_error;
    int other_error;

    if (this == &other)
        return (*this);
    other_error = other.get_error();
    lock_error = ft_pathfinding::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_current_path = ft_move(other._current_path);
    if (this->_current_path.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(this->_current_path.get_error());
        other.set_error(this->_current_path.get_error());
        ft_pathfinding::finalize_lock(this_guard);
        ft_pathfinding::finalize_lock(other_guard);
        return (*this);
    }
    this->set_error(other_error);
    this->_needs_replan = other._needs_replan;
    other.set_error(FT_ERR_SUCCESSS);
    other._needs_replan = false;
    other.set_error(FT_ERR_SUCCESSS);
    ft_pathfinding::finalize_lock(this_guard);
    ft_pathfinding::finalize_lock(other_guard);
    return (*this);
}

void ft_pathfinding::set_error(int error) const noexcept
{

    ft_errno = error;
    this->_error_code = error;
    return ;
}

void ft_pathfinding::update_obstacle(size_t x, size_t y, size_t z, int value) noexcept
{
    bool needs_replan;
    int result_error;
    size_t index;
    ft_vector<ft_path_step> path_snapshot;

    (void)value;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        ft_pathfinding::finalize_lock(guard);
        return ;
    }
    index = 0;
    while (index < this->_current_path.size())
    {
        path_snapshot.push_back(this->_current_path[index]);
        if (path_snapshot.get_error() != FT_ERR_SUCCESSS)
        {
            this->set_error(path_snapshot.get_error());
            ft_pathfinding::finalize_lock(guard);
            return ;
        }
        index += 1;
    }
    ft_pathfinding::finalize_lock(guard);
    index = 0;
    needs_replan = false;
    result_error = FT_ERR_SUCCESSS;
    while (index < path_snapshot.size())
    {
        ft_path_step &step = path_snapshot[index];
        size_t step_x;
        size_t step_y;
        size_t step_z;

        step_x = step.get_x();
        if (step.get_error() != FT_ERR_SUCCESSS)
        {
            result_error = step.get_error();
            break ;
        }
        step_y = step.get_y();
        if (step.get_error() != FT_ERR_SUCCESSS)
        {
            result_error = step.get_error();
            break ;
        }
        step_z = step.get_z();
        if (step.get_error() != FT_ERR_SUCCESSS)
        {
            result_error = step.get_error();
            break ;
        }
        if (step_x == x && step_y == y && step_z == z)
            needs_replan = true;
        index += 1;
    }
    guard = ft_unique_lock<pt_mutex>(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        ft_pathfinding::finalize_lock(guard);
        return ;
    }
    if (needs_replan)
        this->_needs_replan = true;
    this->set_error(result_error);
    ft_pathfinding::finalize_lock(guard);
    return ;
}

int ft_pathfinding::recalculate_path(const ft_map3d &grid,
    size_t start_x, size_t start_y, size_t start_z,
    size_t goal_x, size_t goal_y, size_t goal_z,
    ft_vector<ft_path_step> &out_path) noexcept
{
    int unlock_error;
    int result;
    int result_error;
    size_t index;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        ft_pathfinding::finalize_lock(guard);
        return (guard.get_error());
    }
    if (!this->_needs_replan && this->_current_path.size() > 0)
    {
        out_path.clear();
        index = 0;
        while (index < this->_current_path.size())
        {
            out_path.push_back(this->_current_path[index]);
            if (out_path.get_error() != FT_ERR_SUCCESSS)
            {
                this->set_error(out_path.get_error());
                ft_pathfinding::finalize_lock(guard);
                return (out_path.get_error());
            }
            index += 1;
        }
        result_error = FT_ERR_SUCCESSS;
        this->set_error(result_error);
        ft_pathfinding::finalize_lock(guard);
        return (result_error);
    }
    unlock_error = FT_ERR_SUCCESSS;
    if (guard.owns_lock())
    {
        guard.unlock();
        unlock_error = guard.get_error();
    }
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        this->set_error(unlock_error);
        return (unlock_error);
    }
    result = this->astar_grid(grid, start_x, start_y, start_z,
            goal_x, goal_y, goal_z, out_path);
    guard.lock();
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(guard.get_error());
        ft_pathfinding::finalize_lock(guard);
        return (guard.get_error());
    }
    if (result == FT_ERR_SUCCESSS)
    {
        this->_current_path.clear();
        index = 0;
        while (index < out_path.size())
        {
            this->_current_path.push_back(out_path[index]);
            if (this->_current_path.get_error() != FT_ERR_SUCCESSS)
            {
                this->_needs_replan = true;
                this->set_error(this->_current_path.get_error());
                ft_pathfinding::finalize_lock(guard);
                return (this->_current_path.get_error());
            }
            index += 1;
        }
        this->_needs_replan = false;
        result_error = FT_ERR_SUCCESSS;
        this->set_error(result_error);
    }
    else
    {
        this->_needs_replan = true;
        result_error = result;
        this->set_error(result_error);
    }
    ft_pathfinding::finalize_lock(guard);
    return (result);
}

int ft_pathfinding::astar_grid(const ft_map3d &grid,
    size_t start_x, size_t start_y, size_t start_z,
    size_t goal_x, size_t goal_y, size_t goal_z,
    ft_vector<ft_path_step> &out_path) const noexcept
{
    int result_error;
    size_t grid_width;
    size_t grid_height;
    size_t grid_depth;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_pathfinding *>(this)->set_error(guard.get_error());
        ft_pathfinding::finalize_lock(guard);
        return (guard.get_error());
    }
    out_path.clear();
    grid_width = grid.get_width();
    grid_height = grid.get_height();
    grid_depth = grid.get_depth();
    if (start_x >= grid_width || start_y >= grid_height || start_z >= grid_depth ||
        goal_x >= grid_width || goal_y >= grid_height || goal_z >= grid_depth)
    {
        const_cast<ft_pathfinding *>(this)->set_error(FT_ERR_GAME_INVALID_MOVE);
        result_error = FT_ERR_GAME_INVALID_MOVE;
        ft_pathfinding::finalize_lock(guard);
        return (result_error);
    }
    struct node
    {
        size_t  x;
        size_t  y;
        size_t  z;
        int     g;
        int     f;
        int     parent;
    };
    ft_vector<node> nodes;
    ft_vector<size_t> open_set;
    node start;
    size_t dx;
    size_t dy;
    size_t dz;

    start.x = start_x;
    start.y = start_y;
    start.z = start_z;
    start.g = 0;
    dx = distance_component(start_x, goal_x);
    dy = distance_component(start_y, goal_y);
    dz = distance_component(start_z, goal_z);
    start.f = static_cast<int>(dx + dy + dz);
    start.parent = -1;
    nodes.push_back(start);
    if (nodes.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_pathfinding *>(this)->set_error(nodes.get_error());
        ft_pathfinding::finalize_lock(guard);
        return (nodes.get_error());
    }
    open_set.push_back(0);
    if (open_set.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_pathfinding *>(this)->set_error(open_set.get_error());
        ft_pathfinding::finalize_lock(guard);
        return (open_set.get_error());
    }
    while (open_set.size() > 0)
    {
        size_t open_index;
        size_t best_open;
        size_t current_index;
        ft_path_step current_step;
        bool found_goal;

        open_index = 0;
        best_open = 0;
        while (open_index < open_set.size())
        {
            size_t current_open;
            size_t best_candidate;

            current_open = open_set[open_index];
            best_candidate = open_set[best_open];
            if (nodes[current_open].f < nodes[best_candidate].f)
                best_open = open_index;
            open_index += 1;
        }
        current_index = open_set[best_open];
        if (current_step.set_coordinates(nodes[current_index].x,
                nodes[current_index].y, nodes[current_index].z) != FT_ERR_SUCCESSS)
        {
            const_cast<ft_pathfinding *>(this)->set_error(current_step.get_error());
            ft_pathfinding::finalize_lock(guard);
            return (current_step.get_error());
        }
        found_goal = false;
        if (current_step.get_x() == goal_x)
        {
            if (current_step.get_error() != FT_ERR_SUCCESSS)
            {
                const_cast<ft_pathfinding *>(this)->set_error(current_step.get_error());
                ft_pathfinding::finalize_lock(guard);
                return (current_step.get_error());
            }
            if (current_step.get_y() == goal_y)
            {
                if (current_step.get_error() != FT_ERR_SUCCESSS)
                {
                    const_cast<ft_pathfinding *>(this)->set_error(current_step.get_error());
                    ft_pathfinding::finalize_lock(guard);
                    return (current_step.get_error());
                }
                if (current_step.get_z() == goal_z)
                {
                    if (current_step.get_error() != FT_ERR_SUCCESSS)
                    {
                        const_cast<ft_pathfinding *>(this)->set_error(current_step.get_error());
                        ft_pathfinding::finalize_lock(guard);
                        return (current_step.get_error());
                    }
                    found_goal = true;
                }
            }
        }
        if (found_goal)
        {
            ft_vector<ft_path_step> reverse_path;
            size_t trace_index;

            trace_index = current_index;
            while (true)
            {
                ft_path_step step;

                if (step.set_coordinates(nodes[trace_index].x,
                        nodes[trace_index].y, nodes[trace_index].z) != FT_ERR_SUCCESSS)
                {
                    const_cast<ft_pathfinding *>(this)->set_error(step.get_error());
                    ft_pathfinding::finalize_lock(guard);
                    return (step.get_error());
                }
                reverse_path.push_back(step);
                if (reverse_path.get_error() != FT_ERR_SUCCESSS)
                {
                    const_cast<ft_pathfinding *>(this)->set_error(reverse_path.get_error());
                    ft_pathfinding::finalize_lock(guard);
                    return (reverse_path.get_error());
                }
                if (nodes[trace_index].parent == -1)
                    break;
                trace_index = static_cast<size_t>(nodes[trace_index].parent);
            }
            size_t rev_index;

            rev_index = reverse_path.size();
            while (rev_index > 0)
            {
                ft_path_step step;

                rev_index -= 1;
                step = reverse_path[rev_index];
                out_path.push_back(step);
                if (out_path.get_error() != FT_ERR_SUCCESSS)
                {
                    const_cast<ft_pathfinding *>(this)->set_error(out_path.get_error());
                    ft_pathfinding::finalize_lock(guard);
                    return (out_path.get_error());
                }
            }
            result_error = FT_ERR_SUCCESSS;
            const_cast<ft_pathfinding *>(this)->set_error(result_error);
            ft_pathfinding::finalize_lock(guard);
            return (result_error);
        }
        open_set.erase(open_set.begin() + best_open);
        if (open_set.get_error() != FT_ERR_SUCCESSS)
        {
            const_cast<ft_pathfinding *>(this)->set_error(open_set.get_error());
            ft_pathfinding::finalize_lock(guard);
            return (open_set.get_error());
        }
        int dir_x[6] = {1, -1, 0, 0, 0, 0};
        int dir_y[6] = {0, 0, 1, -1, 0, 0};
        int dir_z[6] = {0, 0, 0, 0, 1, -1};
        size_t neighbor_index;

        neighbor_index = 0;
        while (neighbor_index < 6)
        {
            size_t neighbor_x;
            size_t neighbor_y;
            size_t neighbor_z;

            neighbor_x = nodes[current_index].x;
            if (dir_x[neighbor_index] > 0)
                neighbor_x = neighbor_x + 1;
            else if (dir_x[neighbor_index] < 0)
                neighbor_x = neighbor_x - 1;
            neighbor_y = nodes[current_index].y;
            if (dir_y[neighbor_index] > 0)
                neighbor_y = neighbor_y + 1;
            else if (dir_y[neighbor_index] < 0)
                neighbor_y = neighbor_y - 1;
            neighbor_z = nodes[current_index].z;
            if (dir_z[neighbor_index] > 0)
                neighbor_z = neighbor_z + 1;
            else if (dir_z[neighbor_index] < 0)
                neighbor_z = neighbor_z - 1;
            if (neighbor_x < grid_width && neighbor_y < grid_height && neighbor_z < grid_depth &&
                grid.get(neighbor_x, neighbor_y, neighbor_z) == 0)
            {
                size_t search_index;
                bool found;

                search_index = 0;
                found = false;
                while (search_index < nodes.size())
                {
                    if (nodes[search_index].x == neighbor_x &&
                        nodes[search_index].y == neighbor_y &&
                        nodes[search_index].z == neighbor_z)
                    {
                        found = true;
                        break;
                    }
                    search_index += 1;
                }
                int tentative_g;

                tentative_g = nodes[current_index].g + 1;
                if (found)
                {
                    if (tentative_g < nodes[search_index].g)
                    {
                        size_t ndx;
                        size_t ndy;
                        size_t ndz;
                        size_t check_index;
                        bool in_open;

                        nodes[search_index].g = tentative_g;
                        ndx = distance_component(neighbor_x, goal_x);
                        ndy = distance_component(neighbor_y, goal_y);
                        ndz = distance_component(neighbor_z, goal_z);
                        nodes[search_index].f = tentative_g + static_cast<int>(ndx + ndy + ndz);
                        nodes[search_index].parent = static_cast<int>(current_index);
                        check_index = 0;
                        in_open = false;
                        while (check_index < open_set.size())
                        {
                            if (open_set[check_index] == search_index)
                                in_open = true;
                            check_index += 1;
                        }
                        if (!in_open)
                        {
                            open_set.push_back(search_index);
                            if (open_set.get_error() != FT_ERR_SUCCESSS)
                            {
                                const_cast<ft_pathfinding *>(this)->set_error(open_set.get_error());
                                ft_pathfinding::finalize_lock(guard);
                                return (open_set.get_error());
                            }
                        }
                    }
                }
                else
                {
                    node new_node;
                    size_t ndx;
                    size_t ndy;
                    size_t ndz;

                    new_node.x = neighbor_x;
                    new_node.y = neighbor_y;
                    new_node.z = neighbor_z;
                    new_node.g = tentative_g;
                    ndx = distance_component(neighbor_x, goal_x);
                    ndy = distance_component(neighbor_y, goal_y);
                    ndz = distance_component(neighbor_z, goal_z);
                    new_node.f = tentative_g + static_cast<int>(ndx + ndy + ndz);
                    new_node.parent = static_cast<int>(current_index);
                    nodes.push_back(new_node);
                    if (nodes.get_error() != FT_ERR_SUCCESSS)
                    {
                        const_cast<ft_pathfinding *>(this)->set_error(nodes.get_error());
                        ft_pathfinding::finalize_lock(guard);
                        return (nodes.get_error());
                    }
                    open_set.push_back(nodes.size() - 1);
                    if (open_set.get_error() != FT_ERR_SUCCESSS)
                    {
                        const_cast<ft_pathfinding *>(this)->set_error(open_set.get_error());
                        ft_pathfinding::finalize_lock(guard);
                        return (open_set.get_error());
                    }
                }
            }
            neighbor_index += 1;
        }
    }
    const_cast<ft_pathfinding *>(this)->set_error(FT_ERR_GAME_INVALID_MOVE);
    result_error = FT_ERR_GAME_INVALID_MOVE;
    ft_pathfinding::finalize_lock(guard);
    return (result_error);
}

int ft_pathfinding::dijkstra_graph(const ft_graph<int> &graph,
    size_t start_vertex, size_t goal_vertex,
    ft_vector<size_t> &out_path) const noexcept
{
    int result_error;
    size_t graph_size;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_pathfinding *>(this)->set_error(guard.get_error());
        ft_pathfinding::finalize_lock(guard);
        return (guard.get_error());
    }
    out_path.clear();
    graph_size = graph.size();
    if (start_vertex >= graph_size || goal_vertex >= graph_size)
    {
        const_cast<ft_pathfinding *>(this)->set_error(FT_ERR_GAME_INVALID_MOVE);
        result_error = FT_ERR_GAME_INVALID_MOVE;
        ft_pathfinding::finalize_lock(guard);
        return (result_error);
    }
    ft_vector<int> distance;
    ft_vector<int> previous;
    ft_vector<size_t> queue;
    size_t neighbor_index;

    distance.resize(graph_size, -1);
    if (distance.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_pathfinding *>(this)->set_error(distance.get_error());
        ft_pathfinding::finalize_lock(guard);
        return (distance.get_error());
    }
    previous.resize(graph_size, -1);
    if (previous.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_pathfinding *>(this)->set_error(previous.get_error());
        ft_pathfinding::finalize_lock(guard);
        return (previous.get_error());
    }
    queue.push_back(start_vertex);
    if (queue.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_pathfinding *>(this)->set_error(queue.get_error());
        ft_pathfinding::finalize_lock(guard);
        return (queue.get_error());
    }
    distance[start_vertex] = 0;
    while (queue.size() > 0)
    {
        size_t queue_index;
        size_t best_queue;
        size_t current;

        queue_index = 0;
        best_queue = 0;
        while (queue_index < queue.size())
        {
            if (distance[queue[queue_index]] < distance[queue[best_queue]])
                best_queue = queue_index;
            queue_index += 1;
        }
        current = queue[best_queue];
        queue.erase(queue.begin() + best_queue);
        if (queue.get_error() != FT_ERR_SUCCESSS)
        {
            const_cast<ft_pathfinding *>(this)->set_error(queue.get_error());
            ft_pathfinding::finalize_lock(guard);
            return (queue.get_error());
        }
        if (current == goal_vertex)
            break;
        ft_vector<size_t> neighbors;

        graph.neighbors(current, neighbors);
        if (neighbors.get_error() != FT_ERR_SUCCESSS)
        {
            const_cast<ft_pathfinding *>(this)->set_error(neighbors.get_error());
            ft_pathfinding::finalize_lock(guard);
            return (neighbors.get_error());
        }
        neighbor_index = 0;
        while (neighbor_index < neighbors.size())
        {
            size_t neighbor;
            int alt;

            neighbor = neighbors[neighbor_index];
            alt = distance[current] + 1;
            if (distance[neighbor] == -1 || alt < distance[neighbor])
            {
                size_t search_index;
                bool found;

                distance[neighbor] = alt;
                previous[neighbor] = static_cast<int>(current);
                search_index = 0;
                found = false;
                while (search_index < queue.size())
                {
                    if (queue[search_index] == neighbor)
                        found = true;
                    search_index += 1;
                }
                if (!found)
                {
                    queue.push_back(neighbor);
                    if (queue.get_error() != FT_ERR_SUCCESSS)
                    {
                        const_cast<ft_pathfinding *>(this)->set_error(queue.get_error());
                        ft_pathfinding::finalize_lock(guard);
                        return (queue.get_error());
                    }
                }
            }
            neighbor_index += 1;
        }
    }
    if (distance[goal_vertex] == -1)
    {
        const_cast<ft_pathfinding *>(this)->set_error(FT_ERR_GAME_INVALID_MOVE);
        result_error = FT_ERR_GAME_INVALID_MOVE;
        ft_pathfinding::finalize_lock(guard);
        return (result_error);
    }
    ft_vector<size_t> reverse_path;
    size_t vertex;

    vertex = goal_vertex;
    while (true)
    {
        reverse_path.push_back(vertex);
        if (reverse_path.get_error() != FT_ERR_SUCCESSS)
        {
            const_cast<ft_pathfinding *>(this)->set_error(reverse_path.get_error());
            ft_pathfinding::finalize_lock(guard);
            return (reverse_path.get_error());
        }
        if (vertex == start_vertex)
            break;
        vertex = static_cast<size_t>(previous[vertex]);
    }
    size_t rev_index;

    rev_index = reverse_path.size();
    while (rev_index > 0)
    {
        rev_index -= 1;
        out_path.push_back(reverse_path[rev_index]);
        if (out_path.get_error() != FT_ERR_SUCCESSS)
        {
            const_cast<ft_pathfinding *>(this)->set_error(out_path.get_error());
            ft_pathfinding::finalize_lock(guard);
            return (out_path.get_error());
        }
    }
    result_error = FT_ERR_SUCCESSS;
    const_cast<ft_pathfinding *>(this)->set_error(result_error);
    ft_pathfinding::finalize_lock(guard);
    return (result_error);
}

int ft_pathfinding::get_error() const noexcept
{
    int error_code;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_pathfinding *>(this)->set_error(guard.get_error());
        ft_pathfinding::finalize_lock(guard);
        return (guard.get_error());
    }
    error_code = this->_error_code;
    const_cast<ft_pathfinding *>(this)->set_error(error_code);
    ft_pathfinding::finalize_lock(guard);
    return (error_code);
}

const char *ft_pathfinding::get_error_str() const noexcept
{
    int error_code;

    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ERR_SUCCESSS)
    {
        const_cast<ft_pathfinding *>(this)->set_error(guard.get_error());
        ft_pathfinding::finalize_lock(guard);
        return (ft_strerror(guard.get_error()));
    }
    error_code = this->_error_code;
    const_cast<ft_pathfinding *>(this)->set_error(error_code);
    ft_pathfinding::finalize_lock(guard);
    return (ft_strerror(error_code));
}
