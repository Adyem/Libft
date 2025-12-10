#include "../../Time/time.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include "../../CPP_class/class_nullptr.hpp"

FT_TEST(test_time_high_resolution_now_requires_output,
        "time_high_resolution_now rejects null output pointers")
{
    bool    result;

    result = time_high_resolution_now(ft_nullptr);
    FT_ASSERT(result == false);
    FT_ASSERT(ft_errno == FT_ERR_INVALID_ARGUMENT);
    return (1);
}

FT_TEST(test_time_high_resolution_now_reports_success,
        "time_high_resolution_now populates the output and clears ft_errno")
{
    t_high_resolution_time_point first_point;
    bool                         result;

    result = time_high_resolution_now(&first_point);
    FT_ASSERT(result == true);
    FT_ASSERT(ft_errno == FT_ER_SUCCESSS);
    return (1);
}

FT_TEST(test_time_high_resolution_diff_seconds_tracks_elapsed_time,
        "time_high_resolution_diff_seconds reflects elapsed nanoseconds as seconds")
{
    t_high_resolution_time_point start_point;
    t_high_resolution_time_point end_point;
    long long                    elapsed_ns;
    double                       elapsed_seconds;

    FT_ASSERT(time_high_resolution_now(&start_point) == true);
    time_sleep_ms(2);
    FT_ASSERT(time_high_resolution_now(&end_point) == true);
    elapsed_ns = time_high_resolution_diff_ns(start_point, end_point);
    FT_ASSERT(elapsed_ns >= 0);
    elapsed_seconds = time_high_resolution_diff_seconds(start_point, end_point);
    FT_ASSERT(elapsed_seconds >= 0.0);
    FT_ASSERT(elapsed_seconds >= static_cast<double>(elapsed_ns) / 1000000000.0 - 1e-9);
    return (1);
}
