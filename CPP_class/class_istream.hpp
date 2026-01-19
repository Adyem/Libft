#ifndef FT_ISTREAM_HPP
#define FT_ISTREAM_HPP

#include <cstddef>
#include "../Errno/errno_internal.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/unique_lock.hpp"

class ft_istream
{
    private:
        std::size_t _gcount;
        bool _bad;
        mutable int _error_code;
        mutable pt_recursive_mutex _mutex;
        static thread_local ft_operation_error_stack _operation_errors;

        void set_error_unlocked(int error_code) const noexcept;
        static void record_operation_error(int error_code) noexcept;
        int lock_self(ft_unique_lock<pt_recursive_mutex> &guard) const noexcept;
        static int lock_pair(const ft_istream &first, const ft_istream &second,
            ft_unique_lock<pt_recursive_mutex> &first_guard,
            ft_unique_lock<pt_recursive_mutex> &second_guard) noexcept;

    protected:
        ft_istream() noexcept;
        ft_istream(const ft_istream &other) noexcept;
        ft_istream(ft_istream &&other) noexcept;
        void set_error(int error_code) const noexcept;
        virtual std::size_t do_read(char *buffer, std::size_t count) = 0;

    public:
        virtual ~ft_istream() noexcept;

        ft_istream &operator=(const ft_istream &other) noexcept;
        ft_istream &operator=(ft_istream &&other) noexcept;

        void read(char *buffer, std::size_t count);
        std::size_t gcount() const noexcept;
        bool bad() const noexcept;
        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
        static const char *last_operation_error_str() noexcept;
        static const char *operation_error_str_at(ft_size_t index) noexcept;
        static int last_operation_error() noexcept;
        static int operation_error_at(ft_size_t index) noexcept;
        static void pop_operation_errors() noexcept;
        static int pop_oldest_operation_error() noexcept;
        static int operation_error_index() noexcept;
        pt_recursive_mutex *get_mutex_for_validation() const noexcept;
};

#endif
