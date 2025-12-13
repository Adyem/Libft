#ifndef FT_OFSTREAM_HPP
#define FT_OFSTREAM_HPP

#include "class_file.hpp"
#include "../Errno/errno.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"

class ft_ofstream
{
    private:
        ft_file _file;
        mutable int _error_code;
        mutable pt_mutex _mutex;

        void set_error_unlocked(int error_code) const noexcept;
        void set_error(int error_code) const noexcept;
        int lock_self(ft_unique_lock<pt_mutex> &guard) const noexcept;
        static void restore_errno(ft_unique_lock<pt_mutex> &guard) noexcept;

    public:
        ft_ofstream() noexcept;
        ~ft_ofstream() noexcept;

        int open(const char *filename) noexcept;
        ssize_t write(const char *string) noexcept;
        void close() noexcept;
        int get_error() const noexcept;
        const char *get_error_str() const noexcept;
};

#endif
