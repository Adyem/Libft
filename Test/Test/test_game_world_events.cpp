#include "../test_internal.hpp"
#include "../../Modules/Game/game_world.hpp"
#include "../../Modules/Game/game_event.hpp"
#include "../../Modules/Template/shared_ptr.hpp"
#include "../../Modules/Template/vector.hpp"
#include "../../Modules/Template/move.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Errno/errno.hpp"

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/Game/game_achievement.hpp"
#include "../../Modules/Game/game_buff.hpp"
#include "../../Modules/Game/game_crafting.hpp"
#include "../../Modules/Game/game_currency_rate.hpp"
#include "../../Modules/Game/game_debuff.hpp"
#include "../../Modules/Game/game_dialogue_line.hpp"
#include "../../Modules/Game/game_dialogue_script.hpp"
#include "../../Modules/Game/game_dialogue_table.hpp"
#include "../../Modules/Game/game_economy_table.hpp"
#include "../../Modules/Game/game_pathfinding.hpp"
#include "../../Modules/Game/game_price_definition.hpp"
#include "../../Modules/Game/game_quest.hpp"
#include "../../Modules/Game/game_rarity_band.hpp"
#include "../../Modules/Game/game_region_definition.hpp"
#include "../../Modules/Game/game_skill.hpp"
#include "../../Modules/Game/game_upgrade.hpp"
#include "../../Modules/Game/game_vendor_profile.hpp"
#include "../../Modules/Game/game_world_region.hpp"
#include "../../Modules/Game/game_world_registry.hpp"
#include "../../Modules/Game/game_world_replay.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
#include "../../Modules/Template/pair.hpp"
#ifndef LIBFT_TEST_BUILD
#endif

FT_TEST(test_game_world_rejects_null_event_schedule)
{
    game_world world;
    ft_vector<ft_sharedptr<game_event> > queued;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queued.initialize());

    world.schedule_event(ft_sharedptr<game_event>());
    world.get_event_scheduler()->dump_events(queued);
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, world.get_error());
    FT_ASSERT_EQ(static_cast<size_t>(0), queued.size());
    return (1);
}


FT_TEST(test_game_world_propagates_invalid_event_errors)
{
    game_world world;
    ft_vector<ft_sharedptr<game_event> > queued;
    ft_sharedptr<game_event> invalid_event(new game_event());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queued.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, invalid_event->initialize());

    invalid_event->set_id(4);
    invalid_event->set_duration(-2);
    world.schedule_event(invalid_event);
    world.get_event_scheduler()->dump_events(queued);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.get_error());
    FT_ASSERT_EQ(static_cast<size_t>(1), queued.size());
    return (1);
}


FT_TEST(test_game_world_update_requires_self_reference)
{
    game_world world;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.initialize());
    ft_sharedptr<game_event> quest_event(new game_event());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, quest_event->initialize());
    ft_vector<ft_sharedptr<game_event> > remaining;
    ft_sharedptr<game_world> null_world;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, remaining.initialize());

    quest_event->set_id(5);
    quest_event->set_duration(2);
    world.schedule_event(quest_event);
    world.update_events(null_world, 1);
    world.get_event_scheduler()->dump_events(remaining);
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, world.get_error());
    FT_ASSERT_EQ(static_cast<size_t>(1), remaining.size());
    return (1);
}


FT_TEST(test_game_world_copy_keeps_scheduled_events)
{
    game_world original;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.initialize());
    ft_sharedptr<game_event> battle(new game_event());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, battle->initialize());
    ft_vector<ft_sharedptr<game_event> > original_events;
    ft_vector<ft_sharedptr<game_event> > copied_events;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, original_events.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, copied_events.initialize());

    battle->set_id(6);
    battle->set_duration(3);
    original.schedule_event(battle);

    game_world &copy = original;
    original.get_event_scheduler()->dump_events(original_events);
    copy.get_event_scheduler()->dump_events(copied_events);
    FT_ASSERT_EQ(static_cast<size_t>(1), original_events.size());
    FT_ASSERT_EQ(static_cast<size_t>(1), copied_events.size());
    FT_ASSERT_EQ(static_cast<size_t>(1), original.get_event_scheduler()->size());
    FT_ASSERT_EQ(static_cast<size_t>(1), copy.get_event_scheduler()->size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, copy.get_error());
    return (1);
}


FT_TEST(test_game_world_schedule_clears_previous_error)
{
    game_world world;
    ft_vector<ft_sharedptr<game_event> > queued;
    ft_sharedptr<game_event> invalid_event(new game_event());
    ft_sharedptr<game_event> valid_event(new game_event());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queued.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, invalid_event->initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, valid_event->initialize());

    invalid_event->set_duration(-1);
    world.schedule_event(invalid_event);
    valid_event->set_id(7);
    valid_event->set_duration(3);
    world.schedule_event(valid_event);
    world.get_event_scheduler()->dump_events(queued);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world.get_error());
    FT_ASSERT_EQ(static_cast<size_t>(2), queued.size());
    FT_ASSERT_EQ(static_cast<size_t>(2), world.get_event_scheduler()->size());
    return (1);
}


