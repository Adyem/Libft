#include "../PThread/pthread_internal.hpp"
#include "game_debuff.hpp"
#include "../Printf/printf.hpp"
 #include "../System_utils/system_utils.hpp"

thread_local int ft_debuff::_last_error = FT_ERR_SUCCESS;

void ft_debuff::set_error(int error_code) const noexcept
{
    ft_debuff::_last_error = error_code;
    return ;
}

int ft_debuff::get_error() const noexcept
{
    return (ft_debuff::_last_error);
}

const char *ft_debuff::get_error_str() const noexcept
{
    return (ft_strerror(this->get_error()));
}

ft_debuff::ft_debuff() noexcept
    : _id(0), _duration(0), _modifier1(0), _modifier2(0), _modifier3(0),
      _modifier4(0), _mutex(ft_nullptr),
      _initialized_state(ft_debuff::_state_uninitialized)
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_debuff::~ft_debuff() noexcept
{
    if (this->_initialized_state == ft_debuff::_state_uninitialized)
        return ;
    if (this->_initialized_state == ft_debuff::_state_initialized)
        (void)this->destroy();
    return ;
}

void ft_debuff::abort_lifecycle_error(const char *method_name,
    const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_debuff lifecycle error: %s: %s\n", method_name, reason);
    su_abort();
    return ;
}

