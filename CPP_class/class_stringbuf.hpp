#ifndef FT_STRINGBUF_HPP
#define FT_STRINGBUF_HPP

#include "class_string.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread_internal.hpp"

class ft_stringbuf
{
    private:
        ft_string _storage;
        std::size_t _position;
        mutable pt_recursive_mutex *_mutex;

        int lock_mutex(void) const noexcept;
        int unlock_mutex(void) const noexcept;
        int prepare_thread_safety(void) noexcept;
        void teardown_thread_safety(void) noexcept;
        int enable_thread_safety(void) noexcept;
        void disable_thread_safety(void) noexcept;
        bool is_thread_safe_enabled(void) const noexcept;

    public:
        ft_stringbuf(const ft_string &string) noexcept;
        ft_stringbuf(const ft_stringbuf &other) noexcept;
        ft_stringbuf &operator=(const ft_stringbuf &other) noexcept;
        ft_stringbuf(ft_stringbuf &&other) noexcept;
        ft_stringbuf &operator=(ft_stringbuf &&other) noexcept;
        ~ft_stringbuf() noexcept;

        std::size_t read(char *buffer, std::size_t count);
        bool is_valid() const noexcept;
        ft_string str() const;
#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_validation() const noexcept;
#endif
};

#endif
