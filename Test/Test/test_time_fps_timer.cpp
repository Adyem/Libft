#include "../test_internal.hpp"
#include "../../Modules/Time/time_fps.hpp"
#include "../../Modules/Time/time_timer.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_time_fps_get_frames_sets_errno_success)
{
    time_fps    frames_per_second;
    long        value;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, frames_per_second.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, frames_per_second.set_frames_per_second(30));
    value = frames_per_second.get_frames_per_second();
    FT_ASSERT_EQ(30, value);
    return (1);
}

FT_TEST(test_time_fps_invalid_configuration_reports_error)
{
    time_fps    frames_per_second;
    int         result;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, frames_per_second.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, frames_per_second.set_frames_per_second(30));
    result = frames_per_second.set_frames_per_second(10);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, result);
    FT_ASSERT_EQ(0L, frames_per_second.get_frames_per_second());
    return (1);
}

FT_TEST(test_time_fps_sleep_invalid_state_sets_errno)
{
    time_fps    frames_per_second;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, frames_per_second.initialize());
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, frames_per_second.set_frames_per_second(10));
    frames_per_second.sleep_to_next_frame();
    FT_ASSERT_EQ(0L, frames_per_second.get_frames_per_second());
    return (1);
}

FT_TEST(test_time_timer_start_negative_sets_error)
{
    time_timer  timer;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, timer.initialize());
    timer.start(-5);
    FT_ASSERT_EQ(static_cast<long>(-1), timer.update());
    return (1);
}

FT_TEST(test_time_timer_zero_duration_finishes_immediately)
{
    time_timer  timer;
    long        remaining;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, timer.initialize());
    timer.start(0);
    remaining = timer.update();
    FT_ASSERT_EQ(0, remaining);
    return (1);
}

FT_TEST(test_time_timer_add_time_requires_running_timer)
{
    time_timer  timer;
    long        result;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, timer.initialize());
    result = timer.add_time(100);
    FT_ASSERT_EQ(-1, result);
    return (1);
}
