#include "compatebility_internal.hpp"
#include "../Libft/libft.hpp"
#include "../Errno/errno.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include <cstdlib>
#include <ctime>
#include <cerrno>
#include <cstring>

#if defined(_WIN32) || defined(_WIN64)
# include <winsock2.h>
# include <windows.h>
# include <sysinfoapi.h>
#else
# include <unistd.h>
# if defined(__APPLE__) && defined(__MACH__)
#  include <sys/types.h>
#  include <sys/sysctl.h>
# endif
#endif

static int global_force_unsetenv_enabled = 0;
static int global_force_unsetenv_result = 0;
static int global_force_unsetenv_errno_value = 0;
static int global_force_putenv_enabled = 0;
static int global_force_putenv_result = 0;
static int global_force_putenv_errno_value = 0;
#if defined(_WIN32) || defined(_WIN64)
static int global_force_unsetenv_last_error = 0;
static int global_force_unsetenv_socket_error = 0;
static int global_force_putenv_last_error = 0;
#endif

void cmp_set_force_unsetenv_result(int result, int errno_value)
{
    global_force_unsetenv_enabled = 1;
    global_force_unsetenv_result = result;
    global_force_unsetenv_errno_value = errno_value;
#if defined(_WIN32) || defined(_WIN64)
    global_force_unsetenv_last_error = 0;
    global_force_unsetenv_socket_error = 0;
#endif
    return ;
}

void cmp_clear_force_unsetenv_result(void)
{
    global_force_unsetenv_enabled = 0;
    return ;
}

void cmp_set_force_putenv_result(int result, int errno_value)
{
    global_force_putenv_enabled = 1;
    global_force_putenv_result = result;
    global_force_putenv_errno_value = errno_value;
#if defined(_WIN32) || defined(_WIN64)
    global_force_putenv_last_error = 0;
#endif
    return ;
}

void cmp_clear_force_putenv_result(void)
{
    global_force_putenv_enabled = 0;
    return ;
}

void cmp_set_force_unsetenv_windows_errors(int last_error, int socket_error)
{
#if defined(_WIN32) || defined(_WIN64)
    global_force_unsetenv_last_error = last_error;
    global_force_unsetenv_socket_error = socket_error;
#else
    (void)last_error;
    (void)socket_error;
#endif
    return ;
}

void cmp_set_force_putenv_windows_error(int last_error)
{
#if defined(_WIN32) || defined(_WIN64)
    global_force_putenv_last_error = last_error;
#else
    (void)last_error;
#endif
    return ;
}

int cmp_setenv(const char *name, const char *value, int overwrite)
{
#if defined(_WIN32) || defined(_WIN64)
    if (!overwrite && std::getenv(name) != ft_nullptr)
        return (0);
    return (_putenv_s(name, value));
#else
    return (setenv(name, value, overwrite));
#endif
}

int cmp_unsetenv(const char *name)
{
#if defined(_WIN32) || defined(_WIN64)
    if (global_force_unsetenv_enabled != 0)
    {
        errno = global_force_unsetenv_errno_value;
        SetLastError(global_force_unsetenv_last_error);
        WSASetLastError(global_force_unsetenv_socket_error);
        return (global_force_unsetenv_result);
    }
    return (_putenv_s(name, ""));
#else
    if (global_force_unsetenv_enabled != 0)
    {
        errno = global_force_unsetenv_errno_value;
        return (global_force_unsetenv_result);
    }
    return (unsetenv(name));
#endif
}

int cmp_putenv(char *string)
{
    int result;

    if (string == ft_nullptr)
    {
        ft_errno = FT_EINVAL;
        return (-1);
    }
    if (global_force_putenv_enabled != 0)
    {
        errno = global_force_putenv_errno_value;
#if defined(_WIN32) || defined(_WIN64)
        SetLastError(global_force_putenv_last_error);
#endif
        result = global_force_putenv_result;
        if (result != 0)
        {
#if defined(_WIN32) || defined(_WIN64)
            if (global_force_putenv_last_error != 0)
                ft_errno = global_force_putenv_last_error + ERRNO_OFFSET;
            else if (global_force_putenv_errno_value != 0)
                ft_errno = global_force_putenv_errno_value + ERRNO_OFFSET;
            else
                ft_errno = FT_EINVAL;
#else
            if (global_force_putenv_errno_value != 0)
                ft_errno = global_force_putenv_errno_value + ERRNO_OFFSET;
            else
                ft_errno = FT_EINVAL;
#endif
        }
        else
            ft_errno = ER_SUCCESS;
        return (result);
    }
#if defined(_WIN32) || defined(_WIN64)
    result = _putenv(string);
    if (result != 0)
    {
        DWORD last_error;

        last_error = GetLastError();
        if (last_error != 0)
            ft_errno = static_cast<int>(last_error) + ERRNO_OFFSET;
        else if (errno != 0)
            ft_errno = errno + ERRNO_OFFSET;
        else
            ft_errno = FT_EINVAL;
        return (result);
    }
    ft_errno = ER_SUCCESS;
    return (result);
#else
    result = putenv(string);
    if (result != 0)
    {
        if (errno != 0)
            ft_errno = errno + ERRNO_OFFSET;
        else
            ft_errno = FT_EINVAL;
        return (result);
    }
    ft_errno = ER_SUCCESS;
    return (result);
#endif
}

