#ifndef FT_BIG_NUMBER_HPP
#define FT_BIG_NUMBER_HPP

#include "../Libft/libft.hpp"
#include "../Errno/errno_internal.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/unique_lock.hpp"
#include <cstdint>

class ft_big_number;

typedef ft_unique_lock<pt_recursive_mutex>    ft_big_number_mutex_guard;

class ft_big_number
{
    private:

        char*           _digits;
        ft_size_t       _size;
        ft_size_t       _capacity;
        bool            _is_negative;
        mutable pt_recursive_mutex    _mutex;
        mutable ft_operation_error_stack _operation_errors = {{}, {}, 0};

        void    reserve(ft_size_t new_capacity) noexcept;
        void    shrink_capacity() noexcept;
        void    push_error_unlocked(int error_code) const noexcept;
        void    push_error(int error_code) const noexcept;
        int     lock_self(ft_big_number_mutex_guard &guard) const noexcept;
        static int  lock_pair(const ft_big_number &first, const ft_big_number &second,
                ft_big_number_mutex_guard &first_guard,
                ft_big_number_mutex_guard &second_guard) noexcept;
        static void sleep_backoff() noexcept;
        static int  initialize_errno_keeper() noexcept;
        static void update_errno_keeper(int &stored_errno, int new_value) noexcept;
        static void finalize_errno_keeper(int stored_errno) noexcept;
        void    record_operation_error(int error_code) const noexcept;
        static void reset_error_owner(const ft_big_number *owner) noexcept;
        void    clear_unlocked() noexcept;
        void    append_digit_unlocked(char digit) noexcept;
        void    append_unlocked(const char* digits) noexcept;
        void    append_unsigned_unlocked(unsigned long value) noexcept;
        void    trim_leading_zeros_unlocked() noexcept;
        void    reduce_to_unlocked(ft_size_t new_size) noexcept;
        bool    is_zero_value() const noexcept;
        int     compare_magnitude(const ft_big_number& other) const noexcept;
        ft_big_number    add_magnitude(const ft_big_number& other) const noexcept;
        ft_big_number    subtract_magnitude(const ft_big_number& other) const noexcept;

    public:
        ft_big_number() noexcept;
        ft_big_number(const ft_big_number& other) noexcept;
        ft_big_number(ft_big_number&& other) noexcept;
        ~ft_big_number() noexcept;

        ft_big_number& operator=(const ft_big_number& other) noexcept;
        ft_big_number& operator=(ft_big_number&& other) noexcept;
        ft_big_number operator+(const ft_big_number& other) const noexcept;
        ft_big_number operator-(const ft_big_number& other) const noexcept;
        ft_big_number operator*(const ft_big_number& other) const noexcept;
        ft_big_number operator/(const ft_big_number& other) const noexcept;
        ft_big_number operator%(const ft_big_number& other) const noexcept;
        bool          operator==(const ft_big_number& other) const noexcept;
        bool          operator!=(const ft_big_number& other) const noexcept;
        bool          operator<(const ft_big_number& other) const noexcept;
        bool          operator<=(const ft_big_number& other) const noexcept;
        bool          operator>(const ft_big_number& other) const noexcept;
        bool          operator>=(const ft_big_number& other) const noexcept;

        void        assign(const char* number) noexcept;
        void        assign_base(const char* digits, int base) noexcept;
        void        append_digit(char digit) noexcept;
        void        append(const char* digits) noexcept;
        void        append_unsigned(unsigned long value) noexcept;
        void        trim_leading_zeros() noexcept;
        void        reduce_to(ft_size_t new_size) noexcept;
        void        clear() noexcept;

        const char* c_str() const noexcept;
        ft_size_t   size() const noexcept;
        bool        empty() const noexcept;
        bool        is_negative() const noexcept;
        bool        is_positive() const noexcept;
        ft_string   to_string_base(int base) noexcept;
        ft_big_number mod_pow(const ft_big_number& exponent, const ft_big_number& modulus) const noexcept;
        static const char *last_operation_error_str() noexcept;
        static const char *operation_error_str_at(ft_size_t index) noexcept;
        void        reset_system_error() const noexcept;
        static int  last_error() noexcept;
        static unsigned long long last_op_id() noexcept;
        static int  error_for(unsigned long long operation_id) noexcept;
        static int  last_operation_error() noexcept;
        static int  operation_error_at(ft_size_t index) noexcept;
        static void pop_operation_errors() noexcept;
        static int  pop_oldest_operation_error() noexcept;
        static int  operation_error_index() noexcept;

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex    *get_mutex_for_testing() noexcept;
#endif
};

ft_string   big_number_to_hex_string(const ft_big_number& number) noexcept;
ft_big_number   big_number_from_hex_string(const char* hex_digits) noexcept;

#endif
