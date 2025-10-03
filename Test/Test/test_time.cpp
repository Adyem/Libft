#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Compatebility/compatebility_internal.hpp"
#include "../../CPP_class/class_string_class.hpp"
#include "../../Time/time.hpp"
#include "../../Time/fps.hpp"
#include "../../Errno/errno.hpp"
#include "../../PThread/mutex.hpp"
#include <chrono>
#include <cstdint>
#include <ctime>
#include <cstring>
#include <cerrno>
#include <climits>
#include <vector>
#include <sys/time.h>
#if defined(__unix__) || defined(__APPLE__)
# include <sys/syscall.h>
# include <unistd.h>
#endif

static bool g_time_override_enabled = false;
static timeval g_time_override_value = {0, 0};
static bool g_time_force_failure = false;
static int g_time_failure_errno = EINVAL;
static bool g_time_sleep_capture_enabled = false;
static std::vector<unsigned int> g_time_sleep_calls;
static bool g_time_local_force_failure = false;
static int g_time_local_failure_errno = EINVAL;
static bool g_time_now_force_failure = false;
static int g_time_now_failure_errno = EINVAL;

void time_now_set_force_failure(bool enable_failure, int error_code)
{
    g_time_now_force_failure = enable_failure;
    g_time_now_failure_errno = error_code;
    return ;
}

extern t_time_format_gmtime_override_function g_time_format_gmtime_override;
extern t_time_format_strftime_override_function g_time_format_strftime_override;
extern t_time_format_mutex_override_function g_time_format_lock_override;
extern t_time_format_mutex_override_function g_time_format_unlock_override;

static void time_format_set_mutex_overrides(t_time_format_mutex_override_function lock_override, t_time_format_mutex_override_function unlock_override)
{
    g_time_format_lock_override = lock_override;
    g_time_format_unlock_override = unlock_override;
    return ;
}

static void time_format_set_time_overrides(t_time_format_gmtime_override_function gmtime_override, t_time_format_strftime_override_function strftime_override)
{
    g_time_format_gmtime_override = gmtime_override;
    g_time_format_strftime_override = strftime_override;
    return ;
}

static void time_format_reset_overrides(void)
{
    g_time_format_lock_override = ft_nullptr;
    g_time_format_unlock_override = ft_nullptr;
    g_time_format_gmtime_override = ft_nullptr;
    g_time_format_strftime_override = ft_nullptr;
    return ;
}

static int time_format_test_force_lock_failure(pt_mutex *mutex, pthread_t thread_id)
{
    (void)mutex;
    (void)thread_id;
    ft_errno = PT_ERR_ALREADY_LOCKED;
    return (PT_ERR_ALREADY_LOCKED);
}

static std::tm *time_format_test_force_gmtime_failure(const std::time_t *time_value)
{
    (void)time_value;
    return (ft_nullptr);
}

static int time_format_test_force_unlock_failure(pt_mutex *mutex, pthread_t thread_id)
{
    int unlock_result;

    time_format_set_mutex_overrides(ft_nullptr, ft_nullptr);
    unlock_result = mutex->unlock(thread_id);
    if (unlock_result != FT_SUCCESS && ft_errno == ER_SUCCESS)
        ft_errno = PT_ERR_MUTEX_OWNER;
    if (unlock_result == FT_SUCCESS)
        ft_errno = PT_ERR_MUTEX_OWNER;
    return (PT_ERR_MUTEX_OWNER);
}

static size_t time_format_test_force_strftime_failure(char *buffer, size_t size, const char *format, const std::tm *time_value)
{
    (void)buffer;
    (void)size;
    (void)format;
    (void)time_value;
    ft_errno = FT_EINVAL;
    return (0);
}

int pt_thread_sleep(unsigned int milliseconds)
{
    if (g_time_sleep_capture_enabled)
    {
        g_time_sleep_calls.push_back(milliseconds);
        return (0);
    }
    return (cmp_thread_sleep(milliseconds));
}

static int fallback_gettimeofday(struct timeval *time_value)
{
    std::chrono::system_clock::time_point current_time = std::chrono::system_clock::now();
    std::chrono::microseconds microseconds_since_epoch = std::chrono::duration_cast<std::chrono::microseconds>(current_time.time_since_epoch());
    time_value->tv_sec = static_cast<time_t>(microseconds_since_epoch.count() / 1000000);
    time_value->tv_usec = static_cast<suseconds_t>(microseconds_since_epoch.count() % 1000000);
    return (0);
}

