#include "file_watch.hpp"
#include "../Compatebility/compatebility_file_watch.hpp"
#include "../PThread/unique_lock.hpp"
#include "../Template/move.hpp"

ft_file_watch::ft_file_watch()
    : _path(), _callback(ft_nullptr), _user_data(ft_nullptr), _thread(),
      _running(false), _stopped(true), _mutex(), _state(cmp_file_watch_create())
{
    return ;
}

ft_file_watch::~ft_file_watch()
{
    this->stop();
    cmp_file_watch_destroy(this->_state);
    return ;
}

int ft_file_watch::watch_directory(const char *path, void (*callback)(const char *, int, void *), void *user_data)
{
    int status = FT_ERR_SUCCESSS;
    int result = -1;

    {
        ft_thread new_thread;
        ft_unique_lock<pt_mutex> mutex_guard(this->_mutex);
        int mutex_error = ft_global_error_stack_drop_last_error();

        if (mutex_error != FT_ERR_SUCCESSS)
        {
            status = mutex_error;
        }
        else if (path == ft_nullptr || callback == ft_nullptr)
        {
            status = FT_ERR_INVALID_ARGUMENT;
        }
        else
        {
            if (!this->_stopped)
            {
                mutex_guard.unlock();
                this->stop();
                mutex_guard = ft_unique_lock<pt_mutex>(this->_mutex);
                int relock_error = ft_global_error_stack_drop_last_error();

                if (relock_error != FT_ERR_SUCCESSS)
                    status = relock_error;
            }
            if (status == FT_ERR_SUCCESSS)
            {
                this->_path = ft_string(path);
                unsigned long long path_operation_id = this->_path.last_operation_id();
                int path_error = this->_path.pop_operation_error(path_operation_id);

                if (path_error != FT_ERR_SUCCESSS)
                {
                    status = path_error;
                    this->_callback = ft_nullptr;
                    this->_user_data = ft_nullptr;
                    this->_path.clear();
                }
                else
                {
                    this->_callback = callback;
                    this->_user_data = user_data;
                    if (cmp_file_watch_start(this->_state, path) != 0)
                    {
                        status = cmp_file_watch_last_error(this->_state);
                        this->_callback = ft_nullptr;
                        this->_user_data = ft_nullptr;
                        this->_path.clear();
                    }
                    if (status == FT_ERR_SUCCESSS)
                    {
                        this->_running = true;
                        this->_stopped = false;
                        new_thread = ft_thread(&ft_file_watch::event_loop, this);
                        if (new_thread.get_error() != FT_ERR_SUCCESSS)
                        {
                            status = new_thread.get_error();
                            this->_running = false;
                            this->_stopped = true;
                        this->close_handles_locked();
                            this->_callback = ft_nullptr;
                            this->_user_data = ft_nullptr;
                            this->_path.clear();
                        }
                        else
                        {
                            this->_thread = ft_thread(ft_move(new_thread));
                            result = 0;
                        }
                    }
                }
            }
        }
        if (mutex_guard.owns_lock())
            mutex_guard.unlock();
    }
    ft_global_error_stack_push_entry(status);
    return (result);
}

void ft_file_watch::stop()
{
    int status = FT_ERR_SUCCESSS;

    {
        ft_thread thread_to_join;
        ft_unique_lock<pt_mutex> mutex_guard(this->_mutex);
        int mutex_error = ft_global_error_stack_drop_last_error();

        if (mutex_error != FT_ERR_SUCCESSS)
        {
            status = mutex_error;
        }
        else if (this->_stopped)
        {
            if (mutex_guard.owns_lock())
                mutex_guard.unlock();
        }
        else
        {
            this->_stopped = true;
            this->_running = false;
            this->close_handles_locked();
            thread_to_join = ft_thread(ft_move(this->_thread));
            this->_thread = ft_thread();
            this->_callback = ft_nullptr;
            this->_user_data = ft_nullptr;
            this->_path.clear();
            if (mutex_guard.owns_lock())
                mutex_guard.unlock();
            if (thread_to_join.joinable())
                thread_to_join.join();
        }
    }
    ft_global_error_stack_push_entry(status);
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
    bool running;

    ft_unique_lock<pt_mutex> mutex_guard(this->_mutex);
    int mutex_error = ft_global_error_stack_drop_last_error();

    if (mutex_error != FT_ERR_SUCCESSS)
    {
        return (false);
    }
    running = this->_running;
    *callback = ft_nullptr;
    user_data = ft_nullptr;
    path_snapshot.clear();
    if (!running)
    {
        mutex_guard.unlock();
        return (false);
    }
    *callback = this->_callback;
    user_data = this->_user_data;
    path_snapshot = this->_path;
    unsigned long long path_operation_id = path_snapshot.last_operation_id();
    int path_error = path_snapshot.pop_operation_error(path_operation_id);
    if (path_error != FT_ERR_SUCCESSS)
        path_snapshot.clear();
    mutex_guard.unlock();
    return (true);
}


pt_mutex *ft_file_watch::get_mutex_for_validation() const noexcept
{
    return (&this->_mutex);
}

void ft_file_watch::event_loop()
{
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
