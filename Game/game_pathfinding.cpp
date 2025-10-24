#include "game_pathfinding.hpp"
#include <utility>
#include "../Template/move.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/pthread.hpp"

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

static void game_pathfinding_restore_errno(ft_unique_lock<pt_mutex> &guard,
        int entry_errno) noexcept
{
    if (guard.owns_lock())
        guard.unlock();
    ft_errno = entry_errno;
    return ;
}

ft_path_step::ft_path_step() noexcept
    : _x(0), _y(0), _z(0), _error_code(ER_SUCCESS), _mutex()
{
    this->set_error(ER_SUCCESS);
    return ;
}

ft_path_step::~ft_path_step() noexcept
{
    return ;
}

int ft_path_step::lock_pair(const ft_path_step &first,
        const ft_path_step &second,
        ft_unique_lock<pt_mutex> &first_guard,
        ft_unique_lock<pt_mutex> &second_guard) noexcept
{
    const ft_path_step *ordered_first;
    const ft_path_step *ordered_second;
    bool swapped;

    if (&first == &second)
    {
        ft_unique_lock<pt_mutex> single_guard(first._mutex);

        if (single_guard.get_error() != ER_SUCCESS)
        {
            ft_errno = single_guard.get_error();
            return (single_guard.get_error());
        }
        first_guard = ft_move(single_guard);
        second_guard = ft_unique_lock<pt_mutex>();
        ft_errno = ER_SUCCESS;
        return (ER_SUCCESS);
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
    while (true)
    {
        ft_unique_lock<pt_mutex> lower_guard(ordered_first->_mutex);

        if (lower_guard.get_error() != ER_SUCCESS)
        {
            ft_errno = lower_guard.get_error();
            return (lower_guard.get_error());
        }
        ft_unique_lock<pt_mutex> upper_guard(ordered_second->_mutex);
        if (upper_guard.get_error() == ER_SUCCESS)
        {
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
            ft_errno = ER_SUCCESS;
            return (ER_SUCCESS);
        }
        if (upper_guard.get_error() != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            ft_errno = upper_guard.get_error();
            return (upper_guard.get_error());
        }
        if (lower_guard.owns_lock())
            lower_guard.unlock();
        game_pathfinding_sleep_backoff();
    }
}

ft_path_step::ft_path_step(const ft_path_step &other) noexcept
    : _x(0), _y(0), _z(0), _error_code(ER_SUCCESS), _mutex()
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(other_guard.get_error());
        game_pathfinding_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_x = other._x;
    this->_y = other._y;
    this->_z = other._z;
    this->_error_code = other._error_code;
    this->set_error(other._error_code);
    game_pathfinding_restore_errno(other_guard, entry_errno);
    return ;
}

ft_path_step &ft_path_step::operator=(const ft_path_step &other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = ft_path_step::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_x = other._x;
    this->_y = other._y;
    this->_z = other._z;
    this->_error_code = other._error_code;
    this->set_error(other._error_code);
    game_pathfinding_restore_errno(this_guard, entry_errno);
    game_pathfinding_restore_errno(other_guard, entry_errno);
    return (*this);
}

ft_path_step::ft_path_step(ft_path_step &&other) noexcept
    : _x(0), _y(0), _z(0), _error_code(ER_SUCCESS), _mutex()
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(other_guard.get_error());
        game_pathfinding_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_x = other._x;
    this->_y = other._y;
    this->_z = other._z;
    this->_error_code = other._error_code;
    other._x = 0;
    other._y = 0;
    other._z = 0;
    other._error_code = ER_SUCCESS;
    this->set_error(this->_error_code);
    other.set_error(ER_SUCCESS);
    game_pathfinding_restore_errno(other_guard, entry_errno);
    return ;
}

ft_path_step &ft_path_step::operator=(ft_path_step &&other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = ft_path_step::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_x = other._x;
    this->_y = other._y;
    this->_z = other._z;
    this->_error_code = other._error_code;
    other._x = 0;
    other._y = 0;
    other._z = 0;
    other._error_code = ER_SUCCESS;
    this->set_error(this->_error_code);
    other.set_error(ER_SUCCESS);
    game_pathfinding_restore_errno(this_guard, entry_errno);
    game_pathfinding_restore_errno(other_guard, entry_errno);
    return (*this);
}

