#ifndef FT_FD_ISTREAM_HPP
#define FT_FD_ISTREAM_HPP

#include "class_istream.hpp"
#include "../System_utils/system_utils.hpp"
#include "../PThread/recursive_mutex.hpp"
#include <cstdint>

class ft_fd_istream : public ft_istream
{
    private:
        int _fd;
        mutable pt_recursive_mutex *_mutex;
        uint8_t _initialized_state;
        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;
        int lock_mutex(void) const noexcept;
        int unlock_mutex(void) const noexcept;
        static void abort_lifecycle_error(const char *method_name,
                const char *reason) noexcept;
        void abort_if_not_initialized(const char *method_name) const noexcept;
    public:
        ft_fd_istream(int fd) noexcept;
        ft_fd_istream(const ft_fd_istream &other) noexcept = delete;
        ft_fd_istream(ft_fd_istream &&other) noexcept = delete;
        ~ft_fd_istream() noexcept;

        ft_fd_istream &operator=(const ft_fd_istream &other) noexcept = delete;
        ft_fd_istream &operator=(ft_fd_istream &&other) noexcept = delete;

        void set_fd(int fd) noexcept;
        int get_fd() const noexcept;
        int enable_thread_safety(void) noexcept;
        int disable_thread_safety(void) noexcept;
        bool is_thread_safe(void) const noexcept;

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_validation() const noexcept;
#endif

    protected:
        ssize_t do_read(char *buffer, std::size_t count);
};

#endif