extern "C" int gettimeofday(struct timeval *time_value, void *timezone_pointer)
{
    if (g_time_force_failure)
    {
        errno = g_time_failure_errno;
        return (-1);
    }
    if (g_time_override_enabled)
    {
        *time_value = g_time_override_value;
        return (0);
    }
#if defined(__unix__) || defined(__APPLE__)
# if defined(SYS_gettimeofday)
    long syscall_result = syscall(SYS_gettimeofday, time_value, timezone_pointer);
    if (syscall_result == 0)
        return (0);
    return (fallback_gettimeofday(time_value));
# else
    (void)timezone_pointer;
    return (fallback_gettimeofday(time_value));
# endif
#else
    (void)timezone_pointer;
    return (fallback_gettimeofday(time_value));
#endif
}

extern "C" std::time_t time(std::time_t *time_pointer)
{
    std::time_t current_time;

    if (g_time_now_force_failure)
    {
        errno = g_time_now_failure_errno;
        if (time_pointer)
            *time_pointer = static_cast<std::time_t>(-1);
        return (static_cast<std::time_t>(-1));
    }
    current_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    if (time_pointer)
        *time_pointer = current_time;
    return (current_time);
}

#if defined(_WIN32) || defined(_WIN64)
extern "C" errno_t localtime_s(std::tm *output, const std::time_t *time_value)
{
    std::tm *temporary;

    if (g_time_local_force_failure)
    {
        errno = g_time_local_failure_errno;
        return (g_time_local_failure_errno);
    }
    temporary = std::localtime(time_value);
    if (!temporary)
    {
        errno = EINVAL;
        return (EINVAL);
    }
    *output = *temporary;
    return (0);
}
#elif defined(_POSIX_VERSION)
extern "C" std::tm *localtime_r(const std::time_t *time_value, std::tm *output)
{
    std::tm *temporary;

    if (g_time_local_force_failure)
    {
        errno = g_time_local_failure_errno;
        return (ft_nullptr);
    }
    temporary = std::localtime(time_value);
    if (!temporary)
    {
        return (ft_nullptr);
    }
    *output = *temporary;
    return (output);
}
#endif

FT_TEST(test_time_ms_basic, "ft_time_ms increasing")
{
    int64_t first;
    int64_t second;

    first = ft_time_ms();
    second = ft_time_ms();
    FT_ASSERT(first >= 0);
    FT_ASSERT(second >= first);
    return (1);
}

FT_TEST(test_time_ms_large_values, "ft_time_ms handles large timeval values without overflow")
{
    int64_t milliseconds_result;
    int64_t expected_result;

    g_time_override_enabled = true;
    g_time_override_value.tv_sec = static_cast<time_t>(INT64_C(1) << 40);
    g_time_override_value.tv_usec = 567000;
    expected_result = static_cast<int64_t>(g_time_override_value.tv_sec) * 1000;
    expected_result += static_cast<int64_t>(g_time_override_value.tv_usec) / 1000;
    milliseconds_result = ft_time_ms();
    g_time_override_enabled = false;
    FT_ASSERT_EQ(expected_result, milliseconds_result);
    return (1);
}

FT_TEST(test_time_format_basic, "ft_time_format basic")
{
    char buffer[32];

    FT_ASSERT(ft_time_format(buffer, sizeof(buffer)) != ft_nullptr);
    return (1);
}

FT_TEST(test_time_format_errors, "ft_time_format edge cases")
{
    char buffer[32];

    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(ft_nullptr, ft_time_format(ft_nullptr, sizeof(buffer)));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(ft_nullptr, ft_time_format(buffer, 0));
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_cmp_timegm_null_pointer_sets_errno, "cmp_timegm validates null pointers")
{
    std::time_t conversion_result;

    ft_errno = ER_SUCCESS;
    conversion_result = cmp_timegm(ft_nullptr);
    FT_ASSERT_EQ(static_cast<std::time_t>(-1), conversion_result);
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_time_format_small_buffer, "ft_time_format detects insufficient space")
{
    char buffer[1];

    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(ft_nullptr, ft_time_format(buffer, sizeof(buffer)));
    FT_ASSERT_EQ(FT_ERANGE, ft_errno);
    return (1);
}

