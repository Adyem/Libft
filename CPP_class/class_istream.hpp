#ifndef FT_ISTREAM_HPP
#define FT_ISTREAM_HPP

#include <cstddef>
#include <cstdint>
#include <sys/types.h>
#include "../PThread/recursive_mutex.hpp"

class ft_istream
{
    private:
        std::size_t _gcount;
        bool _is_valid;
        mutable pt_recursive_mutex *_mutex;
        uint8_t _initialized_state;
        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;

        static void abort_lifecycle_error(const char *method_name,
                const char *reason) noexcept;
        void abort_if_not_initialized(const char *method_name) const noexcept;
        int lock_mutex(void) const noexcept;
        int unlock_mutex(void) const noexcept;

    protected:
        ft_istream() noexcept;
        virtual ssize_t do_read(char *buffer, std::size_t count) = 0;

    public:
        virtual ~ft_istream() noexcept;

        ft_istream(const ft_istream &other) noexcept = delete;
        ft_istream(ft_istream &&other) noexcept = delete;
        ft_istream &operator=(const ft_istream &other) noexcept = delete;
        ft_istream &operator=(ft_istream &&other) noexcept = delete;

        int initialize() noexcept;
        int destroy() noexcept;
        int enable_thread_safety(void) noexcept;
        int disable_thread_safety(void) noexcept;
        bool is_thread_safe(void) const noexcept;
        ssize_t read(char *buffer, std::size_t count) noexcept;
        std::size_t gcount() const noexcept;
        bool is_valid() const noexcept;
#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_validation() const noexcept;
#endif
};

#endif
