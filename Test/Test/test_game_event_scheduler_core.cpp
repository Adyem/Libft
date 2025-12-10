#include "../../Game/game_event_scheduler.hpp"
#include "../../Game/game_event.hpp"
#include "../../Game/game_world.hpp"
#include "../../Template/vector.hpp"
#include "../../Template/shared_ptr.hpp"
#include "../../Template/function.hpp"
#include "../../System_utils/test_runner.hpp"

int test_event_scheduler_cancel_missing_event(void)
{
    ft_event_scheduler scheduler;
    ft_sharedptr<ft_event> first(new ft_event());
    ft_vector<ft_sharedptr<ft_event> > events;

    first->set_id(1);
    first->set_duration(3);
    scheduler.schedule_event(first);
    scheduler.cancel_event(2);
    scheduler.dump_events(events);
    if (scheduler.get_error() != FT_ERR_GAME_GENERAL_ERROR)
        return (0);
    if (events.size() != 1)
        return (0);
    if (events[0]->get_id() != 1)
        return (0);
    return (1);
}

int test_event_scheduler_reschedule_duration(void)
{
    ft_event_scheduler scheduler;
    ft_sharedptr<ft_event> timed(new ft_event());
    ft_vector<ft_sharedptr<ft_event> > events;

    timed->set_id(4);
    timed->set_duration(4);
    scheduler.schedule_event(timed);
    scheduler.reschedule_event(4, 7);
    scheduler.dump_events(events);
    if (scheduler.get_error() != FT_ER_SUCCESSS)
        return (0);
    if (events.size() != 1)
        return (0);
    if (events[0]->get_duration() != 7)
        return (0);
    return (1);
}

int test_event_scheduler_clear_queue(void)
{
    ft_event_scheduler scheduler;
    ft_sharedptr<ft_event> pending(new ft_event());
    ft_vector<ft_sharedptr<ft_event> > events;

    pending->set_id(9);
    pending->set_duration(2);
    scheduler.schedule_event(pending);
    scheduler.clear();
    scheduler.dump_events(events);
    if (scheduler.get_error() != FT_ER_SUCCESSS)
        return (0);
    if (events.size() != 0)
        return (0);
    return (1);
}

int test_event_scheduler_prioritizes_shorter_events(void)
{
    ft_event_scheduler scheduler;
    ft_sharedptr<ft_event> long_event(new ft_event());
    ft_sharedptr<ft_event> short_event(new ft_event());
    ft_vector<ft_sharedptr<ft_event> > events;

    long_event->set_id(11);
    long_event->set_duration(5);
    short_event->set_id(12);
    short_event->set_duration(1);
    scheduler.schedule_event(long_event);
    scheduler.schedule_event(short_event);
    scheduler.dump_events(events);
    if (scheduler.get_error() != FT_ER_SUCCESSS)
        return (0);
    if (events.size() != 2)
        return (0);
    if (events[0]->get_id() != 12 || events[0]->get_duration() != 1)
        return (0);
    if (events[1]->get_id() != 11 || events[1]->get_duration() != 5)
        return (0);
    return (1);
}

int test_event_scheduler_rejects_null_schedule(void)
{
    ft_event_scheduler scheduler;
    ft_vector<ft_sharedptr<ft_event> > events;

    scheduler.schedule_event(ft_sharedptr<ft_event>());
    scheduler.dump_events(events);
    if (scheduler.get_error() != FT_ERR_GAME_GENERAL_ERROR)
        return (0);
    if (events.size() != 0)
        return (0);
    return (1);
}

