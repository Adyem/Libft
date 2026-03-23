#include "../PThread/pthread_internal.hpp"
#include "game_world_registry.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include "../Errno/errno_internal.hpp"
#include <new>

thread_local int32_t game_world_registry::_last_error = FT_ERR_SUCCESS;

game_world_registry::game_world_registry() noexcept
    : _regions(), _world_regions(), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

game_world_registry::game_world_registry(const game_world_registry &other) noexcept
    : _regions(), _world_regions(), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    int32_t initialize_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "game_world_registry::game_world_registry(copy)",
            "source object is uninitialised");
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(FT_ERR_INVALID_STATE);
        return ;
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(other.get_error());
        return ;
    }
    initialize_error = this->initialize(other);
    if (initialize_error != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(initialize_error);
    }
    return ;
}

game_world_registry::game_world_registry(game_world_registry &&other) noexcept
    : _regions(), _world_regions(), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    int32_t move_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "game_world_registry::game_world_registry(move)",
            "source object is uninitialised");
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(FT_ERR_INVALID_STATE);
        return ;
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(other.get_error());
        return ;
    }
    move_error = this->move(other);
    if (move_error != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(move_error);
    }
    return ;
}

game_world_registry::~game_world_registry() noexcept
{
    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
        return ;
    (void)this->destroy();
    return ;
}

int32_t game_world_registry::set_error(int32_t error_code) noexcept
{
    game_world_registry::_last_error = error_code;
    return (error_code);
}

int32_t game_world_registry::initialize() noexcept
{
    int32_t regions_error;
    int32_t worlds_error;

    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(this->_initialised_state, "game_world_registry::initialize",
            "called while object is already initialised");
        return (FT_ERR_INVALID_STATE);
    }
    regions_error = this->_regions.initialize();
    if (regions_error != FT_ERR_SUCCESS)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(regions_error);
        return (regions_error);
    }
    worlds_error = this->_world_regions.initialize();
    if (worlds_error != FT_ERR_SUCCESS)
    {
        (void)this->_regions.destroy();
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(worlds_error);
        return (worlds_error);
    }
    this->_regions.clear();
    this->_world_regions.clear();
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t game_world_registry::initialize(const game_world_registry &other) noexcept
{
    int32_t initialize_error;
    int32_t destroy_error;
    ft_size_t count;
    ft_size_t index;
    const Pair<int32_t, game_region_definition> *region_entry;
    const Pair<int32_t, game_region_definition> *region_end;
    const Pair<int32_t, game_world_region> *world_entry;
    const Pair<int32_t, game_world_region> *world_end;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "game_world_registry::initialize(copy)",
            "source object is uninitialised");
        return (FT_ERR_INVALID_STATE);
    }
    if (&other == this)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        destroy_error = this->destroy();
        if (destroy_error != FT_ERR_SUCCESS)
        {
            this->set_error(destroy_error);
            return (destroy_error);
        }
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        this->set_error(static_cast<uint32_t>(other.get_error()));
        return (FT_ERR_SUCCESS);
    }
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        initialize_error = this->destroy();
        if (initialize_error != FT_ERR_SUCCESS)
        {
            this->set_error(initialize_error);
            return (initialize_error);
        }
    }
    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        this->set_error(initialize_error);
        return (initialize_error);
    }
    count = other._regions.size();
    region_end = other._regions.end();
    region_entry = region_end - count;
    index = 0;
    while (index < count)
    {
        this->_regions.insert(region_entry->key, region_entry->value);
        region_entry++;
        index += 1;
    }
    count = other._world_regions.size();
    world_end = other._world_regions.end();
    world_entry = world_end - count;
    index = 0;
    while (index < count)
    {
        this->_world_regions.insert(world_entry->key, world_entry->value);
        world_entry++;
        index += 1;
    }
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t game_world_registry::initialize(game_world_registry &&other) noexcept
{
    return (this->move(other));
}

int32_t game_world_registry::move(game_world_registry &other) noexcept
{
    int32_t initialize_error;

    if (&other == this)
        return (FT_ERR_SUCCESS);
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "game_world_registry::move",
            "source object is uninitialised");
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    initialize_error = this->initialize(static_cast<const game_world_registry &>(other));
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    if (other._initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)other.destroy();
    return (FT_ERR_SUCCESS);
}

int32_t game_world_registry::destroy() noexcept
{
    int32_t disable_error;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    this->_regions.clear();
    this->_world_regions.clear();
    (void)this->_regions.destroy();
    (void)this->_world_regions.destroy();
    disable_error = this->disable_thread_safety();
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    this->set_error(disable_error);
    return (disable_error);
}

