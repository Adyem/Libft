#ifndef FILE_HPP
# define FILE_HPP

#ifndef DEBUG
# define DEBUG 0
#endif

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cstring>
#include "../PThread/mutex.hpp"
#include "../PThread/unique_lock.hpp"

class ft_file
{
    private:
        int _fd;
        mutable int _error_code;
        mutable pt_mutex _mutex;

        void        set_error(int error_code);
        static int lock_pair(const ft_file &first, const ft_file &second,
                ft_unique_lock<pt_mutex> &first_guard,
                ft_unique_lock<pt_mutex> &second_guard);
        static void restore_errno(ft_unique_lock<pt_mutex> &guard,
                int entry_errno,
                bool restore_previous_on_success = false) noexcept;
        static void sleep_backoff();

    public:
        ft_file() noexcept;
        ft_file(const char* filename, int flags, mode_t mode) noexcept;
        ft_file(const char* filename, int flags) noexcept;
        ft_file(int fd) noexcept;
        ~ft_file() noexcept;

        ft_file(const ft_file&) = delete;
        ft_file &operator=(const ft_file&) = delete;
        ft_file(ft_file&& other) noexcept;
        ft_file& operator=(ft_file&& other) noexcept;

        int            get_fd() const;
        int            get_error() const noexcept;
        const char    *get_error_str() const noexcept;

        int            open(const char* filename, int flags, mode_t mode) noexcept;
        int            open(const char* filename, int flags) noexcept;
        ssize_t        write(const char *string) noexcept;
        void        close() noexcept;
        int            seek(off_t offset, int whence) noexcept;
        ssize_t        read(char *buffer, int count) noexcept;
        int            printf(const char *format, ...)
                        __attribute__((format(printf, 2, 3), hot));

        operator int() const;
};

#endif
