#ifndef FT_OFSTREAM_HPP
#define FT_OFSTREAM_HPP

#include "class_file.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"

class ft_ofstream
{
    private:
        ft_file _file;
        mutable pt_recursive_mutex *_mutex;

        int lock_mutex(void) const noexcept;
        int unlock_mutex(void) const noexcept;
        int prepare_thread_safety(void) noexcept;
        void teardown_thread_safety(void) noexcept;
        int enable_thread_safety(void) noexcept;
        void disable_thread_safety(void) noexcept;
        bool is_thread_safe_enabled(void) const noexcept;
    public:
        ft_ofstream() noexcept;
        ~ft_ofstream() noexcept;

        int open(const char *filename) noexcept;
        ssize_t write(const char *string) noexcept;
        void close() noexcept;
};

#endif
