#include "game_reputation.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"

thread_local int ft_reputation::_last_error = FT_ERR_SUCCESS;

void ft_reputation::set_error(int error_code) const noexcept
{
    ft_reputation::_last_error = error_code;
    return ;
}

int ft_reputation::get_error() const noexcept
{
    return (ft_reputation::_last_error);
}

const char *ft_reputation::get_error_str() const noexcept
{
    return (ft_strerror(this->get_error()));
}

ft_reputation::ft_reputation() noexcept
    : _milestones(), _reps(), _total_rep(0), _current_rep(0),
      _mutex(ft_nullptr), _initialized_state(ft_reputation::_state_uninitialized)
{
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_reputation::~ft_reputation() noexcept
{
    if (this->_initialized_state == ft_reputation::_state_uninitialized)
        return ;
    if (this->_initialized_state == ft_reputation::_state_initialized)
        (void)this->destroy();
    return ;
}

void ft_reputation::abort_lifecycle_error(const char *method_name,
    const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_reputation lifecycle error: %s: %s\n", method_name,
        reason);
    su_abort();
    return ;
}

void ft_reputation::abort_if_not_initialized(const char *method_name) const
{
    if (this->_initialized_state == ft_reputation::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int ft_reputation::initialize() noexcept
{
    if (this->_initialized_state == ft_reputation::_state_initialized)
    {
        this->abort_lifecycle_error("ft_reputation::initialize",
            "called while object is already initialized");
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    this->_milestones.clear();
    this->_reps.clear();
    this->_total_rep = 0;
    this->_current_rep = 0;
    this->_initialized_state = ft_reputation::_state_initialized;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_reputation::initialize(const ft_map<int, int> &milestones,
    int total) noexcept
{
    int initialize_error;

    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    this->_milestones = milestones;
    this->_total_rep = total;
    this->_current_rep = 0;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_reputation::initialize(const ft_reputation &other) noexcept
{
    int initialize_error;

    if (&other == this)
        return (FT_ERR_SUCCESS);
    initialize_error = this->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
        return (initialize_error);
    this->_milestones = other._milestones;
    this->_reps = other._reps;
    this->_total_rep = other._total_rep;
    this->_current_rep = other._current_rep;
    this->set_error(FT_ERR_SUCCESS);
    return (FT_ERR_SUCCESS);
}

int ft_reputation::initialize(ft_reputation &&other) noexcept
{
    return (this->initialize(static_cast<const ft_reputation &>(other)));
}

int ft_reputation::destroy() noexcept
{
    int disable_error;

    if (this->_initialized_state != ft_reputation::_state_initialized)
    {
        this->set_error(FT_ERR_INVALID_STATE);
        return (FT_ERR_INVALID_STATE);
    }
    this->_milestones.clear();
    this->_reps.clear();
    this->_total_rep = 0;
    this->_current_rep = 0;
    disable_error = this->disable_thread_safety();
    this->_initialized_state = ft_reputation::_state_destroyed;
    this->set_error(disable_error);
    return (disable_error);
}

int ft_reputation::enable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int initialize_error;

    this->abort_if_not_initialized("ft_reputation::enable_thread_safety");
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

int ft_reputation::disable_thread_safety() noexcept
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

bool ft_reputation::is_thread_safe() const noexcept
{
    this->abort_if_not_initialized("ft_reputation::is_thread_safe");
    const bool result = (this->_mutex != ft_nullptr);
    this->set_error(FT_ERR_SUCCESS);
    return (result);
}

int ft_reputation::lock_internal(bool *lock_acquired) const noexcept
{
    int lock_error;

    this->abort_if_not_initialized("ft_reputation::lock_internal");
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

int ft_reputation::unlock_internal(bool lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_reputation::unlock_internal");
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

int ft_reputation::lock(bool *lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_reputation::lock");
    return (this->lock_internal(lock_acquired));
}

void ft_reputation::unlock(bool lock_acquired) const noexcept
{
    this->abort_if_not_initialized("ft_reputation::unlock");
    (void)this->unlock_internal(lock_acquired);
    return ;
}

int ft_reputation::get_total_rep() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int total;
    int unlock_error;

    this->abort_if_not_initialized("ft_reputation::get_total_rep");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (0);
    }
    total = this->_total_rep;
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        this->set_error(unlock_error);
        return (0);
    }
    this->set_error(FT_ERR_SUCCESS);
    return (total);
}

void ft_reputation::set_total_rep(int rep) noexcept
{
    bool lock_acquired;
    int lock_error;
    int unlock_error;
    bool valid;

    this->abort_if_not_initialized("ft_reputation::set_total_rep");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    valid = (rep >= 0);
    if (valid)
        this->_total_rep = rep;
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        this->set_error(unlock_error);
        return ;
    }
    this->set_error(valid ? FT_ERR_SUCCESS : FT_ERR_INVALID_ARGUMENT);
    return ;
}

void ft_reputation::add_total_rep(int rep) noexcept
{
    bool lock_acquired;
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("ft_reputation::add_total_rep");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_total_rep += rep;
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        this->set_error(unlock_error);
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_reputation::sub_total_rep(int rep) noexcept
{
    bool lock_acquired;
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("ft_reputation::sub_total_rep");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_total_rep -= rep;
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        this->set_error(unlock_error);
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int ft_reputation::get_current_rep() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int current;
    int unlock_error;

    this->abort_if_not_initialized("ft_reputation::get_current_rep");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (0);
    }
    current = this->_current_rep;
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        this->set_error(unlock_error);
        return (0);
    }
    this->set_error(FT_ERR_SUCCESS);
    return (current);
}

void ft_reputation::set_current_rep(int rep) noexcept
{
    bool lock_acquired;
    int lock_error;
    int unlock_error;
    bool valid;

    this->abort_if_not_initialized("ft_reputation::set_current_rep");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    valid = (rep >= 0);
    if (valid)
        this->_current_rep = rep;
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        this->set_error(unlock_error);
        return ;
    }
    this->set_error(valid ? FT_ERR_SUCCESS : FT_ERR_INVALID_ARGUMENT);
    return ;
}

void ft_reputation::add_current_rep(int rep) noexcept
{
    bool lock_acquired;
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("ft_reputation::add_current_rep");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_current_rep += rep;
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        this->set_error(unlock_error);
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

void ft_reputation::sub_current_rep(int rep) noexcept
{
    bool lock_acquired;
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("ft_reputation::sub_current_rep");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_current_rep -= rep;
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        this->set_error(unlock_error);
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_map<int, int> &ft_reputation::get_milestones() noexcept
{
    bool lock_acquired;
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("ft_reputation::get_milestones");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (this->_milestones);
    }
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        this->set_error(unlock_error);
        return (this->_milestones);
    }
    this->set_error(FT_ERR_SUCCESS);
    return (this->_milestones);
}

const ft_map<int, int> &ft_reputation::get_milestones() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("ft_reputation::get_milestones const");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (this->_milestones);
    }
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        this->set_error(unlock_error);
        return (this->_milestones);
    }
    this->set_error(FT_ERR_SUCCESS);
    return (this->_milestones);
}

