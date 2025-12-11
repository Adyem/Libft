#include "../../Game/game_world.hpp"
#include "../../Game/game_event.hpp"
#include "../../Template/shared_ptr.hpp"
#include "../../Template/vector.hpp"
#include "../../Template/move.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

FT_TEST(test_game_world_rejects_null_event_schedule, "Game world rejects scheduling null events")
{
    ft_world world;
    ft_vector<ft_sharedptr<ft_event> > queued;

    world.schedule_event(ft_sharedptr<ft_event>());
    world.get_event_scheduler()->dump_events(queued);
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, world.get_error());
    FT_ASSERT_EQ((size_t)0, queued.size());
    return (1);
}


FT_TEST(test_game_world_propagates_invalid_event_errors, "Game world refuses events already in error state")
{
    ft_world world;
    ft_vector<ft_sharedptr<ft_event> > queued;
    ft_sharedptr<ft_event> invalid_event(new ft_event());

    invalid_event->set_id(4);
    invalid_event->set_duration(-2);
    world.schedule_event(invalid_event);
    world.get_event_scheduler()->dump_events(queued);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, world.get_error());
    FT_ASSERT_EQ((size_t)0, queued.size());
    return (1);
}


FT_TEST(test_game_world_update_requires_self_reference, "Game world update requires a valid world reference")
{
    ft_world world;
    ft_sharedptr<ft_event> quest_event(new ft_event());
    ft_vector<ft_sharedptr<ft_event> > remaining;
    ft_sharedptr<ft_world> null_world;

    quest_event->set_id(5);
    quest_event->set_duration(2);
    world.schedule_event(quest_event);
    world.update_events(null_world, 1);
    world.get_event_scheduler()->dump_events(remaining);
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, world.get_error());
    FT_ASSERT_EQ((size_t)1, remaining.size());
    FT_ASSERT_EQ(2, remaining[0]->get_duration());
    return (1);
}


FT_TEST(test_game_world_copy_keeps_scheduled_events, "Game world copy constructor retains scheduled events")
{
    ft_world original;
    ft_sharedptr<ft_event> battle(new ft_event());
    ft_vector<ft_sharedptr<ft_event> > original_events;
    ft_vector<ft_sharedptr<ft_event> > copied_events;

    battle->set_id(6);
    battle->set_duration(3);
    original.schedule_event(battle);

    ft_world copy(original);
    original.get_event_scheduler()->dump_events(original_events);
    copy.get_event_scheduler()->dump_events(copied_events);
    FT_ASSERT_EQ((size_t)1, original_events.size());
    FT_ASSERT_EQ((size_t)1, copied_events.size());
    FT_ASSERT_EQ(6, original_events[0]->get_id());
    FT_ASSERT_EQ(6, copied_events[0]->get_id());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, original.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, copy.get_error());
    return (1);
}


FT_TEST(test_game_world_schedule_clears_previous_error, "Game world resets error state after successful scheduling")
{
    ft_world world;
    ft_vector<ft_sharedptr<ft_event> > queued;
    ft_sharedptr<ft_event> invalid_event(new ft_event());
    ft_sharedptr<ft_event> valid_event(new ft_event());

    invalid_event->set_duration(-1);
    world.schedule_event(invalid_event);
    valid_event->set_id(7);
    valid_event->set_duration(3);
    world.schedule_event(valid_event);
    world.get_event_scheduler()->dump_events(queued);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, world.get_error());
    FT_ASSERT_EQ((size_t)1, queued.size());
    FT_ASSERT_EQ(7, queued[0]->get_id());
    FT_ASSERT_EQ(3, queued[0]->get_duration());
    return (1);
}


FT_TEST(test_game_world_update_removes_completed_events, "Game world removes events that finish during update")
{
    ft_sharedptr<ft_world> world(new ft_world());
    ft_sharedptr<ft_event> immediate(new ft_event());
    ft_vector<ft_sharedptr<ft_event> > remaining;

    immediate->set_id(30);
    immediate->set_duration(1);
    world->schedule_event(immediate);
    world->update_events(world, 1);
    world->get_event_scheduler()->dump_events(remaining);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, world->get_error());
    FT_ASSERT_EQ((size_t)0, remaining.size());
    return (1);
}


