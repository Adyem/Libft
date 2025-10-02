#include "../../Time/time.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_time_monotonic_point_add_ms_positive_offset,
        "time_monotonic_point_add_ms adds positive offsets to the baseline")
{
    t_monotonic_time_point baseline;
    t_monotonic_time_point adjusted;

    baseline.milliseconds = 1250;
    adjusted = time_monotonic_point_add_ms(baseline, 750);
    FT_ASSERT_EQ(2000LL, adjusted.milliseconds);
    return (1);
}

FT_TEST(test_time_monotonic_point_add_ms_negative_offset,
        "time_monotonic_point_add_ms supports negative offsets")
{
    t_monotonic_time_point baseline;
    t_monotonic_time_point adjusted;

    baseline.milliseconds = 500;
    adjusted = time_monotonic_point_add_ms(baseline, -250);
    FT_ASSERT_EQ(250LL, adjusted.milliseconds);
    return (1);
}

FT_TEST(test_time_monotonic_point_diff_ms_forward_interval,
        "time_monotonic_point_diff_ms returns positive deltas when end is after start")
{
    t_monotonic_time_point start_point;
    t_monotonic_time_point end_point;
    long long difference;

    start_point.milliseconds = 100;
    end_point.milliseconds = 425;
    difference = time_monotonic_point_diff_ms(start_point, end_point);
    FT_ASSERT_EQ(325LL, difference);
    return (1);
}

FT_TEST(test_time_monotonic_point_diff_ms_reverse_interval,
        "time_monotonic_point_diff_ms reports negative deltas when end precedes start")
{
    t_monotonic_time_point start_point;
    t_monotonic_time_point end_point;
    long long difference;

    start_point.milliseconds = 900;
    end_point.milliseconds = 450;
    difference = time_monotonic_point_diff_ms(start_point, end_point);
    FT_ASSERT_EQ(-450LL, difference);
    return (1);
}

FT_TEST(test_time_monotonic_point_compare_orders_points,
        "time_monotonic_point_compare orders points by millisecond value")
{
    t_monotonic_time_point first_point;
    t_monotonic_time_point second_point;

    first_point.milliseconds = 5000;
    second_point.milliseconds = 7500;
    FT_ASSERT_EQ(-1, time_monotonic_point_compare(first_point, second_point));
    FT_ASSERT_EQ(1, time_monotonic_point_compare(second_point, first_point));
    second_point.milliseconds = 5000;
    FT_ASSERT_EQ(0, time_monotonic_point_compare(first_point, second_point));
    return (1);
}

FT_TEST(test_time_monotonic_point_now_is_monotonic,
        "time_monotonic_point_now produces non-decreasing millisecond readings")
{
    t_monotonic_time_point first_point;
    t_monotonic_time_point second_point;
    long long elapsed;

    first_point = time_monotonic_point_now();
    time_sleep_ms(2);
    second_point = time_monotonic_point_now();
    elapsed = time_monotonic_point_diff_ms(first_point, second_point);
    FT_ASSERT(elapsed >= 0);
    return (1);
}
