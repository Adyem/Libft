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

        int lock_mutex(void) const noexcept;
        int unlock_mutex(void) const noexcept;
        int prepare_thread_safety(void) noexcept;
        void teardown_thread_safety(void) noexcept;
        int enable_thread_safety(void) noexcept;
        void disable_thread_safety(void) noexcept;
        bool is_thread_safe_enabled(void) const noexcept;

    public:
        ft_file() noexcept;
        ~ft_file() noexcept;

        ft_file(const ft_file&) = delete;
        ft_file &operator=(const ft_file&) = delete;
        ft_file(ft_file&& other) = delete;
        ft_file& operator=(ft_file&& other) = delete;

        int            get_fd() const;

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
#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex &recursive_mutex(void) noexcept;
#endif
};

#endif