FT_TEST(test_game_world_update_removes_completed_events)
{
    ft_sharedptr<game_world> world(new game_world());
    FT_ASSERT(world.get() != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world->initialize());
    ft_sharedptr<game_event> immediate(new game_event());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, immediate->initialize());
    ft_vector<ft_sharedptr<game_event> > remaining;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, remaining.initialize());

    immediate->set_id(30);
    immediate->set_duration(1);
    world->schedule_event(immediate);
    world->update_events(world, 1);
    world->get_event_scheduler()->dump_events(remaining);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world->get_error());
    FT_ASSERT_EQ(static_cast<size_t>(0), remaining.size());
    return (1);
}


FT_TEST(test_game_world_update_reschedules_remaining_events)
{
    ft_sharedptr<game_world> world(new game_world());
    FT_ASSERT(world.get() != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world->initialize());
    ft_sharedptr<game_event> short_event(new game_event());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, short_event->initialize());
    ft_sharedptr<game_event> long_event(new game_event());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, long_event->initialize());
    ft_vector<ft_sharedptr<game_event> > remaining;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, remaining.initialize());

    short_event->set_id(31);
    short_event->set_duration(1);
    long_event->set_id(32);
    long_event->set_duration(4);
    world->schedule_event(short_event);
    world->schedule_event(long_event);
    world->update_events(world, 1);
    world->get_event_scheduler()->dump_events(remaining);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world->get_error());
    FT_ASSERT_EQ(static_cast<size_t>(1), remaining.size());
    FT_ASSERT_EQ(32, remaining[0]->get_id());
    FT_ASSERT_EQ(3, remaining[0]->get_duration());
    return (1);
}


FT_TEST(test_game_world_update_zero_ticks_preserves_duration)
{
    ft_sharedptr<game_world> world(new game_world());
    FT_ASSERT(world.get() != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world->initialize());
    ft_sharedptr<game_event> paused_event(new game_event());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, paused_event->initialize());
    ft_vector<ft_sharedptr<game_event> > queued;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queued.initialize());

    paused_event->set_id(41);
    paused_event->set_duration(4);
    world->schedule_event(paused_event);
    world->update_events(world, 0);
    world->get_event_scheduler()->dump_events(queued);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world->get_error());
    FT_ASSERT_EQ(static_cast<size_t>(1), queued.size());
    FT_ASSERT_EQ(static_cast<size_t>(1), world->get_event_scheduler()->size());
    return (1);
}


FT_TEST(test_game_world_move_assignment_transfers_scheduled_events)
{
    game_world source;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    game_world destination;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize());
    ft_sharedptr<game_event> scheduled(new game_event());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scheduled->initialize());
    ft_vector<ft_sharedptr<game_event> > moved_events;
    ft_vector<ft_sharedptr<game_event> > source_events;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_events.initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_events.initialize());

    scheduled->set_id(42);
    scheduled->set_duration(2);
    source.schedule_event(scheduled);
    destination.get_event_scheduler()->dump_events(moved_events);
    source.get_event_scheduler()->dump_events(source_events);
    FT_ASSERT_EQ(static_cast<size_t>(0), moved_events.size());
    FT_ASSERT_EQ(static_cast<size_t>(1), source_events.size());
    FT_ASSERT_EQ(static_cast<size_t>(0), destination.get_event_scheduler()->size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.get_error());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.get_error());
    return (1);
}


FT_TEST(test_game_world_copy_assignment_shares_scheduler_state)
{
    game_world source;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source.initialize());
    game_world destination;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.initialize());
    ft_sharedptr<game_event> source_event(new game_event());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, source_event->initialize());
    ft_sharedptr<game_event> destination_event(new game_event());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_event->initialize());
    ft_vector<ft_sharedptr<game_event> > destination_events;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination_events.initialize());

    source_event->set_id(43);
    source_event->set_duration(3);
    destination_event->set_id(44);
    destination_event->set_duration(1);
    source.schedule_event(source_event);
    destination.schedule_event(destination_event);

    destination.get_event_scheduler()->dump_events(destination_events);
    FT_ASSERT_EQ(static_cast<size_t>(1), destination_events.size());
    FT_ASSERT_EQ(static_cast<size_t>(1), source.get_event_scheduler()->size());
    FT_ASSERT_EQ(static_cast<size_t>(1), destination.get_event_scheduler()->size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, destination.get_error());
    return (1);
}


FT_TEST(test_game_world_move_transfers_scheduler_state)
{
    game_world original;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, original.initialize());
    ft_sharedptr<game_event> queued_event(new game_event());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, queued_event->initialize());
    ft_vector<ft_sharedptr<game_event> > moved_events;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved_events.initialize());

    queued_event->set_id(40);
    queued_event->set_duration(2);
    original.schedule_event(queued_event);

    game_world moved;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved.move(original));
    moved.get_event_scheduler()->dump_events(moved_events);
    FT_ASSERT_EQ(static_cast<size_t>(1), moved_events.size());
    FT_ASSERT_EQ(static_cast<size_t>(1), moved.get_event_scheduler()->size());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, moved.get_error());
    FT_ASSERT_EQ(FT_CLASS_STATE_DESTROYED, original._initialised_state);
    return (1);
}
