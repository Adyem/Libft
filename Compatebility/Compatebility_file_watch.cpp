#include "compatebility_internal.hpp"
#include "compatebility_file_watch.hpp"
#include "../Errno/errno.hpp"

#include <atomic>
#include <cerrno>
#include <cstring>

#if defined(__linux__)
# include <sys/inotify.h>
# include <unistd.h>
#endif

#if defined(__APPLE__) || defined(__FreeBSD__)
# include <fcntl.h>
# include <sys/event.h>
# include <unistd.h>
#endif

#if defined(_WIN32)
# include <windows.h>
#endif

struct cmp_file_watch_context
{
    std::atomic<bool>    running;
    int32_t                  last_error;
#if defined(__linux__)
    int32_t                  fd;
    int32_t                  watch;
    char                 buffer[4096];
    ft_size_t               buffer_offset;
    ft_size_t               buffer_size;
#elif defined(__APPLE__) || defined(__FreeBSD__)
    int32_t                  kqueue_fd;
    int32_t                  watch_fd;
#elif defined(_WIN32)
    HANDLE               directory;
    char                 buffer[4096];
    DWORD                buffer_offset;
    DWORD                buffer_size;
#endif
};

static int32_t cmp_file_watch_translate_error(void)
{
    int32_t error_code = FT_ERR_IO;

#if defined(_WIN32)
    DWORD last_error = GetLastError();

    if (last_error != 0)
        error_code = ft_map_system_error(static_cast<int32_t>(last_error));
    else
        error_code = FT_ERR_IO;
#else
    if (errno != 0)
        error_code = ft_map_system_error(errno);
#endif
    return (error_code);
}

static file_watch_event_type cmp_file_watch_translate_inotify_mask(uint32_t mask)
{
    if (mask & IN_CREATE)
        return (FILE_WATCH_EVENT_CREATE);
    if (mask & IN_DELETE)
        return (FILE_WATCH_EVENT_DELETE);
    return (FILE_WATCH_EVENT_MODIFY);
}

 #if defined(__APPLE__) || defined(__FreeBSD__)
static file_watch_event_type cmp_file_watch_translate_bsd_flags(uint32_t flags)
{
    if (flags & NOTE_DELETE)
        return (FILE_WATCH_EVENT_DELETE);
    return (FILE_WATCH_EVENT_MODIFY);
}
 #endif

 #if defined(_WIN32)
static file_watch_event_type cmp_file_watch_translate_windows_action(DWORD action)
{
    if (action == FILE_ACTION_ADDED)
        return (FILE_WATCH_EVENT_CREATE);
    if (action == FILE_ACTION_REMOVED)
        return (FILE_WATCH_EVENT_DELETE);
    return (FILE_WATCH_EVENT_MODIFY);
}
 #endif

cmp_file_watch_context *cmp_file_watch_create(void)
{
    cmp_file_watch_context *context = new cmp_file_watch_context();

    context->running.store(false);
    context->last_error = FT_ERR_SUCCESSS;
#if defined(__linux__)
    context->fd = -1;
    context->watch = -1;
    context->buffer_offset = 0;
    context->buffer_size = 0;
#elif defined(__APPLE__) || defined(__FreeBSD__)
    context->kqueue_fd = -1;
    context->watch_fd = -1;
#elif defined(_WIN32)
    context->directory = INVALID_HANDLE_VALUE;
    context->buffer_offset = 0;
    context->buffer_size = 0;
#endif
    return (context);
}

void cmp_file_watch_destroy(cmp_file_watch_context *context)
{
    if (!context)
        return ;
    cmp_file_watch_stop(context);
    delete context;
    return ;
}

int32_t cmp_file_watch_start(cmp_file_watch_context *context, const char *path)
{
    if (!context || !path)
        return (-1);
    cmp_file_watch_stop(context);
    context->last_error = FT_ERR_SUCCESSS;
#if defined(__linux__)
    context->fd = inotify_init();
    if (context->fd < 0)
    {
        context->last_error = cmp_file_watch_translate_error();
        return (-1);
    }
    context->watch = inotify_add_watch(context->fd, path, IN_CREATE | IN_MODIFY | IN_DELETE);
    if (context->watch < 0)
    {
        context->last_error = cmp_file_watch_translate_error();
        close(context->fd);
        context->fd = -1;
        return (-1);
    }
    context->buffer_offset = 0;
    context->buffer_size = 0;
#elif defined(__APPLE__) || defined(__FreeBSD__)
    context->watch_fd = open(path, O_EVTONLY);
    if (context->watch_fd < 0)
    {
        context->last_error = cmp_file_watch_translate_error();
        return (-1);
    }
    context->kqueue_fd = kqueue();
    if (context->kqueue_fd < 0)
    {
        context->last_error = cmp_file_watch_translate_error();
        close(context->watch_fd);
        context->watch_fd = -1;
        return (-1);
    }
    struct kevent configuration;

    EV_SET(&configuration, context->watch_fd, EVFILT_VNODE, EV_ADD | EV_CLEAR,
        NOTE_WRITE | NOTE_EXTEND | NOTE_DELETE | NOTE_RENAME, 0, ft_nullptr);
    if (kevent(context->kqueue_fd, &configuration, 1, ft_nullptr, 0, ft_nullptr) < 0)
    {
        context->last_error = cmp_file_watch_translate_error();
        close(context->kqueue_fd);
        context->kqueue_fd = -1;
        close(context->watch_fd);
        context->watch_fd = -1;
        return (-1);
    }
#elif defined(_WIN32)
    context->directory = CreateFileA(path, FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        ft_nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, ft_nullptr);
    if (context->directory == INVALID_HANDLE_VALUE)
    {
        context->last_error = cmp_file_watch_translate_error();
        return (-1);
    }
    context->buffer_offset = 0;
    context->buffer_size = 0;
#endif
    context->running.store(true);
    return (0);
}

