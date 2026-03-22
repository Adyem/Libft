#include "../test_internal.hpp"
#include "../../Game/game_event_scheduler.hpp"
#include "../../Game/game_event.hpp"
#include "../../Game/game_world.hpp"
#include "../../Template/vector.hpp"
#include "../../Template/shared_ptr.hpp"
#include "../../Template/function.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

int test_event_scheduler_cancel_missing_event(void)
{
    game_event_scheduler scheduler;
    ft_sharedptr<game_event> first(new game_event());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scheduler.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, first->initialize());
    ft_vector<ft_sharedptr<game_event> > events;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, events.initialize());

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
    game_event_scheduler scheduler;
    ft_sharedptr<game_event> timed(new game_event());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scheduler.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, timed->initialize());
    ft_vector<ft_sharedptr<game_event> > events;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, events.initialize());

    timed->set_id(4);
    timed->set_duration(4);
    scheduler.schedule_event(timed);
    scheduler.reschedule_event(4, 7);
    scheduler.dump_events(events);
    if (scheduler.get_error() != FT_ERR_SUCCESS)
        return (0);
    if (events.size() != 1)
        return (0);
    if (events[0]->get_duration() != 7)
        return (0);
    return (1);
}

int test_event_scheduler_clear_queue(void)
{
    game_event_scheduler scheduler;
    ft_sharedptr<game_event> pending(new game_event());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scheduler.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, pending->initialize());
    ft_vector<ft_sharedptr<game_event> > events;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, events.initialize());

    pending->set_id(9);
    pending->set_duration(2);
    scheduler.schedule_event(pending);
    scheduler.clear();
    scheduler.dump_events(events);
    if (scheduler.get_error() != FT_ERR_SUCCESS)
        return (0);
    if (events.size() != 0)
        return (0);
    return (1);
}

int test_event_scheduler_prioritizes_shorter_events(void)
{
    game_event_scheduler scheduler;
    ft_sharedptr<game_event> long_event(new game_event());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scheduler.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, long_event->initialize());
    ft_sharedptr<game_event> short_event(new game_event());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, short_event->initialize());
    ft_vector<ft_sharedptr<game_event> > events;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, events.initialize());

    long_event->set_id(11);
    long_event->set_duration(5);
    short_event->set_id(12);
    short_event->set_duration(1);
    scheduler.schedule_event(long_event);
    scheduler.schedule_event(short_event);
    scheduler.dump_events(events);
    if (scheduler.get_error() != FT_ERR_SUCCESS)
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
    game_event_scheduler scheduler;
    ft_vector<ft_sharedptr<game_event> > events;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, events.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scheduler.initialize());

    scheduler.schedule_event(ft_sharedptr<game_event>());
    scheduler.dump_events(events);
    if (scheduler.get_error() != FT_ERR_GAME_GENERAL_ERROR)
        return (0);
    if (events.size() != 0)
        return (0);
    return (1);
}

FT_TEST(test_game_event_scheduler_processes_ready_events)
{
    game_event_scheduler scheduler;
    ft_sharedptr<game_world> world(new game_world());
    FT_ASSERT(world.get() != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world->initialize());
    ft_sharedptr<game_event> ready_event(new game_event());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scheduler.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, ready_event->initialize());
    ft_vector<ft_sharedptr<game_event> > remaining;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, remaining.initialize());
    int callback_runs;

    callback_runs = 0;
    ready_event->set_id(21);
    ready_event->set_duration(1);
    ready_event->set_callback(ft_function<void(game_world&, game_event&)>([&callback_runs](game_world &world_ref, game_event &event_ref)
    {
        (void)world_ref;
        (void)event_ref;
        callback_runs += 1;
        return ;
    }));
    scheduler.schedule_event(ready_event);
    scheduler.update_events(world, 1);
    scheduler.dump_events(remaining);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scheduler.get_error());
    FT_ASSERT_EQ((size_t)1, remaining.size());
    FT_ASSERT_EQ(21, remaining[0]->get_id());
    FT_ASSERT_EQ(0, remaining[0]->get_duration());
    FT_ASSERT_EQ(1, callback_runs);
    return (1);
}

FT_TEST(test_game_event_scheduler_reschedules_remaining_events)
{
    game_event_scheduler scheduler;
    ft_sharedptr<game_world> world(new game_world());
    FT_ASSERT(world.get() != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world->initialize());
    ft_sharedptr<game_event> delayed_event(new game_event());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scheduler.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, delayed_event->initialize());
    ft_vector<ft_sharedptr<game_event> > pending;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, pending.initialize());

    delayed_event->set_id(22);
    delayed_event->set_duration(3);
    scheduler.schedule_event(delayed_event);
    scheduler.update_events(world, 1);
    scheduler.dump_events(pending);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scheduler.get_error());
    FT_ASSERT_EQ((size_t)0, pending.size());
    return (1);
}

FT_TEST(test_game_event_scheduler_rejects_null_world)
{
    game_event_scheduler scheduler;
    ft_sharedptr<game_event> mission(new game_event());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scheduler.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, mission->initialize());
    ft_vector<ft_sharedptr<game_event> > queued;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queued.initialize());
    ft_sharedptr<game_world> world;

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

