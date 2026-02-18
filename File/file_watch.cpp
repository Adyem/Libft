#include "file_watch.hpp"
#include "../Compatebility/compatebility_file_watch.hpp"
#include "../Template/move.hpp"
#include "../CMA/CMA.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"

void ft_file_watch::abort_lifecycle_error(const char *method_name,
    const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_file_watch lifecycle error: %s: %s\n",
        method_name, reason);
    su_abort();
    return ;
}

void ft_file_watch::abort_if_not_initialized(const char *method_name) const
{
    if (this->_initialized_state == ft_file_watch::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

int ft_file_watch::lock_watch(bool *lock_acquired) const
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

int ft_file_watch::unlock_watch(bool lock_acquired) const
{
    int unlock_error;

    if (lock_acquired == false)
        return (FT_ERR_SUCCESS);
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    unlock_error = this->_mutex->unlock();
    return (unlock_error);
}

ft_file_watch::ft_file_watch()
    : _path(), _callback(ft_nullptr), _user_data(ft_nullptr), _thread(),
      _running(false), _stopped(true), _mutex(ft_nullptr), _state(ft_nullptr),
      _initialized_state(ft_file_watch::_state_uninitialized)
{
    return ;
}

ft_file_watch::~ft_file_watch()
{
    if (this->_initialized_state == ft_file_watch::_state_uninitialized)
        this->abort_lifecycle_error("ft_file_watch::~ft_file_watch",
            "destructor called while object is uninitialized");
    if (this->_initialized_state == ft_file_watch::_state_initialized)
        (void)this->destroy();
    return ;
}

int ft_file_watch::initialize()
{
    if (this->_initialized_state == ft_file_watch::_state_initialized)
    {
        this->abort_lifecycle_error("ft_file_watch::initialize",
            "initialize called while already initialized");
    }
    this->_path.clear();
    this->_callback = ft_nullptr;
    this->_user_data = ft_nullptr;
    this->_running = false;
    this->_stopped = true;
    this->_state = cmp_file_watch_create();
    if (this->_state == ft_nullptr)
    {
        this->_initialized_state = ft_file_watch::_state_destroyed;
        return (FT_ERR_NO_MEMORY);
    }
    this->_initialized_state = ft_file_watch::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int ft_file_watch::destroy()
{
    int disable_error;

    if (this->_initialized_state != ft_file_watch::_state_initialized)
    {
        this->abort_lifecycle_error("ft_file_watch::destroy",
            "destroy called while object is not initialized");
    }
    this->stop();
    if (this->_state != ft_nullptr)
    {
        cmp_file_watch_destroy(this->_state);
        this->_state = ft_nullptr;
    }
    disable_error = this->disable_thread_safety();
    this->_initialized_state = ft_file_watch::_state_destroyed;
    return (disable_error);
}

int ft_file_watch::enable_thread_safety()
{
    pt_recursive_mutex *mutex_pointer;
    void *memory_pointer;
    int initialize_error;

    this->abort_if_not_initialized("ft_file_watch::enable_thread_safety");
    if (this->_mutex != ft_nullptr)
        return (FT_ERR_SUCCESS);
    memory_pointer = cma_malloc(sizeof(pt_recursive_mutex));
    if (memory_pointer == ft_nullptr)
        return (FT_ERR_NO_MEMORY);
    mutex_pointer = new(memory_pointer) pt_recursive_mutex();
    initialize_error = mutex_pointer->initialize();
    if (initialize_error != FT_ERR_SUCCESS)
    {
        mutex_pointer->~pt_recursive_mutex();
        cma_free(memory_pointer);
        return (initialize_error);
    }
    this->_mutex = mutex_pointer;
    return (FT_ERR_SUCCESS);
}

int ft_file_watch::disable_thread_safety()
{
    int destroy_error;

    this->abort_if_not_initialized("ft_file_watch::disable_thread_safety");
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    this->_mutex->~pt_recursive_mutex();
    cma_free(this->_mutex);
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

bool ft_file_watch::is_thread_safe() const
{
    this->abort_if_not_initialized("ft_file_watch::is_thread_safe");
    return (this->_mutex != ft_nullptr);
}

int ft_file_watch::watch_directory(const char *path,
    void (*callback)(const char *, int, void *), void *user_data)
{
    ft_thread new_thread;
    int lock_error;
    bool lock_acquired;

    this->abort_if_not_initialized("ft_file_watch::watch_directory");
    lock_error = this->lock_watch(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (-1);
    if (path == ft_nullptr || callback == ft_nullptr)
    {
        (void)this->unlock_watch(lock_acquired);
        return (-1);
    }
    if (!this->_stopped)
    {
        (void)this->unlock_watch(lock_acquired);
        this->stop();
        lock_error = this->lock_watch(&lock_acquired);
        if (lock_error != FT_ERR_SUCCESS)
            return (-1);
    }
    this->_path = ft_string(path);
    if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
    {
        this->_callback = ft_nullptr;
        this->_user_data = ft_nullptr;
        this->_path.clear();
        (void)this->unlock_watch(lock_acquired);
        return (-1);
    }
    this->_callback = callback;
    this->_user_data = user_data;
    if (cmp_file_watch_start(this->_state, path) != FT_ERR_SUCCESS)
    {
        this->_callback = ft_nullptr;
        this->_user_data = ft_nullptr;
        this->_path.clear();
        (void)this->unlock_watch(lock_acquired);
        return (-1);
    }
    this->_running = true;
    this->_stopped = false;
    new_thread = ft_thread(&ft_file_watch::event_loop, this);
    this->_thread = ft_move(new_thread);
    (void)this->unlock_watch(lock_acquired);
    return (0);
}

void ft_file_watch::stop()
{
    ft_thread thread_to_join;
    int lock_error;
    bool lock_acquired;

    this->abort_if_not_initialized("ft_file_watch::stop");
    lock_error = this->lock_watch(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    if (this->_stopped)
    {
        (void)this->unlock_watch(lock_acquired);
        return ;
    }
    this->_stopped = true;
    this->_running = false;
    this->close_handles_locked();
    thread_to_join = ft_move(this->_thread);
    this->_thread = ft_thread();
    this->_callback = ft_nullptr;
    this->_user_data = ft_nullptr;
    this->_path.clear();
    (void)this->unlock_watch(lock_acquired);
    if (thread_to_join.joinable())
        thread_to_join.join();
    return ;
}

void ft_file_watch::close_handles_locked()
{
    cmp_file_watch_stop(this->_state);
    return ;
}

bool ft_file_watch::snapshot_callback(void (**callback)(const char *, int, void *),
    void *&user_data, ft_string &path_snapshot) const
{
    int lock_error;
    bool lock_acquired;

    this->abort_if_not_initialized("ft_file_watch::snapshot_callback");
    lock_error = this->lock_watch(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (false);
    *callback = ft_nullptr;
    user_data = ft_nullptr;
    path_snapshot.clear();
    if (!this->_running)
    {
        (void)this->unlock_watch(lock_acquired);
        return (false);
    }
    *callback = this->_callback;
    user_data = this->_user_data;
    path_snapshot = this->_path;
    if (ft_string::last_operation_error() != FT_ERR_SUCCESS)
        path_snapshot.clear();
    (void)this->unlock_watch(lock_acquired);
    return (true);
}

#ifdef LIBFT_TEST_BUILD
pt_recursive_mutex *ft_file_watch::get_mutex_for_validation() const noexcept
{
    return (this->_mutex);
}
#endif

void ft_file_watch::event_loop()
{
    this->abort_if_not_initialized("ft_file_watch::event_loop");
    while (true)
    {
        cmp_file_watch_event event;
        if (!cmp_file_watch_wait_event(this->_state, &event))
            break;
        void (*local_callback)(const char *, int, void *);
        void *local_user_data;
        ft_string path_snapshot;

        local_callback = ft_nullptr;
        local_user_data = ft_nullptr;
        if (!this->snapshot_callback(&local_callback, local_user_data, path_snapshot))
            continue ;
        const char *event_name;

        if (event.has_name)
            event_name = event.name;
        else
            event_name = path_snapshot.c_str();
        if (local_callback != ft_nullptr)
            local_callback(event_name, event.event_type, local_user_data);
    }
    return ;
}
