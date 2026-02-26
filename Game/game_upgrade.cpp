#include "game_upgrade.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"

thread_local int ft_upgrade::_last_error = FT_ERR_SUCCESS;

void ft_upgrade::set_error(int error_code) const noexcept
{
    ft_upgrade::_last_error = error_code;
    return ;
}

int ft_upgrade::get_error() const noexcept
{
    return (ft_upgrade::_last_error);
}

const char *ft_upgrade::get_error_str() const noexcept
{
    return (ft_strerror(this->get_error()));
}

ft_upgrade::ft_upgrade() noexcept
    : _id(0), _current_level(0), _max_level(0), _modifier1(0), _modifier2(0),
      _modifier3(0), _modifier4(0), _mutex(ft_nullptr),
      _initialized_state(ft_upgrade::_state_uninitialized)
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_upgrade::~ft_upgrade() noexcept
{
    if (this->_initialized_state == ft_upgrade::_state_initialized)
        (void)this->destroy();
    return ;
}

void ft_upgrade::abort_lifecycle_error(const char *method_name,
    const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_upgrade lifecycle error: %s: %s\n", method_name, reason);
    su_abort();
    return ;
}

void ft_upgrade::abort_if_not_initialized(const char *method_name) const
{
    if (this->_initialized_state == ft_upgrade::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int ft_upgrade::initialize() noexcept
{
    if (this->_initialized_state == ft_upgrade::_state_initialized)
    {
        this->abort_lifecycle_error("ft_upgrade::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_id = 0;
    this->_current_level = 0;
    this->_max_level = 0;
    this->_modifier1 = 0;
    this->_modifier2 = 0;
    this->_modifier3 = 0;
    this->_modifier4 = 0;
    this->_initialized_state = ft_upgrade::_state_initialized;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_upgrade::initialize(const ft_upgrade &other) noexcept
{
    int initialize_error;

    if (&other == this)
        return (FT_ERR_SUCCESS);
    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    this->_id = other._id;
    this->_current_level = other._current_level;
    this->_max_level = other._max_level;
    this->_modifier1 = other._modifier1;
    this->_modifier2 = other._modifier2;
    this->_modifier3 = other._modifier3;
    this->_modifier4 = other._modifier4;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_upgrade::initialize(ft_upgrade &&other) noexcept
{
    return (this->initialize(static_cast<const ft_upgrade &>(other)));
}

int ft_upgrade::destroy() noexcept
{
    int disable_error;

    if (this->_initialized_state != ft_upgrade::_state_initialized)
    {
        this->_initialized_state = ft_upgrade::_state_destroyed;
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    disable_error = this->disable_thread_safety();
    this->_id = 0;
    this->_current_level = 0;
    this->_max_level = 0;
    this->_modifier1 = 0;
    this->_modifier2 = 0;
    this->_modifier3 = 0;
    this->_modifier4 = 0;
    this->_initialized_state = ft_upgrade::_state_destroyed;
    this->set_error(disable_error);
    return (disable_error);
}

int ft_upgrade::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int initialize_error;

    this->abort_if_not_initialized("ft_upgrade::enable_thread_safety");
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

int ft_upgrade::disable_thread_safety() noexcept
{
    pt_recursive_mutex *old_mutex;
    int destroy_error;

    if (this->_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    old_mutex = this->_mutex;
    this->_mutex = ft_nullptr;
    destroy_error = old_mutex->destroy();
    delete old_mutex;
    this->set_error(destroy_error);
    return (destroy_error);
}

bool ft_upgrade::is_thread_safe() const noexcept
{
    this->abort_if_not_initialized("ft_upgrade::is_thread_safe");
    return (this->_mutex != ft_nullptr);
}

int ft_upgrade::lock_internal(bool *lock_acquired) const noexcept
{
    int lock_error;

    this->abort_if_not_initialized("ft_upgrade::lock_internal");
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (this->_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    lock_error = this->_mutex->lock();
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

int ft_upgrade::unlock_internal(bool lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_upgrade::unlock_internal");
    if (lock_acquired == false)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    if (this->_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESS);
        return (FT_ERR_SUCCESS);
    }
    int unlock_error = this->_mutex->unlock();
    if (unlock_error != FT_ERR_SUCCESS)
        this->set_error(unlock_error);
    else
        this->set_error(FT_ERR_SUCCESS);
    return (unlock_error);
}

int ft_upgrade::lock(bool *lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_upgrade::lock");
    return (this->lock_internal(lock_acquired));
}

void ft_upgrade::unlock(bool lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_upgrade::unlock");
    (void)this->unlock_internal(lock_acquired);
    return ;
}

int ft_upgrade::get_id() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int identifier;
    int unlock_error;

    this->abort_if_not_initialized("ft_upgrade::get_id");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    identifier = this->_id;
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
        return (unlock_error);
    this->set_error(FT_ERR_SUCCESS);
    return (identifier);
}

void ft_upgrade::set_id(int id) noexcept
{
    bool lock_acquired;
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("ft_upgrade::set_id");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_id = id;
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
        return ;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

uint16_t ft_upgrade::get_current_level() const noexcept
{
    bool lock_acquired;
    int lock_error;
    uint16_t level_value;
    int unlock_error;

    this->abort_if_not_initialized("ft_upgrade::get_current_level");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (0);
    }
    level_value = this->_current_level;
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
        return (0);
    this->set_error(FT_ERR_SUCCESS);
    return (level_value);
}

void ft_upgrade::set_current_level(uint16_t level) noexcept
{
    bool lock_acquired;
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("ft_upgrade::set_current_level");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_current_level = level;
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
        return ;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_upgrade::add_level(uint16_t level) noexcept
{
    bool lock_acquired;
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("ft_upgrade::add_level");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_current_level = static_cast<uint16_t>(this->_current_level + level);
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
        return ;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_upgrade::sub_level(uint16_t level) noexcept
{
    bool lock_acquired;
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("ft_upgrade::sub_level");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_current_level = static_cast<uint16_t>(this->_current_level - level);
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
        return ;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

uint16_t ft_upgrade::get_max_level() const noexcept
{
    bool lock_acquired;
    int lock_error;
    uint16_t max_value;
    int unlock_error;

    this->abort_if_not_initialized("ft_upgrade::get_max_level");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (0);
    }
    max_value = this->_max_level;
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
        return (0);
    this->set_error(FT_ERR_SUCCESS);
    return (max_value);
}

void ft_upgrade::set_max_level(uint16_t level) noexcept
{
    bool lock_acquired;
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("ft_upgrade::set_max_level");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_max_level = level;
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
        return ;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int ft_upgrade::get_modifier1() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int value;
    int unlock_error;

    this->abort_if_not_initialized("ft_upgrade::get_modifier1");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    value = this->_modifier1;
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
        return (unlock_error);
    this->set_error(FT_ERR_SUCCESS);
    return (value);
}

void ft_upgrade::set_modifier1(int mod) noexcept
{
    bool lock_acquired;
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("ft_upgrade::set_modifier1");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_modifier1 = mod;
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
        return ;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_upgrade::add_modifier1(int mod) noexcept
{
    bool lock_acquired;
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("ft_upgrade::add_modifier1");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_modifier1 += mod;
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
        return ;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_upgrade::sub_modifier1(int mod) noexcept
{
    bool lock_acquired;
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("ft_upgrade::sub_modifier1");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_modifier1 -= mod;
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
        return ;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int ft_upgrade::get_modifier2() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int value;
    int unlock_error;

    this->abort_if_not_initialized("ft_upgrade::get_modifier2");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    value = this->_modifier2;
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
        return (unlock_error);
    this->set_error(FT_ERR_SUCCESS);
    return (value);
}

void ft_upgrade::set_modifier2(int mod) noexcept
{
    bool lock_acquired;
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("ft_upgrade::set_modifier2");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_modifier2 = mod;
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
        return ;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_upgrade::add_modifier2(int mod) noexcept
{
    bool lock_acquired;
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("ft_upgrade::add_modifier2");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_modifier2 += mod;
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
        return ;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_upgrade::sub_modifier2(int mod) noexcept
{
    bool lock_acquired;
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("ft_upgrade::sub_modifier2");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_modifier2 -= mod;
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
        return ;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int ft_upgrade::get_modifier3() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int value;
    int unlock_error;

    this->abort_if_not_initialized("ft_upgrade::get_modifier3");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    value = this->_modifier3;
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
        return (unlock_error);
    this->set_error(FT_ERR_SUCCESS);
    return (value);
}

void ft_upgrade::set_modifier3(int mod) noexcept
{
    bool lock_acquired;
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("ft_upgrade::set_modifier3");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_modifier3 = mod;
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
        return ;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_upgrade::add_modifier3(int mod) noexcept
{
    bool lock_acquired;
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("ft_upgrade::add_modifier3");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_modifier3 += mod;
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
        return ;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_upgrade::sub_modifier3(int mod) noexcept
{
    bool lock_acquired;
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("ft_upgrade::sub_modifier3");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_modifier3 -= mod;
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
        return ;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int ft_upgrade::get_modifier4() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int value;
    int unlock_error;

    this->abort_if_not_initialized("ft_upgrade::get_modifier4");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    value = this->_modifier4;
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
        return (unlock_error);
    this->set_error(FT_ERR_SUCCESS);
    return (value);
}

void ft_upgrade::set_modifier4(int mod) noexcept
{
    bool lock_acquired;
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("ft_upgrade::set_modifier4");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_modifier4 = mod;
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
        return ;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_upgrade::add_modifier4(int mod) noexcept
{
    bool lock_acquired;
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("ft_upgrade::add_modifier4");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_modifier4 += mod;
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
        return ;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_upgrade::sub_modifier4(int mod) noexcept
{
    bool lock_acquired;
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("ft_upgrade::sub_modifier4");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_modifier4 -= mod;
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
        return ;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *ft_upgrade::get_mutex_for_validation() const noexcept
{
    this->abort_if_not_initialized("ft_upgrade::get_mutex_for_validation");
    return (this->_mutex);
}
#endif
