#ifndef FT_STRINGBUF_HPP
#define FT_STRINGBUF_HPP

#include "class_string.hpp"
#include "../Errno/errno_internal.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/unique_lock.hpp"

class ft_stringbuf
{
    private:
        ft_string _storage;
        std::size_t _position;
        mutable int _error_code;
        mutable pt_recursive_mutex _mutex;
        static thread_local ft_operation_error_stack _operation_errors;

        void set_error_unlocked(int error_code) const noexcept;
        static void record_operation_error(int error_code) noexcept;
        void set_error(int error_code) const noexcept;
        int lock_self(ft_unique_lock<pt_recursive_mutex> &guard) const noexcept;
        static int lock_pair(const ft_stringbuf &first, const ft_stringbuf &second,
            ft_unique_lock<pt_recursive_mutex> &first_guard,
            ft_unique_lock<pt_recursive_mutex> &second_guard) noexcept;

    public:
        ft_stringbuf(const ft_string &string) noexcept;
        ft_stringbuf(const ft_stringbuf &other) noexcept;
        ft_stringbuf &operator=(const ft_stringbuf &other) noexcept;
        ft_stringbuf(ft_stringbuf &&other) noexcept;
        ft_stringbuf &operator=(ft_stringbuf &&other) noexcept;
        ~ft_stringbuf() noexcept;

        std::size_t read(char *buffer, std::size_t count);
        bool is_bad() const noexcept;
        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
        ft_string str() const;
        static const char *last_operation_error_str() noexcept;
        static const char *operation_error_str_at(ft_size_t index) noexcept;
        static int last_operation_error() noexcept;
        static int operation_error_at(ft_size_t index) noexcept;
        static void pop_operation_errors() noexcept;
        static int pop_oldest_operation_error() noexcept;
        static int operation_error_index() noexcept;
        pt_recursive_mutex *get_mutex_for_validation() const noexcept;
#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_testing() noexcept;
#endif
};

#endif
