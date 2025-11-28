#include "game_world_registry.hpp"
#include "game_narrative_helpers.hpp"

ft_world_registry::ft_world_registry() noexcept
    : _regions(), _world_regions(), _error_code(ER_SUCCESS)
{
    return ;
}

ft_world_registry::~ft_world_registry() noexcept
{
    return ;
}

void ft_world_registry::copy_registry_unlocked(const ft_world_registry &other) noexcept
{
    int entry_errno;

    entry_errno = ft_errno;
    this->_regions = other._regions;
    this->_world_regions = other._world_regions;
    this->_error_code = other._error_code;
    this->set_error(this->_error_code);
    ft_errno = entry_errno;
    return ;
}

ft_world_registry::ft_world_registry(const ft_world_registry &other) noexcept
    : _regions(), _world_regions(), _error_code(ER_SUCCESS)
{
    int entry_errno;
    ft_unique_lock<pt_mutex> other_guard(other._mutex);

    entry_errno = ft_errno;
    if (other_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(other_guard.get_error());
        game_narrative_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->copy_registry_unlocked(other);
    game_narrative_restore_errno(other_guard, entry_errno);
    return ;
}

ft_world_registry &ft_world_registry::operator=(const ft_world_registry &other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    this_guard = ft_unique_lock<pt_mutex>(this->_mutex);
    if (this_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(this_guard.get_error());
        game_narrative_restore_errno(this_guard, entry_errno);
        return (*this);
    }
    other_guard = ft_unique_lock<pt_mutex>(other._mutex);
    if (other_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(other_guard.get_error());
        game_narrative_restore_errno(this_guard, entry_errno);
        game_narrative_restore_errno(other_guard, entry_errno);
        return (*this);
    }
    this->copy_registry_unlocked(other);
    game_narrative_restore_errno(this_guard, entry_errno);
    game_narrative_restore_errno(other_guard, entry_errno);
    return (*this);
}

ft_world_registry::ft_world_registry(ft_world_registry &&other) noexcept
    : _regions(), _world_regions(), _error_code(ER_SUCCESS)
{
    int entry_errno;
    ft_unique_lock<pt_mutex> other_guard(other._mutex);

    entry_errno = ft_errno;
    if (other_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(other_guard.get_error());
        game_narrative_restore_errno(other_guard, entry_errno);
        return ;
    }
    this->copy_registry_unlocked(other);
    other._regions.clear();
    other._world_regions.clear();
    other._error_code = ER_SUCCESS;
    this->set_error(this->_error_code);
    other.set_error(ER_SUCCESS);
    game_narrative_restore_errno(other_guard, entry_errno);
    return ;
}

ft_world_registry &ft_world_registry::operator=(ft_world_registry &&other) noexcept
{
    ft_unique_lock<pt_mutex> this_guard;
    ft_unique_lock<pt_mutex> other_guard;
    int entry_errno;

    if (this == &other)
        return (*this);
    entry_errno = ft_errno;
    this_guard = ft_unique_lock<pt_mutex>(this->_mutex);
    if (this_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(this_guard.get_error());
        game_narrative_restore_errno(this_guard, entry_errno);
        return (*this);
    }
    other_guard = ft_unique_lock<pt_mutex>(other._mutex);
    if (other_guard.get_error() != ER_SUCCESS)
    {
        this->set_error(other_guard.get_error());
        game_narrative_restore_errno(this_guard, entry_errno);
        game_narrative_restore_errno(other_guard, entry_errno);
        return (*this);
    }
    this->copy_registry_unlocked(other);
    other._regions.clear();
    other._world_regions.clear();
    other._error_code = ER_SUCCESS;
    this->set_error(this->_error_code);
    other.set_error(ER_SUCCESS);
    game_narrative_restore_errno(this_guard, entry_errno);
    game_narrative_restore_errno(other_guard, entry_errno);
    return (*this);
}

int ft_world_registry::register_region(const ft_region_definition &region) noexcept
{
    int entry_errno;
    int identifier;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    entry_errno = ft_errno;
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_narrative_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    if (region.get_error() != ER_SUCCESS)
    {
        this->set_error(region.get_error());
        game_narrative_restore_errno(guard, entry_errno);
        return (region.get_error());
    }
    identifier = region.get_region_id();
    this->_regions.insert(identifier, region);
    this->set_error(this->_regions.get_error());
    game_narrative_restore_errno(guard, entry_errno);
    return (this->_error_code);
}

int ft_world_registry::register_world(const ft_world_region &world_region) noexcept
{
    int entry_errno;
    int identifier;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    entry_errno = ft_errno;
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        game_narrative_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    if (world_region.get_error() != ER_SUCCESS)
    {
        this->set_error(world_region.get_error());
        game_narrative_restore_errno(guard, entry_errno);
        return (world_region.get_error());
    }
    identifier = world_region.get_world_id();
    this->_world_regions.insert(identifier, world_region);
    this->set_error(this->_world_regions.get_error());
    game_narrative_restore_errno(guard, entry_errno);
    return (this->_error_code);
}

int ft_world_registry::fetch_region(int region_id, ft_region_definition &out_region) const noexcept
{
    int entry_errno;
    const Pair<int, ft_region_definition> *entry;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    entry_errno = ft_errno;
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_world_registry *>(this)->set_error(guard.get_error());
        game_narrative_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    entry = this->_regions.find(region_id);
    if (entry == this->_regions.end())
    {
        const_cast<ft_world_registry *>(this)->set_error(FT_ERR_NOT_FOUND);
        game_narrative_restore_errno(guard, entry_errno);
        return (FT_ERR_NOT_FOUND);
    }
    out_region = entry->value;
    const_cast<ft_world_registry *>(this)->set_error(out_region.get_error());
    game_narrative_restore_errno(guard, entry_errno);
    return (out_region.get_error());
}

int ft_world_registry::fetch_world(int world_id, ft_world_region &out_world) const noexcept
{
    int entry_errno;
    const Pair<int, ft_world_region> *entry;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    entry_errno = ft_errno;
    if (guard.get_error() != ER_SUCCESS)
    {
        const_cast<ft_world_registry *>(this)->set_error(guard.get_error());
        game_narrative_restore_errno(guard, entry_errno);
        return (guard.get_error());
    }
    entry = this->_world_regions.find(world_id);
    if (entry == this->_world_regions.end())
    {
        const_cast<ft_world_registry *>(this)->set_error(FT_ERR_NOT_FOUND);
        game_narrative_restore_errno(guard, entry_errno);
        return (FT_ERR_NOT_FOUND);
    }
    out_world = entry->value;
    const_cast<ft_world_registry *>(this)->set_error(out_world.get_error());
    game_narrative_restore_errno(guard, entry_errno);
    return (out_world.get_error());
}

ft_map<int, ft_region_definition> &ft_world_registry::get_regions() noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_regions);
}