FT_TEST(test_game_event_scheduler_processes_ready_events, "ft_event_scheduler runs callbacks for ready events")
{
    ft_event_scheduler scheduler;
    ft_sharedptr<ft_world> world(new ft_world());
    ft_sharedptr<ft_event> ready_event(new ft_event());
    ft_vector<ft_sharedptr<ft_event> > remaining;
    int callback_runs;

    callback_runs = 0;
    ready_event->set_id(21);
    ready_event->set_duration(1);
    ready_event->set_callback(ft_function<void(ft_world&, ft_event&)>([&callback_runs](ft_world &world_ref, ft_event &event_ref)
    {
        (void)world_ref;
        (void)event_ref;
        callback_runs += 1;
        return ;
    }));
    scheduler.schedule_event(ready_event);
    scheduler.update_events(world, 1);
    scheduler.dump_events(remaining);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, scheduler.get_error());
    FT_ASSERT_EQ((size_t)0, remaining.size());
    FT_ASSERT_EQ(1, callback_runs);
    return (1);
}

FT_TEST(test_game_event_scheduler_reschedules_remaining_events, "ft_event_scheduler keeps unfinished events queued")
{
    ft_event_scheduler scheduler;
    ft_sharedptr<ft_world> world(new ft_world());
    ft_sharedptr<ft_event> delayed_event(new ft_event());
    ft_vector<ft_sharedptr<ft_event> > pending;

    delayed_event->set_id(22);
    delayed_event->set_duration(3);
    scheduler.schedule_event(delayed_event);
    scheduler.update_events(world, 1);
    scheduler.dump_events(pending);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, scheduler.get_error());
    FT_ASSERT_EQ((size_t)1, pending.size());
    FT_ASSERT_EQ(22, pending[0]->get_id());
    FT_ASSERT_EQ(2, pending[0]->get_duration());
    return (1);
}

FT_TEST(test_game_event_scheduler_rejects_null_world, "ft_event_scheduler::update_events requires a world")
{
    ft_event_scheduler scheduler;
    ft_sharedptr<ft_event> mission(new ft_event());
    ft_vector<ft_sharedptr<ft_event> > queued;
    ft_sharedptr<ft_world> world;

    mission->set_id(23);
    mission->set_duration(2);
    scheduler.schedule_event(mission);
    scheduler.update_events(world, 1);
    scheduler.dump_events(queued);
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, scheduler.get_error());
    FT_ASSERT_EQ((size_t)1, queued.size());
    FT_ASSERT_EQ(23, queued[0]->get_id());
    FT_ASSERT_EQ(2, queued[0]->get_duration());
    return (1);
}

FT_TEST(test_game_event_scheduler_profiles_updates, "ft_event_scheduler records profiling data")
{
    ft_event_scheduler scheduler;
    ft_sharedptr<ft_world> world(new ft_world());
    ft_sharedptr<ft_event> task(new ft_event());
    t_event_scheduler_profile profile;

    scheduler.enable_profiling(true);
    task->set_id(24);
    task->set_duration(1);
    scheduler.schedule_event(task);
    scheduler.update_events(world, 1);
    scheduler.snapshot_profile(profile);
    FT_ASSERT_EQ(true, scheduler.profiling_enabled());
    FT_ASSERT_EQ(1, profile.update_count);
    FT_ASSERT_EQ(1, profile.events_processed);
    FT_ASSERT_EQ(0, profile.events_rescheduled);
    FT_ASSERT_EQ((size_t)0, profile.max_queue_depth);
    FT_ASSERT_EQ((size_t)1, profile.max_ready_batch);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, profile.last_error_code);
    return (1);
}

FT_TEST(test_game_event_scheduler_reset_profile_clears_metrics, "ft_event_scheduler::reset_profile zeroes collected stats")
{
    ft_event_scheduler scheduler;
    ft_sharedptr<ft_world> world(new ft_world());
    ft_sharedptr<ft_event> quick_task(new ft_event());
    t_event_scheduler_profile profile;

    scheduler.enable_profiling(true);
    quick_task->set_id(25);
    quick_task->set_duration(1);
    scheduler.schedule_event(quick_task);
    scheduler.update_events(world, 1);
    scheduler.reset_profile();
    scheduler.snapshot_profile(profile);
    FT_ASSERT_EQ(true, scheduler.profiling_enabled());
    FT_ASSERT_EQ(0, profile.update_count);
    FT_ASSERT_EQ(0, profile.events_processed);
    FT_ASSERT_EQ(0, profile.events_rescheduled);
    FT_ASSERT_EQ((size_t)0, profile.max_queue_depth);
    FT_ASSERT_EQ((size_t)0, profile.max_ready_batch);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, profile.last_error_code);
    return (1);
}

