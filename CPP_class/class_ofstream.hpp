#ifndef FT_OFSTREAM_HPP
#define FT_OFSTREAM_HPP

#include "class_file.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/unique_lock.hpp"

class ft_ofstream
{
    private:
        ft_file _file;
        mutable pt_recursive_mutex _mutex;

        int lock_self(ft_unique_lock<pt_recursive_mutex> &guard) const noexcept;
        static int finalize_lock(ft_unique_lock<pt_recursive_mutex> &guard) noexcept;
        static int capture_guard_error() noexcept;
    public:
        ft_ofstream() noexcept;
        ~ft_ofstream() noexcept;

        int open(const char *filename) noexcept;
        ssize_t write(const char *string) noexcept;
        void close() noexcept;
};

#endif