void cmp_file_watch_stop(cmp_file_watch_context *context)
{
    if (!context)
        return ;
    context->running.store(false);
#if defined(__linux__)
    if (context->watch >= 0 && context->fd >= 0)
        inotify_rm_watch(context->fd, context->watch);
    if (context->fd >= 0)
        close(context->fd);
    context->watch = -1;
    context->fd = -1;
    context->buffer_offset = 0;
    context->buffer_size = 0;
#elif defined(__APPLE__) || defined(__FreeBSD__)
    if (context->watch_fd >= 0)
        close(context->watch_fd);
    if (context->kqueue_fd >= 0)
        close(context->kqueue_fd);
    context->watch_fd = -1;
    context->kqueue_fd = -1;
#elif defined(_WIN32)
    if (context->directory != INVALID_HANDLE_VALUE)
    {
        CancelIo(context->directory);
        CloseHandle(context->directory);
    }
    context->directory = INVALID_HANDLE_VALUE;
    context->buffer_offset = 0;
    context->buffer_size = 0;
#endif
    return ;
}

bool cmp_file_watch_wait_event(cmp_file_watch_context *context,
    cmp_file_watch_event *event)
{
    if (!context || !event)
        return (false);
    if (!context->running.load())
        return (false);
#if defined(__linux__)
    while (context->running.load())
    {
        if (context->buffer_offset < context->buffer_size)
        {
            const struct inotify_event *notify_event =
                reinterpret_cast<const struct inotify_event *>(
                    context->buffer + context->buffer_offset);
            ft_size_t entry_size = sizeof(struct inotify_event) + static_cast<ft_size_t>(notify_event->len);

            if (entry_size > 0)
                context->buffer_offset += entry_size;
            else
                context->buffer_offset += sizeof(struct inotify_event);
            event->event_type = cmp_file_watch_translate_inotify_mask(notify_event->mask);
            event->has_name = (notify_event->len > 0);
            if (event->has_name)
            {
                ft_size_t name_length = notify_event->len;
                if (name_length >= sizeof(event->name))
                    name_length = sizeof(event->name) - 1;
                std::memcpy(event->name, notify_event->name, name_length);
                event->name[name_length] = '\0';
            }
            else
            {
                event->name[0] = '\0';
            }
            return (true);
        }
        ssize_t read_result = ::read(context->fd, context->buffer, sizeof(context->buffer));
        if (read_result <= 0)
        {
            context->running.store(false);
            context->last_error = cmp_file_watch_translate_error();
            return (false);
        }
        context->buffer_size = static_cast<ft_size_t>(read_result);
        context->buffer_offset = 0;
    }
#elif defined(__APPLE__) || defined(__FreeBSD__)
    while (context->running.load())
    {
        struct kevent observed_event;
        int32_t event_count = kevent(context->kqueue_fd, ft_nullptr, 0, &observed_event, 1, ft_nullptr);
        if (event_count <= 0)
        {
            context->running.store(false);
            context->last_error = cmp_file_watch_translate_error();
            return (false);
        }
        event->event_type = cmp_file_watch_translate_bsd_flags(observed_event.fflags);
        event->has_name = false;
        event->name[0] = '\0';
        return (true);
    }
#elif defined(_WIN32)
    while (context->running.load())
    {
        if (context->buffer_offset < context->buffer_size)
        {
            FILE_NOTIFY_INFORMATION *notification =
                reinterpret_cast<FILE_NOTIFY_INFORMATION *>(context->buffer + context->buffer_offset);
            DWORD next_offset = notification->NextEntryOffset;
            if (next_offset == 0)
                next_offset = context->buffer_size - context->buffer_offset;
            context->buffer_offset += next_offset;
            event->event_type = cmp_file_watch_translate_windows_action(notification->Action);
            if (notification->FileNameLength > 0)
            {
                int32_t converted = WideCharToMultiByte(CP_UTF8, 0, notification->FileName,
                    notification->FileNameLength / sizeof(WCHAR), event->name,
                    static_cast<int32_t>(sizeof(event->name) - 1), ft_nullptr, ft_nullptr);
                if (converted > 0)
                {
                    event->name[converted] = '\0';
                    event->has_name = true;
                }
                else
                {
                    event->name[0] = '\0';
                    event->has_name = false;
                }
            }
            else
            {
                event->name[0] = '\0';
                event->has_name = false;
            }
            return (true);
        }
        DWORD bytes_returned = 0;
        BOOL success = ReadDirectoryChangesW(context->directory, context->buffer,
            static_cast<DWORD>(sizeof(context->buffer)), FALSE,
            FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE,
            &bytes_returned, ft_nullptr, ft_nullptr);
        if (success == FALSE)
        {
            DWORD windows_error = GetLastError();
            context->running.store(false);
            if (windows_error != ERROR_OPERATION_ABORTED)
                context->last_error = ft_map_system_error(static_cast<int32_t>(windows_error));
            else
                context->last_error = FT_ERR_SUCCESSS;
            return (false);
        }
        if (bytes_returned == 0)
            continue ;
        context->buffer_size = bytes_returned;
        context->buffer_offset = 0;
    }
#endif
    return (false);
}

int32_t cmp_file_watch_last_error(const cmp_file_watch_context *context)
{
    if (!context)
        return (FT_ERR_INVALID_ARGUMENT);
    return (context->last_error);
}
