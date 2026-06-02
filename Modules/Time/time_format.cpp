#include "time.hpp"
#include "../Basic/class_nullptr.hpp"
#include "../CPP_class/class_string.hpp"
#include "../Errno/errno.hpp"
#include "../Basic/basic.hpp"
#include "../CMA/CMA.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread_internal.hpp"
#include "../PThread/pthread.hpp"
#include <ctime>
#include <cstdio>
#include <limits>
#include "../Basic/limits.hpp"
#include "../PThread/recursive_mutex.hpp"

static ft_string *time_format_failure(int32_t error_code) noexcept
{
    (void)(error_code);
    return (ft_nullptr);
}

static void time_format_delete_string(ft_string *string) noexcept
{
    if (string == ft_nullptr)
        return ;
    (void)string->destroy();
    delete string;
    return ;
}

static pt_mutex *g_time_format_gmtime_mutex = ft_nullptr;

static ft_bool time_format_append_literal(ft_string *string, const char *literal)
{
    if (!string || !literal)
        return (FT_FALSE);
    if (string->append(literal) != FT_ERR_SUCCESS)
        return (FT_FALSE);
    return (FT_TRUE);
}

static ft_bool time_format_append_unsigned_decimal(ft_string *string, uint64_t value)
{
    char reversed_digits[32];
    ft_size_t digit_count;

    if (!string)
        return (FT_FALSE);
    digit_count = 0;
    if (value == 0)
        reversed_digits[digit_count++] = '0';
    while (value > 0)
    {
        if (digit_count >= sizeof(reversed_digits))
            return (FT_FALSE);
        reversed_digits[digit_count] = static_cast<char>('0' + (value % UINT64_C(10)));
        value /= UINT64_C(10);
        digit_count++;
    }
    while (digit_count > 0)
    {
        digit_count--;
        if (string->append(reversed_digits[digit_count]) != FT_ERR_SUCCESS)
            return (FT_FALSE);
    }
    return (FT_TRUE);
}

ft_size_t  time_format_timezone_offset(char *buffer, ft_size_t size, int32_t offset_minutes)
{
    char sign_character;
    int64_t absolute_offset;
    int32_t offset_hours;
    int32_t offset_minutes_part;
    int32_t snprintf_result;

    if (!buffer || size == 0)
    {
        (void)(FT_ERR_INVALID_ARGUMENT);
        return (0);
    }
    if (offset_minutes >= 0)
        sign_character = '+';
    else
        sign_character = '-';
    absolute_offset = static_cast<int64_t>(offset_minutes);
    if (absolute_offset < 0)
        absolute_offset = -absolute_offset;
    offset_hours = static_cast<int32_t>(absolute_offset / 60LL);
    offset_minutes_part = static_cast<int32_t>(absolute_offset % 60LL);
    snprintf_result = std::snprintf(buffer, size, "%c%02d:%02d",
            sign_character, offset_hours, offset_minutes_part);
    if (snprintf_result < 0 || snprintf_result >= static_cast<int32_t>(size))
    {
        (void)(FT_ERR_OUT_OF_RANGE);
        return (0);
    }
    (void)(FT_ERR_SUCCESS);
    return (static_cast<ft_size_t>(snprintf_result));
}

#ifdef LIBFT_TEST_BUILD
static void time_format_untrack_runtime_leaks(void)
{
    if (g_time_format_gmtime_mutex != ft_nullptr)
        (void)cma_untrack_leak(g_time_format_gmtime_mutex);
    if (g_time_format_gmtime_mutex != ft_nullptr
        && g_time_format_gmtime_mutex->_native_mutex != ft_nullptr)
        (void)cma_untrack_leak(g_time_format_gmtime_mutex->_native_mutex);
    return ;
}
#endif

