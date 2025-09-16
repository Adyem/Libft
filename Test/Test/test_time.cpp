#include "../../Libft/libft.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../CPP_class/class_string_class.hpp"
#include "../../Time/time.hpp"
#include <ctime>
#include <cstring>

FT_TEST(test_time_ms_basic, "ft_time_ms increasing")
{
    long first;
    long second;

    first = ft_time_ms();
    second = ft_time_ms();
    FT_ASSERT(first >= 0);
    FT_ASSERT(second >= first);
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
