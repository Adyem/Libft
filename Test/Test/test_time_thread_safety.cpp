#include "../../Time/time.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

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

FT_TEST(test_time_monotonic_point_lock_cycle_preserves_errno,
        "time_monotonic_point lock and unlock maintain ft_errno state")
{
    t_monotonic_time_point time_point;
    bool                   lock_acquired;
    int                    saved_errno;

    time_point = time_monotonic_point_create(64);
    FT_ASSERT_EQ(0, time_monotonic_point_prepare_thread_safety(&time_point));
    saved_errno = FT_ERR_INVALID_ARGUMENT;
    ft_errno = saved_errno;
    lock_acquired = false;
    FT_ASSERT_EQ(0, time_monotonic_point_lock(&time_point, &lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    FT_ASSERT_EQ(saved_errno, ft_errno);
    time_monotonic_point_unlock(&time_point, lock_acquired);
    FT_ASSERT_EQ(saved_errno, ft_errno);
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

FT_TEST(test_time_duration_lock_cycle_preserves_errno,
        "time_duration_ms lock and unlock keep ft_errno unchanged")
{
    t_duration_milliseconds duration;
    bool                    lock_acquired;
    int                     saved_errno;

    duration = time_duration_ms_create(250);
    FT_ASSERT_EQ(0, time_duration_ms_prepare_thread_safety(&duration));
    saved_errno = FT_ERR_NO_MEMORY;
    ft_errno = saved_errno;
    lock_acquired = false;
    FT_ASSERT_EQ(0, time_duration_ms_lock(&duration, &lock_acquired));
    FT_ASSERT_EQ(true, lock_acquired);
    FT_ASSERT_EQ(saved_errno, ft_errno);
    time_duration_ms_unlock(&duration, lock_acquired);
    FT_ASSERT_EQ(saved_errno, ft_errno);
    time_duration_ms_teardown_thread_safety(&duration);
    return (1);
}

