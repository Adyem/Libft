#include "../../Time/time.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

static t_time_info create_thread_safe_time_info(void)
{
    t_time_info info;

    info.mutex = ft_nullptr;
    info.thread_safe_enabled = false;
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

FT_TEST(test_time_monotonic_point_prepare_thread_safety_enables_mutex,
        "time_monotonic_point_prepare_thread_safety installs mutex guard")
{
    t_monotonic_time_point time_point;

    time_point = time_monotonic_point_create(42);
    FT_ASSERT_EQ(false, time_monotonic_point_is_thread_safe_enabled(&time_point));
    FT_ASSERT_EQ(0, time_monotonic_point_prepare_thread_safety(&time_point));
    FT_ASSERT_EQ(true, time_monotonic_point_is_thread_safe_enabled(&time_point));
    time_monotonic_point_teardown_thread_safety(&time_point);
    FT_ASSERT_EQ(false, time_monotonic_point_is_thread_safe_enabled(&time_point));
    return (1);
}

FT_TEST(test_time_monotonic_point_lock_cycle_resets_errno,
        "time_monotonic_point lock and unlock set ft_errno to success")
{
    t_monotonic_time_point time_point;
    bool                   lock_acquired;

    time_point = time_monotonic_point_create(64);
    FT_ASSERT_EQ(0, time_monotonic_point_prepare_thread_safety(&time_point));
    ft_errno = FT_ERR_INVALID_ARGUMENT;
    lock_acquired = false;
    FT_ASSERT_EQ(0, time_monotonic_point_lock(&time_point, &lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    time_monotonic_point_unlock(&time_point, lock_acquired);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    time_monotonic_point_teardown_thread_safety(&time_point);
    return (1);
}

FT_TEST(test_time_duration_prepare_thread_safety_enables_mutex,
        "time_duration_ms_prepare_thread_safety allocates mutex guard")
{
    t_duration_milliseconds duration;

    duration = time_duration_ms_create(100);
    FT_ASSERT_EQ(false, time_duration_ms_is_thread_safe_enabled(&duration));
    FT_ASSERT_EQ(0, time_duration_ms_prepare_thread_safety(&duration));
    FT_ASSERT_EQ(true, time_duration_ms_is_thread_safe_enabled(&duration));
    time_duration_ms_teardown_thread_safety(&duration);
    FT_ASSERT_EQ(false, time_duration_ms_is_thread_safe_enabled(&duration));
    return (1);
}

FT_TEST(test_time_duration_lock_cycle_resets_errno,
        "time_duration_ms lock and unlock set ft_errno to success")
{
    t_duration_milliseconds duration;
    bool                    lock_acquired;

    duration = time_duration_ms_create(250);
    FT_ASSERT_EQ(0, time_duration_ms_prepare_thread_safety(&duration));
    ft_errno = FT_ERR_NO_MEMORY;
    lock_acquired = false;
    FT_ASSERT_EQ(0, time_duration_ms_lock(&duration, &lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    time_duration_ms_unlock(&duration, lock_acquired);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    time_duration_ms_teardown_thread_safety(&duration);
    return (1);
}

FT_TEST(test_time_info_prepare_thread_safety_enables_mutex,
        "time_info_prepare_thread_safety installs mutex guard")
{
    t_time_info info;

    info = create_thread_safe_time_info();
    FT_ASSERT_EQ(false, time_info_is_thread_safe_enabled(&info));
    FT_ASSERT_EQ(0, time_info_prepare_thread_safety(&info));
    FT_ASSERT_EQ(true, time_info_is_thread_safe_enabled(&info));
    time_info_teardown_thread_safety(&info);
    FT_ASSERT_EQ(false, time_info_is_thread_safe_enabled(&info));
    return (1);
}

FT_TEST(test_time_info_lock_cycle_resets_errno,
        "time_info lock and unlock set ft_errno to success")
{
    t_time_info info;
    bool        lock_acquired;

    info = create_thread_safe_time_info();
    FT_ASSERT_EQ(0, time_info_prepare_thread_safety(&info));
    lock_acquired = false;
    ft_errno = FT_ERR_INVALID_STATE;
    FT_ASSERT_EQ(0, time_info_lock(&info, &lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    time_info_unlock(&info, lock_acquired);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, ft_errno);
    time_info_teardown_thread_safety(&info);
    return (1);
}

