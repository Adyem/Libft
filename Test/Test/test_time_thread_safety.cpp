#include "../test_internal.hpp"
#include "../../Modules/Time/time.hpp"
#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

static t_time_info create_thread_safe_time_info(void)
{
    t_time_info info;

    info.mutex = ft_nullptr;
    info.thread_safe_enabled = FT_FALSE;
    info.seconds = 0;
    info.minutes = 0;
    info.hours = 0;
    info.month_day = 0;
    info.month = 0;
    info.year = 0;
    info.week_day = 0;
    info.year_day = 0;
    info.is_daylight_saving = 0;
    return (info);
}

FT_TEST(test_time_monotonic_point_enable_thread_safety_enables_mutex)
{
    t_monotonic_time_point time_point;

    time_point = time_monotonic_point_create(42);
    FT_ASSERT_EQ(FT_FALSE, time_monotonic_point_is_thread_safe(&time_point));
    FT_ASSERT_EQ(0, time_monotonic_point_enable_thread_safety(&time_point));
    FT_ASSERT_EQ(FT_TRUE, time_monotonic_point_is_thread_safe(&time_point));
    time_monotonic_point_disable_thread_safety(&time_point);
    FT_ASSERT_EQ(FT_FALSE, time_monotonic_point_is_thread_safe(&time_point));
    return (1);
}

FT_TEST(test_time_monotonic_point_lock_cycle_resets_errno)
{
    t_monotonic_time_point time_point;
    ft_bool                   lock_acquired;

    time_point = time_monotonic_point_create(64);
    FT_ASSERT_EQ(0, time_monotonic_point_enable_thread_safety(&time_point));
    lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(0, time_monotonic_point_lock(&time_point, &lock_acquired));
    FT_ASSERT_EQ(FT_TRUE, lock_acquired);
    time_monotonic_point_unlock(&time_point, lock_acquired);
    time_monotonic_point_disable_thread_safety(&time_point);
    return (1);
}

FT_TEST(test_time_duration_enable_thread_safety_enables_mutex)
{
    t_duration_milliseconds duration;

    duration = time_duration_ms_create(100);
    FT_ASSERT_EQ(FT_FALSE, time_duration_ms_is_thread_safe(&duration));
    FT_ASSERT_EQ(0, time_duration_ms_enable_thread_safety(&duration));
    FT_ASSERT_EQ(FT_TRUE, time_duration_ms_is_thread_safe(&duration));
    time_duration_ms_disable_thread_safety(&duration);
    FT_ASSERT_EQ(FT_FALSE, time_duration_ms_is_thread_safe(&duration));
    return (1);
}

FT_TEST(test_time_duration_lock_cycle_resets_errno)
{
    t_duration_milliseconds duration;
    ft_bool                    lock_acquired;

    duration = time_duration_ms_create(250);
    FT_ASSERT_EQ(0, time_duration_ms_enable_thread_safety(&duration));
    lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(0, time_duration_ms_lock(&duration, &lock_acquired));
    FT_ASSERT_EQ(FT_TRUE, lock_acquired);
    time_duration_ms_unlock(&duration, lock_acquired);
    time_duration_ms_disable_thread_safety(&duration);
    return (1);
}

FT_TEST(test_time_info_enable_thread_safety_enables_mutex)
{
    t_time_info info;

    info = create_thread_safe_time_info();
    FT_ASSERT_EQ(FT_FALSE, time_info_is_thread_safe(&info));
    FT_ASSERT_EQ(0, time_info_enable_thread_safety(&info));
    FT_ASSERT_EQ(FT_TRUE, time_info_is_thread_safe(&info));
    time_info_disable_thread_safety(&info);
    FT_ASSERT_EQ(FT_FALSE, time_info_is_thread_safe(&info));
    return (1);
}

FT_TEST(test_time_info_lock_cycle_resets_errno)
{
    t_time_info info;
    ft_bool        lock_acquired;

    info = create_thread_safe_time_info();
    FT_ASSERT_EQ(0, time_info_enable_thread_safety(&info));
    lock_acquired = FT_FALSE;
    FT_ASSERT_EQ(0, time_info_lock(&info, &lock_acquired));
    FT_ASSERT_EQ(FT_TRUE, lock_acquired);
    time_info_unlock(&info, lock_acquired);
    time_info_disable_thread_safety(&info);
    return (1);
}
