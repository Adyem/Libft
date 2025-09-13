#ifndef FILE_WATCH_HPP
#define FILE_WATCH_HPP

#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../CPP_class/class_string_class.hpp"

#include <thread>

enum file_watch_event_type
{
    FILE_WATCH_EVENT_CREATE = 1,
    FILE_WATCH_EVENT_MODIFY,
    FILE_WATCH_EVENT_DELETE
};

class ft_file_watch
{
    private:
        ft_string _path;
        void (*_callback)(const char *, int, void *);
        void *_user_data;
        std::thread _thread;
        bool _running;
        mutable int _error_code;
#ifdef __linux__
        int _fd;
        int _watch;
#elif defined(__APPLE__) || defined(__FreeBSD__)
        int _kqueue;
        int _fd;
#elif defined(_WIN32)
        void *_handle;
#endif
        void set_error(int error) const;
        void event_loop();

    public:
        ft_file_watch();
        ~ft_file_watch();

        int watch_directory(const char *path, void (*callback)(const char *, int, void *), void *user_data);
        void stop();
        int get_error() const;
        const char *get_error_str() const;
};

#endif