FT_TEST(test_game_world_update_reschedules_remaining_events, "Game world keeps longer events queued after ticking")
{
    ft_sharedptr<ft_world> world(new ft_world());
    ft_sharedptr<ft_event> short_event(new ft_event());
    ft_sharedptr<ft_event> long_event(new ft_event());
    ft_vector<ft_sharedptr<ft_event> > remaining;

    short_event->set_id(31);
    short_event->set_duration(1);
    long_event->set_id(32);
    long_event->set_duration(4);
    world->schedule_event(short_event);
    world->schedule_event(long_event);
    world->update_events(world, 1);
    world->get_event_scheduler()->dump_events(remaining);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, world->get_error());
    FT_ASSERT_EQ((size_t)1, remaining.size());
    FT_ASSERT_EQ(32, remaining[0]->get_id());
    FT_ASSERT_EQ(3, remaining[0]->get_duration());
    return (1);
}


FT_TEST(test_game_world_update_zero_ticks_preserves_duration, "Game world keeps event duration unchanged when ticking zero")
{
    ft_sharedptr<ft_world> world(new ft_world());
    ft_sharedptr<ft_event> paused_event(new ft_event());
    ft_vector<ft_sharedptr<ft_event> > queued;

    paused_event->set_id(41);
    paused_event->set_duration(4);
    world->schedule_event(paused_event);
    world->update_events(world, 0);
    world->get_event_scheduler()->dump_events(queued);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, world->get_error());
    FT_ASSERT_EQ((size_t)1, queued.size());
    FT_ASSERT_EQ(4, queued[0]->get_duration());
    return (1);
}


FT_TEST(test_game_world_move_assignment_transfers_scheduled_events, "Game world move assignment moves queued events and resets source")
{
    ft_world source;
    ft_world destination;
    ft_sharedptr<ft_event> scheduled(new ft_event());
    ft_vector<ft_sharedptr<ft_event> > moved_events;
    ft_vector<ft_sharedptr<ft_event> > source_events;

    scheduled->set_id(42);
    scheduled->set_duration(2);
    source.schedule_event(scheduled);
    destination = ft_move(source);
    destination.get_event_scheduler()->dump_events(moved_events);
    source.get_event_scheduler()->dump_events(source_events);
    FT_ASSERT_EQ((size_t)1, moved_events.size());
    FT_ASSERT_EQ(42, moved_events[0]->get_id());
    FT_ASSERT_EQ((size_t)0, source_events.size());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, destination.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, source.get_error());
    return (1);
}


FT_TEST(test_game_world_copy_assignment_shares_scheduler_state, "Game world copy assignment points to the same scheduled events")
{
    ft_world source;
    ft_world destination;
    ft_sharedptr<ft_event> source_event(new ft_event());
    ft_sharedptr<ft_event> destination_event(new ft_event());
    ft_vector<ft_sharedptr<ft_event> > destination_events;

    source_event->set_id(43);
    source_event->set_duration(3);
    destination_event->set_id(44);
    destination_event->set_duration(1);
    source.schedule_event(source_event);
    destination.schedule_event(destination_event);

    destination = source;
    destination.get_event_scheduler()->dump_events(destination_events);
    FT_ASSERT_EQ((size_t)1, destination_events.size());
    FT_ASSERT_EQ(43, destination_events[0]->get_id());
    FT_ASSERT_EQ(3, destination_events[0]->get_duration());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, destination.get_error());
    return (1);
}


FT_TEST(test_game_world_move_transfers_scheduler_state, "Game world move constructor transfers queued events")
{
    ft_world original;
    ft_sharedptr<ft_event> queued_event(new ft_event());
    ft_vector<ft_sharedptr<ft_event> > moved_events;

    queued_event->set_id(40);
    queued_event->set_duration(2);
    original.schedule_event(queued_event);

    ft_world moved(ft_move(original));
    moved.get_event_scheduler()->dump_events(moved_events);
    FT_ASSERT_EQ((size_t)1, moved_events.size());
    FT_ASSERT_EQ(40, moved_events[0]->get_id());
    FT_ASSERT_EQ(2, moved_events[0]->get_duration());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, moved.get_error());
    return (1);
}


