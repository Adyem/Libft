#include "../../Game/game_event_scheduler_telemetry.hpp"
#include "../../Observability/observability_game_metrics.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Errno/errno.hpp"
#include "../../Libft/libft.hpp"
#include <vector>

static std::vector<ft_game_observability_sample> g_scheduler_samples;

static bool game_event_scheduler_strings_equal(const char *left, const char *right)
{
    if (left == ft_nullptr && right == ft_nullptr)
        return (true);
    if (left == ft_nullptr || right == ft_nullptr)
        return (false);
    return (ft_strncmp(left, right, ft_strlen(left) + 1) == 0);
}

static void game_event_scheduler_reset_samples(void)
{
    g_scheduler_samples.clear();
    return ;
}

static void game_event_scheduler_capture_sample(const ft_game_observability_sample &sample)
{
    g_scheduler_samples.push_back(sample);
    return ;
}

FT_TEST(test_game_event_scheduler_telemetry_exports_metrics,
    "game event scheduler telemetry exports throughput queue depth and latency")
{
    ft_event_scheduler_telemetry_state state;
    t_event_scheduler_profile profile;

    observability_game_metrics_shutdown();
    game_event_scheduler_reset_samples();
    FT_ASSERT_EQ(0, observability_game_metrics_initialize(game_event_scheduler_capture_sample));
    game_event_scheduler_telemetry_state_initialize(state, "primary");
    profile.update_count = 5;
    profile.events_processed = 25;
    profile.events_rescheduled = 3;
    profile.max_queue_depth = 6;
    profile.max_ready_batch = 4;
    profile.total_processing_ns = 200000000;
    profile.last_update_processing_ns = 50000000;
    profile.last_error_code = FT_ERR_SUCCESSS;
    game_event_scheduler_telemetry_record(state, profile);
    FT_ASSERT_EQ(4, static_cast<int>(g_scheduler_samples.size()));
    FT_ASSERT(game_event_scheduler_strings_equal("event_scheduler.throughput",
        g_scheduler_samples[0].labels.event_name));
    FT_ASSERT(game_event_scheduler_strings_equal("events_per_update",
        g_scheduler_samples[0].labels.attribute));
    FT_ASSERT_EQ(5, g_scheduler_samples[0].delta_value);
    FT_ASSERT_EQ(5, g_scheduler_samples[0].total_value);
    FT_ASSERT(game_event_scheduler_strings_equal("event_scheduler.throughput",
        g_scheduler_samples[1].labels.event_name));
    FT_ASSERT(game_event_scheduler_strings_equal("events_per_second",
        g_scheduler_samples[1].labels.attribute));
    FT_ASSERT_EQ(125, g_scheduler_samples[1].delta_value);
    FT_ASSERT_EQ(125, g_scheduler_samples[1].total_value);
    FT_ASSERT(game_event_scheduler_strings_equal("event_scheduler.queue",
        g_scheduler_samples[2].labels.event_name));
    FT_ASSERT(game_event_scheduler_strings_equal("max_depth",
        g_scheduler_samples[2].labels.attribute));
    FT_ASSERT_EQ(6, g_scheduler_samples[2].delta_value);
    FT_ASSERT_EQ(6, g_scheduler_samples[2].total_value);
    FT_ASSERT(game_event_scheduler_strings_equal("event_scheduler.latency",
        g_scheduler_samples[3].labels.event_name));
    FT_ASSERT(game_event_scheduler_strings_equal("last_update_ns",
        g_scheduler_samples[3].labels.attribute));
    FT_ASSERT_EQ(50000000, g_scheduler_samples[3].delta_value);
    FT_ASSERT_EQ(200000000, g_scheduler_samples[3].total_value);
    FT_ASSERT(g_scheduler_samples[3].success);
    FT_ASSERT_EQ(0, observability_game_metrics_shutdown());
    return (1);
}

FT_TEST(test_game_event_scheduler_telemetry_skips_duplicate_snapshots,
    "game event scheduler telemetry skips duplicate snapshots")
{
    ft_event_scheduler_telemetry_state state;
    t_event_scheduler_profile profile;

    observability_game_metrics_shutdown();
    game_event_scheduler_reset_samples();
    FT_ASSERT_EQ(0, observability_game_metrics_initialize(game_event_scheduler_capture_sample));
    game_event_scheduler_telemetry_state_initialize(state, "secondary");
    profile.update_count = 4;
    profile.events_processed = 16;
    profile.events_rescheduled = 2;
    profile.max_queue_depth = 5;
    profile.max_ready_batch = 3;
    profile.total_processing_ns = 160000000;
    profile.last_update_processing_ns = 40000000;
    profile.last_error_code = FT_ERR_SUCCESSS;
    game_event_scheduler_telemetry_record(state, profile);
    FT_ASSERT_EQ(4, static_cast<int>(g_scheduler_samples.size()));
    game_event_scheduler_telemetry_record(state, profile);
    FT_ASSERT_EQ(4, static_cast<int>(g_scheduler_samples.size()));
    profile.update_count = 8;
    profile.events_processed = 40;
    profile.events_rescheduled = 5;
    profile.max_queue_depth = 7;
    profile.max_ready_batch = 5;
    profile.total_processing_ns = 260000000;
    profile.last_update_processing_ns = 30000000;
    game_event_scheduler_telemetry_record(state, profile);
    FT_ASSERT_EQ(8, static_cast<int>(g_scheduler_samples.size()));
    FT_ASSERT_EQ(6, g_scheduler_samples[4].delta_value);
    FT_ASSERT_EQ(6, g_scheduler_samples[4].total_value);
    FT_ASSERT_EQ(200, g_scheduler_samples[5].delta_value);
    FT_ASSERT_EQ(200, g_scheduler_samples[5].total_value);
    FT_ASSERT_EQ(7, g_scheduler_samples[6].delta_value);
    FT_ASSERT_EQ(7, g_scheduler_samples[6].total_value);
    FT_ASSERT_EQ(30000000, g_scheduler_samples[7].delta_value);
    FT_ASSERT_EQ(260000000, g_scheduler_samples[7].total_value);
    FT_ASSERT_EQ(0, observability_game_metrics_shutdown());
    return (1);
}

FT_TEST(test_game_event_scheduler_telemetry_rejects_null_scheduler_name,
    "game event scheduler telemetry rejects null scheduler name")
{
    ft_event_scheduler_telemetry_state state;
    t_event_scheduler_profile profile;

    observability_game_metrics_shutdown();
    game_event_scheduler_reset_samples();
    FT_ASSERT_EQ(0, observability_game_metrics_initialize(game_event_scheduler_capture_sample));
    game_event_scheduler_telemetry_state_initialize(state, ft_nullptr);
    profile.update_count = 1;
    profile.events_processed = 2;
    profile.events_rescheduled = 0;
    profile.max_queue_depth = 1;
    profile.max_ready_batch = 1;
    profile.total_processing_ns = 1000000;
    profile.last_update_processing_ns = 1000000;
    profile.last_error_code = FT_ERR_SUCCESSS;
    ft_errno = FT_ERR_SUCCESSS;
    game_event_scheduler_telemetry_record(state, profile);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, ft_errno);
    FT_ASSERT_EQ(0, static_cast<int>(g_scheduler_samples.size()));
    FT_ASSERT_EQ(0, observability_game_metrics_shutdown());
    return (1);
}