FT_TEST(test_game_event_scheduler_size_tracks_scheduled_events, "ft_event_scheduler::size reports queue length")
{
    ft_event_scheduler scheduler;
    ft_sharedptr<ft_event> first(new ft_event());
    ft_sharedptr<ft_event> second(new ft_event());

    first->set_id(31);
    first->set_duration(2);
    second->set_id(32);
    second->set_duration(3);
    scheduler.schedule_event(first);
    scheduler.schedule_event(second);
    FT_ASSERT_EQ((size_t)2, scheduler.size());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, scheduler.get_error());
    return (1);
}

FT_TEST(test_game_event_scheduler_cancel_existing_event_clears_queue, "ft_event_scheduler::cancel_event removes matching entries")
{
    ft_event_scheduler scheduler;
    ft_sharedptr<ft_event> quest(new ft_event());
    ft_vector<ft_sharedptr<ft_event> > events;

    quest->set_id(33);
    quest->set_duration(4);
    scheduler.schedule_event(quest);
    scheduler.cancel_event(33);
    scheduler.dump_events(events);
    FT_ASSERT_EQ((size_t)0, events.size());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, scheduler.get_error());
    return (1);
}

FT_TEST(test_game_event_scheduler_update_without_events_is_noop, "ft_event_scheduler::update_events succeeds with empty queue")
{
    ft_event_scheduler scheduler;
    ft_sharedptr<ft_world> world(new ft_world());

    scheduler.update_events(world, 1);
    FT_ASSERT_EQ((size_t)0, scheduler.size());
    FT_ASSERT_EQ(FT_ER_SUCCESSS, scheduler.get_error());
    return (1);
}

FT_TEST(test_game_event_scheduler_snapshot_without_profiling_is_zeroed, "ft_event_scheduler::snapshot_profile reports initial zeros")
{
    ft_event_scheduler scheduler;
    t_event_scheduler_profile profile;

    scheduler.snapshot_profile(profile);
    FT_ASSERT_EQ(false, scheduler.profiling_enabled());
    FT_ASSERT_EQ(0, profile.update_count);
    FT_ASSERT_EQ(0, profile.events_processed);
    FT_ASSERT_EQ(0, profile.events_rescheduled);
    FT_ASSERT_EQ((size_t)0, profile.max_queue_depth);
    FT_ASSERT_EQ((size_t)0, profile.max_ready_batch);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, profile.last_error_code);
    return (1);
}

FT_TEST(test_game_event_scheduler_profile_counts_reschedules, "ft_event_scheduler profiling tracks rescheduled events")
{
    ft_event_scheduler scheduler;
    ft_sharedptr<ft_world> world(new ft_world());
    ft_sharedptr<ft_event> delayed(new ft_event());
    t_event_scheduler_profile profile;

    scheduler.enable_profiling(true);
    delayed->set_id(34);
    delayed->set_duration(2);
    scheduler.schedule_event(delayed);
    scheduler.update_events(world, 1);
    scheduler.snapshot_profile(profile);
    FT_ASSERT_EQ(true, scheduler.profiling_enabled());
    FT_ASSERT_EQ(1, profile.update_count);
    FT_ASSERT_EQ(0, profile.events_processed);
    FT_ASSERT_EQ(1, profile.events_rescheduled);
    FT_ASSERT_EQ((size_t)1, profile.max_queue_depth);
    FT_ASSERT_EQ((size_t)0, profile.max_ready_batch);
    FT_ASSERT_EQ(FT_ER_SUCCESSS, profile.last_error_code);
    return (1);
}