int ft_path_step::set_coordinates(size_t x, size_t y, size_t z) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_pathfinding_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    this->_x = x;
    this->_y = y;
    this->_z = z;
    this->set_error(ER_SUCCESS);
    game_pathfinding_restore_errno(guard, entry_errno);
    return (ER_SUCCESS);
}

int ft_path_step::set_x(size_t x) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_pathfinding_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    this->_x = x;
    this->set_error(ER_SUCCESS);
    game_pathfinding_restore_errno(guard, entry_errno);
    return (ER_SUCCESS);
}

int ft_path_step::set_y(size_t y) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_pathfinding_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    this->_y = y;
    this->set_error(ER_SUCCESS);
    game_pathfinding_restore_errno(guard, entry_errno);
    return (ER_SUCCESS);
}

int ft_path_step::set_z(size_t z) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_pathfinding_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    this->_z = z;
    this->set_error(ER_SUCCESS);
    game_pathfinding_restore_errno(guard, entry_errno);
    return (ER_SUCCESS);
}

size_t ft_path_step::get_x() const noexcept
{
    int entry_errno;
    size_t value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_path_step *>(this)->set_error(guard.get_error());
        game_pathfinding_restore_errno(guard, entry_errno);
        return (0);
    }
    value = this->_x;
    const_cast<ft_path_step *>(this)->set_error(ER_SUCCESS);
    game_pathfinding_restore_errno(guard, entry_errno);
    return (value);
}

size_t ft_path_step::get_y() const noexcept
{
    int entry_errno;
    size_t value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_path_step *>(this)->set_error(guard.get_error());
        game_pathfinding_restore_errno(guard, entry_errno);
        return (0);
    }
    value = this->_y;
    const_cast<ft_path_step *>(this)->set_error(ER_SUCCESS);
    game_pathfinding_restore_errno(guard, entry_errno);
    return (value);
}

size_t ft_path_step::get_z() const noexcept
{
    int entry_errno;
    size_t value;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_path_step *>(this)->set_error(guard.get_error());
        game_pathfinding_restore_errno(guard, entry_errno);
        return (0);
    }
    value = this->_z;
    const_cast<ft_path_step *>(this)->set_error(ER_SUCCESS);
    game_pathfinding_restore_errno(guard, entry_errno);
    return (value);
}

int ft_path_step::get_error() const noexcept
{
    int entry_errno;
    int error_code;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_path_step *>(this)->set_error(guard.get_error());
        game_pathfinding_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    error_code = this->_error_code;
    const_cast<ft_path_step *>(this)->set_error(error_code);
    game_pathfinding_restore_errno(guard, entry_errno);
    return (error_code);
}

const char *ft_path_step::get_error_str() const noexcept
{
    int entry_errno;
    int error_code;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_path_step *>(this)->set_error(guard.get_error());
        game_pathfinding_restore_errno(guard, entry_errno);
        return (ft_strerror(guard.get_error()));
    }
    error_code = this->_error_code;
    const_cast<ft_path_step *>(this)->set_error(error_code);
    game_pathfinding_restore_errno(guard, entry_errno);
    return (ft_strerror(error_code));
}

void ft_path_step::set_error(int error) const noexcept
{
    ft_errno = error;
    this->_error_code = error;
    return ;
}