FT_TEST(test_time_format_time_local_failure, "ft_time_format propagates time_local errors")
{
    char buffer[32];

    std::memset(buffer, 'Z', sizeof(buffer));
    g_time_local_force_failure = true;
    g_time_local_failure_errno = EOVERFLOW;
    ft_errno = ER_SUCCESS;
    FT_ASSERT_EQ(ft_nullptr, ft_time_format(buffer, sizeof(buffer)));
    FT_ASSERT_EQ(EOVERFLOW + ERRNO_OFFSET, ft_errno);
    FT_ASSERT_EQ('Z', buffer[0]);
    g_time_local_force_failure = false;
    return (1);
}

FT_TEST(test_time_format_iso8601_success_sets_errno, "time_format_iso8601 reports success in errno")
{
    ft_string formatted;

    time_format_reset_overrides();
    ft_errno = FT_EINVAL;
    formatted = time_format_iso8601(0);
    FT_ASSERT_EQ(0, std::strcmp(formatted.c_str(), "1970-01-01T00:00:00Z"));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_time_format_iso8601_mutex_lock_failure_sets_errno, "time_format_iso8601 propagates mutex lock errors")
{
    ft_string formatted;

    time_format_reset_overrides();
    time_format_set_mutex_overrides(time_format_test_force_lock_failure, ft_nullptr);
    ft_errno = ER_SUCCESS;
    formatted = time_format_iso8601(0);
    time_format_reset_overrides();
    FT_ASSERT(formatted.empty());
    FT_ASSERT_EQ(PT_ERR_ALREADY_LOCKED, ft_errno);
    return (1);
}

FT_TEST(test_time_format_iso8601_gmtime_failure_sets_errno, "time_format_iso8601 sets errno when gmtime fails")
{
    ft_string formatted;

    time_format_reset_overrides();
    time_format_set_time_overrides(time_format_test_force_gmtime_failure, ft_nullptr);
    ft_errno = ER_SUCCESS;
    formatted = time_format_iso8601(0);
    time_format_reset_overrides();
    FT_ASSERT(formatted.empty());
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_time_format_iso8601_gmtime_unlock_failure_sets_errno, "time_format_iso8601 propagates mutex unlock errors after gmtime failure")
{
    ft_string formatted;

    time_format_reset_overrides();
    time_format_set_mutex_overrides(ft_nullptr, time_format_test_force_unlock_failure);
    time_format_set_time_overrides(time_format_test_force_gmtime_failure, ft_nullptr);
    ft_errno = ER_SUCCESS;
    formatted = time_format_iso8601(0);
    time_format_reset_overrides();
    FT_ASSERT(formatted.empty());
    FT_ASSERT_EQ(PT_ERR_MUTEX_OWNER, ft_errno);
    return (1);
}

FT_TEST(test_time_format_iso8601_unlock_failure_sets_errno, "time_format_iso8601 propagates mutex unlock errors")
{
    ft_string formatted;

    time_format_reset_overrides();
    time_format_set_mutex_overrides(ft_nullptr, time_format_test_force_unlock_failure);
    ft_errno = ER_SUCCESS;
    formatted = time_format_iso8601(0);
    time_format_reset_overrides();
    FT_ASSERT(formatted.empty());
    FT_ASSERT_EQ(PT_ERR_MUTEX_OWNER, ft_errno);
    return (1);
}

FT_TEST(test_time_format_iso8601_strftime_failure_sets_errno, "time_format_iso8601 sets errno when strftime fails")
{
    ft_string formatted;

    time_format_reset_overrides();
    time_format_set_time_overrides(ft_nullptr, time_format_test_force_strftime_failure);
    ft_errno = ER_SUCCESS;
    formatted = time_format_iso8601(0);
    time_format_reset_overrides();
    FT_ASSERT(formatted.empty());
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_time_ms_failure, "ft_time_ms reports gettimeofday failure")
{
    int64_t result;

    g_time_force_failure = true;
    g_time_failure_errno = EINVAL;
    ft_errno = ER_SUCCESS;
    result = ft_time_ms();
    g_time_force_failure = false;
    FT_ASSERT_EQ(-1, result);
    FT_ASSERT_EQ(EINVAL + ERRNO_OFFSET, ft_errno);
    return (1);
}

FT_TEST(test_time_ms_failure_without_errno, "ft_time_ms falls back to FT_ETERM when errno missing")
{
    int64_t result;

    g_time_force_failure = true;
    g_time_failure_errno = 0;
    errno = 0;
    ft_errno = ER_SUCCESS;
    result = ft_time_ms();
    g_time_force_failure = false;
    FT_ASSERT_EQ(-1, result);
    FT_ASSERT_EQ(FT_ETERM, ft_errno);
    return (1);
}

