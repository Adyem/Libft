#include "../../Time/timer.hpp"
#include "../../Time/time.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"
#include <climits>

FT_TEST(test_time_timer_start_rejects_negative_duration, "time_timer::start rejects negative durations")
{
    time_timer timer;
    long update_result;

    ft_errno = ER_SUCCESS;
    timer.start(-5);
    FT_ASSERT_EQ(FT_EINVAL, timer.get_error());
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);

    ft_errno = ER_SUCCESS;
    update_result = timer.update();
    FT_ASSERT_EQ(static_cast<long>(-1), update_result);
    FT_ASSERT_EQ(FT_EINVAL, timer.get_error());
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_time_timer_update_counts_down_to_completion, "time_timer::update tracks elapsed time and signals completion")
{
    time_timer timer;
    long remaining_before_wait;
    long update_after_completion;

    ft_errno = FT_EINVAL;
    timer.start(25);
    remaining_before_wait = timer.update();
    FT_ASSERT(remaining_before_wait >= 0);
    FT_ASSERT(remaining_before_wait <= 25);
    FT_ASSERT_EQ(ER_SUCCESS, timer.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);

    time_sleep_ms(40);
    FT_ASSERT_EQ(static_cast<long>(0), timer.update());
    FT_ASSERT_EQ(ER_SUCCESS, timer.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);

    ft_errno = ER_SUCCESS;
    update_after_completion = timer.update();
    FT_ASSERT_EQ(static_cast<long>(-1), update_after_completion);
    FT_ASSERT_EQ(FT_EINVAL, timer.get_error());
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_time_timer_add_time_extends_active_timer, "time_timer::add_time extends an active countdown")
{
    time_timer timer;
    long remaining_after_add;
    long remaining_midway;

    ft_errno = ER_SUCCESS;
    timer.start(30);
    remaining_after_add = timer.add_time(60);
    FT_ASSERT(remaining_after_add > 0);
    FT_ASSERT(remaining_after_add <= 90);
    FT_ASSERT_EQ(ER_SUCCESS, timer.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);

    time_sleep_ms(40);
    remaining_midway = timer.update();
    FT_ASSERT(remaining_midway > 0);
    FT_ASSERT_EQ(ER_SUCCESS, timer.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);

    time_sleep_ms(70);
    FT_ASSERT_EQ(static_cast<long>(0), timer.update());
    FT_ASSERT_EQ(ER_SUCCESS, timer.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_time_timer_add_time_detects_overflow, "time_timer::add_time detects overflow without stopping the timer")
{
    time_timer timer;
    long add_result;
    long remaining_after_failure;

    ft_errno = ER_SUCCESS;
    timer.start(LONG_MAX - 10);
    add_result = timer.add_time(100);
    FT_ASSERT_EQ(static_cast<long>(-1), add_result);
    FT_ASSERT_EQ(FT_ERANGE, timer.get_error());
    FT_ASSERT_EQ(FT_ERANGE, ft_errno);

    ft_errno = ER_SUCCESS;
    remaining_after_failure = timer.update();
    FT_ASSERT(remaining_after_failure > 0);
    FT_ASSERT_EQ(ER_SUCCESS, timer.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_time_timer_add_time_requires_running_timer, "time_timer::add_time rejects stopped timers")
{
    time_timer timer;
    long add_result;

    ft_errno = ER_SUCCESS;
    add_result = timer.add_time(5);
    FT_ASSERT_EQ(static_cast<long>(-1), add_result);
    FT_ASSERT_EQ(FT_EINVAL, timer.get_error());
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_time_timer_remove_time_can_force_expiration, "time_timer::remove_time can expire the timer immediately")
{
    time_timer timer;
    long update_result;

    ft_errno = ER_SUCCESS;
    timer.start(50);
    FT_ASSERT_EQ(static_cast<long>(0), timer.remove_time(100));
    FT_ASSERT_EQ(ER_SUCCESS, timer.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);

    ft_errno = ER_SUCCESS;
    update_result = timer.update();
    FT_ASSERT_EQ(static_cast<long>(-1), update_result);
    FT_ASSERT_EQ(FT_EINVAL, timer.get_error());
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_time_timer_remove_time_requires_running_timer, "time_timer::remove_time rejects stopped timers and negative adjustments")
{
    time_timer timer;
    long remove_result;

    ft_errno = ER_SUCCESS;
    remove_result = timer.remove_time(10);
    FT_ASSERT_EQ(static_cast<long>(-1), remove_result);
    FT_ASSERT_EQ(FT_EINVAL, timer.get_error());
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);

    timer.start(20);
    ft_errno = ER_SUCCESS;
    remove_result = timer.remove_time(-5);
    FT_ASSERT_EQ(static_cast<long>(-1), remove_result);
    FT_ASSERT_EQ(FT_EINVAL, timer.get_error());
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}

FT_TEST(test_time_timer_sleep_remaining_waits_until_finished, "time_timer::sleep_remaining blocks until the timer completes")
{
    time_timer timer;

    timer.start(25);
    ft_errno = FT_EINVAL;
    timer.sleep_remaining();
    FT_ASSERT_EQ(ER_SUCCESS, timer.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    FT_ASSERT_EQ(static_cast<long>(0), timer.update());
    FT_ASSERT_EQ(ER_SUCCESS, timer.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, ft_errno);
    return (1);
}

FT_TEST(test_time_timer_sleep_remaining_preserves_error_on_inactive_timer, "time_timer::sleep_remaining leaves errors when timer is inactive")
{
    time_timer timer;

    ft_errno = ER_SUCCESS;
    timer.sleep_remaining();
    FT_ASSERT_EQ(FT_EINVAL, timer.get_error());
    FT_ASSERT_EQ(FT_EINVAL, ft_errno);
    return (1);
}