ft_pathfinding::ft_pathfinding() noexcept
    : _error_code(ER_SUCCESS), _current_path(), _needs_replan(false), _mutex()
{
    this->set_error(ER_SUCCESS);
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

void ft_pathfinding::restore_errno(ft_unique_lock<pt_mutex> &guard,
        int entry_errno) noexcept
{
    game_pathfinding_restore_errno(guard, entry_errno);
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

        if (single_guard.get_error() != ER_SUCCESS)
        {
            ft_errno = single_guard.get_error();
            return (single_guard.get_error());
        }
        first_guard = ft_move(single_guard);
        second_guard = ft_unique_lock<pt_mutex>();
        ft_errno = ER_SUCCESS;
        return (ER_SUCCESS);
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
    while (true)
    {
        ft_unique_lock<pt_mutex> lower_guard(ordered_first->_mutex);

        if (lower_guard.get_error() != ER_SUCCESS)
        {
            ft_errno = lower_guard.get_error();
            return (lower_guard.get_error());
        }
        ft_unique_lock<pt_mutex> upper_guard(ordered_second->_mutex);
        if (upper_guard.get_error() == ER_SUCCESS)
        {
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
            ft_errno = ER_SUCCESS;
            return (ER_SUCCESS);
        }
        if (upper_guard.get_error() != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            ft_errno = upper_guard.get_error();
            return (upper_guard.get_error());
        }
        if (lower_guard.owns_lock())
            lower_guard.unlock();
        ft_pathfinding::sleep_backoff();
    }
}

ft_pathfinding::ft_pathfinding(const ft_pathfinding &other) noexcept
    : _error_code(ER_SUCCESS), _current_path(), _needs_replan(false), _mutex()
{
    int entry_errno;
    size_t index;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(other_guard.get_error());
        ft_pathfinding::restore_errno(other_guard, entry_errno);
        return ;
    }
    index = 0;
    while (index < other._current_path.size())
    {
        this->_current_path.push_back(other._current_path[index]);
        if (this->_current_path.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_current_path.get_error());
            ft_pathfinding::restore_errno(other_guard, entry_errno);
            return ;
        }
        index += 1;
    }
    this->_needs_replan = other._needs_replan;
    this->_error_code = other._error_code;
    this->set_error(other._error_code);
    ft_pathfinding::restore_errno(other_guard, entry_errno);
    return ;
}

ft_pathfinding &ft_pathfinding::operator=(const ft_pathfinding &other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;
    size_t index;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = ft_pathfinding::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_current_path.clear();
    index = 0;
    while (index < other._current_path.size())
    {
        this->_current_path.push_back(other._current_path[index]);
        if (this->_current_path.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_current_path.get_error());
            ft_pathfinding::restore_errno(this_guard, entry_errno);
            ft_pathfinding::restore_errno(other_guard, entry_errno);
            return (*this);
        }
        index += 1;
    }
    this->_error_code = other._error_code;
    this->_needs_replan = other._needs_replan;
    this->set_error(other._error_code);
    ft_pathfinding::restore_errno(this_guard, entry_errno);
    ft_pathfinding::restore_errno(other_guard, entry_errno);
    return (*this);
}

ft_pathfinding::ft_pathfinding(ft_pathfinding &&other) noexcept
    : _error_code(ER_SUCCESS), _current_path(), _needs_replan(false), _mutex()
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(other_guard.get_error());
        ft_pathfinding::restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_current_path = ft_move(other._current_path);
    if (this->_current_path.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_current_path.get_error());
        other.set_error(this->_current_path.get_error());
        ft_pathfinding::restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_error_code = other._error_code;
    this->_needs_replan = other._needs_replan;
    other._error_code = ER_SUCCESS;
    other._needs_replan = false;
    this->set_error(this->_error_code);
    other.set_error(ER_SUCCESS);
    ft_pathfinding::restore_errno(other_guard, entry_errno);
    return ;
}