void ft_debuff::abort_if_not_initialized(const char *method_name) const
{
    if (this->_initialized_state == ft_debuff::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int ft_debuff::initialize() noexcept
{
    if (this->_initialized_state == ft_debuff::_state_initialized)
    {
        this->abort_lifecycle_error("ft_debuff::initialize",
            "called while object is already initialized");
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    this->_id = 0;
    this->_duration = 0;
    this->_modifier1 = 0;
    this->_modifier2 = 0;
    this->_modifier3 = 0;
    this->_modifier4 = 0;
    this->_initialized_state = ft_debuff::_state_initialized;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_debuff::initialize(const ft_debuff &other) noexcept
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
    this->_duration = other._duration;
    this->_modifier1 = other._modifier1;
    this->_modifier2 = other._modifier2;
    this->_modifier3 = other._modifier3;
    this->_modifier4 = other._modifier4;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_debuff::initialize(ft_debuff &&other) noexcept
{
    int result = this->initialize(static_cast<const ft_debuff &>(other));
    this->set_error(result);
    return (result);
}

int ft_debuff::destroy() noexcept
{
    int disable_error;

    if (this->_initialized_state != ft_debuff::_state_initialized)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    disable_error = this->disable_thread_safety();
    this->_id = 0;
    this->_duration = 0;
    this->_modifier1 = 0;
    this->_modifier2 = 0;
    this->_modifier3 = 0;
    this->_modifier4 = 0;
    this->_initialized_state = ft_debuff::_state_destroyed;
    this->set_error(disable_error);
    return (disable_error);
}

int ft_debuff::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int initialize_error;

    this->abort_if_not_initialized("ft_debuff::enable_thread_safety");
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

int ft_debuff::disable_thread_safety() noexcept
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

bool ft_debuff::is_thread_safe() const noexcept
{
    this->abort_if_not_initialized("ft_debuff::is_thread_safe");
    bool result = (this->_mutex != ft_nullptr);
    this->set_error(FT_ERR_SUCCESS);
    return (result);
}

int ft_debuff::lock_internal(bool *lock_acquired) const noexcept
{
    int lock_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

int ft_debuff::unlock_internal(bool lock_acquired) const noexcept
{
    if (lock_acquired == false)
        return (FT_ERR_SUCCESS);
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (pt_recursive_mutex_unlock_if_not_null(this->_mutex));
}

int ft_debuff::lock(bool *lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_debuff::lock");
    int lock_result = this->lock_internal(lock_acquired);
    this->set_error(lock_result);
    return (lock_result);
}

void ft_debuff::unlock(bool lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_debuff::unlock");
    int unlock_result = this->unlock_internal(lock_acquired);
    this->set_error(unlock_result);
    (void)unlock_result;
    return ;
}

int ft_debuff::get_id() const noexcept
{
    this->abort_if_not_initialized("ft_debuff::get_id");
    return (this->_id);
}

void ft_debuff::set_id(int id) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_debuff::set_id");
    if (id < 0)
        return ;
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    this->_id = id;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

int ft_debuff::get_duration() const noexcept
{
    this->abort_if_not_initialized("ft_debuff::get_duration");
    return (this->_duration);
}

void ft_debuff::set_duration(int duration) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_debuff::set_duration");
    if (duration < 0)
        return ;
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    this->_duration = duration;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

void ft_debuff::add_duration(int duration) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_debuff::add_duration");
    if (duration < 0)
        return ;
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    this->_duration += duration;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

void ft_debuff::sub_duration(int duration) noexcept
{
    bool lock_acquired;
    int lock_error;

    this->abort_if_not_initialized("ft_debuff::sub_duration");
    if (duration < 0)
        return ;
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    this->_duration -= duration;
    (void)this->unlock_internal(lock_acquired);
    return ;
}

int ft_debuff::get_modifier1() const noexcept
{
    this->abort_if_not_initialized("ft_debuff::get_modifier1");
    return (this->_modifier1);
}

void ft_debuff::set_modifier1(int mod) noexcept
{
    this->abort_if_not_initialized("ft_debuff::set_modifier1");
    this->_modifier1 = mod;
    return ;
}

void ft_debuff::add_modifier1(int mod) noexcept
{
    this->abort_if_not_initialized("ft_debuff::add_modifier1");
    this->_modifier1 += mod;
    return ;
}

void ft_debuff::sub_modifier1(int mod) noexcept
{
    this->abort_if_not_initialized("ft_debuff::sub_modifier1");
    this->_modifier1 -= mod;
    return ;
}

int ft_debuff::get_modifier2() const noexcept
{
    this->abort_if_not_initialized("ft_debuff::get_modifier2");
    return (this->_modifier2);
}

void ft_debuff::set_modifier2(int mod) noexcept
{
    this->abort_if_not_initialized("ft_debuff::set_modifier2");
    this->_modifier2 = mod;
    return ;
}

void ft_debuff::add_modifier2(int mod) noexcept
{
    this->abort_if_not_initialized("ft_debuff::add_modifier2");
    this->_modifier2 += mod;
    return ;
}

void ft_debuff::sub_modifier2(int mod) noexcept
{
    this->abort_if_not_initialized("ft_debuff::sub_modifier2");
    this->_modifier2 -= mod;
    return ;
}

int ft_debuff::get_modifier3() const noexcept
{
    this->abort_if_not_initialized("ft_debuff::get_modifier3");
    return (this->_modifier3);
}

void ft_debuff::set_modifier3(int mod) noexcept
{
    this->abort_if_not_initialized("ft_debuff::set_modifier3");
    this->_modifier3 = mod;
    return ;
}

void ft_debuff::add_modifier3(int mod) noexcept
{
    this->abort_if_not_initialized("ft_debuff::add_modifier3");
    this->_modifier3 += mod;
    return ;
}

void ft_debuff::sub_modifier3(int mod) noexcept
{
    this->abort_if_not_initialized("ft_debuff::sub_modifier3");
    this->_modifier3 -= mod;
    return ;
}

int ft_debuff::get_modifier4() const noexcept
{
    this->abort_if_not_initialized("ft_debuff::get_modifier4");
    return (this->_modifier4);
}

void ft_debuff::set_modifier4(int mod) noexcept
{
    this->abort_if_not_initialized("ft_debuff::set_modifier4");
    this->_modifier4 = mod;
    return ;
}

void ft_debuff::add_modifier4(int mod) noexcept
{
    this->abort_if_not_initialized("ft_debuff::add_modifier4");
    this->_modifier4 += mod;
    return ;
}

void ft_debuff::sub_modifier4(int mod) noexcept
{
    this->abort_if_not_initialized("ft_debuff::sub_modifier4");
    this->_modifier4 -= mod;
    return ;
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *ft_debuff::get_mutex_for_validation() const noexcept
{
    this->abort_if_not_initialized("ft_debuff::get_mutex_for_validation");
    return (this->_mutex);
}
#endif
