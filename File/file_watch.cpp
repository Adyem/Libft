#include "file_watch.hpp"
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
    : _path(), _callback(ft_nullptr), _user_data(ft_nullptr), _thread(), _running(false), _error_code(ER_SUCCESS)
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

void ft_file_watch::set_error(int error) const
{
    this->_error_code = error;
    ft_errno = error;
    return ;
}

int ft_file_watch::watch_directory(const char *path, void (*callback)(const char *, int, void *), void *user_data)
{
    if (path == ft_nullptr || callback == ft_nullptr)
    {
        this->set_error(FT_EINVAL);
        return (-1);
    }
    this->_path = ft_string(path);
    this->_callback = callback;
    this->_user_data = user_data;
#ifdef __linux__
    this->_fd = inotify_init();
    if (this->_fd < 0)
    {
        this->set_error(FILE_INVALID_FD);
        return (-1);
    }
    this->_watch = inotify_add_watch(this->_fd, path, IN_CREATE | IN_MODIFY | IN_DELETE);
    if (this->_watch < 0)
    {
        close(this->_fd);
        this->_fd = -1;
        this->set_error(FILE_INVALID_FD);
        return (-1);
    }
#elif defined(__APPLE__) || defined(__FreeBSD__)
    this->_fd = open(path, O_EVTONLY);
    if (this->_fd < 0)
    {
        this->set_error(FILE_INVALID_FD);
        return (-1);
    }
    this->_kqueue = kqueue();
    if (this->_kqueue < 0)
    {
        close(this->_fd);
        this->_fd = -1;
        this->set_error(FILE_INVALID_FD);
        return (-1);
    }
#elif defined(_WIN32)
    this->_handle = CreateFileA(path, FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        ft_nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, ft_nullptr);
    if (this->_handle == INVALID_HANDLE_VALUE)
    {
        this->_handle = ft_nullptr;
        this->set_error(FILE_INVALID_FD);
        return (-1);
    }
#endif
    this->_running = true;
    this->_thread = std::thread(&ft_file_watch::event_loop, this);
    return (0);
}

void ft_file_watch::stop()
{
    if (this->_running == false && this->_thread.joinable() == false)
        return ;
    this->_running = false;
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
    if (this->_thread.joinable())
        this->_thread.join();
    return ;
}

int ft_file_watch::get_error() const
{
    return (this->_error_code);
}

const char *ft_file_watch::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}

void ft_file_watch::event_loop()
{
#ifdef __linux__
    char buffer[4096];
    while (this->_running)
    {
        ssize_t length;
        char *pointer;
        struct inotify_event *event;

        length = read(this->_fd, buffer, sizeof(buffer));
        if (length <= 0)
            break;
        pointer = buffer;
        while (pointer < buffer + static_cast<size_t>(length))
        {
            event = reinterpret_cast<struct inotify_event *>(pointer);
            if (this->_callback != ft_nullptr)
            {
                if (event->mask & IN_CREATE)
                    this->_callback(event->name, FILE_WATCH_EVENT_CREATE, this->_user_data);
                else if (event->mask & IN_MODIFY)
                    this->_callback(event->name, FILE_WATCH_EVENT_MODIFY, this->_user_data);
                else if (event->mask & IN_DELETE)
                    this->_callback(event->name, FILE_WATCH_EVENT_DELETE, this->_user_data);
            }
            pointer += sizeof(struct inotify_event) + static_cast<size_t>(event->len);
        }
    }
#elif defined(__APPLE__) || defined(__FreeBSD__)
    struct kevent change_event;
    struct kevent event;
    EV_SET(&change_event, this->_fd, EVFILT_VNODE, EV_ADD | EV_CLEAR, NOTE_WRITE | NOTE_EXTEND | NOTE_DELETE | NOTE_RENAME, 0, ft_nullptr);
    while (this->_running)
    {
        int event_count;
        event_count = kevent(this->_kqueue, &change_event, 1, &event, 1, ft_nullptr);
        if (event_count <= 0)
            break;
        if (this->_callback != ft_nullptr)
        {
            if (event.fflags & NOTE_DELETE)
                this->_callback(this->_path.c_str(), FILE_WATCH_EVENT_DELETE, this->_user_data);
            else if ((event.fflags & NOTE_WRITE) || (event.fflags & NOTE_EXTEND) || (event.fflags & NOTE_RENAME))
                this->_callback(this->_path.c_str(), FILE_WATCH_EVENT_MODIFY, this->_user_data);
        }
    }
#elif defined(_WIN32)
    char buffer[1024];
    DWORD bytes_transferred;
    FILE_NOTIFY_INFORMATION *notification;
    while (this->_running)
    {
        if (ReadDirectoryChangesW(this->_handle, buffer, sizeof(buffer), FALSE,
            FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE,
            &bytes_transferred, ft_nullptr, ft_nullptr) == FALSE)
            break;
        notification = reinterpret_cast<FILE_NOTIFY_INFORMATION *>(buffer);
        if (this->_callback != ft_nullptr)
        {
            if (notification->Action == FILE_ACTION_ADDED)
                this->_callback("", FILE_WATCH_EVENT_CREATE, this->_user_data);
            else if (notification->Action == FILE_ACTION_MODIFIED)
                this->_callback("", FILE_WATCH_EVENT_MODIFY, this->_user_data);
            else if (notification->Action == FILE_ACTION_REMOVED)
                this->_callback("", FILE_WATCH_EVENT_DELETE, this->_user_data);
        }
    }
#endif
    return ;
}