ft_pathfinding &ft_pathfinding::operator=(ft_pathfinding &&other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = ft_pathfinding::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != ER_SUCCESS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_current_path = ft_move(other._current_path);
    if (this->_current_path.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_current_path.get_error());
        other.set_error(this->_current_path.get_error());
        ft_pathfinding::restore_errno(this_guard, entry_errno);
        ft_pathfinding::restore_errno(other_guard, entry_errno);
        return (*this);
    }
    this->_error_code = other._error_code;
    this->_needs_replan = other._needs_replan;
    other._error_code = ER_SUCCESS;
    other._needs_replan = false;
    this->set_error(this->_error_code);
    other.set_error(ER_SUCCESS);
    ft_pathfinding::restore_errno(this_guard, entry_errno);
    ft_pathfinding::restore_errno(other_guard, entry_errno);
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
    int entry_errno;
    size_t index;

    (void)value;
    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        ft_pathfinding::restore_errno(guard, entry_errno);
        return ;
    }
    index = 0;
    while (index < this->_current_path.size())
    {
        ft_path_step &step = this->_current_path[index];
        size_t step_x;
        size_t step_y;
        size_t step_z;

        step_x = step.get_x();
        if (step.get_error() != ER_SUCCESS)
        {
            this->set_error(step.get_error());
            ft_pathfinding::restore_errno(guard, entry_errno);
            return ;
        }
        step_y = step.get_y();
        if (step.get_error() != ER_SUCCESS)
        {
            this->set_error(step.get_error());
            ft_pathfinding::restore_errno(guard, entry_errno);
            return ;
        }
        step_z = step.get_z();
        if (step.get_error() != ER_SUCCESS)
        {
            this->set_error(step.get_error());
            ft_pathfinding::restore_errno(guard, entry_errno);
            return ;
        }
        if (step_x == x && step_y == y && step_z == z)
            this->_needs_replan = true;
        index += 1;
    }
    this->set_error(ER_SUCCESS);
    ft_pathfinding::restore_errno(guard, entry_errno);
    return ;
}

int ft_pathfinding::recalculate_path(const ft_map3d &grid,
    size_t start_x, size_t start_y, size_t start_z,
    size_t goal_x, size_t goal_y, size_t goal_z,
    ft_vector<ft_path_step> &out_path) noexcept
{
    int entry_errno;
    int unlock_error;
    int result;
    size_t index;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        ft_pathfinding::restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    if (!this->_needs_replan && this->_current_path.size() > 0)
    {
        out_path.clear();
        index = 0;
        while (index < this->_current_path.size())
        {
            out_path.push_back(this->_current_path[index]);
            if (out_path.get_error() != ER_SUCCESS)
            {
                this->set_error(out_path.get_error());
                ft_pathfinding::restore_errno(guard, entry_errno);
                return (out_path.get_error());
            }
            index += 1;
        }
        this->set_error(ER_SUCCESS);
        ft_pathfinding::restore_errno(guard, entry_errno);
        return (ER_SUCCESS);
    }
    unlock_error = ER_SUCCESS;
    if (guard.owns_lock())
    {
        guard.unlock();
        unlock_error = guard.get_error();
    }
    if (unlock_error != ER_SUCCESS)
    {
        this->set_error(unlock_error);
        ft_errno = entry_errno;
        return (unlock_error);
    }
    ft_errno = entry_errno;
    result = this->astar_grid(grid, start_x, start_y, start_z,
            goal_x, goal_y, goal_z, out_path);
    guard.lock();
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        ft_pathfinding::restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    if (result == ER_SUCCESS)
    {
        this->_current_path.clear();
        index = 0;
        while (index < out_path.size())
        {
            this->_current_path.push_back(out_path[index]);
            if (this->_current_path.get_error() != ER_SUCCESS)
            {
                this->_needs_replan = true;
                this->set_error(this->_current_path.get_error());
                ft_pathfinding::restore_errno(guard, entry_errno);
                return (this->_current_path.get_error());
            }
            index += 1;
        }
        this->_needs_replan = false;
        this->set_error(ER_SUCCESS);
    }
    else
    {
        this->_needs_replan = true;
        this->set_error(result);
    }
    ft_pathfinding::restore_errno(guard, entry_errno);
    return (result);
}

