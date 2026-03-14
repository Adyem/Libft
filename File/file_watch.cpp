#include "file_watch.hpp"
#include "../Compatebility/compatebility_file_watch.hpp"
#include "../Template/move.hpp"
#include "../CMA/CMA.hpp"
#include "../Errno/errno_internal.hpp"
#include "../PThread/pthread_internal.hpp"

ft_file_watch::ft_file_watch()
    : _path(), _callback(ft_nullptr), _user_data(ft_nullptr), _thread(),
    _running(FT_FALSE), _stopped(FT_TRUE), _mutex(ft_nullptr), _state(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    return ;
}

ft_file_watch::ft_file_watch(const ft_file_watch &other)
    : _path(), _callback(ft_nullptr), _user_data(ft_nullptr), _thread(),
      _running(FT_FALSE), _stopped(FT_TRUE), _mutex(ft_nullptr), _state(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    (void)this->initialize(other);
    return ;
}

ft_file_watch::ft_file_watch(ft_file_watch &&other)
    : _path(), _callback(ft_nullptr), _user_data(ft_nullptr), _thread(),
      _running(FT_FALSE), _stopped(FT_TRUE), _mutex(ft_nullptr), _state(ft_nullptr),
      _initialised_state(FT_CLASS_STATE_UNINITIALISED)
{
    (void)this->initialize(ft_move(other));
    return ;
}

ft_file_watch::~ft_file_watch()
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    return ;
}

int32_t ft_file_watch::initialize()
{
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        errno_abort_lifecycle(this->_initialised_state, "ft_file_watch::initialize",
            "initialize called while already initialised");
    if (this->_path.initialize() != FT_ERR_SUCCESS)
        return (FT_ERR_NO_MEMORY);
    this->_callback = ft_nullptr;
    this->_user_data = ft_nullptr;
    this->_running = FT_FALSE;
    this->_stopped = FT_TRUE;
    this->_state = cmp_file_watch_create();
    if (this->_state == ft_nullptr)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (FT_ERR_NO_MEMORY);
    }
    this->_initialised_state = FT_CLASS_STATE_INITIALISED;
    return (FT_ERR_SUCCESS);
}

int32_t ft_file_watch::initialize(const ft_file_watch &other)
{
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_lifecycle(other._initialised_state,
            "ft_file_watch::initialize(const ft_file_watch &) source",
            "source is not initialised");
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (FT_ERR_SUCCESS);
    }
    if (this->initialize() != FT_ERR_SUCCESS)
        return (FT_ERR_INVALID_STATE);
    this->_callback = other._callback;
    this->_user_data = other._user_data;
    this->_path = other._path;
    return (FT_ERR_SUCCESS);
}

int32_t ft_file_watch::initialize(ft_file_watch &&other)
{
    if (other._initialised_state == FT_CLASS_STATE_UNINITIALISED)
        errno_abort_lifecycle(other._initialised_state,
            "ft_file_watch::initialize(ft_file_watch &&) source",
            "source is not initialised");
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (this->_initialised_state == FT_CLASS_STATE_INITIALISED)
        (void)this->destroy();
    if (other._initialised_state == FT_CLASS_STATE_DESTROYED)
    {
        this->_initialised_state = FT_CLASS_STATE_DESTROYED;
        return (FT_ERR_SUCCESS);
    }
    if (this->initialize() != FT_ERR_SUCCESS)
        return (FT_ERR_INVALID_STATE);
    this->_callback = other._callback;
    this->_user_data = other._user_data;
    this->_path = other._path;
    other._callback = ft_nullptr;
    other._user_data = ft_nullptr;
    other._path.clear();
    return (FT_ERR_SUCCESS);
}

int32_t ft_file_watch::destroy()
{
    int32_t disable_error;

    if (this->_initialised_state == FT_CLASS_STATE_UNINITIALISED
        || this->_initialised_state == FT_CLASS_STATE_DESTROYED)
        return (FT_ERR_SUCCESS);
    this->stop();
    if (this->_state != ft_nullptr)
    {
        cmp_file_watch_destroy(this->_state);
        this->_state = ft_nullptr;
    }
    disable_error = this->disable_thread_safety();
    this->_initialised_state = FT_CLASS_STATE_DESTROYED;
    return (disable_error);
}

uint32_t ft_file_watch::move(ft_file_watch &other)
{
    return (static_cast<uint32_t>(this->initialize(ft_move(other))));
}

