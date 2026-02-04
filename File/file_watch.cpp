#include "file_watch.hpp"
#include "../PThread/unique_lock.hpp"
#include "../Template/move.hpp"

#ifdef __linux__
#include <sys/inotify.h>
#include <unistd.h>
#elif defined(__APPLE__) || defined(__FreeBSD__)
#include <sys/event.h>
#include <fcntl.h>
#include <unistd.h>
#elif defined(_WIN32)
#include <windows.h>
#endif

ft_file_watch::ft_file_watch()
    : _path(), _callback(ft_nullptr), _user_data(ft_nullptr), _thread(), _running(false), _stopped(true), _mutex()
#ifdef __linux__
    , _fd(-1), _watch(-1)
#elif defined(__APPLE__) || defined(__FreeBSD__)
    , _kqueue(-1), _fd(-1)
#elif defined(_WIN32)
    , _handle(ft_nullptr)
#endif
{
    return ;
}

ft_file_watch::~ft_file_watch()
{
    this->stop();
    return ;
}

int ft_file_watch::watch_directory(const char *path, void (*callback)(const char *, int, void *), void *user_data)
{
    int status = FT_ERR_SUCCESSS;
    int result = -1;

    {
        ft_thread new_thread;
        ft_unique_lock<pt_mutex> mutex_guard(this->_mutex);
        int mutex_error = ft_global_error_stack_pop_newest();

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
                int relock_error = ft_global_error_stack_pop_newest();

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
#ifdef __linux__
                    this->_fd = inotify_init();
                    if (this->_fd < 0)
                    {
                        this->_callback = ft_nullptr;
                        this->_user_data = ft_nullptr;
                        this->_path.clear();
                        status = FT_ERR_INVALID_HANDLE;
                    }
                    else
                    {
                        this->_watch = inotify_add_watch(this->_fd, path, IN_CREATE | IN_MODIFY | IN_DELETE);
                        if (this->_watch < 0)
                        {
                            close(this->_fd);
                            this->_fd = -1;
                            this->_callback = ft_nullptr;
                            this->_user_data = ft_nullptr;
                            this->_path.clear();
                            status = FT_ERR_INVALID_HANDLE;
                        }
                    }
#elif defined(__APPLE__) || defined(__FreeBSD__)
                    this->_fd = open(path, O_EVTONLY);
                    if (this->_fd < 0)
                    {
                        this->_callback = ft_nullptr;
                        this->_user_data = ft_nullptr;
                        this->_path.clear();
                        status = FT_ERR_INVALID_HANDLE;
                    }
                    else
                    {
                        this->_kqueue = kqueue();
                        if (this->_kqueue < 0)
                        {
                            close(this->_fd);
                            this->_fd = -1;
                            this->_callback = ft_nullptr;
                            this->_user_data = ft_nullptr;
                            this->_path.clear();
                            status = FT_ERR_INVALID_HANDLE;
                        }
                    }
#elif defined(_WIN32)
                    this->_handle = CreateFileA(path, FILE_LIST_DIRECTORY,
                        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                        ft_nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, ft_nullptr);
                    if (this->_handle == INVALID_HANDLE_VALUE)
                    {
                        this->_handle = ft_nullptr;
                        this->_callback = ft_nullptr;
                        this->_user_data = ft_nullptr;
                        this->_path.clear();
                        status = FT_ERR_INVALID_HANDLE;
                    }
#endif
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
    this->record_operation_error(status);
    return (result);
}

void ft_file_watch::stop()
{
    int status = FT_ERR_SUCCESSS;

    {
        ft_thread thread_to_join;
        ft_unique_lock<pt_mutex> mutex_guard(this->_mutex);
        int mutex_error = ft_global_error_stack_pop_newest();

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
    this->record_operation_error(status);
    return ;
}

void ft_file_watch::close_handles_locked()
{
#ifdef __linux__
    if (this->_watch >= 0 && this->_fd >= 0)
        inotify_rm_watch(this->_fd, this->_watch);
    this->_watch = -1;
    if (this->_fd >= 0)
        close(this->_fd);
    this->_fd = -1;
#elif defined(__APPLE__) || defined(__FreeBSD__)
    if (this->_fd >= 0)
        close(this->_fd);
    this->_fd = -1;
    if (this->_kqueue >= 0)
        close(this->_kqueue);
    this->_kqueue = -1;
#elif defined(_WIN32)
    if (this->_handle != ft_nullptr)
        CloseHandle(this->_handle);
    this->_handle = ft_nullptr;
#endif
    return ;
}

bool ft_file_watch::snapshot_callback(void (**callback)(const char *, int, void *),
    void *&user_data, ft_string &path_snapshot) const
{
    bool running;

    ft_unique_lock<pt_mutex> mutex_guard(this->_mutex);
    int mutex_error = ft_global_error_stack_pop_newest();

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

#ifdef __linux__
bool ft_file_watch::get_linux_handles(int &fd) const
{
    bool running;
    ft_unique_lock<pt_mutex> mutex_guard(this->_mutex);
    int mutex_error = ft_global_error_stack_pop_newest();

    if (mutex_error != FT_ERR_SUCCESSS)
        return (false);
    running = this->_running;
    if (!running || this->_fd < 0)
    {
        mutex_guard.unlock();
        return (false);
    }
    fd = this->_fd;
    mutex_guard.unlock();
    return (true);
}
#elif defined(__APPLE__) || defined(__FreeBSD__)
bool ft_file_watch::get_bsd_handles(int &kqueue_handle, int &fd) const
{
    bool running;
    ft_unique_lock<pt_mutex> mutex_guard(this->_mutex);
    int mutex_error = ft_global_error_stack_pop_newest();

    if (mutex_error != FT_ERR_SUCCESSS)
        return (false);
    running = this->_running;
    if (!running || this->_kqueue < 0 || this->_fd < 0)
    {
        mutex_guard.unlock();
        return (false);
    }
    kqueue_handle = this->_kqueue;
    fd = this->_fd;
    mutex_guard.unlock();
    return (true);
}
#elif defined(_WIN32)
bool ft_file_watch::get_windows_handle(void *&handle) const
{
    bool running;
    ft_unique_lock<pt_mutex> mutex_guard(this->_mutex);
    int mutex_error = ft_global_error_stack_pop_newest();

    if (mutex_error != FT_ERR_SUCCESSS)
        return (false);
    running = this->_running;
    if (!running || this->_handle == ft_nullptr)
    {
        mutex_guard.unlock();
        return (false);
    }
    handle = this->_handle;
    mutex_guard.unlock();
    return (true);
}
#endif

pt_mutex *ft_file_watch::get_mutex_for_validation() const noexcept
{
    return (&this->_mutex);
}

ft_operation_error_stack *ft_file_watch::operation_error_stack_handle() const noexcept
{
    return (&this->_operation_errors);
}

void ft_file_watch::record_operation_error(int error_code) const
{
    unsigned long long operation_id = ft_errno_next_operation_id();

    ft_global_error_stack_push_entry_with_id(error_code, operation_id);
    ft_operation_error_stack_push(&this->_operation_errors, error_code, operation_id);
    return ;
}

void ft_file_watch::event_loop()
{
#ifdef __linux__
    char buffer[4096];
    while (1)
    {
        ssize_t length;
        char *pointer;
        struct inotify_event *event;

        int local_fd;

        if (!this->get_linux_handles(local_fd))
            break;
        length = read(local_fd, buffer, sizeof(buffer));
        if (length <= 0)
            break;
        void (*local_callback)(const char *, int, void *);
        void *local_user_data;
        ft_string path_snapshot;

        local_callback = ft_nullptr;
        local_user_data = ft_nullptr;
        if (!this->snapshot_callback(&local_callback, local_user_data, path_snapshot))
            continue ;
        pointer = buffer;
        while (pointer < buffer + static_cast<size_t>(length))
        {
            event = reinterpret_cast<struct inotify_event *>(pointer);
            if (local_callback != ft_nullptr)
            {
                if (event->mask & IN_CREATE)
                    local_callback(event->name, FILE_WATCH_EVENT_CREATE, local_user_data);
                else if (event->mask & IN_MODIFY)
                    local_callback(event->name, FILE_WATCH_EVENT_MODIFY, local_user_data);
                else if (event->mask & IN_DELETE)
                    local_callback(event->name, FILE_WATCH_EVENT_DELETE, local_user_data);
            }
            pointer += sizeof(struct inotify_event) + static_cast<size_t>(event->len);
        }
    }
#elif defined(__APPLE__) || defined(__FreeBSD__)
    struct kevent change_event;
    struct kevent event;
    while (1)
    {
        int event_count;
        int local_kqueue;
        int local_fd;

        if (!this->get_bsd_handles(local_kqueue, local_fd))
            break;
        EV_SET(&change_event, local_fd, EVFILT_VNODE, EV_ADD | EV_CLEAR, NOTE_WRITE | NOTE_EXTEND | NOTE_DELETE | NOTE_RENAME, 0, ft_nullptr);
        event_count = kevent(local_kqueue, &change_event, 1, &event, 1, ft_nullptr);
        if (event_count <= 0)
            break;
        void (*local_callback)(const char *, int, void *);
        void *local_user_data;
        ft_string path_snapshot;

        local_callback = ft_nullptr;
        local_user_data = ft_nullptr;
        if (!this->snapshot_callback(&local_callback, local_user_data, path_snapshot))
            continue ;
        if (local_callback != ft_nullptr)
        {
            const char *callback_path;

            callback_path = path_snapshot.c_str();
            if (event.fflags & NOTE_DELETE)
                local_callback(callback_path, FILE_WATCH_EVENT_DELETE, local_user_data);
            else if ((event.fflags & NOTE_WRITE) || (event.fflags & NOTE_EXTEND) || (event.fflags & NOTE_RENAME))
                local_callback(callback_path, FILE_WATCH_EVENT_MODIFY, local_user_data);
        }
    }
#elif defined(_WIN32)
    char buffer[1024];
    DWORD bytes_transferred;
    FILE_NOTIFY_INFORMATION *notification;
    while (1)
    {
        void *local_handle;

        if (!this->get_windows_handle(local_handle))
            break;
        if (ReadDirectoryChangesW(local_handle, buffer, sizeof(buffer), FALSE,
            FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE,
            &bytes_transferred, ft_nullptr, ft_nullptr) == FALSE)
            break;
        notification = reinterpret_cast<FILE_NOTIFY_INFORMATION *>(buffer);
        void (*local_callback)(const char *, int, void *);
        void *local_user_data;
        ft_string path_snapshot;

        local_callback = ft_nullptr;
        local_user_data = ft_nullptr;
        if (!this->snapshot_callback(&local_callback, local_user_data, path_snapshot))
            continue ;
        if (local_callback != ft_nullptr)
        {
            if (notification->Action == FILE_ACTION_ADDED)
                local_callback("", FILE_WATCH_EVENT_CREATE, local_user_data);
            else if (notification->Action == FILE_ACTION_MODIFIED)
                local_callback("", FILE_WATCH_EVENT_MODIFY, local_user_data);
            else if (notification->Action == FILE_ACTION_REMOVED)
                local_callback("", FILE_WATCH_EVENT_DELETE, local_user_data);
        }
    }
#endif
    return ;
}
