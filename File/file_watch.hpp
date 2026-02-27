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
    private:
        ft_string _path;
        void (*_callback)(const char *, int, void *);
        void *_user_data;
        ft_thread _thread;
        bool _running;
        bool _stopped;
        mutable pt_recursive_mutex *_mutex;
        void event_loop();
        void close_handles_locked();
        bool snapshot_callback(void (**callback)(const char *, int, void *),
            void *&user_data, ft_string &path_snapshot) const;
        cmp_file_watch_context *_state;
        uint8_t _initialized_state;
        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;

        void abort_lifecycle_error(const char *method_name,
            const char *reason) const;
        void abort_if_not_initialized(const char *method_name) const;

    public:
        ft_file_watch();
        ~ft_file_watch();

        ft_file_watch(const ft_file_watch &) = delete;
        ft_file_watch &operator=(const ft_file_watch &) = delete;
        ft_file_watch(ft_file_watch &&) = delete;
        ft_file_watch &operator=(ft_file_watch &&) = delete;

        int initialize();
        int destroy();
        int enable_thread_safety();
        int disable_thread_safety();
        bool is_thread_safe() const;
        int watch_directory(const char *path, void (*callback)(const char *, int, void *), void *user_data);
        void stop();
#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_validation() const noexcept;
#endif
};

#endif
