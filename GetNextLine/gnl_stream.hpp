#ifndef GNL_STREAM_HPP
# define GNL_STREAM_HPP

#include <cstddef>
#include <cstdio>
#include <unistd.h>

#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"

class gnl_stream
{
    private:
        typedef ssize_t (*gnl_stream_read_callback)(void *user_data, char *buffer, size_t max_size) noexcept;

        gnl_stream_read_callback _read_callback;
        void *_user_data;
        int _file_descriptor;
        FILE *_file_handle;
        bool _close_on_reset;
        mutable int _error_code;
        mutable pt_mutex _mutex;

        void set_error_unlocked(int error_code) const noexcept;
        void set_error(int error_code) const noexcept;
        int lock_self(ft_unique_lock<pt_mutex> &guard) const noexcept;
        static void restore_errno(ft_unique_lock<pt_mutex> &guard, int entry_errno) noexcept;
        ssize_t read_from_descriptor(int file_descriptor, char *buffer, size_t max_size) const noexcept;
        ssize_t read_from_file(FILE *file_handle, char *buffer, size_t max_size) const noexcept;

    public:
        gnl_stream() noexcept;
        ~gnl_stream() noexcept;

        gnl_stream(const gnl_stream &other) = delete;
        gnl_stream &operator=(const gnl_stream &other) = delete;
        gnl_stream(gnl_stream &&other) = delete;
        gnl_stream &operator=(gnl_stream &&other) = delete;

        int init_from_fd(int file_descriptor) noexcept;
        int init_from_file(FILE *file_handle, bool close_on_reset) noexcept;
        int init_from_callback(ssize_t (*callback)(void *user_data, char *buffer, size_t max_size) noexcept,
                void *user_data) noexcept;
        void reset() noexcept;
        ssize_t read(char *buffer, size_t max_size) noexcept;
        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
