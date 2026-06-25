#include "../test_internal.hpp"
#include "../../Modules/Time/time.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include <cmath>

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_time_benchmark_computes_average_and_jitter)
{
    t_time_benchmark            benchmark;
    t_time_benchmark_snapshot   snapshot;
    int                         add_result;
    bool                        snapshot_result;

    time_benchmark_init(&benchmark);
    add_result = time_benchmark_add_sample(&benchmark, 10.0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, add_result);
    add_result = time_benchmark_add_sample(&benchmark, 12.0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, add_result);
    add_result = time_benchmark_add_sample(&benchmark, 14.0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, add_result);

    snapshot_result = time_benchmark_snapshot(&benchmark, &snapshot);
    FT_ASSERT_EQ(true, snapshot_result);
    FT_ASSERT_EQ(static_cast<size_t>(3), snapshot.sample_count);
    FT_ASSERT(std::fabs(snapshot.average_ms - 12.0) < 0.0001);
    FT_ASSERT(std::fabs(snapshot.jitter_ms - 2.0) < 0.0001);
    FT_ASSERT(std::fabs(snapshot.minimum_ms - 10.0) < 0.0001);
    FT_ASSERT(std::fabs(snapshot.maximum_ms - 14.0) < 0.0001);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, time_benchmark_get_error(&benchmark));
    return (1);
}

FT_TEST(test_time_benchmark_rejects_negative_samples)
{
    t_time_benchmark            benchmark;
    t_time_benchmark_snapshot   snapshot;
    int                         add_result;
    bool                        snapshot_result;

    time_benchmark_init(&benchmark);
    add_result = time_benchmark_add_sample(&benchmark, 5.0);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, add_result);

    add_result = time_benchmark_add_sample(&benchmark, -1.0);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, add_result);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, time_benchmark_get_error(&benchmark));

    snapshot_result = time_benchmark_snapshot(&benchmark, &snapshot);
    FT_ASSERT_EQ(false, snapshot_result);
    FT_ASSERT_EQ(static_cast<size_t>(1), snapshot.sample_count);
    FT_ASSERT(std::fabs(snapshot.average_ms - 5.0) < 0.0001);

    time_benchmark_reset(&benchmark);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, time_benchmark_get_error(&benchmark));
    FT_ASSERT_EQ(static_cast<size_t>(0), time_benchmark_get_sample_count(&benchmark));
    return (1);
}

FT_TEST(test_time_benchmark_accepts_duration_wrappers)
{
    t_time_benchmark            benchmark;
    t_time_benchmark_snapshot   snapshot;
    t_duration_milliseconds     first_duration;
    t_duration_milliseconds     second_duration;
    int                         add_result;
    bool                        snapshot_result;

    time_benchmark_init(&benchmark);
    first_duration = time_duration_ms_create(5);
    second_duration = time_duration_ms_create(15);
    add_result = time_benchmark_add_duration(&benchmark, first_duration);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, add_result);
    add_result = time_benchmark_add_duration(&benchmark, second_duration);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, add_result);

    snapshot_result = time_benchmark_snapshot(&benchmark, &snapshot);
    FT_ASSERT_EQ(true, snapshot_result);
    FT_ASSERT_EQ(static_cast<size_t>(2), snapshot.sample_count);
    FT_ASSERT(std::fabs(snapshot.average_ms - 10.0) < 0.0001);
    FT_ASSERT(snapshot.jitter_ms >= 5.0);
    FT_ASSERT(snapshot.maximum_ms >= snapshot.minimum_ms);
    return (1);
}
