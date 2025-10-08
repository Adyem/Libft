#include "compatebility_internal.hpp"
#include "../Libft/libft.hpp"
#include "../Errno/errno.hpp"
#include "../CMA/CMA.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include <cstdlib>
#include <ctime>
#include <cerrno>
#include <cstring>

void cmp_set_force_unsetenv_result(int result, int errno_value);
void cmp_clear_force_unsetenv_result(void);
void cmp_set_force_unsetenv_windows_errors(int last_error, int socket_error);
void cmp_set_force_putenv_result(int result, int errno_value);
void cmp_clear_force_putenv_result(void);
void cmp_set_force_putenv_windows_error(int last_error);
void cmp_set_force_cpu_count_success(unsigned int cpu_count);
void cmp_set_force_cpu_count_failure(int errno_value);
void cmp_clear_force_cpu_count_result(void);
void cmp_set_force_total_memory_success(unsigned long long memory_size);
void cmp_set_force_total_memory_failure(int errno_value);
#if defined(_WIN32) || defined(_WIN64)
void cmp_set_force_total_memory_windows_failure(unsigned long last_error);
#endif
void cmp_clear_force_total_memory_result(void);

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
static int global_force_cpu_count_enabled = 0;
static int global_force_cpu_count_should_fail = 0;
static unsigned int global_force_cpu_count_value = 0;
static int global_force_cpu_count_errno_value = 0;
static int global_force_total_memory_enabled = 0;
static int global_force_total_memory_should_fail = 0;
static unsigned long long global_force_total_memory_value = 0;
static int global_force_total_memory_errno_value = 0;
#if defined(_WIN32) || defined(_WIN64)
static unsigned long global_force_total_memory_last_error = 0;
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

void cmp_set_force_cpu_count_success(unsigned int cpu_count)
{
    global_force_cpu_count_enabled = 1;
    global_force_cpu_count_should_fail = 0;
    global_force_cpu_count_value = cpu_count;
    global_force_cpu_count_errno_value = 0;
    return ;
}

void cmp_set_force_cpu_count_failure(int errno_value)
{
    global_force_cpu_count_enabled = 1;
    global_force_cpu_count_should_fail = 1;
    global_force_cpu_count_errno_value = errno_value;
    global_force_cpu_count_value = 0;
    return ;
}

void cmp_clear_force_cpu_count_result(void)
{
    global_force_cpu_count_enabled = 0;
    global_force_cpu_count_should_fail = 0;
    global_force_cpu_count_value = 0;
    global_force_cpu_count_errno_value = 0;
    return ;
}

void cmp_set_force_total_memory_success(unsigned long long memory_size)
{
    global_force_total_memory_enabled = 1;
    global_force_total_memory_should_fail = 0;
    global_force_total_memory_value = memory_size;
    global_force_total_memory_errno_value = 0;
#if defined(_WIN32) || defined(_WIN64)
    global_force_total_memory_last_error = 0;
#endif
    return ;
}

void cmp_set_force_total_memory_failure(int errno_value)
{
    global_force_total_memory_enabled = 1;
    global_force_total_memory_should_fail = 1;
    global_force_total_memory_errno_value = errno_value;
    global_force_total_memory_value = 0;
#if defined(_WIN32) || defined(_WIN64)
    global_force_total_memory_last_error = 0;
#endif
    return ;
}

#if defined(_WIN32) || defined(_WIN64)
void cmp_set_force_total_memory_windows_failure(unsigned long last_error)
{
    global_force_total_memory_enabled = 1;
    global_force_total_memory_should_fail = 1;
    global_force_total_memory_last_error = last_error;
    global_force_total_memory_errno_value = 0;
    global_force_total_memory_value = 0;
    return ;
}
#endif

void cmp_clear_force_total_memory_result(void)
{
    global_force_total_memory_enabled = 0;
    global_force_total_memory_should_fail = 0;
    global_force_total_memory_value = 0;
    global_force_total_memory_errno_value = 0;
#if defined(_WIN32) || defined(_WIN64)
    global_force_total_memory_last_error = 0;
#endif
    return ;
}

