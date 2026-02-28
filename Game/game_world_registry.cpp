#include "../PThread/pthread_internal.hpp"
#include "game_world_registry.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include <new>

thread_local int ft_world_registry::_last_error = FT_ERR_SUCCESS;

ft_world_registry::ft_world_registry() noexcept
    : _regions(), _world_regions(), _mutex(ft_nullptr),
      _initialized_state(ft_world_registry::_state_uninitialized)
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_world_registry::~ft_world_registry() noexcept
{
    if (this->_initialized_state != ft_world_registry::_state_initialized)
        return ;
    (void)this->destroy();
    return ;
}

void ft_world_registry::abort_lifecycle_error(const char *method_name,
    const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_world_registry lifecycle error: %s: %s\n",
        method_name, reason);
    su_abort();
    return ;
}

void ft_world_registry::abort_if_not_initialized(const char *method_name) const
{
    if (this->_initialized_state == ft_world_registry::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

void ft_world_registry::set_error(int error_code) const noexcept
{
    ft_world_registry::_last_error = error_code;
    return ;
}

int ft_world_registry::initialize() noexcept
{
    int regions_error;
    int worlds_error;

    if (this->_initialized_state == ft_world_registry::_state_initialized)
    {
        this->abort_lifecycle_error("ft_world_registry::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    regions_error = this->_regions.initialize();
    if (regions_error != FT_ERR_SUCCESS)
    {
        this->set_error(regions_error);
        return (regions_error);
    }
    worlds_error = this->_world_regions.initialize();
    if (worlds_error != FT_ERR_SUCCESS)
    {
        (void)this->_regions.destroy();
        this->set_error(worlds_error);
        return (worlds_error);
    }
    this->_regions.clear();
    this->_world_regions.clear();
    this->_initialized_state = ft_world_registry::_state_initialized;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_world_registry::initialize(const ft_world_registry &other) noexcept
{
    int initialize_error;
    size_t count;
    size_t index;
    const Pair<int, ft_region_definition> *region_entry;
    const Pair<int, ft_region_definition> *region_end;
    const Pair<int, ft_world_region> *world_entry;
    const Pair<int, ft_world_region> *world_end;

    if (other._initialized_state != ft_world_registry::_state_initialized)
    {
        other.abort_lifecycle_error("ft_world_registry::initialize(copy)",
            "source object is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    if (&other == this)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
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

int ft_world_registry::initialize(ft_world_registry &&other) noexcept
{
    return (this->initialize(static_cast<const ft_world_registry &>(other)));
}

int ft_world_registry::destroy() noexcept
{
    int disable_error;

    if (this->_initialized_state != ft_world_registry::_state_initialized)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    this->_regions.clear();
    this->_world_regions.clear();
    (void)this->_regions.destroy();
    (void)this->_world_regions.destroy();
    disable_error = this->disable_thread_safety();
    this->_initialized_state = ft_world_registry::_state_destroyed;
    this->set_error(disable_error);
    return (disable_error);
}

int ft_world_registry::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int initialize_error;

    this->abort_if_not_initialized("ft_world_registry::enable_thread_safety");
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

int ft_world_registry::disable_thread_safety() noexcept
{
    int destroy_error;

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

bool ft_world_registry::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int ft_world_registry::lock_internal(bool *lock_acquired) const noexcept
{
    int lock_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_world_registry::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    const int unlock_error = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    this->set_error(unlock_error);
    return (unlock_error);
}

int ft_world_registry::lock(bool *lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_world_registry::lock");
    const int lock_result = this->lock_internal(lock_acquired);
    this->set_error(lock_result);
    return (lock_result);
}

void ft_world_registry::unlock(bool lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_world_registry::unlock");
    const int unlock_result = this->unlock_internal(lock_acquired);
    this->set_error(unlock_result);
    return ;
}

int ft_world_registry::register_region(const ft_region_definition &region) noexcept
{
    bool lock_acquired;

    this->abort_if_not_initialized("ft_world_registry::register_region");
    const int lock_result = this->lock_internal(&lock_acquired);
    if (lock_result != FT_ERR_SUCCESS)
    {
        this->set_error(lock_result);
        return (FT_ERR_INVALID_STATE);
    }
    this->_regions.insert(region.get_region_id(), region);
    const int unlock_result = this->unlock_internal(lock_acquired);
    this->set_error(unlock_result);
    return (FT_ERR_SUCCESS);
}

int ft_world_registry::register_world(const ft_world_region &world_region) noexcept
{
    bool lock_acquired;

    this->abort_if_not_initialized("ft_world_registry::register_world");
    const int lock_result = this->lock_internal(&lock_acquired);
    if (lock_result != FT_ERR_SUCCESS)
    {
        this->set_error(lock_result);
        return (FT_ERR_INVALID_STATE);
    }
    this->_world_regions.insert(world_region.get_world_id(), world_region);
    const int unlock_result = this->unlock_internal(lock_acquired);
    this->set_error(unlock_result);
    return (FT_ERR_SUCCESS);
}

int ft_world_registry::fetch_region(int region_id,
    ft_region_definition &out_region) const noexcept
{
    const Pair<int, ft_region_definition> *entry;

    this->abort_if_not_initialized("ft_world_registry::fetch_region");
    entry = this->_regions.find(region_id);
    if (entry == this->_regions.end())
    {
        this->set_error(FT_ERR_NOT_FOUND);
        return (FT_ERR_NOT_FOUND);
    }
    const int initialize_error = out_region.initialize(entry->value);
    this->set_error(initialize_error);
    return (initialize_error);
}

int ft_world_registry::fetch_world(int world_id,
    ft_world_region &out_world) const noexcept
{
    const Pair<int, ft_world_region> *entry;

    this->abort_if_not_initialized("ft_world_registry::fetch_world");
    entry = this->_world_regions.find(world_id);
    if (entry == this->_world_regions.end())
    {
        this->set_error(FT_ERR_NOT_FOUND);
        return (FT_ERR_NOT_FOUND);
    }
    const int initialize_error = out_world.initialize(entry->value);
    this->set_error(initialize_error);
    return (initialize_error);
}

ft_map<int, ft_region_definition> &ft_world_registry::get_regions() noexcept
{
    this->abort_if_not_initialized("ft_world_registry::get_regions");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_regions);
}

const ft_map<int, ft_region_definition> &ft_world_registry::get_regions() const noexcept
{
    this->abort_if_not_initialized("ft_world_registry::get_regions const");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_regions);
}

void ft_world_registry::set_regions(
    const ft_map<int, ft_region_definition> &regions) noexcept
{
    size_t count;
    size_t index;
    const Pair<int, ft_region_definition> *entry;
    const Pair<int, ft_region_definition> *entry_end;

    this->abort_if_not_initialized("ft_world_registry::set_regions");
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

ft_map<int, ft_world_region> &ft_world_registry::get_world_regions() noexcept
{
    this->abort_if_not_initialized("ft_world_registry::get_world_regions");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_world_regions);
}

const ft_map<int, ft_world_region> &ft_world_registry::get_world_regions() const noexcept
{
    this->abort_if_not_initialized("ft_world_registry::get_world_regions const");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_world_regions);
}

void ft_world_registry::set_world_regions(
    const ft_map<int, ft_world_region> &world_regions) noexcept
{
    size_t count;
    size_t index;
    const Pair<int, ft_world_region> *entry;
    const Pair<int, ft_world_region> *entry_end;

    this->abort_if_not_initialized("ft_world_registry::set_world_regions");
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

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *ft_world_registry::get_mutex_for_validation() const noexcept
{
    this->abort_if_not_initialized("ft_world_registry::get_mutex_for_validation");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_mutex);
}
#endif

int ft_world_registry::get_error() const noexcept
{
    return (ft_world_registry::_last_error);
}

const char *ft_world_registry::get_error_str() const noexcept
{
    return (ft_strerror(ft_world_registry::_last_error));
}
