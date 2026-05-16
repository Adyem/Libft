#ifndef FILE_WATCH_HPP
#define FILE_WATCH_HPP

#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../CPP_class/class_string.hpp"

#include "../Compatebility/compatebility_file_watch.hpp"
#include "../PThread/thread.hpp"
#include "../PThread/recursive_mutex.hpp"

class ft_file_watch
{
#ifdef LIBFT_TEST_BUILD
    public:
#else
    private:
#endif
        ft_string _path;
        void (*_callback)(const char *, int32_t, void *);
        void *_user_data;
        ft_thread _thread;
        ft_bool _running;
        ft_bool _stopped;
        mutable pt_recursive_mutex *_mutex;
        void event_loop();
        void close_handles_locked();
        ft_bool snapshot_callback(void (**callback)(const char *, int32_t, void *),
            void *&user_data, ft_string &path_snapshot) const;
        cmp_file_watch_context *_state;
        uint8_t _initialised_state;

    public:
        ft_file_watch();
        ft_file_watch(const ft_file_watch &other) = delete;
        ft_file_watch(ft_file_watch &&other) = delete;
        ~ft_file_watch();

        ft_file_watch &operator=(const ft_file_watch &) = delete;
        ft_file_watch &operator=(ft_file_watch &&) = delete;

        int32_t initialize();
        int32_t initialize(const ft_file_watch &other);
        int32_t initialize(ft_file_watch &&other);
        int32_t destroy();
        uint32_t move(ft_file_watch &other);
        int32_t enable_thread_safety();
        int32_t disable_thread_safety();
        ft_bool is_thread_safe() const;
        int32_t watch_directory(const char *path, void (*callback)(const char *, int32_t, void *), void *user_data);
        void stop();
};

#endif
