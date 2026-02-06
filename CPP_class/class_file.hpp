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
#include "../PThread/recursive_mutex.hpp"

class ft_file
{
    private:
        int _fd;
        mutable pt_recursive_mutex *_mutex;

        static int lock_pair(const ft_file &first, const ft_file &second,
                const ft_file *&lower, const ft_file *&upper);
        static int unlock_pair(const ft_file *lower, const ft_file *upper);
        static void sleep_backoff();
        int lock_mutex(void) const noexcept;
        int unlock_mutex(void) const noexcept;
        int prepare_thread_safety(void) noexcept;
        void teardown_thread_safety(void) noexcept;
        int enable_thread_safety(void) noexcept;
        void disable_thread_safety(void) noexcept;
        bool is_thread_safe_enabled(void) const noexcept;

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
#ifdef LIBFT_TEST_BUILD
        ft_recursive_mutex &recursive_mutex() noexcept;
#endif
};

#endif