static int32_t time_format_lock_gmtime_mutex(ft_bool *lock_acquired)
{
    pt_mutex *mutex_pointer;
    int32_t initialise_error;
    int32_t lock_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = FT_FALSE;
    if (g_time_format_gmtime_mutex == ft_nullptr)
    {
        mutex_pointer = new (std::nothrow) pt_mutex();
        if (mutex_pointer == ft_nullptr)
            return (FT_ERR_NO_MEMORY);
        initialise_error = mutex_pointer->initialize();
        if (initialise_error != FT_ERR_SUCCESS)
        {
            delete mutex_pointer;
            return (initialise_error);
        }
        g_time_format_gmtime_mutex = mutex_pointer;
#ifdef LIBFT_TEST_BUILD
        time_format_untrack_runtime_leaks();
#endif
    }
    lock_error = pt_mutex_lock_if_not_null(g_time_format_gmtime_mutex);
    if (lock_error != FT_ERR_SUCCESS)
        return (lock_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = FT_TRUE;
    return (FT_ERR_SUCCESS);
}

static void time_format_unlock_gmtime_mutex(ft_bool lock_acquired)
{
    if (lock_acquired == FT_FALSE)
        return ;
    (void)pt_mutex_unlock_if_not_null(g_time_format_gmtime_mutex);
    return ;
}

ft_string    *time_format_iso8601(t_time time_value)
{
    std::time_t standard_time;
    std::tm time_storage;
    std::tm *time_pointer;
    char buffer[21];
    ft_string *formatted;
    ft_bool lock_acquired;
    int32_t lock_error;
    ft_size_t strftime_result;

    standard_time = time_value;
    lock_acquired = FT_FALSE;
    lock_error = time_format_lock_gmtime_mutex(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (time_format_failure(lock_error));
    time_pointer = std::gmtime(&standard_time);
    if (!time_pointer)
    {
        time_format_unlock_gmtime_mutex(lock_acquired);
        return (time_format_failure(FT_ERR_INVALID_ARGUMENT));
    }
    time_storage = *time_pointer;
    time_format_unlock_gmtime_mutex(lock_acquired);
    strftime_result = std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", &time_storage);
    if (strftime_result == 0)
    {
        return (time_format_failure(FT_ERR_INVALID_ARGUMENT));
    }
    formatted = new (std::nothrow) ft_string();
    if (formatted == ft_nullptr)
        return (time_format_failure(FT_ERR_NO_MEMORY));
    if (formatted->initialize(buffer) != FT_ERR_SUCCESS)
    {
        time_format_delete_string(formatted);
        return (time_format_failure(FT_ERR_NO_MEMORY));
    }
    (void)(FT_ERR_SUCCESS);
    return (formatted);
}

ft_string    *time_format_iso8601_with_offset(t_time time_value, int32_t offset_minutes)
{
    std::tm *time_pointer;
    std::tm time_storage;
    char buffer[20];
    char offset_buffer[7];
    ft_string *formatted;
    ft_bool lock_acquired;
    int32_t lock_error;
    ft_size_t strftime_result;
    int64_t offset_seconds;
    int64_t adjusted_epoch;
    std::time_t adjusted_time;

    offset_seconds = static_cast<int64_t>(offset_minutes) * 60LL;
    adjusted_epoch = time_value + offset_seconds;
    if (adjusted_epoch > std::numeric_limits<std::time_t>::max())
        return (time_format_failure(FT_ERR_OUT_OF_RANGE));
    if (adjusted_epoch < std::numeric_limits<std::time_t>::min())
        return (time_format_failure(FT_ERR_OUT_OF_RANGE));
    adjusted_time = adjusted_epoch;
    lock_acquired = FT_FALSE;
    lock_error = time_format_lock_gmtime_mutex(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (time_format_failure(lock_error));
    time_pointer = std::gmtime(&adjusted_time);
    if (!time_pointer)
    {
        time_format_unlock_gmtime_mutex(lock_acquired);
        return (time_format_failure(FT_ERR_INVALID_ARGUMENT));
    }
    time_storage = *time_pointer;
    time_format_unlock_gmtime_mutex(lock_acquired);
    strftime_result = std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S", &time_storage);
    if (strftime_result == 0)
    {
        return (time_format_failure(FT_ERR_INVALID_ARGUMENT));
    }
    if (time_format_timezone_offset(offset_buffer, sizeof(offset_buffer), offset_minutes) == 0)
        return (time_format_failure(FT_ERR_INTERNAL));
    formatted = new (std::nothrow) ft_string();
    if (formatted == ft_nullptr)
        return (time_format_failure(FT_ERR_NO_MEMORY));
    if (formatted->initialize(buffer) != FT_ERR_SUCCESS)
    {
        time_format_delete_string(formatted);
        return (time_format_failure(FT_ERR_NO_MEMORY));
    }
    *formatted += offset_buffer;
    if (formatted->get_error() != FT_ERR_SUCCESS)
    {
        time_format_delete_string(formatted);
        return (time_format_failure(FT_ERR_NO_MEMORY));
    }
    (void)(FT_ERR_SUCCESS);
    return (formatted);
}

ft_string    *time_format_rfc3339(t_time time_value)
{
    std::time_t standard_time;
    std::tm time_storage;
    std::tm *time_pointer;
    char buffer[21];
    ft_string *formatted;
    ft_bool lock_acquired;
    int32_t lock_error;
    ft_size_t strftime_result;
    ft_size_t buffer_length;

    standard_time = time_value;
    lock_acquired = FT_FALSE;
    lock_error = time_format_lock_gmtime_mutex(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (time_format_failure(lock_error));
    time_pointer = std::gmtime(&standard_time);
    if (!time_pointer)
    {
        time_format_unlock_gmtime_mutex(lock_acquired);
        return (time_format_failure(FT_ERR_INVALID_ARGUMENT));
    }
    time_storage = *time_pointer;
    time_format_unlock_gmtime_mutex(lock_acquired);
    strftime_result = std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S", &time_storage);
    if (strftime_result == 0)
        return (time_format_failure(FT_ERR_INVALID_ARGUMENT));
    buffer_length = ft_strlen(buffer);
    if (buffer_length + 2 > sizeof(buffer))
        return (time_format_failure(FT_ERR_OUT_OF_RANGE));
    buffer[buffer_length] = 'Z';
    buffer[buffer_length + 1] = '\0';
    formatted = new (std::nothrow) ft_string();
    if (formatted == ft_nullptr)
        return (time_format_failure(FT_ERR_NO_MEMORY));
    if (formatted->initialize(buffer) != FT_ERR_SUCCESS)
    {
        time_format_delete_string(formatted);
        return (time_format_failure(FT_ERR_NO_MEMORY));
    }
    (void)(FT_ERR_SUCCESS);
    return (formatted);
}

ft_string    *time_format_rfc3339_with_offset(t_time time_value, int32_t offset_minutes)
{
    if (offset_minutes == 0)
        return (time_format_rfc3339(time_value));
    return (time_format_iso8601_with_offset(time_value, offset_minutes));
}

ft_string    *time_format_duration(t_duration_milliseconds duration_value)
{
    ft_string *formatted;
    uint64_t magnitude;
    uint64_t days;
    uint64_t hours;
    uint64_t minutes;
    uint64_t seconds;
    uint64_t milliseconds;
    ft_bool is_negative;

    is_negative = FT_FALSE;
    if (duration_value.milliseconds < 0)
        is_negative = FT_TRUE;
    if (duration_value.milliseconds == std::numeric_limits<int64_t>::min())
        magnitude = static_cast<uint64_t>(std::numeric_limits<int64_t>::max()) + UINT64_C(1);
    else if (is_negative)
        magnitude = static_cast<uint64_t>(-duration_value.milliseconds);
    else
        magnitude = static_cast<uint64_t>(duration_value.milliseconds);
    days = magnitude / (UINT64_C(24) * UINT64_C(60) * UINT64_C(60) * UINT64_C(1000));
    magnitude %= (UINT64_C(24) * UINT64_C(60) * UINT64_C(60) * UINT64_C(1000));
    hours = magnitude / (UINT64_C(60) * UINT64_C(60) * UINT64_C(1000));
    magnitude %= (UINT64_C(60) * UINT64_C(60) * UINT64_C(1000));
    minutes = magnitude / (UINT64_C(60) * UINT64_C(1000));
    magnitude %= (UINT64_C(60) * UINT64_C(1000));
    seconds = magnitude / UINT64_C(1000);
    milliseconds = magnitude % UINT64_C(1000);
    formatted = new (std::nothrow) ft_string();
    if (formatted == ft_nullptr)
        return (ft_nullptr);
    if (formatted->initialize() != FT_ERR_SUCCESS)
    {
        delete formatted;
        return (ft_nullptr);
    }
    if (is_negative && !time_format_append_literal(formatted, "-"))
    {
        delete formatted;
        return (ft_nullptr);
    }
    if (!time_format_append_literal(formatted, "P"))
    {
        delete formatted;
        return (ft_nullptr);
    }
    if (days > 0 && (!time_format_append_unsigned_decimal(formatted, days)
            || !time_format_append_literal(formatted, "D")))
    {
        delete formatted;
        return (ft_nullptr);
    }
    if (hours > 0 || minutes > 0 || seconds > 0 || milliseconds > 0 || days == 0)
    {
        if (!time_format_append_literal(formatted, "T"))
        {
            delete formatted;
            return (ft_nullptr);
        }
        if (hours > 0)
        {
            if (!time_format_append_unsigned_decimal(formatted, hours)
                || !time_format_append_literal(formatted, "H"))
            {
                delete formatted;
                return (ft_nullptr);
            }
        }
        if (minutes > 0)
        {
            if (!time_format_append_unsigned_decimal(formatted, minutes)
                || !time_format_append_literal(formatted, "M"))
            {
                delete formatted;
                return (ft_nullptr);
            }
        }
        if (seconds > 0 || milliseconds > 0 || (days == 0 && hours == 0 && minutes == 0))
        {
            if (!time_format_append_unsigned_decimal(formatted, seconds))
            {
                delete formatted;
                return (ft_nullptr);
            }
            if (milliseconds > 0)
            {
                if (!time_format_append_literal(formatted, "."))
                {
                    delete formatted;
                    return (ft_nullptr);
                }
        if (milliseconds < UINT64_C(100) && !time_format_append_literal(formatted, "0"))
                {
                    delete formatted;
                    return (ft_nullptr);
                }
                if (milliseconds < UINT64_C(10) && !time_format_append_literal(formatted, "0"))
                {
                    delete formatted;
                    return (ft_nullptr);
                }
                if (!time_format_append_unsigned_decimal(formatted, milliseconds)
                    || !time_format_append_literal(formatted, "S"))
                {
                    delete formatted;
                    return (ft_nullptr);
                }
            }
            else if (!time_format_append_literal(formatted, "S"))
            {
                delete formatted;
                return (ft_nullptr);
            }
        }
    }
    (void)(FT_ERR_SUCCESS);
    return (formatted);
}