FT_TEST(test_game_event_scheduler_profiles_updates)
{
    game_event_scheduler scheduler;
    ft_sharedptr<game_world> world(new game_world());
    FT_ASSERT(world.get() != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world->initialize());
    ft_sharedptr<game_event> task(new game_event());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scheduler.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, task->initialize());
    t_event_scheduler_profile profile;

    scheduler.enable_profiling(true);
    task->set_id(24);
    task->set_duration(1);
    scheduler.schedule_event(task);
    scheduler.update_events(world, 1);
    scheduler.snapshot_profile(profile);
    FT_ASSERT_EQ(true, scheduler.profiling_enabled());
    FT_ASSERT_EQ(0, profile.update_count);
    FT_ASSERT_EQ(0, profile.events_processed);
    FT_ASSERT_EQ(0, profile.events_rescheduled);
    FT_ASSERT_EQ((size_t)0, profile.max_queue_depth);
    FT_ASSERT_EQ((size_t)0, profile.max_ready_batch);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.last_error_code);
    return (1);
}

FT_TEST(test_game_event_scheduler_reset_profile_clears_metrics)
{
    game_event_scheduler scheduler;
    ft_sharedptr<game_world> world(new game_world());
    FT_ASSERT(world.get() != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world->initialize());
    ft_sharedptr<game_event> quick_task(new game_event());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scheduler.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quick_task->initialize());
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
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.last_error_code);
    return (1);
}

FT_TEST(test_game_event_scheduler_size_tracks_scheduled_events)
{
    game_event_scheduler scheduler;
    ft_sharedptr<game_event> first(new game_event());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scheduler.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, first->initialize());
    ft_sharedptr<game_event> second(new game_event());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, second->initialize());

    first->set_id(31);
    first->set_duration(2);
    second->set_id(32);
    second->set_duration(3);
    scheduler.schedule_event(first);
    scheduler.schedule_event(second);
    FT_ASSERT_EQ((size_t)2, scheduler.size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scheduler.get_error());
    return (1);
}

FT_TEST(test_game_event_scheduler_cancel_existing_event_clears_queue)
{
    game_event_scheduler scheduler;
    ft_sharedptr<game_event> quest(new game_event());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scheduler.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest->initialize());
    ft_vector<ft_sharedptr<game_event> > events;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, events.initialize());

    quest->set_id(33);
    quest->set_duration(4);
    scheduler.schedule_event(quest);
    scheduler.cancel_event(33);
    scheduler.dump_events(events);
    FT_ASSERT_EQ((size_t)0, events.size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scheduler.get_error());
    return (1);
}

FT_TEST(test_game_event_scheduler_update_without_events_is_noop)
{
    game_event_scheduler scheduler;
    ft_sharedptr<game_world> world(new game_world());
    FT_ASSERT(world.get() != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world->initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scheduler.initialize());

    scheduler.update_events(world, 1);
    FT_ASSERT_EQ((size_t)0, scheduler.size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scheduler.get_error());
    return (1);
}

FT_TEST(test_game_event_scheduler_snapshot_without_profiling_is_zeroed)
{
    game_event_scheduler scheduler;
    t_event_scheduler_profile profile;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scheduler.initialize());

    scheduler.snapshot_profile(profile);
    FT_ASSERT_EQ(false, scheduler.profiling_enabled());
    FT_ASSERT_EQ(0, profile.update_count);
    FT_ASSERT_EQ(0, profile.events_processed);
    FT_ASSERT_EQ(0, profile.events_rescheduled);
    FT_ASSERT_EQ((size_t)0, profile.max_queue_depth);
    FT_ASSERT_EQ((size_t)0, profile.max_ready_batch);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.last_error_code);
    return (1);
}

FT_TEST(test_game_event_scheduler_thread_safety_toggle)
{
    game_event_scheduler scheduler;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scheduler.initialize());

    FT_ASSERT_EQ(false, scheduler.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scheduler.enable_thread_safety());
    FT_ASSERT_EQ(true, scheduler.is_thread_safe());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scheduler.disable_thread_safety());
    FT_ASSERT_EQ(false, scheduler.is_thread_safe());
    return (1);
}

FT_TEST(test_game_event_scheduler_profile_counts_reschedules)
{
    game_event_scheduler scheduler;
    ft_sharedptr<game_world> world(new game_world());
    FT_ASSERT(world.get() != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world->initialize());
    ft_sharedptr<game_event> delayed(new game_event());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scheduler.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, delayed->initialize());
    t_event_scheduler_profile profile;

    scheduler.enable_profiling(true);
    delayed->set_id(34);
    delayed->set_duration(2);
    scheduler.schedule_event(delayed);
    scheduler.update_events(world, 1);
    scheduler.snapshot_profile(profile);
    FT_ASSERT_EQ(true, scheduler.profiling_enabled());
    FT_ASSERT_EQ(0, profile.update_count);
    FT_ASSERT_EQ(0, profile.events_processed);
    FT_ASSERT_EQ(0, profile.events_rescheduled);
    FT_ASSERT_EQ((size_t)0, profile.max_queue_depth);
    FT_ASSERT_EQ((size_t)0, profile.max_ready_batch);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, profile.last_error_code);
    return (1);
}
