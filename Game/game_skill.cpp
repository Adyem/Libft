#include "../PThread/pthread_internal.hpp"
#include "game_skill.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"

thread_local int ft_skill::_last_error = FT_ERR_SUCCESS;

void ft_skill::set_error(int error_code) const noexcept
{
    ft_skill::_last_error = error_code;
    return ;
}

int ft_skill::get_error() const noexcept
{
    return (ft_skill::_last_error);
}

const char *ft_skill::get_error_str() const noexcept
{
    return (ft_strerror(this->get_error()));
}

ft_skill::ft_skill() noexcept
    : _id(0), _level(0), _cooldown(0), _modifier1(0), _modifier2(0),
      _modifier3(0), _modifier4(0), _mutex(ft_nullptr),
      _initialized_state(ft_skill::_state_uninitialized)
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_skill::~ft_skill() noexcept
{
    if (this->_initialized_state == ft_skill::_state_uninitialized)
        return ;
    if (this->_initialized_state == ft_skill::_state_initialized)
        (void)this->destroy();
    return ;
}

void ft_skill::abort_lifecycle_error(const char *method_name,
    const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_skill lifecycle error: %s: %s\n", method_name, reason);
    su_abort();
    return ;
}

void ft_skill::abort_if_not_initialized(const char *method_name) const
{
    if (this->_initialized_state == ft_skill::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int ft_skill::initialize() noexcept
{
    if (this->_initialized_state == ft_skill::_state_initialized)
    {
        this->abort_lifecycle_error("ft_skill::initialize",
            "called while object is already initialized");
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    this->_id = 0;
    this->_level = 0;
    this->_cooldown = 0;
    this->_modifier1 = 0;
    this->_modifier2 = 0;
    this->_modifier3 = 0;
    this->_modifier4 = 0;
    this->_initialized_state = ft_skill::_state_initialized;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_skill::initialize(const ft_skill &other) noexcept
{
    int initialize_error;

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
    this->_id = other._id;
    this->_level = other._level;
    this->_cooldown = other._cooldown;
    this->_modifier1 = other._modifier1;
    this->_modifier2 = other._modifier2;
    this->_modifier3 = other._modifier3;
    this->_modifier4 = other._modifier4;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_skill::initialize(ft_skill &&other) noexcept
{
    int result;

    result = this->initialize(static_cast<const ft_skill &>(other));
    this->set_error(result);
    return (result);
}

int ft_skill::destroy() noexcept
{
    int disable_error;

    if (this->_initialized_state != ft_skill::_state_initialized)
        return (FT_ERR_INVALID_STATE);
    disable_error = this->disable_thread_safety();
    this->_id = 0;
    this->_level = 0;
    this->_cooldown = 0;
    this->_modifier1 = 0;
    this->_modifier2 = 0;
    this->_modifier3 = 0;
    this->_modifier4 = 0;
    this->_initialized_state = ft_skill::_state_destroyed;
    this->set_error(disable_error);
    return (disable_error);
}

int ft_skill::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int initialize_error;

    this->abort_if_not_initialized("ft_skill::enable_thread_safety");
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

int ft_skill::disable_thread_safety() noexcept
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

bool ft_skill::is_thread_safe() const noexcept
{
    return (this->_mutex != ft_nullptr);
}

int ft_skill::lock_internal(bool *lock_acquired) const noexcept
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

int ft_skill::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return (FT_ERR_SUCCESS);
    int unlock_error = pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    if (unlock_error != FT_ERR_SUCCESS)
        this->set_error(unlock_error);
    else
        this->set_error(FT_ERR_SUCCESS);
    return (unlock_error);
}

int ft_skill::lock(bool *lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_skill::lock");
    return (this->lock_internal(lock_acquired));
}

void ft_skill::unlock(bool lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_skill::unlock");
    (void)this->unlock_internal(lock_acquired);
    return ;
}

int ft_skill::get_id() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int identifier;
    int unlock_error;

    this->abort_if_not_initialized("ft_skill::get_id");
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

void ft_skill::set_id(int id) noexcept
{
    bool lock_acquired;
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("ft_skill::set_id");
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

int ft_skill::get_level() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int level_value;
    int unlock_error;

    this->abort_if_not_initialized("ft_skill::get_level");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    level_value = this->_level;
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
        return (unlock_error);
    this->set_error(FT_ERR_SUCCESS);
    return (level_value);
}

void ft_skill::set_level(int level) noexcept
{
    bool lock_acquired;
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("ft_skill::set_level");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_level = level;
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
        return ;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int ft_skill::get_cooldown() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int cooldown_value;
    int unlock_error;

    this->abort_if_not_initialized("ft_skill::get_cooldown");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (lock_error);
    }
    cooldown_value = this->_cooldown;
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
        return (unlock_error);
    this->set_error(FT_ERR_SUCCESS);
    return (cooldown_value);
}

void ft_skill::set_cooldown(int cooldown) noexcept
{
    bool lock_acquired;
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("ft_skill::set_cooldown");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_cooldown = cooldown;
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
        return ;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_skill::add_cooldown(int cooldown) noexcept
{
    bool lock_acquired;
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("ft_skill::add_cooldown");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_cooldown += cooldown;
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
        return ;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_skill::sub_cooldown(int cooldown) noexcept
{
    bool lock_acquired;
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("ft_skill::sub_cooldown");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_cooldown -= cooldown;
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
        return ;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int ft_skill::get_modifier1() const noexcept
{
    this->abort_if_not_initialized("ft_skill::get_modifier1");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_modifier1);
}

void ft_skill::set_modifier1(int mod) noexcept
{
    this->abort_if_not_initialized("ft_skill::set_modifier1");
    this->_modifier1 = mod;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_skill::add_modifier1(int mod) noexcept
{
    this->abort_if_not_initialized("ft_skill::add_modifier1");
    this->_modifier1 += mod;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_skill::sub_modifier1(int mod) noexcept
{
    this->abort_if_not_initialized("ft_skill::sub_modifier1");
    this->_modifier1 -= mod;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int ft_skill::get_modifier2() const noexcept
{
    this->abort_if_not_initialized("ft_skill::get_modifier2");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_modifier2);
}

void ft_skill::set_modifier2(int mod) noexcept
{
    this->abort_if_not_initialized("ft_skill::set_modifier2");
    this->_modifier2 = mod;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_skill::add_modifier2(int mod) noexcept
{
    this->abort_if_not_initialized("ft_skill::add_modifier2");
    this->_modifier2 += mod;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_skill::sub_modifier2(int mod) noexcept
{
    this->abort_if_not_initialized("ft_skill::sub_modifier2");
    this->_modifier2 -= mod;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int ft_skill::get_modifier3() const noexcept
{
    this->abort_if_not_initialized("ft_skill::get_modifier3");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_modifier3);
}

void ft_skill::set_modifier3(int mod) noexcept
{
    this->abort_if_not_initialized("ft_skill::set_modifier3");
    this->_modifier3 = mod;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_skill::add_modifier3(int mod) noexcept
{
    this->abort_if_not_initialized("ft_skill::add_modifier3");
    this->_modifier3 += mod;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_skill::sub_modifier3(int mod) noexcept
{
    this->abort_if_not_initialized("ft_skill::sub_modifier3");
    this->_modifier3 -= mod;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int ft_skill::get_modifier4() const noexcept
{
    this->abort_if_not_initialized("ft_skill::get_modifier4");
    this->set_error(FT_ERR_SUCCESS);
    return (this->_modifier4);
}

void ft_skill::set_modifier4(int mod) noexcept
{
    this->abort_if_not_initialized("ft_skill::set_modifier4");
    this->_modifier4 = mod;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_skill::add_modifier4(int mod) noexcept
{
    this->abort_if_not_initialized("ft_skill::add_modifier4");
    this->_modifier4 += mod;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_skill::sub_modifier4(int mod) noexcept
{
    this->abort_if_not_initialized("ft_skill::sub_modifier4");
    this->_modifier4 -= mod;
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *ft_skill::get_mutex_for_validation() const noexcept
{
    this->abort_if_not_initialized("ft_skill::get_mutex_for_validation");
    return (this->_mutex);
}
#endif