int ft_pathfinding::astar_grid(const ft_map3d &grid,
    size_t start_x, size_t start_y, size_t start_z,
    size_t goal_x, size_t goal_y, size_t goal_z,
    ft_vector<ft_path_step> &out_path) const noexcept
{
    int entry_errno;
    size_t grid_width;
    size_t grid_height;
    size_t grid_depth;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_pathfinding *>(this)->set_error(guard.get_error());
        ft_pathfinding::restore_errno(guard, entry_errno);
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
        ft_pathfinding::restore_errno(guard, entry_errno);
        return (const_cast<ft_pathfinding *>(this)->_error_code);
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
    if (nodes.get_error() != ER_SUCCESS)
    {
        const_cast<ft_pathfinding *>(this)->set_error(nodes.get_error());
        ft_pathfinding::restore_errno(guard, entry_errno);
        return (nodes.get_error());
    }
    open_set.push_back(0);
    if (open_set.get_error() != ER_SUCCESS)
    {
        const_cast<ft_pathfinding *>(this)->set_error(open_set.get_error());
        ft_pathfinding::restore_errno(guard, entry_errno);
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
                nodes[current_index].y, nodes[current_index].z) != ER_SUCCESS)
        {
            const_cast<ft_pathfinding *>(this)->set_error(current_step.get_error());
            ft_pathfinding::restore_errno(guard, entry_errno);
            return (current_step.get_error());
        }
        found_goal = false;
        if (current_step.get_x() == goal_x)
        {
            if (current_step.get_error() != ER_SUCCESS)
            {
                const_cast<ft_pathfinding *>(this)->set_error(current_step.get_error());
                ft_pathfinding::restore_errno(guard, entry_errno);
                return (current_step.get_error());
            }
            if (current_step.get_y() == goal_y)
            {
                if (current_step.get_error() != ER_SUCCESS)
                {
                    const_cast<ft_pathfinding *>(this)->set_error(current_step.get_error());
                    ft_pathfinding::restore_errno(guard, entry_errno);
                    return (current_step.get_error());
                }
                if (current_step.get_z() == goal_z)
                {
                    if (current_step.get_error() != ER_SUCCESS)
                    {
                        const_cast<ft_pathfinding *>(this)->set_error(current_step.get_error());
                        ft_pathfinding::restore_errno(guard, entry_errno);
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
                        nodes[trace_index].y, nodes[trace_index].z) != ER_SUCCESS)
                {
                    const_cast<ft_pathfinding *>(this)->set_error(step.get_error());
                    ft_pathfinding::restore_errno(guard, entry_errno);
                    return (step.get_error());
                }
                reverse_path.push_back(step);
                if (reverse_path.get_error() != ER_SUCCESS)
                {
                    const_cast<ft_pathfinding *>(this)->set_error(reverse_path.get_error());
                    ft_pathfinding::restore_errno(guard, entry_errno);
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
                if (out_path.get_error() != ER_SUCCESS)
                {
                    const_cast<ft_pathfinding *>(this)->set_error(out_path.get_error());
                    ft_pathfinding::restore_errno(guard, entry_errno);
                    return (out_path.get_error());
                }
            }
            const_cast<ft_pathfinding *>(this)->set_error(ER_SUCCESS);
            ft_pathfinding::restore_errno(guard, entry_errno);
            return (ER_SUCCESS);
        }
        open_set.erase(open_set.begin() + best_open);
        if (open_set.get_error() != ER_SUCCESS)
        {
            const_cast<ft_pathfinding *>(this)->set_error(open_set.get_error());
            ft_pathfinding::restore_errno(guard, entry_errno);
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
                            if (open_set.get_error() != ER_SUCCESS)
                            {
                                const_cast<ft_pathfinding *>(this)->set_error(open_set.get_error());
                                ft_pathfinding::restore_errno(guard, entry_errno);
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
                    if (nodes.get_error() != ER_SUCCESS)
                    {
                        const_cast<ft_pathfinding *>(this)->set_error(nodes.get_error());
                        ft_pathfinding::restore_errno(guard, entry_errno);
                        return (nodes.get_error());
                    }
                    open_set.push_back(nodes.size() - 1);
                    if (open_set.get_error() != ER_SUCCESS)
                    {
                        const_cast<ft_pathfinding *>(this)->set_error(open_set.get_error());
                        ft_pathfinding::restore_errno(guard, entry_errno);
                        return (open_set.get_error());
                    }
                }
            }
            neighbor_index += 1;
        }
    }
    const_cast<ft_pathfinding *>(this)->set_error(FT_ERR_GAME_INVALID_MOVE);
    ft_pathfinding::restore_errno(guard, entry_errno);
    return (const_cast<ft_pathfinding *>(this)->_error_code);
}

