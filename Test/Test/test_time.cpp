#include "../../Time/time.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"
#include <climits>
#include <chrono>

static std::chrono::system_clock::time_point    g_mock_time_now;

static std::chrono::system_clock::time_point    test_time_now_hook(void)
{
    return (g_mock_time_now);
}

FT_TEST(test_time_now_ms_returns_hooked_value, "time_now_ms returns hooked millisecond value")
{
    long                        result;
    std::chrono::milliseconds   expected_duration(123456);

    g_mock_time_now = std::chrono::system_clock::time_point(expected_duration);
    time_set_clock_now_hook(test_time_now_hook);
    ft_errno = FT_ERR_OUT_OF_RANGE;
    result = time_now_ms();
    time_reset_clock_now_hook();
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(static_cast<long>(expected_duration.count()), result);
    return (1);
}

#if LONG_MAX < LLONG_MAX
FT_TEST(test_time_now_ms_detects_overflow, "time_now_ms detects overflow and reports FT_ERR_OUT_OF_RANGE")
{
    long                                        result;
    std::chrono::milliseconds::rep             overflow_count;

    overflow_count = static_cast<std::chrono::milliseconds::rep>(LONG_MAX) + 1;
    g_mock_time_now = std::chrono::system_clock::time_point(std::chrono::milliseconds(overflow_count));
    time_set_clock_now_hook(test_time_now_hook);
    ft_errno = ER_SUCCESS;
    result = time_now_ms();
    time_reset_clock_now_hook();
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, ft_errno);
    FT_ASSERT_EQ(LONG_MAX, result);
    return (1);
}
#endif
