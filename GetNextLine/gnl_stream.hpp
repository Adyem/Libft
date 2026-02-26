#ifndef GNL_STREAM_HPP
# define GNL_STREAM_HPP

#include <cstddef>
#include <cstdio>
#include <unistd.h>
#include <cstdint>

#include "../PThread/recursive_mutex.hpp"
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
        uint8_t _initialized_state;
        mutable pt_recursive_mutex *_mutex = ft_nullptr;
        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;

        void    abort_lifecycle_error(const char *method_name,
                    const char *reason) const noexcept;
        void    abort_if_not_initialized(const char *method_name) const noexcept;
        ssize_t read_from_descriptor(int file_descriptor, char *buffer,
                    size_t max_size, int *error_code) const noexcept;
        ssize_t read_from_file(FILE *file_handle, char *buffer, size_t max_size, 
                    int *error_code) const noexcept;
    public:
        gnl_stream() noexcept;
        ~gnl_stream() noexcept;

        gnl_stream(const gnl_stream &other) = delete;
        gnl_stream &operator=(const gnl_stream &other) = delete;
        gnl_stream(gnl_stream &&other) = delete;
        gnl_stream &operator=(gnl_stream &&other) = delete;

        int initialize() noexcept;
        int destroy() noexcept;
        int     enable_thread_safety(void) noexcept;
        int     disable_thread_safety(void) noexcept;
        bool    is_thread_safe(void) const noexcept;
        int init_from_fd(int file_descriptor) noexcept;
        int init_from_file(FILE *file_handle, bool close_on_reset) noexcept;
        int init_from_callback(ssize_t (*callback)(void *user_data,
                char *buffer, size_t max_size) noexcept, void *user_data) noexcept;
        void reset() noexcept;
        ssize_t read(char *buffer, size_t max_size) noexcept;

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_validation() const noexcept;
#endif
};

#endif
