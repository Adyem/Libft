#ifndef GAME_EVENT_SCHEDULER_TELEMETRY_HPP
# define GAME_EVENT_SCHEDULER_TELEMETRY_HPP

#include "game_event_scheduler.hpp"

struct game_event_scheduler_telemetry_state
{
    const char *scheduler_name;
    int64_t last_update_count;
    int64_t last_events_processed;
    int64_t last_events_rescheduled;
    int64_t last_total_processing_ns;
    ft_size_t last_max_queue_depth;
    ft_size_t last_max_ready_batch;
    int64_t last_last_update_ns;
};

void game_event_scheduler_telemetry_state_initialize(game_event_scheduler_telemetry_state &state, const char *scheduler_name) noexcept;
void game_event_scheduler_telemetry_state_reset(game_event_scheduler_telemetry_state &state) noexcept;
void game_event_scheduler_telemetry_record(game_event_scheduler_telemetry_state &state, const t_event_scheduler_profile &profile) noexcept;
void game_event_scheduler_publish_telemetry(game_event_scheduler &scheduler, game_event_scheduler_telemetry_state &state) noexcept;

#endif
