#include "../../System_utils/test_runner.hpp"
#include "../../Time/time.hpp"
#include "../../Errno/errno.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include <cerrno>
#include <chrono>
#include <ctime>

void time_now_set_force_failure(bool enable_failure, int error_code);

FT_TEST(test_time_now_success_resets_errno, "time_now success resets ft_errno")
{
    t_time time_value;

    time_now_set_force_failure(false, 0);
    errno = 0;
    ft_errno = FT_EINVAL;
    time_value = time_now();
    FT_ASSERT(time_value != static_cast<t_time>(-1));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_time_now_ms_success_resets_errno, "time_now_ms success resets ft_errno")
{
    long milliseconds_value;

    time_now_set_force_failure(false, 0);
    errno = 0;
    ft_errno = FT_EINVAL;
    milliseconds_value = time_now_ms();
    FT_ASSERT(milliseconds_value != static_cast<long>(-1));
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_time_now_failure_sets_errno, "time_now failure propagates errno")
{
    t_time time_value;
    int error_code;

    error_code = EACCES;
    time_now_set_force_failure(true, error_code);
    errno = 0;
    ft_errno = ER_SUCCESS;
    time_value = time_now();
    time_now_set_force_failure(false, 0);
    FT_ASSERT_EQ(static_cast<t_time>(-1), time_value);
    FT_ASSERT_EQ(error_code + ERRNO_OFFSET, ft_errno);
    return (1);
}
