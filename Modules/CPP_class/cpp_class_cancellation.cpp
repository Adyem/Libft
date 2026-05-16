#include "cancellation.hpp"

int32_t ft_cancellation_state::set_error(int32_t error_code) noexcept
{
    _last_error = error_code;
    return (error_code);
}

int32_t ft_cancellation_source::set_error(int32_t error_code) noexcept
{
    _last_error = error_code;
    return (error_code);
}

int32_t ft_cancellation_state::lock_internal(ft_bool *lock_acquired) const
{
    int32_t lock_result;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = FT_FALSE;
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    lock_result = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_result != FT_ERR_SUCCESS)
        return (set_error(lock_result));
    if (lock_acquired != ft_nullptr)
        *lock_acquired = FT_TRUE;
    return (FT_ERR_SUCCESS);
}

int32_t ft_cancellation_state::unlock_internal(ft_bool lock_acquired) const
{
    if (lock_acquired == FT_FALSE)
        return (FT_ERR_SUCCESS);
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (FT_ERR_SUCCESS);
}

ft_cancellation_state::ft_cancellation_state() noexcept
    : _cancelled(FT_FALSE), _callbacks(), _mutex(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    return ;
}

ft_cancellation_state::~ft_cancellation_state() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    if (this->_mutex != ft_nullptr)
        (void)this->disable_thread_safety();
    return ;
}

int32_t ft_cancellation_state::initialize() noexcept
{
    int32_t callbacks_result;

    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(this->_initialised_state, "ft_cancellation_state::initialize", "called while object is already initialised");
        return (set_error(FT_ERR_INVALID_STATE));
    }
    this->_cancelled.store(FT_FALSE, std::memory_order_release);
    callbacks_result = this->_callbacks.initialize();
    if (callbacks_result != FT_ERR_SUCCESS)
    {
        (void)this->_callbacks.destroy();
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (set_error(callbacks_result));
    }
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (set_error(FT_ERR_SUCCESS));
}

int32_t ft_cancellation_state::destroy() noexcept
{
    ft_bool lock_acquired;
    int32_t lock_error;
    int32_t disable_error;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
        return (set_error(FT_ERR_SUCCESS));
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (set_error(lock_error));
    this->_callbacks.clear();
    this->_cancelled.store(FT_FALSE, std::memory_order_release);
    (void)this->unlock_internal(lock_acquired);
    disable_error = this->disable_thread_safety();
    (void)this->_callbacks.destroy();
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    if (disable_error != FT_ERR_SUCCESS)
        return (set_error(disable_error));
    return (set_error(FT_ERR_SUCCESS));
}

int32_t ft_cancellation_state::move(ft_cancellation_state &other) noexcept
{
    int32_t callbacks_move_result;
    int32_t destroy_result;

    if (this == &other)
        return (set_error(FT_ERR_SUCCESS));
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state,
            "ft_cancellation_state::move", "source object is uninitialised");
        return (set_error(FT_ERR_INVALID_STATE));
    }
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        destroy_result = this->destroy();
        if (destroy_result != FT_ERR_SUCCESS)
            return (set_error(destroy_result));
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_cancelled.store(FT_FALSE, std::memory_order_release);
        this->_mutex = ft_nullptr;
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (set_error(FT_ERR_SUCCESS));
    }
    callbacks_move_result = this->_callbacks.move(other._callbacks);
    if (callbacks_move_result != FT_ERR_SUCCESS)
    {
        this->_cancelled.store(FT_FALSE, std::memory_order_release);
        this->_mutex = ft_nullptr;
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (set_error(callbacks_move_result));
    }
    this->_cancelled.store(other._cancelled.load(std::memory_order_acquire),
        std::memory_order_release);
    this->_mutex = other._mutex;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    other._cancelled.store(FT_FALSE, std::memory_order_release);
    other._mutex = ft_nullptr;
    other._initialised_state = FT_CLASS_STATE_DESTROYED;
    return (set_error(FT_ERR_SUCCESS));
}

int32_t ft_cancellation_state::register_callback(
    const ft_function<void()> &callback) noexcept
{
    ft_bool already_cancelled;
    ft_bool lock_acquired;
    int32_t lock_error;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_cancellation_state::register_callback");
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (set_error(lock_error));
    already_cancelled = this->_cancelled.load(std::memory_order_acquire);
    if (already_cancelled == FT_FALSE)
        this->_callbacks.push_back(callback);
    (void)this->unlock_internal(lock_acquired);
    if (already_cancelled == FT_TRUE)
        callback();
    return (set_error(FT_ERR_SUCCESS));
}