FT_TEST(test_time_now_failure_sets_errno_offset, "time_now reports errno offset on failure")
{
    t_time time_result;

    g_time_now_force_failure = true;
    g_time_now_failure_errno = EINVAL;
    errno = 0;
    ft_errno = ER_SUCCESS;
    time_result = time_now();
    g_time_now_force_failure = false;
    FT_ASSERT_EQ(static_cast<t_time>(-1), time_result);
    FT_ASSERT_EQ(EINVAL + ERRNO_OFFSET, ft_errno);
    return (1);
}

FT_TEST(test_time_parse_iso8601_timezone_independent, "time_parse_iso8601 handles UTC Z regardless of TZ")
{
    char *original_timezone;
    ft_string timezone_backup;
    std::tm local_epoch_input;
    std::time_t local_epoch_result;
    std::tm parsed_time;
    t_time parsed_epoch;
    int success;
    bool parse_result;

    success = 0;
    original_timezone = ft_getenv("TZ");
    if (original_timezone != ft_nullptr)
        timezone_backup = original_timezone;
    if (ft_setenv("TZ", "GMT+3", 1) != 0)
    {
        ft_test_fail("ft_setenv(\"TZ\", \"GMT+3\", 1) == 0", __FILE__, __LINE__);
        goto cleanup;
    }
#if defined(_WIN32) || defined(_WIN64)
    _tzset();
#else
    tzset();
#endif
    std::memset(&local_epoch_input, 0, sizeof(local_epoch_input));
    local_epoch_input.tm_year = 70;
    local_epoch_input.tm_mday = 1;
    local_epoch_input.tm_isdst = 0;
    local_epoch_result = std::mktime(&local_epoch_input);
    if (local_epoch_result == static_cast<std::time_t>(-1))
    {
        ft_test_fail("local_epoch_result != (time_t)-1", __FILE__, __LINE__);
        goto cleanup;
    }
    if (local_epoch_result == 0)
    {
        if (ft_setenv("TZ", "GMT-3", 1) != 0)
        {
            ft_test_fail("ft_setenv(\"TZ\", \"GMT-3\", 1) == 0", __FILE__, __LINE__);
            goto cleanup;
        }
#if defined(_WIN32) || defined(_WIN64)
        _tzset();
#else
        tzset();
#endif
        std::memset(&local_epoch_input, 0, sizeof(local_epoch_input));
        local_epoch_input.tm_year = 70;
        local_epoch_input.tm_mday = 1;
        local_epoch_input.tm_isdst = 0;
        local_epoch_result = std::mktime(&local_epoch_input);
        if (local_epoch_result == static_cast<std::time_t>(-1))
        {
            ft_test_fail("local_epoch_result != (time_t)-1", __FILE__, __LINE__);
            goto cleanup;
        }
    }
    if (local_epoch_result == 0)
    {
        ft_test_fail("local_epoch_result != 0", __FILE__, __LINE__);
        goto cleanup;
    }
    parse_result = time_parse_iso8601("1970-01-01T00:00:00Z", &parsed_time, &parsed_epoch);
    if (!parse_result)
    {
        ft_test_fail("parse_result", __FILE__, __LINE__);
        goto cleanup;
    }
    if (parsed_epoch != 0)
    {
        ft_test_fail("parsed_epoch == 0", __FILE__, __LINE__);
        goto cleanup;
    }
    success = 1;
cleanup:
    if (original_timezone != ft_nullptr)
        ft_setenv("TZ", timezone_backup.c_str(), 1);
    else
        ft_unsetenv("TZ");
#if defined(_WIN32) || defined(_WIN64)
    _tzset();
#else
    tzset();
#endif
    return (success);
}

