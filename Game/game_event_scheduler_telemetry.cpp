#include "game_event_scheduler_telemetry.hpp"
#include "../Observability/observability_game_metrics.hpp"
#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include <limits>

static const long long g_event_scheduler_ns_per_second = 1000000000LL;

static void game_event_scheduler_telemetry_emit(const char *event_name,
        const char *attribute,
        long long delta_value,
        long long total_value,
        const char *unit,
        int error_code,
        bool success,
        const char *entity) noexcept
{
    ft_game_observability_sample sample;

    sample.labels.event_name = event_name;
    sample.labels.entity = entity;
    sample.labels.attribute = attribute;
    sample.delta_value = delta_value;
    sample.total_value = total_value;
    sample.unit = unit;
    sample.error_code = error_code;
    sample.error_tag = ft_nullptr;
    sample.success = success;
    observability_game_metrics_record(sample);
    return ;
}

static void game_event_scheduler_telemetry_update_state(ft_event_scheduler_telemetry_state &state,
        const t_event_scheduler_profile &profile) noexcept
{
    state.last_update_count = profile.update_count;
    state.last_events_processed = profile.events_processed;
    state.last_events_rescheduled = profile.events_rescheduled;
    state.last_total_processing_ns = profile.total_processing_ns;
    state.last_max_queue_depth = profile.max_queue_depth;
    state.last_max_ready_batch = profile.max_ready_batch;
    state.last_last_update_ns = profile.last_update_processing_ns;
    return ;
}

void game_event_scheduler_telemetry_state_initialize(ft_event_scheduler_telemetry_state &state,
        const char *scheduler_name) noexcept
{
    state.scheduler_name = scheduler_name;
    game_event_scheduler_telemetry_state_reset(state);
    return ;
}

void game_event_scheduler_telemetry_state_reset(ft_event_scheduler_telemetry_state &state) noexcept
{
    state.last_update_count = 0;
    state.last_events_processed = 0;
    state.last_events_rescheduled = 0;
    state.last_total_processing_ns = 0;
    state.last_max_queue_depth = 0;
    state.last_max_ready_batch = 0;
    state.last_last_update_ns = 0;
    return ;
}

void game_event_scheduler_telemetry_record(ft_event_scheduler_telemetry_state &state,
        const t_event_scheduler_profile &profile) noexcept
{
    long long delta_updates;
    long long delta_processed;
    long long delta_processing_ns;
    long long events_per_update;
    long long events_per_second;
    long long queue_depth_value;
    long long latency_value;
    bool metrics_changed;
    bool success;

    if (state.scheduler_name == ft_nullptr)
    {
        ft_errno = FT_ERR_INVALID_ARGUMENT;
        return ;
    }
    if (profile.update_count < state.last_update_count
        || profile.events_processed < state.last_events_processed
        || profile.events_rescheduled < state.last_events_rescheduled
        || profile.total_processing_ns < state.last_total_processing_ns
        || profile.max_queue_depth < state.last_max_queue_depth
        || profile.max_ready_batch < state.last_max_ready_batch)
        game_event_scheduler_telemetry_state_reset(state);
    delta_updates = profile.update_count - state.last_update_count;
    if (delta_updates < 0)
        delta_updates = profile.update_count;
    delta_processed = profile.events_processed - state.last_events_processed;
    if (delta_processed < 0)
        delta_processed = profile.events_processed;
    delta_processing_ns = profile.total_processing_ns - state.last_total_processing_ns;
    if (delta_processing_ns < 0)
        delta_processing_ns = profile.total_processing_ns;
    long long adjusted_processing_ns;

    adjusted_processing_ns = delta_processing_ns;
    if (delta_updates > 0
        && profile.last_update_processing_ns > 0
        && state.last_last_update_ns > 0
        && profile.last_update_processing_ns < state.last_last_update_ns)
    {
        long long maximum_value;
        long long projected_processing_ns;

        maximum_value = std::numeric_limits<long long>::max();
        if (profile.last_update_processing_ns > 0
            && delta_updates > maximum_value / profile.last_update_processing_ns)
            projected_processing_ns = maximum_value;
        else
            projected_processing_ns = delta_updates * profile.last_update_processing_ns;
        if (projected_processing_ns > adjusted_processing_ns)
            adjusted_processing_ns = projected_processing_ns;
    }
    delta_processing_ns = adjusted_processing_ns;
    metrics_changed = false;
    if (delta_updates > 0)
        metrics_changed = true;
    if (delta_processed > 0)
        metrics_changed = true;
    if (profile.max_queue_depth != state.last_max_queue_depth)
        metrics_changed = true;
    if (profile.last_update_processing_ns != state.last_last_update_ns)
        metrics_changed = true;
    if (!metrics_changed)
    {
        game_event_scheduler_telemetry_update_state(state, profile);
        ft_errno = FT_ERR_SUCCESSS;
        return ;
    }
    events_per_update = 0;
    if (delta_updates > 0)
        events_per_update = delta_processed / delta_updates;
    events_per_second = 0;
    if (delta_processing_ns > 0 && delta_processed > 0)
    {
        events_per_second = (delta_processed * g_event_scheduler_ns_per_second) / delta_processing_ns;
        if (events_per_second < 0)
            events_per_second = 0;
    }
    queue_depth_value = static_cast<long long>(profile.max_queue_depth);
    latency_value = profile.last_update_processing_ns;
    success = profile.last_error_code == FT_ERR_SUCCESSS;
    game_event_scheduler_telemetry_emit("event_scheduler.throughput",
        "events_per_update",
        events_per_update,
        events_per_update,
        "events/update",
        profile.last_error_code,
        success,
        state.scheduler_name);
    game_event_scheduler_telemetry_emit("event_scheduler.throughput",
        "events_per_second",
        events_per_second,
        events_per_second,
        "events/sec",
        profile.last_error_code,
        success,
        state.scheduler_name);
    game_event_scheduler_telemetry_emit("event_scheduler.queue",
        "max_depth",
        queue_depth_value,
        queue_depth_value,
        "events",
        profile.last_error_code,
        success,
        state.scheduler_name);
    game_event_scheduler_telemetry_emit("event_scheduler.latency",
        "last_update_ns",
        latency_value,
        profile.total_processing_ns,
        "nanoseconds",
        profile.last_error_code,
        success,
        state.scheduler_name);
    game_event_scheduler_telemetry_update_state(state, profile);
    ft_errno = FT_ERR_SUCCESSS;
    return ;
}

void game_event_scheduler_publish_telemetry(ft_event_scheduler &scheduler,
        ft_event_scheduler_telemetry_state &state) noexcept
{
    t_event_scheduler_profile profile;
    scheduler.snapshot_profile(profile);
    if (scheduler.get_error() != FT_ERR_SUCCESSS)
    {
        ft_errno = scheduler.get_error();
        return ;
    }
    game_event_scheduler_telemetry_record(state, profile);
    return ;
}
