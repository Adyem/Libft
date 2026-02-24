#include "game_buff.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"

ft_buff::ft_buff() noexcept
    : _id(0), _duration(0), _modifier1(0), _modifier2(0), _modifier3(0),
      _modifier4(0), _mutex(ft_nullptr),
      _initialized_state(ft_buff::_state_uninitialized)
{
    return ;
}

ft_buff::~ft_buff() noexcept
{
    if (this->_initialized_state == ft_buff::_state_uninitialized)
        return ;
    if (this->_initialized_state == ft_buff::_state_initialized)
        (void)this->destroy();
    return ;
}

void ft_buff::abort_lifecycle_error(const char *method_name,
    const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_buff lifecycle error: %s: %s\n", method_name, reason);
    su_abort();
    return ;
}

void ft_buff::abort_if_not_initialized(const char *method_name) const
{
    if (this->_initialized_state == ft_buff::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int ft_buff::initialize() noexcept
{
    if (this->_initialized_state == ft_buff::_state_initialized)
    {
        this->abort_lifecycle_error("ft_buff::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_id = 0;
    this->_duration = 0;
    this->_modifier1 = 0;
    this->_modifier2 = 0;
    this->_modifier3 = 0;
    this->_modifier4 = 0;
    this->_initialized_state = ft_buff::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int ft_buff::initialize(const ft_buff &other) noexcept
{
    int initialize_error;

    if (&other == this)
        return (FT_ERR_SUCCESS);
    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    this->_id = other._id;
    this->_duration = other._duration;
    this->_modifier1 = other._modifier1;
    this->_modifier2 = other._modifier2;
    this->_modifier3 = other._modifier3;
    this->_modifier4 = other._modifier4;
    return (FT_ERR_SUCCESS);
}

int ft_buff::initialize(ft_buff &&other) noexcept
{
    return (this->initialize(static_cast<const ft_buff &>(other)));
}

int ft_buff::destroy() noexcept
{
    int disable_error;

    if (this->_initialized_state != ft_buff::_state_initialized)
        return (FT_ERR_INVALID_STATE);
    disable_error = this->disable_thread_safety();
    this->_id = 0;
    this->_duration = 0;
    this->_modifier1 = 0;
    this->_modifier2 = 0;
    this->_modifier3 = 0;
    this->_modifier4 = 0;
    this->_initialized_state = ft_buff::_state_destroyed;
    return (disable_error);
}

int ft_buff::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int initialize_error;

    this->abort_if_not_initialized("ft_buff::enable_thread_safety");
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

int ft_buff::disable_thread_safety() noexcept
{
    int destroy_error;

    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    delete this->_mutex;
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

bool ft_buff::is_thread_safe() const noexcept
{
    this->abort_if_not_initialized("ft_buff::is_thread_safe");
    return (this->_mutex != ft_nullptr);
}

int ft_buff::lock_internal(bool *lock_acquired) const noexcept
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

int ft_buff::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return (FT_ERR_SUCCESS);
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (this->_mutex->unlock());
}

int ft_buff::lock(bool *lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_buff::lock");
    return (this->lock_internal(lock_acquired));
}

void ft_buff::unlock(bool lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_buff::unlock");
    (void)this->unlock_internal(lock_acquired);
    return ;
}

int ft_buff::get_id() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int value;

    this->abort_if_not_initialized("ft_buff::get_id");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (0);
    value = this->_id;
    (void)this->unlock_internal(lock_acquired);
    return (value);
}

void ft_buff::set_id(int id) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_buff::set_id");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    if (id >= 0)
        this->_id = id;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

int ft_buff::get_duration() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int value;

    this->abort_if_not_initialized("ft_buff::get_duration");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (0);
    value = this->_duration;
    (void)this->unlock_internal(lock_acquired);
    return (value);
}

void ft_buff::set_duration(int duration) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_buff::set_duration");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    if (duration >= 0)
        this->_duration = duration;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

void ft_buff::add_duration(int duration) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_buff::add_duration");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    if (duration >= 0)
        this->_duration += duration;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

void ft_buff::sub_duration(int duration) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_buff::sub_duration");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    if (duration >= 0)
        this->_duration -= duration;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

int ft_buff::get_modifier1() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int value;

    this->abort_if_not_initialized("ft_buff::get_modifier1");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (0);
    value = this->_modifier1;
    (void)this->unlock_internal(lock_acquired);
    return (value);
}

void ft_buff::set_modifier1(int mod) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_buff::set_modifier1");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    this->_modifier1 = mod;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

void ft_buff::add_modifier1(int mod) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_buff::add_modifier1");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    this->_modifier1 += mod;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

void ft_buff::sub_modifier1(int mod) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_buff::sub_modifier1");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    this->_modifier1 -= mod;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

int ft_buff::get_modifier2() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int value;

    this->abort_if_not_initialized("ft_buff::get_modifier2");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (0);
    value = this->_modifier2;
    (void)this->unlock_internal(lock_acquired);
    return (value);
}

void ft_buff::set_modifier2(int mod) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_buff::set_modifier2");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    this->_modifier2 = mod;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

void ft_buff::add_modifier2(int mod) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_buff::add_modifier2");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    this->_modifier2 += mod;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

void ft_buff::sub_modifier2(int mod) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_buff::sub_modifier2");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    this->_modifier2 -= mod;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

int ft_buff::get_modifier3() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int value;

    this->abort_if_not_initialized("ft_buff::get_modifier3");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (0);
    value = this->_modifier3;
    (void)this->unlock_internal(lock_acquired);
    return (value);
}

void ft_buff::set_modifier3(int mod) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_buff::set_modifier3");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    this->_modifier3 = mod;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

void ft_buff::add_modifier3(int mod) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_buff::add_modifier3");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    this->_modifier3 += mod;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

void ft_buff::sub_modifier3(int mod) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_buff::sub_modifier3");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    this->_modifier3 -= mod;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

int ft_buff::get_modifier4() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int value;

    this->abort_if_not_initialized("ft_buff::get_modifier4");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (0);
    value = this->_modifier4;
    (void)this->unlock_internal(lock_acquired);
    return (value);
}

void ft_buff::set_modifier4(int mod) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_buff::set_modifier4");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    this->_modifier4 = mod;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

void ft_buff::add_modifier4(int mod) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_buff::add_modifier4");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    this->_modifier4 += mod;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

void ft_buff::sub_modifier4(int mod) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_buff::sub_modifier4");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    this->_modifier4 -= mod;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *ft_buff::get_mutex_for_validation() const noexcept
{
    return (this->_mutex);
}
#endif