int32_t ft_cancellation_state::request_cancel() noexcept
{
    ft_vector<ft_function<void()> > callbacks_to_run;
    ft_bool lock_acquired;
    int32_t lock_error;
    int32_t callbacks_result;
    ft_size_t index;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_cancellation_state::request_cancel");
    if (this->_cancelled.exchange(FT_TRUE, std::memory_order_acq_rel))
        return (set_error(FT_ERR_SUCCESS));
    callbacks_result = callbacks_to_run.initialize();
    if (callbacks_result != FT_ERR_SUCCESS)
        return (set_error(callbacks_result));
    lock_acquired = FT_FALSE;
    lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
    {
        (void)callbacks_to_run.destroy();
        return (set_error(lock_error));
    }
    index = 0;
    while (index < this->_callbacks.size())
    {
        callbacks_to_run.push_back(this->_callbacks[index]);
        ++index;
    }
    this->_callbacks.clear();
    (void)this->unlock_internal(lock_acquired);
    index = 0;
    while (index < callbacks_to_run.size())
    {
        callbacks_to_run[index]();
        ++index;
    }
    (void)callbacks_to_run.destroy();
    return (set_error(FT_ERR_SUCCESS));
}

ft_bool ft_cancellation_state::is_cancelled() const noexcept
{
    ft_bool cancelled;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
    {
        set_error(FT_ERR_INVALID_STATE);
        return (FT_TRUE);
    }
    cancelled = this->_cancelled.load(std::memory_order_acquire);
    set_error(FT_ERR_SUCCESS);
    return (cancelled);
}

int32_t ft_cancellation_state::enable_thread_safety() noexcept
{
    pt_recursive_mutex *new_mutex;
    int32_t initialize_result;

    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_cancellation_state::enable_thread_safety");
    if (this->_mutex != ft_nullptr)
        return (set_error(FT_ERR_SUCCESS));
    new_mutex = new (std::nothrow) pt_recursive_mutex();
    if (new_mutex == ft_nullptr)
        return (set_error(FT_ERR_NO_MEMORY));
    initialize_result = new_mutex->initialize();
    if (initialize_result != FT_ERR_SUCCESS)
    {
        delete new_mutex;
        return (set_error(initialize_result));
    }
    this->_mutex = new_mutex;
    return (set_error(FT_ERR_SUCCESS));
}

int32_t ft_cancellation_state::disable_thread_safety() noexcept
{
    pt_recursive_mutex *mutex_pointer;
    int32_t destroy_result;

    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED
        && this->_initialised_state != FT_CLASS_STATE_DESTROYED)
        return (set_error(FT_ERR_INVALID_STATE));
    mutex_pointer = this->_mutex;
    if (mutex_pointer == ft_nullptr)
        return (set_error(FT_ERR_SUCCESS));
    this->_mutex = ft_nullptr;
    destroy_result = mutex_pointer->destroy();
    delete mutex_pointer;
    if (destroy_result != FT_ERR_SUCCESS)
        return (set_error(destroy_result));
    return (set_error(FT_ERR_SUCCESS));
}

int32_t ft_cancellation_state::get_error() const noexcept
{
    errno_abort_if_uninitialised(this->_initialised_state,
        "ft_cancellation_state::get_error");
    return (this->_last_error);
}

const char *ft_cancellation_state::get_error_str() const noexcept
{
    errno_abort_if_uninitialised(this->_initialised_state,
        "ft_cancellation_state::get_error_str");
    return (ft_strerror(this->_last_error));
}

ft_cancellation_token::ft_cancellation_token(
    ft_cancellation_state *state_pointer) noexcept
    : _state(state_pointer)
{
    return ;
}

ft_cancellation_token::ft_cancellation_token() noexcept : _state(ft_nullptr)
{
    return ;
}

ft_cancellation_token::ft_cancellation_token(
    const ft_cancellation_token &other) noexcept : _state(other._state)
{
    return ;
}

ft_cancellation_token::ft_cancellation_token(
    ft_cancellation_token &&other) noexcept : _state(other._state)
{
    other._state = ft_nullptr;
    return ;
}

