#include "ft_world_region.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include <new>

static void game_world_region_copy_ids(const ft_vector<int> &source,
    ft_vector<int> &destination)
{
    ft_vector<int>::const_iterator entry;
    ft_vector<int>::const_iterator end_entry;

    destination.clear();
    entry = source.begin();
    end_entry = source.end();
    while (entry != end_entry)
    {
        destination.push_back(*entry);
        ++entry;
    }
    return ;
}

ft_world_region::ft_world_region() noexcept
    : _world_id(0), _region_ids(), _mutex(ft_nullptr),
      _initialized_state(ft_world_region::_state_uninitialized)
{
    return ;
}

ft_world_region::~ft_world_region() noexcept
{
    if (this->_initialized_state != ft_world_region::_state_initialized)
    {
        this->_initialized_state = ft_world_region::_state_destroyed;
        return ;
    }
    (void)this->destroy();
    return ;
}

void ft_world_region::abort_lifecycle_error(const char *method_name,
    const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_world_region lifecycle error: %s: %s\n",
        method_name, reason);
    su_abort();
    return ;
}

void ft_world_region::abort_if_not_initialized(const char *method_name) const
{
    if (this->_initialized_state == ft_world_region::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int ft_world_region::initialize() noexcept
{
    if (this->_initialized_state == ft_world_region::_state_initialized)
    {
        this->abort_lifecycle_error("ft_world_region::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_initialized_state = ft_world_region::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int ft_world_region::initialize(const ft_world_region &other) noexcept
{
    int initialize_error;

    if (other._initialized_state != ft_world_region::_state_initialized)
    {
        other.abort_lifecycle_error("ft_world_region::initialize(copy)",
            "source object is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    if (&other == this)
        return (FT_ERR_SUCCESS);
    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    this->_world_id = other._world_id;
    game_world_region_copy_ids(other._region_ids, this->_region_ids);
    return (FT_ERR_SUCCESS);
}

int ft_world_region::initialize(ft_world_region &&other) noexcept
{
    return (this->initialize(static_cast<const ft_world_region &>(other)));
}

int ft_world_region::initialize(int world_id,
    const ft_vector<int> &region_ids) noexcept
{
    int initialize_error;

    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    this->_world_id = world_id;
    game_world_region_copy_ids(region_ids, this->_region_ids);
    return (FT_ERR_SUCCESS);
}

int ft_world_region::destroy() noexcept
{
    int disable_error;

    if (this->_initialized_state != ft_world_region::_state_initialized)
    {
        this->_initialized_state = ft_world_region::_state_destroyed;
        return (FT_ERR_SUCCESS);
    }
    disable_error = this->disable_thread_safety();
    this->_world_id = 0;
    this->_region_ids.clear();
    this->_initialized_state = ft_world_region::_state_destroyed;
    return (disable_error);
}

int ft_world_region::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int initialize_error;

    this->abort_if_not_initialized("ft_world_region::enable_thread_safety");
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    mutex_pointer = new (std::nothrow) pt_recursive_mutex();
    if (mutex_pointer == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    initialize_error = mutex_pointer->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        return (initialize_error);
    }
    this->_mutex = mutex_pointer;
    return (FT_ERR_SUCCESS);
}

int ft_world_region::disable_thread_safety() noexcept
{
    int destroy_error;

    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

bool ft_world_region::is_thread_safe() const noexcept
{
    this->abort_if_not_initialized("ft_world_region::is_thread_safe");
    return (this->_mutex != ft_nullptr);
}

int ft_world_region::lock_internal(bool *lock_acquired) const noexcept
{
    int lock_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    lock_error = this->_mutex->lock();
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

int ft_world_region::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return (FT_ERR_SUCCESS);
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (this->_mutex->unlock());
}

int ft_world_region::lock(bool *lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_world_region::lock");
    return (this->lock_internal(lock_acquired));
}

void ft_world_region::unlock(bool lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_world_region::unlock");
    (void)this->unlock_internal(lock_acquired);
    return ;
}

int ft_world_region::get_world_id() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int world_id;

    this->abort_if_not_initialized("ft_world_region::get_world_id");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (0);
    world_id = this->_world_id;
    (void)this->unlock_internal(lock_acquired);
    return (world_id);
}

void ft_world_region::set_world_id(int world_id) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_world_region::set_world_id");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    this->_world_id = world_id;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

const ft_vector<int> &ft_world_region::get_region_ids() const noexcept
{
    this->abort_if_not_initialized("ft_world_region::get_region_ids const");
    return (this->_region_ids);
}

ft_vector<int> &ft_world_region::get_region_ids() noexcept
{
    this->abort_if_not_initialized("ft_world_region::get_region_ids");
    return (this->_region_ids);
}

void ft_world_region::set_region_ids(const ft_vector<int> &region_ids) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_world_region::set_region_ids");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    game_world_region_copy_ids(region_ids, this->_region_ids);
    (void)this->unlock_internal(lock_acquired);
    return ;
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *ft_world_region::get_mutex_for_validation() const noexcept
{
    this->abort_if_not_initialized("ft_world_region::get_mutex_for_validation");
    return (this->_mutex);
}
#endif
