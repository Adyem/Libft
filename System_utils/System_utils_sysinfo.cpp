#include "system_utils.hpp"
#include "../CPP_class/class_nullptr.hpp"

#if defined(_WIN32) || defined(_WIN64)
# include <windows.h>
#else
# include <unistd.h>
# if defined(__APPLE__) && defined(__MACH__)
#  include <sys/types.h>
#  include <sys/sysctl.h>
# endif
#endif

unsigned int    su_get_cpu_count(void)
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
    long cpu_count;

    cpu_count = sysconf(_SC_NPROCESSORS_ONLN);
    if (cpu_count < 0)
        return (0);
    return (static_cast<unsigned int>(cpu_count));
#endif
}

unsigned long long su_get_total_memory(void)
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
    long pages;
    long page_size;

    pages = sysconf(_SC_PHYS_PAGES);
    page_size = sysconf(_SC_PAGE_SIZE);
    if (pages < 0 || page_size < 0)
        return (0);
    return (static_cast<unsigned long long>(pages) *
            static_cast<unsigned long long>(page_size));
#endif
}

