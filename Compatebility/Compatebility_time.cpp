#include "compatebility_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/mutex.hpp"
#include <cerrno>
#include <ctime>
#if !defined(_WIN32) && !defined(_WIN64)
# include <unistd.h>
# include <sys/time.h>
#else
# include <sysinfoapi.h>
#endif

#if !defined(_WIN32) && !defined(_WIN64) && !defined(_POSIX_VERSION)
static int  cmp_localtime_from_shared_state(const std::time_t *time_value, std::tm *output)
{
    static pt_mutex  localtime_mutex;
    std::tm          *shared_result;

    if (localtime_mutex.lock(THREAD_ID) != FT_SUCCESS)
        return (-1);
    shared_result = std::localtime(time_value);
    if (!shared_result)
    {
        if (localtime_mutex.unlock(THREAD_ID) != FT_SUCCESS)
            return (-1);
        if (errno != 0)
            ft_errno = errno + ERRNO_OFFSET;
        else
            ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
    *output = *shared_result;
    if (localtime_mutex.unlock(THREAD_ID) != FT_SUCCESS)
        return (-1);
    ft_errno = ER_SUCCESS;
    return (0);
}
#endif

int cmp_localtime(const std::time_t *time_value, std::tm *output)
{
    if (!time_value || !output)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
#if defined(_WIN32) || defined(_WIN64)
    errno_t error_code;

    error_code = localtime_s(output, time_value);
    if (error_code == 0)
    {
        ft_errno = ER_SUCCESS;
        return (0);
    }
    ft_errno = error_code + ERRNO_OFFSET;
    return (-1);
#else
# if defined(_POSIX_VERSION)
    if (localtime_r(time_value, output) != ft_nullptr)
    {
        ft_errno = ER_SUCCESS;
        return (0);
    }
    if (errno != 0)
        ft_errno = errno + ERRNO_OFFSET;
    else
        ft_errno = FT_ERR_INVALID_ARGUMENT;
    return (-1);
# else
#  if !defined(_WIN32) && !defined(_WIN64)
    return (cmp_localtime_from_shared_state(time_value, output));
#  else
    (void)time_value;
    (void)output;
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    return (-1);
#  endif
# endif
#endif
}

int cmp_time_get_time_of_day(struct timeval *time_value)
{
    if (!time_value)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return (-1);
    }
#if defined(_WIN32) || defined(_WIN64)
    FILETIME file_time;
    ULARGE_INTEGER file_time_value;
    unsigned long long microseconds_since_epoch;

    GetSystemTimeAsFileTime(&file_time);
    file_time_value.LowPart = file_time.dwLowDateTime;
    file_time_value.HighPart = file_time.dwHighDateTime;
    microseconds_since_epoch = (file_time_value.QuadPart - 116444736000000000ULL) / 10ULL;
    time_value->tv_sec = static_cast<long>(microseconds_since_epoch / 1000000ULL);
    time_value->tv_usec = static_cast<long>(microseconds_since_epoch % 1000000ULL);
    ft_errno = ER_SUCCESS;
    return (0);
#else
    if (gettimeofday(time_value, ft_nullptr) == 0)
    {
        ft_errno = ER_SUCCESS;
        return (0);
    }
    ft_errno = errno + ERRNO_OFFSET;
    return (-1);
#endif
}