int32_t game_world_registry::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int32_t initialize_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_world_registry::enable_thread_safety");
    if (this->_mutex != ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    if (mutex_pointer == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return (FT_ERR_NO_MEMORY);
    }
    initialize_error = mutex_pointer->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        this->set_error(initialize_error);
        return (initialize_error);
    }
    this->_mutex = mutex_pointer;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t game_world_registry::disable_thread_safety() noexcept
{
    int32_t destroy_error;

    if (this->_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    this->set_error(destroy_error);
    return (destroy_error);
}

ft_bool game_world_registry::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int32_t game_world_registry::lock_internal(ft_bool *lock_acquired) const noexcept
{
    int32_t lock_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = FT_FALSE;
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = FT_TRUE;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t game_world_registry::unlock_internal(ft_bool lock_acquired) const noexcept
{
    if (lock_acquired == FT_FALSE)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (FT_ERR_SUCCESS);
}

int32_t game_world_registry::lock(ft_bool *lock_acquired) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_world_registry::lock");
    const int32_t lock_result = this->lock_internal(lock_acquired);
    this->set_error(lock_result);
    return (lock_result);
}

void game_world_registry::unlock(ft_bool lock_acquired) const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_world_registry::unlock");
    (void)this->unlock_internal(lock_acquired);
    return ;
}

int32_t game_world_registry::register_region(const game_region_definition &region) noexcept
{
    ft_bool lock_acquired;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_world_registry::register_region");
    const int32_t lock_result = this->lock_internal(&lock_acquired);
    if (lock_result != FT_ERR_SUCCESS)
    {
        this->set_error(lock_result);
        return (FT_ERR_INVALID_STATE);
    }
    this->_regions.insert(region.get_region_id(), region);
    (void)this->unlock_internal(lock_acquired);
    return (FT_ERR_SUCCESS);
}

int32_t game_world_registry::register_world(const game_world_region &world_region) noexcept
{
    ft_bool lock_acquired;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_world_registry::register_world");
    const int32_t lock_result = this->lock_internal(&lock_acquired);
    if (lock_result != FT_ERR_SUCCESS)
    {
        this->set_error(lock_result);
        return (FT_ERR_INVALID_STATE);
    }
    this->_world_regions.insert(world_region.get_world_id(), world_region);
    (void)this->unlock_internal(lock_acquired);
    return (FT_ERR_SUCCESS);
}

int32_t game_world_registry::fetch_region(int32_t region_id,
    game_region_definition &out_region) const noexcept
{
    const Pair<int32_t, game_region_definition> *entry;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_world_registry::fetch_region");
    entry = this->_regions.find(region_id);
    if (entry == this->_regions.end())
    {
        this->set_error(FT_ERR_NOT_FOUND);
        return (FT_ERR_NOT_FOUND);
    }
    const int32_t initialize_error = out_region.initialize(entry->value);
    this->set_error(initialize_error);
    return (initialize_error);
}

int32_t game_world_registry::fetch_world(int32_t world_id,
    game_world_region &out_world) const noexcept
{
    const Pair<int32_t, game_world_region> *entry;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_world_registry::fetch_world");
    entry = this->_world_regions.find(world_id);
    if (entry == this->_world_regions.end())
    {
        this->set_error(FT_ERR_NOT_FOUND);
        return (FT_ERR_NOT_FOUND);
    }
    const int32_t initialize_error = out_world.initialize(entry->value);
    this->set_error(initialize_error);
    return (initialize_error);
}

ft_map<int32_t, game_region_definition> &game_world_registry::get_regions() noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_world_registry::get_regions");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_regions);
}

const ft_map<int32_t, game_region_definition> &game_world_registry::get_regions() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_world_registry::get_regions const");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_regions);
}

void game_world_registry::set_regions(
    const ft_map<int32_t, game_region_definition> &regions) noexcept
{
    ft_size_t count;
    ft_size_t index;
    const Pair<int32_t, game_region_definition> *entry;
    const Pair<int32_t, game_region_definition> *entry_end;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_world_registry::set_regions");
    this->_regions.clear();
    count = regions.size();
    entry_end = regions.end();
    entry = entry_end - count;
    index = 0;
    while (index < count)
    {
        this->_regions.insert(entry->key, entry->value);
        entry++;
        index += 1;
    }
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_map<int32_t, game_world_region> &game_world_registry::get_world_regions() noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_world_registry::get_world_regions");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_world_regions);
}

const ft_map<int32_t, game_world_region> &game_world_registry::get_world_regions() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_world_registry::get_world_regions const");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_world_regions);
}

void game_world_registry::set_world_regions(
    const ft_map<int32_t, game_world_region> &world_regions) noexcept
{
    ft_size_t count;
    ft_size_t index;
    const Pair<int32_t, game_world_region> *entry;
    const Pair<int32_t, game_world_region> *entry_end;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "game_world_registry::set_world_regions");
    this->_world_regions.clear();
    count = world_regions.size();
    entry_end = world_regions.end();
    entry = entry_end - count;
    index = 0;
    while (index < count)
    {
        this->_world_regions.insert(entry->key, entry->value);
        entry++;
        index += 1;
    }
    this->set_error(FT_ERR_SUCCESS);
    return ;
}


int32_t game_world_registry::get_error() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
            "game_world_registry::get_error");
    return (game_world_registry::_last_error);
}

const char *game_world_registry::get_error_str() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_if_uninitialised_or_destroyed(this->_initialised_state,
            "game_world_registry::get_error_str");
    return (ft_strerror(game_world_registry::_last_error));
}
