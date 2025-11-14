#ifndef GAME_EVENT_SCHEDULER_TELEMETRY_HPP
# define GAME_EVENT_SCHEDULER_TELEMETRY_HPP

#include "game_event_scheduler.hpp"

struct ft_event_scheduler_telemetry_state
{
    const char *scheduler_name;
    long long last_update_count;
    long long last_events_processed;
    long long last_events_rescheduled;
    long long last_total_processing_ns;
    size_t last_max_queue_depth;
    size_t last_max_ready_batch;
    long long last_last_update_ns;
};

void game_event_scheduler_telemetry_state_initialize(ft_event_scheduler_telemetry_state &state, const char *scheduler_name) noexcept;
void game_event_scheduler_telemetry_state_reset(ft_event_scheduler_telemetry_state &state) noexcept;
void game_event_scheduler_telemetry_record(ft_event_scheduler_telemetry_state &state, const t_event_scheduler_profile &profile) noexcept;
void game_event_scheduler_publish_telemetry(ft_event_scheduler &scheduler, ft_event_scheduler_telemetry_state &state) noexcept;

#endif
