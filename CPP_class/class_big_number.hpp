#ifndef FT_BIG_NUMBER_HPP
#define FT_BIG_NUMBER_HPP

#include "../Libft/libft.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"

class ft_big_number;

typedef ft_unique_lock<pt_mutex>    ft_big_number_mutex_guard;

class ft_big_number
{
    private:
        char*           _digits;
        ft_size_t       _size;
        ft_size_t       _capacity;
        bool            _is_negative;
        mutable int     _error_code;
        mutable pt_mutex    _mutex;

        void    reserve(ft_size_t new_capacity) noexcept;
        void    shrink_capacity() noexcept;
        void    set_error_unlocked(int error_code) const noexcept;
        void    set_error(int error_code) const noexcept;
        int     lock_self(ft_big_number_mutex_guard &guard) const noexcept;
        static int  lock_pair(const ft_big_number &first, const ft_big_number &second,
                ft_big_number_mutex_guard &first_guard,
                ft_big_number_mutex_guard &second_guard) noexcept;
        static void sleep_backoff() noexcept;
        static int  initialize_errno_keeper() noexcept;
        static void update_errno_keeper(int &stored_errno, int new_value) noexcept;
        static void finalize_errno_keeper(int stored_errno) noexcept;
        static void unlock_guard_preserve_errno(ft_big_number_mutex_guard &guard,
                int &stored_errno) noexcept;
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
        int         get_error() const noexcept;
        const char* get_error_str() const noexcept;
};

ft_string   big_number_to_hex_string(const ft_big_number& number) noexcept;
ft_big_number   big_number_from_hex_string(const char* hex_digits) noexcept;

#endif
