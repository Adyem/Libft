#include "compatebility_internal.hpp"
#include "../Libft/libft.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include <cstdlib>

#if defined(_WIN32) || defined(_WIN64)
# include <windows.h>
# include <sysinfoapi.h>
#else
# include <unistd.h>
# if defined(__APPLE__) && defined(__MACH__)
#  include <sys/types.h>
#  include <sys/sysctl.h>
# endif
#endif

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
    return (_putenv_s(name, ""));
#else
    return (unsetenv(name));
#endif
}

int cmp_putenv(char *string)
{
    if (string == ft_nullptr)
        return (-1);
#if defined(_WIN32) || defined(_WIN64)
    return (_putenv(string));
#else
    return (putenv(string));
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
    return (ft_strjoin_multiple(2, home_drive, home_path));
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
