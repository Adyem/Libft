#ifndef FT_STRINGBUF_HPP
#define FT_STRINGBUF_HPP

#include "class_string.hpp"
#include "../PThread/recursive_mutex.hpp"
#include <cstdint>
#include <cstddef>

class ft_stringbuf
{
    private:
        ft_string _storage;
        std::size_t _position;
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

    public:
        ft_stringbuf() noexcept;
        ~ft_stringbuf() noexcept;

        ft_stringbuf(const ft_stringbuf &other) noexcept = delete;
        ft_stringbuf &operator=(const ft_stringbuf &other) noexcept = delete;
        ft_stringbuf(ft_stringbuf &&other) noexcept = delete;
        ft_stringbuf &operator=(ft_stringbuf &&other) noexcept = delete;

        int initialize(const ft_string &string) noexcept;
        int destroy() noexcept;
        ssize_t read(char *buffer, std::size_t count) noexcept;
        bool is_valid() const noexcept;
        int str(ft_string &value) const noexcept;
        int enable_thread_safety(void) noexcept;
        int disable_thread_safety(void) noexcept;
        bool is_thread_safe(void) const noexcept;
#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_validation() const noexcept;
#endif
};

#endif