ft_cancellation_token::~ft_cancellation_token() noexcept
{
    return ;
}

ft_bool ft_cancellation_token::is_valid() const noexcept
{
    return (this->_state != ft_nullptr);
}

ft_bool ft_cancellation_token::is_cancellation_requested() const noexcept
{
    if (this->_state == ft_nullptr)
        return (FT_TRUE);
    return (this->_state->is_cancelled());
}

int32_t ft_cancellation_token::register_callback(
    const ft_function<void()> &callback) const noexcept
{
    if (this->_state == ft_nullptr)
        return (FT_ERR_INVALID_STATE);
    return (this->_state->register_callback(callback));
}

ft_cancellation_source::ft_cancellation_source() noexcept
    : _state(ft_nullptr), _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    return ;
}

ft_cancellation_source::~ft_cancellation_source() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    return ;
}

int32_t ft_cancellation_source::initialize() noexcept
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        errno_abort_lifecycle(this->_initialised_state, "ft_cancellation_source::initialize", "called while object is already initialised");
        return (set_error(FT_ERR_INVALID_STATE));
    }
    this->_state = new (std::nothrow) ft_cancellation_state();
    if (this->_state == ft_nullptr)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (set_error(FT_ERR_NO_MEMORY));
    }
    if (this->_state->initialize() != FT_ERR_SUCCESS)
    {
        delete this->_state;
        this->_state = ft_nullptr;
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (set_error(FT_ERR_INVALID_STATE));
    }
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (set_error(FT_ERR_SUCCESS));
}

int32_t ft_cancellation_source::destroy() noexcept
{
    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
        return (set_error(FT_ERR_SUCCESS));
    if (this->_state != ft_nullptr)
    {
        delete this->_state;
        this->_state = ft_nullptr;
    }
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return (set_error(FT_ERR_SUCCESS));
}

int32_t ft_cancellation_source::move(ft_cancellation_source &other) noexcept
{
    int32_t destroy_result;

    if (this == &other)
        return (set_error(FT_ERR_SUCCESS));
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
    {
        errno_abort_lifecycle(other._initialised_state,
            "ft_cancellation_source::move", "source object is uninitialised");
        return (set_error(FT_ERR_INVALID_STATE));
    }
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
    {
        destroy_result = this->destroy();
        if (destroy_result != FT_ERR_SUCCESS)
            return (set_error(destroy_result));
    }
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_state = ft_nullptr;
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (set_error(FT_ERR_SUCCESS));
    }
    this->_state = other._state;
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    other._state = ft_nullptr;
    other._initialised_state = FT_CLASS_STATE_DESTROYED;
    return (set_error(FT_ERR_SUCCESS));
}

ft_cancellation_token ft_cancellation_source::get_token() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_cancellation_source::get_token");
    return (ft_cancellation_token(this->_state));
}

int32_t ft_cancellation_source::request_cancel() noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_cancellation_source::request_cancel");
    if (this->_state == ft_nullptr)
        return (set_error(FT_ERR_INVALID_STATE));
    return (set_error(this->_state->request_cancel()));
}

ft_bool ft_cancellation_source::is_cancellation_requested() const noexcept
{
    errno_abort_if_uninitialised_or_destroyed(this->_initialised_state, "ft_cancellation_source::is_cancellation_requested");
    if (this->_state == ft_nullptr)
    {
        (void)set_error(FT_ERR_INVALID_STATE);
        return (FT_TRUE);
    }
    if (this->_state->is_cancelled() == FT_TRUE)
    {
        (void)set_error(FT_ERR_SUCCESS);
        return (FT_TRUE);
    }
    (void)set_error(FT_ERR_SUCCESS);
    return (FT_FALSE);
}

int32_t ft_cancellation_source::get_error() const noexcept
{
    errno_abort_if_uninitialised(this->_initialised_state,
        "ft_cancellation_source::get_error");
    return (_last_error);
}

const char *ft_cancellation_source::get_error_str() const noexcept
{
    errno_abort_if_uninitialised(this->_initialised_state,
        "ft_cancellation_source::get_error_str");
    return (ft_strerror(_last_error));
}

thread_local int32_t ft_cancellation_state::_last_error = FT_ERR_SUCCESS;
thread_local int32_t ft_cancellation_source::_last_error = FT_ERR_SUCCESS;
