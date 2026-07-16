#include "../test_internal.hpp"

#include "../../Modules/Game/game_event_scheduler.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

FT_TEST(test_game_event_scheduler_thread_safe_lifecycle)
{
    game_event_scheduler scheduler;
    t_event_scheduler_profile profile;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, scheduler.initialize());
    FT_ASSERT_EQ(static_cast<ft_size_t>(0), scheduler.size());
    FT_ASSERT(scheduler.profiling_enabled() == FT_FALSE);
    scheduler.enable_profiling(FT_TRUE);
    FT_ASSERT(scheduler.profiling_enabled() == FT_TRUE);
    scheduler.snapshot_profile(profile);
    FT_ASSERT_EQ(0, profile.update_count);
    FT_ASSERT_EQ(0, profile.events_processed);
    FT_ASSERT_EQ(0, profile.events_rescheduled);
    FT_ASSERT_EQ(static_cast<ft_size_t>(0), profile.max_queue_depth);
    FT_ASSERT_EQ(static_cast<ft_size_t>(0), profile.max_ready_batch);
    FT_ASSERT_EQ(0, profile.total_processing_ns);
    FT_ASSERT_EQ(0, profile.last_update_processing_ns);
    FT_ASSERT_EQ(0, profile.last_error_code);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scheduler.enable_thread_safety());
    FT_ASSERT(scheduler.is_thread_safe() == FT_TRUE);
    scheduler.clear();
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scheduler.destroy());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scheduler.destroy());
    return (1);
}
