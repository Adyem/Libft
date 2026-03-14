#include "../PThread/pthread_internal.hpp"
#include "ft_world_region.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include "../Errno/errno_internal.hpp"
#include <new>

thread_local int32_t ft_world_region::_last_error = FT_ERR_SUCCESS;

static void game_world_region_copy_ids(const ft_vector<int32_t> &source,
    ft_vector<int32_t> &destination)
{
    ft_vector<int32_t>::const_iterator entry;
    ft_vector<int32_t>::const_iterator end_entry;

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
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_world_region::ft_world_region(const ft_world_region &other) noexcept
    : _world_id(0), _region_ids(), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    int32_t initialize_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "ft_world_region::ft_world_region(copy)",
            "source object is not initialised");
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

ft_world_region::ft_world_region(ft_world_region &&other) noexcept
    : _world_id(0), _region_ids(), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    int32_t move_error;

    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "ft_world_region::ft_world_region(move)",
            "source object is not initialised");
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

ft_world_region::~ft_world_region() noexcept
{
    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
        return ;
    (void)this->destroy();
    return ;
}

int32_t ft_world_region::set_error(int32_t error_code) noexcept
{
    ft_world_region::_last_error = error_code;
    return (error_code);
}

int32_t ft_world_region::initialize() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(this->_initialised_state, "ft_world_region::initialize",
            "called while object is already initialised");
        return (FT_ERR_INVALID_STATE);
    }
    this->_world_id = 0;
    this->_region_ids.clear();
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t ft_world_region::initialize(const ft_world_region &other) noexcept
{
    int32_t initialize_error;

    if (other._initialised_state != FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state, "ft_world_region::initialize(copy)",
            "source object is not initialised");
        return (FT_ERR_INVALID_STATE);
    }
    if (&other == this)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        int32_t destroy_error = this->destroy();
        if (destroy_error != FT_ERR_SUCCESS)
        {
            this->set_error(destroy_error);
            return (destroy_error);
        }
    }
    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        this->set_error(initialize_error);
        return (initialize_error);
    }
    this->_world_id = other._world_id;
    game_world_region_copy_ids(other._region_ids, this->_region_ids);
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t ft_world_region::initialize(ft_world_region &&other) noexcept
{
    return (this->initialize(static_cast<const ft_world_region &>(other)));
}

int32_t ft_world_region::move(ft_world_region &other) noexcept
{
    return (this->initialize(static_cast<ft_world_region &&>(other)));
}

int32_t ft_world_region::initialize(int32_t world_id,
    const ft_vector<int32_t> &region_ids) noexcept
{
    int32_t initialize_error;

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
    this->_world_id = world_id;
    game_world_region_copy_ids(region_ids, this->_region_ids);
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int32_t ft_world_region::destroy() noexcept
{
    int32_t disable_error;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    disable_error = this->disable_thread_safety();
    this->_world_id = 0;
    this->_region_ids.clear();
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    this->set_error(disable_error);
    return (disable_error);
}

int32_t ft_world_region::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int32_t initialize_error;

    errno_abort_if_uninitialised(this->_initialised_state, "ft_world_region::enable_thread_safety");
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

int32_t ft_world_region::disable_thread_safety() noexcept
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

ft_bool ft_world_region::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int32_t ft_world_region::lock_internal(ft_bool *lock_acquired) const noexcept
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

int32_t ft_world_region::unlock_internal(ft_bool lock_acquired) const noexcept
{
    if (lock_acquired == FT_FALSE)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (FT_ERR_SUCCESS);
}

int32_t ft_world_region::lock(ft_bool *lock_acquired) const noexcept
{
    errno_abort_if_uninitialised(this->_initialised_state, "ft_world_region::lock");
    const int32_t lock_result = this->lock_internal(lock_acquired);
    this->set_error(lock_result);
    return (lock_result);
}

void ft_world_region::unlock(ft_bool lock_acquired) const noexcept
{
    errno_abort_if_uninitialised(this->_initialised_state, "ft_world_region::unlock");
    const int32_t unlock_result = this->unlock_internal(lock_acquired);
    (void)unlock_result;
    return ;
}

int32_t ft_world_region::get_world_id() const noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;
    int32_t world_id;

    errno_abort_if_uninitialised(this->_initialised_state, "ft_world_region::get_world_id");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (0);
    }
    world_id = this->_world_id;
    const int32_t unlock_result = this->unlock_internal(lock_acquired);
    (void)unlock_result;
    return (world_id);
}

void ft_world_region::set_world_id(int32_t world_id) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised(this->_initialised_state, "ft_world_region::set_world_id");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_world_id = world_id;
    const int32_t unlock_result = this->unlock_internal(lock_acquired);
    (void)unlock_result;
    return ;
}

const ft_vector<int32_t> &ft_world_region::get_region_ids() const noexcept
{
    errno_abort_if_uninitialised(this->_initialised_state, "ft_world_region::get_region_ids const");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_region_ids);
}

ft_vector<int32_t> &ft_world_region::get_region_ids() noexcept
{
    errno_abort_if_uninitialised(this->_initialised_state, "ft_world_region::get_region_ids");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_region_ids);
}

void ft_world_region::set_region_ids(const ft_vector<int32_t> &region_ids) noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised(this->_initialised_state, "ft_world_region::set_region_ids");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    game_world_region_copy_ids(region_ids, this->_region_ids);
    const int32_t unlock_result = this->unlock_internal(lock_acquired);
    (void)unlock_result;
    return ;
}


int32_t ft_world_region::get_error() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_if_uninitialised(this->_initialised_state,
            "ft_world_region::get_error");
    return (ft_world_region::_last_error);
}

const char *ft_world_region::get_error_str() const noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_if_uninitialised(this->_initialised_state,
            "ft_world_region::get_error_str");
    return (ft_strerror(this->get_error()));
}