const char *cmp_system_strerror(int error_code)
{
#if defined(_WIN32) || defined(_WIN64)
    static char message_buffer[512];
    DWORD system_error;
    DWORD format_result;

    if (error_code <= ERRNO_OFFSET)
        return (NULL);
    system_error = static_cast<DWORD>(error_code - ERRNO_OFFSET);
    format_result = FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        system_error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        message_buffer,
        static_cast<DWORD>(sizeof(message_buffer)),
        NULL);
    if (format_result != 0)
    {
        while (format_result > 0)
        {
            char current_character;

            current_character = message_buffer[format_result - 1];
            if (current_character != '\r' && current_character != '\n'
                && current_character != '\t' && current_character != ' '
                && current_character != '\0')
                break;
            message_buffer[format_result - 1] = '\0';
            format_result--;
        }
        if (format_result != 0)
            return (message_buffer);
    }
    return (strerror(static_cast<int>(system_error)));
#else
    if (error_code > ERRNO_OFFSET)
    {
        int standard_errno;

        standard_errno = error_code - ERRNO_OFFSET;
        return (strerror(standard_errno));
    }
    return (NULL);
#endif
}

char *cmp_get_home_directory(void)
{
#if defined(_WIN32) || defined(_WIN64)
    char *home = ft_getenv("USERPROFILE");
    if (home != ft_nullptr)
        return (home);
    char *home_drive = ft_getenv("HOMEDRIVE");
    char *home_path = ft_getenv("HOMEPATH");
    if (home_drive == ft_nullptr || home_path == ft_nullptr)
        return (ft_nullptr);
    return (cma_strjoin_multiple(2, home_drive, home_path));
#else
    return (ft_getenv("HOME"));
#endif
}

unsigned int cmp_get_cpu_count(void)
{
#if defined(_WIN32) || defined(_WIN64)
    SYSTEM_INFO system_info;
    GetSystemInfo(&system_info);
    return (system_info.dwNumberOfProcessors);
#elif defined(__APPLE__) && defined(__MACH__)
    int cpu_count;
    size_t size;
    size = sizeof(cpu_count);
    if (sysctlbyname("hw.ncpu", &cpu_count, &size, ft_nullptr, 0) != 0)
        return (0);
    return (static_cast<unsigned int>(cpu_count));
#else
    long cpu_count = sysconf(_SC_NPROCESSORS_ONLN);
    if (cpu_count < 0)
        return (0);
    return (static_cast<unsigned int>(cpu_count));
#endif
}

unsigned long long cmp_get_total_memory(void)
{
#if defined(_WIN32) || defined(_WIN64)
    MEMORYSTATUSEX memory_status;
    memory_status.dwLength = sizeof(memory_status);
    if (GlobalMemoryStatusEx(&memory_status) == 0)
        return (0);
    return (memory_status.ullTotalPhys);
#elif defined(__APPLE__) && defined(__MACH__)
    unsigned long long memory_size;
    size_t size;
    size = sizeof(memory_size);
    if (sysctlbyname("hw.memsize", &memory_size, &size, ft_nullptr, 0) != 0)
        return (0);
    return (memory_size);
#else
    long pages = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGE_SIZE);
    if (pages < 0 || page_size < 0)
        return (0);
    return (static_cast<unsigned long long>(pages) *
            static_cast<unsigned long long>(page_size));
#endif
}

std::time_t cmp_timegm(std::tm *time_pointer)
{
    if (time_pointer == ft_nullptr)
        return (static_cast<std::time_t>(-1));
#if defined(_WIN32) || defined(_WIN64)
    return (_mkgmtime(time_pointer));
#else
    return (::timegm(time_pointer));
#endif
}
