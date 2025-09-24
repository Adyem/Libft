#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../CPP_class/class_string_class.hpp"
#include "../../Time/time.hpp"
#include <chrono>
#include <cstdint>
#include <ctime>
#include <cstring>
#include <sys/time.h>
#if defined(__unix__) || defined(__APPLE__)
# include <sys/syscall.h>
# include <unistd.h>
#endif

static bool g_time_override_enabled = false;
static timeval g_time_override_value = {0, 0};

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

    FT_ASSERT_EQ(ft_nullptr, ft_time_format(ft_nullptr, sizeof(buffer)));
    FT_ASSERT_EQ(ft_nullptr, ft_time_format(buffer, 0));
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