const ft_map<int, ft_region_definition> &ft_world_registry::get_regions() const noexcept
{
    const_cast<ft_world_registry *>(this)->set_error(ER_SUCCESS);
    return (this->_regions);
}

void ft_world_registry::set_regions(const ft_map<int, ft_region_definition> &regions) noexcept
{
    int entry_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    entry_errno = ft_errno;
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_regions = regions;
    this->_error_code = this->_regions.get_error();
    game_narrative_restore_errno(guard, entry_errno);
    return ;
}

ft_map<int, ft_world_region> &ft_world_registry::get_world_regions() noexcept
{
    this->set_error(ER_SUCCESS);
    return (this->_world_regions);
}

const ft_map<int, ft_world_region> &ft_world_registry::get_world_regions() const noexcept
{
    const_cast<ft_world_registry *>(this)->set_error(ER_SUCCESS);
    return (this->_world_regions);
}

void ft_world_registry::set_world_regions(const ft_map<int, ft_world_region> &world_regions) noexcept
{
    int entry_errno;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    entry_errno = ft_errno;
    if (guard.get_error() != ER_SUCCESS)
    {
        this->set_error(guard.get_error());
        return ;
    }
    this->_world_regions = world_regions;
    this->_error_code = this->_world_regions.get_error();
    game_narrative_restore_errno(guard, entry_errno);
    return ;
}

int ft_world_registry::get_error() const noexcept
{
    int entry_errno;
    int error_code;
    ft_unique_lock<pt_mutex> guard(this->_mutex);

    entry_errno = ft_errno;
    if (guard.get_error() != ER_SUCCESS)
    {
        ft_errno = entry_errno;
        return (guard.get_error());
    }
    error_code = this->_error_code;
    game_narrative_restore_errno(guard, entry_errno);
    return (error_code);
}

const char *ft_world_registry::get_error_str() const noexcept
{
    int error_code;

    error_code = this->get_error();
    return (ft_strerror(error_code));
}

void ft_world_registry::set_error(int error_code) const noexcept
{
    this->_error_code = error_code;
    ft_errno = error_code;
    return ;
}