void ft_reputation::set_milestones(const ft_map<int, int> &milestones) noexcept
{
    bool lock_acquired;
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("ft_reputation::set_milestones");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_milestones = milestones;
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        this->set_error(unlock_error);
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int ft_reputation::get_milestone(int id) const noexcept
{
    const Pair<int, int> *entry;

    this->abort_if_not_initialized("ft_reputation::get_milestone");
    bool lock_acquired;
    int lock_error;
    int value;

    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (0);
    }
    entry = this->_milestones.find(id);
    if (entry == this->_milestones.end())
    {
        int unlock_error = this->unlock_internal(lock_acquired);
        if (unlock_error != FT_ERR_SUCCESS)
        {
            this->set_error(unlock_error);
            return (0);
        }
        this->set_error(FT_ERR_NOT_FOUND);
        return (0);
    }
    value = entry->value;
    int unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        this->set_error(unlock_error);
        return (0);
    }
    this->set_error(FT_ERR_SUCCESS);
    return (value);
}

void ft_reputation::set_milestone(int id, int value) noexcept
{
    bool lock_acquired;
    int lock_error;
    Pair<int, int> *entry;

    this->abort_if_not_initialized("ft_reputation::set_milestone");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    entry = this->_milestones.find(id);
    if (entry == this->_milestones.end())
        this->_milestones.insert(id, value);
    else
        entry->value = value;
    int unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        this->set_error(unlock_error);
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

ft_map<int, int> &ft_reputation::get_reps() noexcept
{
    bool lock_acquired;
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("ft_reputation::get_reps");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (this->_reps);
    }
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        this->set_error(unlock_error);
        return (this->_reps);
    }
    this->set_error(FT_ERR_SUCCESS);
    return (this->_reps);
}

const ft_map<int, int> &ft_reputation::get_reps() const noexcept
{
    bool lock_acquired;
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("ft_reputation::get_reps const");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (this->_reps);
    }
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        this->set_error(unlock_error);
        return (this->_reps);
    }
    this->set_error(FT_ERR_SUCCESS);
    return (this->_reps);
}

void ft_reputation::set_reps(const ft_map<int, int> &reps) noexcept
{
    bool lock_acquired;
    int lock_error;
    int unlock_error;

    this->abort_if_not_initialized("ft_reputation::set_reps");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    this->_reps = reps;
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        this->set_error(unlock_error);
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

int ft_reputation::get_rep(int id) const noexcept
{
    const Pair<int, int> *entry;
    bool lock_acquired;
    int lock_error;
    int value;
    int unlock_error;

    this->abort_if_not_initialized("ft_reputation::get_rep");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return (0);
    }
    entry = this->_reps.find(id);
    if (entry == this->_reps.end())
    {
        unlock_error = this->unlock_internal(lock_acquired);
        if (unlock_error != FT_ERR_SUCCESS)
        {
            this->set_error(unlock_error);
            return (0);
        }
        this->set_error(FT_ERR_NOT_FOUND);
        return (0);
    }
    value = entry->value;
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        this->set_error(unlock_error);
        return (0);
    }
    this->set_error(FT_ERR_SUCCESS);
    return (value);
}

void ft_reputation::set_rep(int id, int value) noexcept
{
    bool lock_acquired;
    int lock_error;
    Pair<int, int> *entry;
    int unlock_error;

    this->abort_if_not_initialized("ft_reputation::set_rep");
    lock_acquired = false;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        this->set_error(lock_error);
        return ;
    }
    entry = this->_reps.find(id);
    if (entry == this->_reps.end())
        this->_reps.insert(id, value);
    else
        entry->value = value;
    unlock_error = this->unlock_internal(lock_acquired);
    if (unlock_error != FT_ERR_SUCCESS)
    {
        this->set_error(unlock_error);
        return ;
    }
    this->set_error(FT_ERR_SUCCESS);
    return ;
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *ft_reputation::get_mutex_for_validation() const noexcept
{
    this->abort_if_not_initialized("ft_reputation::get_mutex_for_validation");
    return (this->_mutex);
}
#endif
