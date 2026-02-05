#ifndef FILE_WATCH_HPP
#define FILE_WATCH_HPP

#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../CPP_class/class_string.hpp"

#include "../Compatebility/compatebility_file_watch.hpp"
#include "../PThread/thread.hpp"
#include "../PThread/mutex.hpp"

class ft_file_watch
{
    private:
        ft_string _path;
        void (*_callback)(const char *, int, void *);
        void *_user_data;
        ft_thread _thread;
        bool _running;
        bool _stopped;
        mutable pt_mutex _mutex;
        void event_loop();
        void close_handles_locked();
        bool snapshot_callback(void (**callback)(const char *, int, void *),
            void *&user_data, ft_string &path_snapshot) const;
        cmp_file_watch_context *_state;

    public:
        ft_file_watch();
        ~ft_file_watch();

        int watch_directory(const char *path, void (*callback)(const char *, int, void *), void *user_data);
        void stop();
        pt_mutex *get_mutex_for_validation() const noexcept;
};

#endif
