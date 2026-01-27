#ifndef FILE_HPP
# define FILE_HPP

#ifndef DEBUG
# define DEBUG 0
#endif

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cstring>
#include <cstddef>
#include "../Errno/errno_internal.hpp"
#include "../PThread/recursive_mutex.hpp"

class ft_file
{
    private:
        int _fd;
        mutable pt_recursive_mutex _mutex;
        mutable ft_operation_error_stack _operation_errors = {{}, {}, 0};
        bool _is_open;

        void        record_operation_error(int error_code) const;
        static int lock_pair(const ft_file &first, const ft_file &second,
                const ft_file *&lower, const ft_file *&upper);
        static int unlock_pair(const ft_file *lower, const ft_file *upper);
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
        ssize_t        write_buffer(const char *buffer, size_t length) noexcept;
        void        close() noexcept;
        int            seek(off_t offset, int whence) noexcept;
        ssize_t        read(char *buffer, int count) noexcept;
        int            printf(const char *format, ...)
                        __attribute__((format(printf, 2, 3), hot));
        int            copy_to(const char *destination_path) noexcept;
        int            copy_to_with_buffer(const char *destination_path, size_t buffer_size) noexcept;

        operator int() const;
        pt_recursive_mutex &recursive_mutex() noexcept;
        ft_operation_error_stack &operation_error_stack() const noexcept;
};

#endif