int32_t ft_file_watch::enable_thread_safety()
{
    pt_recursive_mutex *mutex_pointer;
    void *memory_pointer;
    int32_t initialize_error;

    errno_abort_if_uninitialised(this->_initialised_state,
        "ft_file_watch::enable_thread_safety");
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

int32_t ft_file_watch::disable_thread_safety()
{
    int32_t destroy_error;

    errno_abort_if_uninitialised(this->_initialised_state,
        "ft_file_watch::disable_thread_safety");
    if (this->_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    destroy_error = this->_mutex->destroy();
    this->_mutex->~pt_recursive_mutex();
    cma_free(this->_mutex);
    this->_mutex = ft_nullptr;
    return (destroy_error);
}

ft_bool ft_file_watch::is_thread_safe() const
{
    return (this->_mutex != ft_nullptr);
}

int32_t ft_file_watch::watch_directory(const char *path,
    void (*callback)(const char *, int32_t, void *), void *user_data)
{
    ft_thread new_thread;
    int32_t lock_error;
    if (this->_initialised_state != FT_CLASS_STATE_INITIALISED)
        return (FT_ERR_INVALID_STATE);
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (path == ft_nullptr || callback == ft_nullptr)
    {
        (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
        return (FT_ERR_INVALID_ARGUMENT);
    }
    if (!this->_stopped)
    {
        (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
        this->stop();
        lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
        if (lock_error != FT_ERR_SUCCESS)
            return (lock_error);
    }
    this->_path.clear();
    int32_t initialization_error =
        this->_path.assign(path, static_cast<ft_size_t>(ft_strlen(path)));
    if (initialization_error != FT_ERR_SUCCESS)
    {
        this->_callback = ft_nullptr;
        this->_user_data = ft_nullptr;
        this->_path.clear();
        (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
        return (initialization_error);
    }
    this->_callback = callback;
    this->_user_data = user_data;
    if (cmp_file_watch_start(this->_state, path) != FT_ERR_SUCCESS)
    {
        this->_callback = ft_nullptr;
        this->_user_data = ft_nullptr;
        this->_path.clear();
        (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
        return (FT_ERR_INVALID_STATE);
    }
    this->_running = FT_TRUE;
    this->_stopped = FT_FALSE;
    new_thread = ft_thread(&ft_file_watch::event_loop, this);
    this->_thread = ft_move(new_thread);
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (FT_ERR_SUCCESS);
}

void ft_file_watch::stop()
{
    ft_thread thread_to_join;
    int32_t lock_error;

    errno_abort_if_uninitialised(this->_initialised_state, "ft_file_watch::stop");
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    if (this->_stopped)
    {
        (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
        return ;
    }
    this->_stopped = FT_TRUE;
    this->_running = FT_FALSE;
    this->close_handles_locked();
    thread_to_join = ft_move(this->_thread);
    this->_thread = ft_thread();
    this->_callback = ft_nullptr;
    this->_user_data = ft_nullptr;
    this->_path.clear();
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    if (thread_to_join.joinable())
        thread_to_join.join();
    return ;
}

void ft_file_watch::close_handles_locked()
{
    cmp_file_watch_stop(this->_state);
    return ;
}

ft_bool ft_file_watch::snapshot_callback(void (**callback)(const char *, int32_t, void *),
    void *&user_data, ft_string &path_snapshot) const
{
    int32_t lock_error;

    errno_abort_if_uninitialised(this->_initialised_state,
        "ft_file_watch::snapshot_callback");
    lock_error = pt_recursive_mutex_lock_if_not_null(this->_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (FT_FALSE);
    *callback = ft_nullptr;
    user_data = ft_nullptr;
    path_snapshot.clear();
    if (!this->_running)
    {
        (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
        return (FT_FALSE);
    }
    *callback = this->_callback;
    user_data = this->_user_data;
    path_snapshot = this->_path;
    if (ft_string::get_error() != FT_ERR_SUCCESS)
        path_snapshot.clear();
    (void)pt_recursive_mutex_unlock_if_not_null(this->_mutex);
    return (FT_TRUE);
}

void ft_file_watch::event_loop()
{
    errno_abort_if_uninitialised(this->_initialised_state, "ft_file_watch::event_loop");
    while (FT_TRUE)
    {
        cmp_file_watch_event event;
        if (!cmp_file_watch_wait_event(this->_state, &event))
            break ;
        void (*local_callback)(const char *, int32_t, void *);
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