FT_TEST(test_time_parse_iso8601_numeric_offset, "time_parse_iso8601 supports numeric timezone offsets")
{
    std::tm base_time;
    std::tm parsed_time;
    std::time_t base_epoch;
    std::time_t expected_epoch;
    std::time_t epoch_copy;
    std::tm *utc_time;
    t_time parsed_epoch;

    std::memset(&base_time, 0, sizeof(base_time));
    base_time.tm_year = 120;
    base_time.tm_mon = 4;
    base_time.tm_mday = 10;
    base_time.tm_hour = 12;
    base_time.tm_min = 30;
    base_time.tm_sec = 45;
    base_time.tm_isdst = 0;
    base_epoch = cmp_timegm(&base_time);
    FT_ASSERT(base_epoch != static_cast<std::time_t>(-1));
    ft_errno = ER_SUCCESS;
    FT_ASSERT(time_parse_iso8601("2020-05-10T12:30:45+02:30", &parsed_time, &parsed_epoch));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    expected_epoch = base_epoch - static_cast<std::time_t>((2 * 60 + 30) * 60);
    FT_ASSERT_EQ(static_cast<t_time>(expected_epoch), parsed_epoch);
    epoch_copy = static_cast<std::time_t>(parsed_epoch);
    utc_time = std::gmtime(&epoch_copy);
    FT_ASSERT(utc_time != ft_nullptr);
    FT_ASSERT_EQ(utc_time->tm_year, parsed_time.tm_year);
    FT_ASSERT_EQ(utc_time->tm_mon, parsed_time.tm_mon);
    FT_ASSERT_EQ(utc_time->tm_mday, parsed_time.tm_mday);
    FT_ASSERT_EQ(utc_time->tm_hour, parsed_time.tm_hour);
    FT_ASSERT_EQ(utc_time->tm_min, parsed_time.tm_min);
    FT_ASSERT_EQ(utc_time->tm_sec, parsed_time.tm_sec);
    return (1);
}

FT_TEST(test_time_parse_iso8601_rejects_invalid_offset_minutes, "time_parse_iso8601 validates timezone offsets")
{
    bool parse_result;

    ft_errno = ER_SUCCESS;
    parse_result = time_parse_iso8601("2020-05-10T12:30:45+02:99", ft_nullptr, ft_nullptr);
    FT_ASSERT_EQ(false, parse_result);
    FT_ASSERT_EQ(FT_ERANGE, ft_errno);
    return (1);
}

FT_TEST(test_time_parse_custom_interpret_as_utc, "time_parse_custom converts parsed values from UTC")
{
    std::tm parsed_time;
    t_time parsed_epoch;
    std::time_t epoch_copy;
    std::tm *utc_time;
    bool parse_result;

    ft_errno = ER_SUCCESS;
    parse_result = time_parse_custom("1970-01-01 00:00:00", "%Y-%m-%d %H:%M:%S", &parsed_time, &parsed_epoch, true);
    FT_ASSERT(parse_result);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(static_cast<t_time>(0), parsed_epoch);
    epoch_copy = static_cast<std::time_t>(parsed_epoch);
    utc_time = std::gmtime(&epoch_copy);
    FT_ASSERT(utc_time != ft_nullptr);
    FT_ASSERT_EQ(utc_time->tm_year, parsed_time.tm_year);
    FT_ASSERT_EQ(utc_time->tm_mon, parsed_time.tm_mon);
    FT_ASSERT_EQ(utc_time->tm_mday, parsed_time.tm_mday);
    FT_ASSERT_EQ(utc_time->tm_hour, parsed_time.tm_hour);
    FT_ASSERT_EQ(utc_time->tm_min, parsed_time.tm_min);
    FT_ASSERT_EQ(utc_time->tm_sec, parsed_time.tm_sec);
    return (1);
}

FT_TEST(test_time_local_null_output, "time_local rejects null destination")
{
    ft_errno = ER_SUCCESS;
    time_local(0, ft_nullptr);
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_time_local_matches_cmp_localtime, "time_local mirrors cmp_localtime output")
{
    std::time_t current_time;
    std::tm expected;
    t_time_info actual;

    current_time = std::time(ft_nullptr);
    if (cmp_localtime(&current_time, &expected) != 0)
    {
        ft_test_fail("cmp_localtime(&current_time, &expected) == 0", __FILE__, __LINE__);
        return (0);
    }
    ft_errno = FT_EINVAL;
    time_local(static_cast<t_time>(current_time), &actual);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(expected.tm_sec, actual.seconds);
    FT_ASSERT_EQ(expected.tm_min, actual.minutes);
    FT_ASSERT_EQ(expected.tm_hour, actual.hours);
    FT_ASSERT_EQ(expected.tm_mday, actual.month_day);
    FT_ASSERT_EQ(expected.tm_mon, actual.month);
    FT_ASSERT_EQ(expected.tm_year, actual.year);
    FT_ASSERT_EQ(expected.tm_wday, actual.week_day);
    FT_ASSERT_EQ(expected.tm_yday, actual.year_day);
    FT_ASSERT_EQ(expected.tm_isdst, actual.is_daylight_saving);
    return (1);
}

