#ifndef FT_OFSTREAM_HPP
#define FT_OFSTREAM_HPP

#include "class_file.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"
#include <cstdint>

class ft_ofstream
{
    private:
        ft_file _file;
        mutable pt_recursive_mutex *_mutex;
        uint8_t _initialised_state;
        static const uint8_t _state_uninitialised = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialised = 2;

        static void abort_lifecycle_error(const char *method_name,
                    const char *reason) noexcept;
        void abort_if_not_initialised(const char *method_name) const noexcept;
        int lock_mutex(void) const noexcept;
        int unlock_mutex(void) const noexcept;
    public:
        ft_ofstream() noexcept;
        ~ft_ofstream() noexcept;

        ft_ofstream(const ft_ofstream &other) noexcept = delete;
        ft_ofstream &operator=(const ft_ofstream &other) noexcept = delete;
        ft_ofstream(ft_ofstream &&other) noexcept = delete;
        ft_ofstream &operator=(ft_ofstream &&other) noexcept = delete;

        int initialize() noexcept;
        int destroy() noexcept;
        int enable_thread_safety(void) noexcept;
        int disable_thread_safety(void) noexcept;
        bool is_thread_safe(void) const noexcept;

        int open(const char *filename) noexcept;
        ssize_t write(const char *string) noexcept;
        int close() noexcept;
};

#endif
