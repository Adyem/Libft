#include "ft_world_region.hpp"
#include "game_narrative_helpers.hpp"
#include "../Template/move.hpp"

static void game_world_copy_region_ids(const ft_vector<int> &source, ft_vector<int> &destination)
{
    ft_vector<int>::const_iterator entry;
    ft_vector<int>::const_iterator end;

    destination.clear();
    entry = source.begin();
    end = source.end();
    while (entry != end)
    {
        destination.push_back(*entry);
        ++entry;
    }
    return ;
}

int ft_world_region::lock_pair(const ft_world_region &first, const ft_world_region &second,
        ft_unique_lock<pt_mutex> &first_guard,
        ft_unique_lock<pt_mutex> &second_guard)
{
    const ft_world_region *ordered_first;
    const ft_world_region *ordered_second;
    bool swapped;

    if (&first == &second)
    {
        ft_unique_lock<pt_mutex> single_guard(first._mutex);

        if (single_guard.get_error() != FT_ER_SUCCESSS)
        {
            ft_errno = single_guard.get_error();
            return (single_guard.get_error());
        }
        first_guard = ft_move(single_guard);
        second_guard = ft_unique_lock<pt_mutex>();
        ft_errno = FT_ER_SUCCESSS;
        return (FT_ER_SUCCESSS);
    }
    ordered_first = &first;
    ordered_second = &second;
    swapped = false;
    if (ordered_first > ordered_second)
    {
        const ft_world_region *temporary;

        temporary = ordered_first;
        ordered_first = ordered_second;
        ordered_second = temporary;
        swapped = true;
    }
    while (true)
    {
        ft_unique_lock<pt_mutex> lower_guard(ordered_first->_mutex);

        if (lower_guard.get_error() != FT_ER_SUCCESSS)
        {
            ft_errno = lower_guard.get_error();
            return (lower_guard.get_error());
        }
        ft_unique_lock<pt_mutex> upper_guard(ordered_second->_mutex);
        if (upper_guard.get_error() == FT_ER_SUCCESSS)
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
            ft_errno = FT_ER_SUCCESSS;
            return (FT_ER_SUCCESSS);
        }
        if (upper_guard.get_error() != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            ft_errno = upper_guard.get_error();
            return (upper_guard.get_error());
        }
        if (lower_guard.owns_lock())
            lower_guard.unlock();
        game_narrative_sleep_backoff();
    }
}

ft_world_region::ft_world_region() noexcept
    : _world_id(0), _region_ids(), _error_code(FT_ER_SUCCESSS)
{
    return ;
}

ft_world_region::ft_world_region(int world_id, const ft_vector<int> &region_ids) noexcept
    : _world_id(world_id), _region_ids(), _error_code(FT_ER_SUCCESSS)
{
    game_world_copy_region_ids(region_ids, this->_region_ids);
    return ;
}

ft_world_region::~ft_world_region() noexcept
{
    return ;
}

ft_world_region::ft_world_region(const ft_world_region &other) noexcept
    : _world_id(0), _region_ids(), _error_code(FT_ER_SUCCESSS)
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(other_guard.get_error());
        game_narrative_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_world_id = other._world_id;
    game_world_copy_region_ids(other._region_ids, this->_region_ids);
    this->_error_code = other._error_code;
    game_narrative_restore_errno(other_guard, entry_errno);
    return ;
}

ft_world_region &ft_world_region::operator=(const ft_world_region &other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = ft_world_region::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ER_SUCCESSS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_world_id = other._world_id;
    game_world_copy_region_ids(other._region_ids, this->_region_ids);
    this->_error_code = other._error_code;
    game_narrative_restore_errno(this_guard, entry_errno);
    game_narrative_restore_errno(other_guard, entry_errno);
    return (*this);
}

ft_world_region::ft_world_region(ft_world_region &&other) noexcept
    : _world_id(0), _region_ids(), _error_code(FT_ER_SUCCESSS)
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> other_guard(other._mutex);
    if (other_guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(other_guard.get_error());
        game_narrative_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->_world_id = other._world_id;
    game_world_copy_region_ids(other._region_ids, this->_region_ids);
    this->_error_code = other._error_code;
    other._world_id = 0;
    other._region_ids.clear();
    other._error_code = FT_ER_SUCCESSS;
    game_narrative_restore_errno(other_guard, entry_errno);
    return ;
}

ft_world_region &ft_world_region::operator=(ft_world_region &&other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;
    int lock_error;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    lock_error = ft_world_region::lock_pair(*this, other, this_guard, other_guard);
    if (lock_error != FT_ER_SUCCESSS)
    {
        this->set_error(lock_error);
        return (*this);
    }
    this->_world_id = other._world_id;
    game_world_copy_region_ids(other._region_ids, this->_region_ids);
    this->_error_code = other._error_code;
    other._world_id = 0;
    other._region_ids.clear();
    other._error_code = FT_ER_SUCCESSS;
    game_narrative_restore_errno(this_guard, entry_errno);
    game_narrative_restore_errno(other_guard, entry_errno);
    return (*this);
}

int ft_world_region::get_world_id() const noexcept
{
    int entry_errno;
    int world_id;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return (0);
    }
    world_id = this->_world_id;
    game_narrative_restore_errno(guard, entry_errno);
    return (world_id);
}

void ft_world_region::set_world_id(int world_id) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_world_id = world_id;
    this->_error_code = FT_ER_SUCCESSS;
    game_narrative_restore_errno(guard, entry_errno);
    return ;
}

const ft_vector<int> &ft_world_region::get_region_ids() const noexcept
{
    int entry_errno;
    const ft_vector<int> *region_ids;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return (this->_region_ids);
    }
    region_ids = &this->_region_ids;
    game_narrative_restore_errno(guard, entry_errno);
    return (*region_ids);
}

ft_vector<int> &ft_world_region::get_region_ids() noexcept
{
    int entry_errno;
    ft_vector<int> *region_ids;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return (this->_region_ids);
    }
    region_ids = &this->_region_ids;
    game_narrative_restore_errno(guard, entry_errno);
    return (*region_ids);
}

void ft_world_region::set_region_ids(const ft_vector<int> &region_ids) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    game_world_copy_region_ids(region_ids, this->_region_ids);
    this->_error_code = FT_ER_SUCCESSS;
    game_narrative_restore_errno(guard, entry_errno);
    return ;
}

int ft_world_region::get_error() const noexcept
{
    int entry_errno;
    int error_code;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    entry_errno = ft_errno;
    if (guard.get_error() != FT_ER_SUCCESSS)
    {
        ft_errno = entry_errno;
        return (guard.get_error());
    }
    error_code = this->_error_code;
    game_narrative_restore_errno(guard, entry_errno);
    return (error_code);
}

const char *ft_world_region::get_error_str() const noexcept
{
    int error_code;

    error_code = this->get_error();
    return (ft_strerror(error_code));
}

void ft_world_region::set_error(int error_code) const noexcept
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}
