#include "compatebility_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Errno/errno.hpp"
#include "../Libft/libft.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../PThread/pthread.hpp"
#include <cerrno>
#include <ctime>
#include <climits>

#if !defined(_WIN32) && !defined(_WIN64)
# include <unistd.h>
# include <sys/time.h>
#else
# include <sysinfoapi.h>
# include <profileapi.h>
#endif

#if !defined(_WIN32) && !defined(_WIN64) && !defined(_POSIX_VERSION)
static int cmp_lock_pt_mutex(pt_mutex *mutex)
{
    if (mutex == ft_nullptr)
    {
        cmp_set_last_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    int lock_error = pt_mutex_lock_with_error(*mutex);
    if (lock_error != FT_ERR_SUCCESSS)
    {
        cmp_set_last_error(lock_error);
        return (-1);
    }
    return (FT_ERR_SUCCESSS);
}

static int cmp_unlock_pt_mutex(pt_mutex *mutex)
{
    if (mutex == ft_nullptr)
    {
        cmp_set_last_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    int unlock_error = pt_mutex_unlock_with_error(*mutex);
    if (unlock_error != FT_ERR_SUCCESSS)
    {
        cmp_set_last_error(unlock_error);
        return (-1);
    }
    return (FT_ERR_SUCCESSS);
}

static int cmp_localtime_from_shared_state(const std::time_t *time_value, std::tm *output)
{
    static pt_mutex localtime_mutex;
    std::tm *shared_result;

    if (cmp_lock_pt_mutex(&localtime_mutex) != FT_ERR_SUCCESSS)
        return (-1);
    shared_result = std::localtime(time_value);
    if (shared_result == ft_nullptr)
    {
        if (cmp_unlock_pt_mutex(&localtime_mutex) != FT_ERR_SUCCESSS)
            return (-1);
        if (errno != 0)
            cmp_set_last_error(ft_map_system_error(errno));
        else
            cmp_set_last_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    *output = *shared_result;
    if (cmp_unlock_pt_mutex(&localtime_mutex) != FT_ERR_SUCCESSS)
        return (-1);
    cmp_set_last_error(FT_ERR_SUCCESSS);
    return (0);
}
#endif

int cmp_localtime(const std::time_t *time_value, std::tm *output)
{
    if (time_value == ft_nullptr || output == ft_nullptr)
    {
        cmp_set_last_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
#if defined(_WIN32) || defined(_WIN64)
    errno_t error_code;

    error_code = localtime_s(output, time_value);
    if (error_code == 0)
    {
        cmp_set_last_error(FT_ERR_SUCCESSS);
        return (0);
    }
    cmp_set_last_error(ft_map_system_error(static_cast<int>(error_code)));
    return (-1);
#else
# if defined(_POSIX_VERSION)
    if (localtime_r(time_value, output) != ft_nullptr)
    {
        cmp_set_last_error(FT_ERR_SUCCESSS);
        return (0);
    }
    if (errno != 0)
        cmp_set_last_error(ft_map_system_error(errno));
    else
        cmp_set_last_error(FT_ERR_INVALID_ARGUMENT);
    return (-1);
# else
    return (cmp_localtime_from_shared_state(time_value, output));
# endif
#endif
}

int cmp_time_get_time_of_day(struct timeval *time_value)
{
    if (time_value == ft_nullptr)
    {
        cmp_set_last_error(FT_ERR_INVALID_ARGUMENT);
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
    cmp_set_last_error(FT_ERR_SUCCESSS);
    return (0);
#else
    if (gettimeofday(time_value, ft_nullptr) == 0)
    {
        cmp_set_last_error(FT_ERR_SUCCESSS);
        return (0);
    }
    cmp_set_last_error(ft_map_system_error(errno));
    return (-1);
#endif
}

static int cmp_timespec_to_nanoseconds(const struct timespec *time_value, long long *nanoseconds_out)
{
    __int128 seconds_component;
    __int128 total_nanoseconds;

    if (time_value == ft_nullptr || nanoseconds_out == ft_nullptr)
    {
        cmp_set_last_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
    seconds_component = static_cast<__int128>(time_value->tv_sec);
    seconds_component *= static_cast<__int128>(1000000000LL);
    total_nanoseconds = seconds_component;
    total_nanoseconds += static_cast<__int128>(time_value->tv_nsec);
    if (total_nanoseconds > static_cast<__int128>(LLONG_MAX))
    {
        cmp_set_last_error(FT_ERR_OUT_OF_RANGE);
        return (-1);
    }
    if (total_nanoseconds < static_cast<__int128>(LLONG_MIN))
    {
        cmp_set_last_error(FT_ERR_OUT_OF_RANGE);
        return (-1);
    }
    *nanoseconds_out = static_cast<long long>(total_nanoseconds);
    cmp_set_last_error(FT_ERR_SUCCESSS);
    return (0);
}

int cmp_high_resolution_time(long long *nanoseconds_out)
{
    if (nanoseconds_out == ft_nullptr)
    {
        cmp_set_last_error(FT_ERR_INVALID_ARGUMENT);
        return (-1);
    }
#if defined(_WIN32) || defined(_WIN64)
    LARGE_INTEGER performance_counter;
    LARGE_INTEGER performance_frequency;
    long double scaled_value;

    if (!QueryPerformanceCounter(&performance_counter))
    {
        cmp_set_last_error(ft_map_system_error(GetLastError()));
        return (-1);
    }
    if (!QueryPerformanceFrequency(&performance_frequency))
    {
        cmp_set_last_error(ft_map_system_error(GetLastError()));
        return (-1);
    }
    if (performance_frequency.QuadPart <= 0)
    {
        cmp_set_last_error(FT_ERR_INVALID_OPERATION);
        return (-1);
    }
    scaled_value = static_cast<long double>(performance_counter.QuadPart);
    scaled_value *= 1000000000.0L;
    scaled_value /= static_cast<long double>(performance_frequency.QuadPart);
    if (scaled_value >= static_cast<long double>(LLONG_MAX))
    {
        cmp_set_last_error(FT_ERR_OUT_OF_RANGE);
        return (-1);
    }
    if (scaled_value <= static_cast<long double>(LLONG_MIN))
    {
        cmp_set_last_error(FT_ERR_OUT_OF_RANGE);
        return (-1);
    }
    *nanoseconds_out = static_cast<long long>(scaled_value);
    cmp_set_last_error(FT_ERR_SUCCESSS);
    return (0);
#else
    struct timespec time_value;
    int call_result;

# if defined(CLOCK_MONOTONIC_RAW)
    clockid_t clock_identifier;
    int attempt_index;

    attempt_index = 0;
    call_result = -1;
    while (attempt_index < 2)
    {
        if (attempt_index == 0)
            clock_identifier = CLOCK_MONOTONIC_RAW;
        else
            clock_identifier = CLOCK_MONOTONIC;
        call_result = clock_gettime(clock_identifier, &time_value);
        if (call_result == 0)
            break;
        if (errno != EINVAL)
        {
            cmp_set_last_error(ft_map_system_error(errno));
            return (-1);
        }
        attempt_index += 1;
    }
    if (call_result != 0)
    {
        cmp_set_last_error(FT_ERR_INVALID_OPERATION);
        return (-1);
    }
# else
    if (clock_gettime(CLOCK_MONOTONIC, &time_value) != 0)
    {
        cmp_set_last_error(ft_map_system_error(errno));
        return (-1);
    }
# endif
    if (cmp_timespec_to_nanoseconds(&time_value, nanoseconds_out) != 0)
        return (-1);
    cmp_set_last_error(FT_ERR_SUCCESSS);
    return (0);
#endif
}