int ft_pathfinding::dijkstra_graph(const ft_graph<int> &graph,
    size_t start_vertex, size_t goal_vertex,
    ft_vector<size_t> &out_path) const noexcept
{
    int entry_errno;
    size_t graph_size;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_pathfinding *>(this)->set_error(guard.get_error());
        ft_pathfinding::restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    out_path.clear();
    graph_size = graph.size();
    if (start_vertex >= graph_size || goal_vertex >= graph_size)
    {
        const_cast<ft_pathfinding *>(this)->set_error(FT_ERR_GAME_INVALID_MOVE);
        ft_pathfinding::restore_errno(guard, entry_errno);
        return (const_cast<ft_pathfinding *>(this)->_error_code);
    }
    ft_vector<int> distance;
    ft_vector<int> previous;
    ft_vector<size_t> queue;
    size_t neighbor_index;

    distance.resize(graph_size, -1);
    if (distance.get_error() != ER_SUCCESS)
    {
        const_cast<ft_pathfinding *>(this)->set_error(distance.get_error());
        ft_pathfinding::restore_errno(guard, entry_errno);
        return (distance.get_error());
    }
    previous.resize(graph_size, -1);
    if (previous.get_error() != ER_SUCCESS)
    {
        const_cast<ft_pathfinding *>(this)->set_error(previous.get_error());
        ft_pathfinding::restore_errno(guard, entry_errno);
        return (previous.get_error());
    }
    queue.push_back(start_vertex);
    if (queue.get_error() != ER_SUCCESS)
    {
        const_cast<ft_pathfinding *>(this)->set_error(queue.get_error());
        ft_pathfinding::restore_errno(guard, entry_errno);
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
        if (queue.get_error() != ER_SUCCESS)
        {
            const_cast<ft_pathfinding *>(this)->set_error(queue.get_error());
            ft_pathfinding::restore_errno(guard, entry_errno);
            return (queue.get_error());
        }
        if (current == goal_vertex)
            break;
        ft_vector<size_t> neighbors;

        graph.neighbors(current, neighbors);
        if (neighbors.get_error() != ER_SUCCESS)
        {
            const_cast<ft_pathfinding *>(this)->set_error(neighbors.get_error());
            ft_pathfinding::restore_errno(guard, entry_errno);
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
                    if (queue.get_error() != ER_SUCCESS)
                    {
                        const_cast<ft_pathfinding *>(this)->set_error(queue.get_error());
                        ft_pathfinding::restore_errno(guard, entry_errno);
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
        ft_pathfinding::restore_errno(guard, entry_errno);
        return (const_cast<ft_pathfinding *>(this)->_error_code);
    }
    ft_vector<size_t> reverse_path;
    size_t vertex;

    vertex = goal_vertex;
    while (true)
    {
        reverse_path.push_back(vertex);
        if (reverse_path.get_error() != ER_SUCCESS)
        {
            const_cast<ft_pathfinding *>(this)->set_error(reverse_path.get_error());
            ft_pathfinding::restore_errno(guard, entry_errno);
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
        if (out_path.get_error() != ER_SUCCESS)
        {
            const_cast<ft_pathfinding *>(this)->set_error(out_path.get_error());
            ft_pathfinding::restore_errno(guard, entry_errno);
            return (out_path.get_error());
        }
    }
    const_cast<ft_pathfinding *>(this)->set_error(ER_SUCCESS);
    ft_pathfinding::restore_errno(guard, entry_errno);
    return (ER_SUCCESS);
}

int ft_pathfinding::get_error() const noexcept
{
    int entry_errno;
    int error_code;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_pathfinding *>(this)->set_error(guard.get_error());
        ft_pathfinding::restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    error_code = this->_error_code;
    const_cast<ft_pathfinding *>(this)->set_error(error_code);
    ft_pathfinding::restore_errno(guard, entry_errno);
    return (error_code);
}

const char *ft_pathfinding::get_error_str() const noexcept
{
    int entry_errno;
    int error_code;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_pathfinding *>(this)->set_error(guard.get_error());
        ft_pathfinding::restore_errno(guard, entry_errno);
        return (ft_strerror(guard.get_error()));
    }
    error_code = this->_error_code;
    const_cast<ft_pathfinding *>(this)->set_error(error_code);
    ft_pathfinding::restore_errno(guard, entry_errno);
    return (ft_strerror(error_code));
}