int cmp_setenv(const char *name, const char *value, int overwrite)
{
    if (name == ft_nullptr || value == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
#if defined(_WIN32) || defined(_WIN64)
    if (!overwrite && getenv(name) != ft_nullptr)
    {
        ft_errno = ER_SUCCESS;
        return (0);
    }
    errno = 0;
    int result = _putenv_s(name, value);
    if (result != 0)
    {
        DWORD last_error;

        last_error = GetLastError();
        if (last_error != 0)
            ft_errno = static_cast<int>(last_error) + ERRNO_OFFSET;
        else if (errno != 0)
            ft_errno = errno + ERRNO_OFFSET;
        else
            ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (result);
    }
    ft_errno = ER_SUCCESS;
    return (result);
#else
    errno = 0;
    int result = setenv(name, value, overwrite);
    if (result != 0)
    {
        if (errno != 0)
            ft_errno = errno + ERRNO_OFFSET;
        else
            ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (result);
    }
    ft_errno = ER_SUCCESS;
    return (result);
#endif
}

int cmp_unsetenv(const char *name)
{
    if (name == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
#if defined(_WIN32) || defined(_WIN64)
    if (global_force_unsetenv_enabled != 0)
    {
        errno = global_force_unsetenv_errno_value;
        SetLastError(global_force_unsetenv_last_error);
        WSASetLastError(global_force_unsetenv_socket_error);
        int forced_result = global_force_unsetenv_result;
        if (forced_result != 0)
        {
            if (global_force_unsetenv_last_error != 0)
                ft_errno = global_force_unsetenv_last_error + ERRNO_OFFSET;
            else if (global_force_unsetenv_socket_error != 0)
                ft_errno = global_force_unsetenv_socket_error + ERRNO_OFFSET;
            else if (global_force_unsetenv_errno_value != 0)
                ft_errno = global_force_unsetenv_errno_value + ERRNO_OFFSET;
            else
                ft_errno = FT_ERR_INVALID_ARGUMENT;
        }
        else
            ft_errno = ER_SUCCESS;
        return (forced_result);
    }
    errno = 0;
    int result = _putenv_s(name, "");
    if (result != 0)
    {
        DWORD last_error;

        last_error = GetLastError();
        if (last_error != 0)
            ft_errno = static_cast<int>(last_error) + ERRNO_OFFSET;
        else if (errno != 0)
            ft_errno = errno + ERRNO_OFFSET;
        else
            ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (result);
    }
    ft_errno = ER_SUCCESS;
    return (result);
#else
    if (global_force_unsetenv_enabled != 0)
    {
        errno = global_force_unsetenv_errno_value;
        int forced_result = global_force_unsetenv_result;
        if (forced_result != 0)
        {
            if (global_force_unsetenv_errno_value != 0)
                ft_errno = global_force_unsetenv_errno_value + ERRNO_OFFSET;
            else
                ft_errno = FT_ERR_INVALID_ARGUMENT;
        }
        else
            ft_errno = ER_SUCCESS;
        return (forced_result);
    }
    errno = 0;
    int result = unsetenv(name);
    if (result != 0)
    {
        if (errno != 0)
            ft_errno = errno + ERRNO_OFFSET;
        else
            ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (result);
    }
    ft_errno = ER_SUCCESS;
    return (result);
#endif
}

int cmp_putenv(char *string)
{
    int result;

    if (string == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
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
                ft_errno = FT_ERR_INVALID_ARGUMENT;
#else
            if (global_force_putenv_errno_value != 0)
                ft_errno = global_force_putenv_errno_value + ERRNO_OFFSET;
            else
                ft_errno = FT_ERR_INVALID_ARGUMENT;
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
            ft_errno = FT_ERR_INVALID_ARGUMENT;
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
            ft_errno = FT_ERR_INVALID_ARGUMENT;
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
    char *home;
    char *home_drive;
    char *home_path;
    char *combined_home;

    home = ft_getenv("USERPROFILE");
    if (home != ft_nullptr)
    {
        ft_errno = ER_SUCCESS;
        return (home);
    }
    home_drive = ft_getenv("HOMEDRIVE");
    home_path = ft_getenv("HOMEPATH");
    if (home_drive == ft_nullptr || home_path == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    combined_home = cma_strjoin_multiple(2, home_drive, home_path);
    if (combined_home == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    ft_errno = ER_SUCCESS;
    return (combined_home);
#else
    char *home;

    home = ft_getenv("HOME");
    if (home == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (ft_nullptr);
    }
    ft_errno = ER_SUCCESS;
    return (home);
#endif
}

unsigned int cmp_get_cpu_count(void)
{
    if (global_force_cpu_count_enabled != 0)
    {
        if (global_force_cpu_count_should_fail != 0)
        {
            if (global_force_cpu_count_errno_value != 0)
                ft_errno = global_force_cpu_count_errno_value + ERRNO_OFFSET;
            else
                ft_errno = FT_ERR_TERMINATED;
            return (0);
        }
        ft_errno = ER_SUCCESS;
        return (global_force_cpu_count_value);
    }
#if defined(_WIN32) || defined(_WIN64)
    SYSTEM_INFO system_info;

    GetSystemInfo(&system_info);
    ft_errno = ER_SUCCESS;
    return (system_info.dwNumberOfProcessors);
#elif defined(__APPLE__) && defined(__MACH__)
    int cpu_count;
    size_t size;

    size = sizeof(cpu_count);
    errno = 0;
    if (sysctlbyname("hw.ncpu", &cpu_count, &size, ft_nullptr, 0) != 0)
    {
        if (errno != 0)
            ft_errno = errno + ERRNO_OFFSET;
        else
            ft_errno = FT_ERR_TERMINATED;
        return (0);
    }
    ft_errno = ER_SUCCESS;
    return (static_cast<unsigned int>(cpu_count));
#else
    long cpu_count;

    errno = 0;
    cpu_count = sysconf(_SC_NPROCESSORS_ONLN);
    if (cpu_count < 0)
    {
        if (errno != 0)
            ft_errno = errno + ERRNO_OFFSET;
        else
            ft_errno = FT_ERR_TERMINATED;
        return (0);
    }
    ft_errno = ER_SUCCESS;
    return (static_cast<unsigned int>(cpu_count));
#endif
}

unsigned long long cmp_get_total_memory(void)
{
    if (global_force_total_memory_enabled != 0)
    {
        if (global_force_total_memory_should_fail != 0)
        {
#if defined(_WIN32) || defined(_WIN64)
            if (global_force_total_memory_last_error != 0)
                ft_errno = static_cast<int>(global_force_total_memory_last_error) + ERRNO_OFFSET;
            else if (global_force_total_memory_errno_value != 0)
                ft_errno = global_force_total_memory_errno_value + ERRNO_OFFSET;
            else
                ft_errno = FT_ERR_TERMINATED;
#else
            if (global_force_total_memory_errno_value != 0)
                ft_errno = global_force_total_memory_errno_value + ERRNO_OFFSET;
            else
                ft_errno = FT_ERR_TERMINATED;
#endif
            return (0);
        }
        ft_errno = ER_SUCCESS;
        return (global_force_total_memory_value);
    }
#if defined(_WIN32) || defined(_WIN64)
    MEMORYSTATUSEX memory_status;

    memory_status.dwLength = sizeof(memory_status);
    if (GlobalMemoryStatusEx(&memory_status) == 0)
    {
        DWORD last_error;

        last_error = GetLastError();
        if (last_error != 0)
            ft_errno = static_cast<int>(last_error) + ERRNO_OFFSET;
        else
            ft_errno = FT_ERR_TERMINATED;
        return (0);
    }
    ft_errno = ER_SUCCESS;
    return (memory_status.ullTotalPhys);
#elif defined(__APPLE__) && defined(__MACH__)
    unsigned long long memory_size;
    size_t size;

    size = sizeof(memory_size);
    errno = 0;
    if (sysctlbyname("hw.memsize", &memory_size, &size, ft_nullptr, 0) != 0)
    {
        if (errno != 0)
            ft_errno = errno + ERRNO_OFFSET;
        else
            ft_errno = FT_ERR_TERMINATED;
        return (0);
    }
    ft_errno = ER_SUCCESS;
    return (memory_size);
#else
    long pages;
    long page_size;

    errno = 0;
    pages = sysconf(_SC_PHYS_PAGES);
    if (pages < 0)
    {
        if (errno != 0)
            ft_errno = errno + ERRNO_OFFSET;
        else
            ft_errno = FT_ERR_TERMINATED;
        return (0);
    }
    errno = 0;
    page_size = sysconf(_SC_PAGE_SIZE);
    if (page_size < 0)
    {
        if (errno != 0)
            ft_errno = errno + ERRNO_OFFSET;
        else
            ft_errno = FT_ERR_TERMINATED;
        return (0);
    }
    ft_errno = ER_SUCCESS;
    return (static_cast<unsigned long long>(pages) *
            static_cast<unsigned long long>(page_size));
#endif
}

std::time_t cmp_timegm(std::tm *time_pointer)
{
    std::time_t conversion_result;

    if (time_pointer == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (static_cast<std::time_t>(-1));
    }
    errno = 0;
#if defined(_WIN32) || defined(_WIN64)
    conversion_result = _mkgmtime(time_pointer);
#else
    conversion_result = ::timegm(time_pointer);
#endif
    if (conversion_result == static_cast<std::time_t>(-1))
    {
        if (errno != 0)
            ft_errno = errno + ERRNO_OFFSET;
        else
            ft_errno = ER_SUCCESS;
    }
    else
        ft_errno = ER_SUCCESS;
    return (conversion_result);
}
