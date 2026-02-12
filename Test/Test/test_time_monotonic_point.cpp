#include "../test_internal.hpp"
#include "../../Time/time.hpp"
#include "../../System_utils/test_runner.hpp"
#include <climits>

#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_time_monotonic_point_add_ms_positive_offset,
        "time_monotonic_point_add_ms adds positive offsets to the baseline")
{
    t_monotonic_time_point baseline;
    t_monotonic_time_point adjusted;

    baseline = time_monotonic_point_create(1250);
    adjusted = time_monotonic_point_add_ms(baseline, 750);
    FT_ASSERT_EQ(2000LL, adjusted.milliseconds);
    return (1);
}

FT_TEST(test_time_monotonic_point_add_ms_negative_offset,
        "time_monotonic_point_add_ms supports negative offsets")
{
    t_monotonic_time_point baseline;
    t_monotonic_time_point adjusted;

    baseline = time_monotonic_point_create(500);
    adjusted = time_monotonic_point_add_ms(baseline, -250);
    FT_ASSERT_EQ(250LL, adjusted.milliseconds);
    return (1);
}

FT_TEST(test_time_monotonic_point_add_ms_clamps_overflow,
        "time_monotonic_point_add_ms clamps additions that exceed long long range")
{
    t_monotonic_time_point baseline;
    t_monotonic_time_point adjusted;

    baseline = time_monotonic_point_create(LLONG_MAX - 5);
    adjusted = time_monotonic_point_add_ms(baseline, 10);
    FT_ASSERT_EQ(LLONG_MAX, adjusted.milliseconds);
    baseline = time_monotonic_point_create(LLONG_MIN + 5);
    adjusted = time_monotonic_point_add_ms(baseline, -10);
    FT_ASSERT_EQ(LLONG_MIN, adjusted.milliseconds);
    return (1);
}

FT_TEST(test_time_monotonic_point_diff_ms_forward_interval,
        "time_monotonic_point_diff_ms returns positive deltas when end is after start")
{
    t_monotonic_time_point start_point;
    t_monotonic_time_point end_point;
    long long difference;

    start_point = time_monotonic_point_create(100);
    end_point = time_monotonic_point_create(425);
    difference = time_monotonic_point_diff_ms(start_point, end_point);
    FT_ASSERT_EQ(325LL, difference);
    return (1);
}

FT_TEST(test_time_monotonic_point_diff_ms_clamps_overflow,
        "time_monotonic_point_diff_ms clamps differences that exceed long long range")
{
    t_monotonic_time_point start_point;
    t_monotonic_time_point end_point;
    long long difference;

    start_point = time_monotonic_point_create(LLONG_MIN);
    end_point = time_monotonic_point_create(LLONG_MAX);
    difference = time_monotonic_point_diff_ms(start_point, end_point);
    FT_ASSERT_EQ(LLONG_MAX, difference);
    start_point = time_monotonic_point_create(LLONG_MAX);
    end_point = time_monotonic_point_create(LLONG_MIN);
    difference = time_monotonic_point_diff_ms(start_point, end_point);
    FT_ASSERT_EQ(LLONG_MIN, difference);
    return (1);
}

FT_TEST(test_time_monotonic_point_diff_ms_reverse_interval,
        "time_monotonic_point_diff_ms reports negative deltas when end precedes start")
{
    t_monotonic_time_point start_point;
    t_monotonic_time_point end_point;
    long long difference;

    start_point = time_monotonic_point_create(900);
    end_point = time_monotonic_point_create(450);
    difference = time_monotonic_point_diff_ms(start_point, end_point);
    FT_ASSERT_EQ(-450LL, difference);
    return (1);
}

FT_TEST(test_time_monotonic_point_compare_orders_points,
        "time_monotonic_point_compare orders points by millisecond value")
{
    t_monotonic_time_point first_point;
    t_monotonic_time_point second_point;

    first_point = time_monotonic_point_create(5000);
    second_point = time_monotonic_point_create(7500);
    FT_ASSERT_EQ(-1, time_monotonic_point_compare(first_point, second_point));
    FT_ASSERT_EQ(1, time_monotonic_point_compare(second_point, first_point));
    second_point = time_monotonic_point_create(5000);
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

FT_TEST(test_time_get_monotonic_wall_anchor_midpoint,
        "time_get_monotonic_wall_anchor positions the midpoint between samples")
{
    t_monotonic_time_point before_point;
    t_monotonic_time_point anchor_point;
    t_monotonic_time_point after_point;
    long long               anchor_wall_ms;
    int                     compare_before;
    int                     compare_after;

    before_point = time_monotonic_point_now();
    FT_ASSERT(time_get_monotonic_wall_anchor(anchor_point, anchor_wall_ms));
    after_point = time_monotonic_point_now();
    compare_before = time_monotonic_point_compare(before_point, anchor_point);
    compare_after = time_monotonic_point_compare(after_point, anchor_point);
    FT_ASSERT(compare_before <= 0);
    FT_ASSERT(compare_after >= 0);
    (void)anchor_wall_ms;
    return (1);
}

FT_TEST(test_time_monotonic_to_wall_ms_translates_forward,
        "time_monotonic_to_wall_ms applies positive offsets to the wall clock")
{
    t_monotonic_time_point anchor_point;
    t_monotonic_time_point target_point;
    long long               anchor_wall_ms;
    long long               translated_wall_ms;

    anchor_point = time_monotonic_point_create(1000);
    anchor_wall_ms = 5000;
    target_point = time_monotonic_point_add_ms(anchor_point, 250);
    FT_ASSERT(time_monotonic_to_wall_ms(target_point, anchor_point,
            anchor_wall_ms, translated_wall_ms));
    FT_ASSERT_EQ(5250LL, translated_wall_ms);
    return (1);
}

FT_TEST(test_time_wall_ms_to_monotonic_translates_backward,
        "time_wall_ms_to_monotonic converts wall time deltas back to monotonic")
{
    t_monotonic_time_point anchor_point;
    t_monotonic_time_point translated_point;
    long long               anchor_wall_ms;

    anchor_point = time_monotonic_point_create(-2000);
    anchor_wall_ms = -3000;
    FT_ASSERT(time_wall_ms_to_monotonic(-2500, anchor_point, anchor_wall_ms,
            translated_point));
    FT_ASSERT_EQ(-1500LL, translated_point.milliseconds);
    return (1);
}