FT_TEST(test_time_sleep_handles_large_second_values, "time_sleep splits long delays into safe chunks")
{
    size_t call_count;

    g_time_sleep_calls.clear();
    g_time_sleep_capture_enabled = true;
    time_sleep(4294968U);
    g_time_sleep_capture_enabled = false;
    call_count = g_time_sleep_calls.size();
    FT_ASSERT_EQ(static_cast<size_t>(2), call_count);
    FT_ASSERT_EQ(static_cast<unsigned int>(UINT_MAX), g_time_sleep_calls[0]);
    FT_ASSERT_EQ(static_cast<unsigned int>(705), g_time_sleep_calls[1]);
    return (1);
}

FT_TEST(test_time_duration_ms_create_preserves_value, "time_duration_ms_create stores the provided milliseconds")
{
    t_duration_milliseconds duration;

    duration = time_duration_ms_create(123456789LL);
    FT_ASSERT_EQ(123456789LL, duration.milliseconds);
    return (1);
}

FT_TEST(test_time_duration_ms_create_accepts_negative_values, "time_duration_ms_create keeps negative inputs unchanged")
{
    t_duration_milliseconds duration;

    duration = time_duration_ms_create(-3500LL);
    FT_ASSERT_EQ(-3500LL, duration.milliseconds);
    return (1);
}

FT_TEST(test_time_fps_constructor_rejects_low_rates, "time_fps constructor enforces the minimum frame rate")
{
    time_fps controller(10);

    FT_ASSERT_EQ(FT_EINVAL, controller.get_error());
    FT_ASSERT_EQ(0L, controller.get_frames_per_second());
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    FT_ASSERT_EQ(FT_EINVAL, controller.get_error());
    return (1);
}

FT_TEST(test_time_fps_constructor_accepts_valid_rate, "time_fps stores valid frame rates and clears errno")
{
    time_fps controller(60);
    long frames_per_second;

    frames_per_second = controller.get_frames_per_second();
    FT_ASSERT_EQ(60L, frames_per_second);
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(ER_SUCCESS, controller.get_error());
    return (1);
}

FT_TEST(test_time_fps_set_frames_per_second_updates_rate, "time_fps set_frames_per_second updates pacing state")
{
    time_fps controller(30);
    int update_result;

    update_result = controller.set_frames_per_second(48);
    FT_ASSERT_EQ(0, update_result);
    FT_ASSERT_EQ(48L, controller.get_frames_per_second());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(ER_SUCCESS, controller.get_error());
    return (1);
}

FT_TEST(test_time_fps_set_frames_per_second_rejects_low_value, "time_fps set_frames_per_second validates inputs")
{
    time_fps controller(30);
    int update_result;

    update_result = controller.set_frames_per_second(15);
    FT_ASSERT_EQ(1, update_result);
    FT_ASSERT_EQ(FT_EINVAL, controller.get_error());
    FT_ASSERT_EQ(0L, controller.get_frames_per_second());
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    FT_ASSERT_EQ(FT_EINVAL, controller.get_error());
    return (1);
}

FT_TEST(test_time_fps_sleep_to_next_frame_uses_sleep_ms, "time_fps sleep_to_next_frame waits for the remaining frame time")
{
    time_fps controller(40);
    unsigned int sleep_duration;

    g_time_sleep_calls.clear();
    g_time_sleep_capture_enabled = true;
    controller.sleep_to_next_frame();
    controller.sleep_to_next_frame();
    g_time_sleep_capture_enabled = false;
    FT_ASSERT(!g_time_sleep_calls.empty());
    sleep_duration = g_time_sleep_calls.back();
    FT_ASSERT(sleep_duration > 0U);
    FT_ASSERT(sleep_duration <= 25U);
    FT_ASSERT_EQ(ER_SUCCESS, controller.get_error());
    return (1);
}

FT_TEST(test_time_fps_sleep_to_next_frame_handles_invalid_state, "time_fps sleep_to_next_frame reports errors when pacing is invalid")
{
    time_fps controller(10);

    g_time_sleep_calls.clear();
    g_time_sleep_capture_enabled = true;
    controller.sleep_to_next_frame();
    g_time_sleep_capture_enabled = false;
    FT_ASSERT(g_time_sleep_calls.empty());
    FT_ASSERT_EQ(FT_EINVAL, controller.get_error());
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}
