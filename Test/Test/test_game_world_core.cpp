#include "../../Game/game_world.hpp"
#include "../../Game/game_event.hpp"
#include "../../Game/game_quest.hpp"
#include "../../Game/game_achievement.hpp"
#include "../../Game/game_economy_table.hpp"
#include "../../Game/ft_price_definition.hpp"
#include "../../Game/ft_vendor_profile.hpp"
#include "../../Game/ft_currency_rate.hpp"
#include "../../Game/game_crafting.hpp"
#include "../../Game/game_dialogue_table.hpp"
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
    FT_ASSERT_EQ(ER_SUCCESS, original.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, copy.get_error());
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
    FT_ASSERT_EQ(ER_SUCCESS, world.get_error());
    FT_ASSERT_EQ((size_t)1, queued.size());
    FT_ASSERT_EQ(7, queued[0]->get_id());
    FT_ASSERT_EQ(3, queued[0]->get_duration());
    return (1);
}

FT_TEST(test_game_quest_advance_progress_to_completion, "Game quest advances phases until completion")
{
    ft_quest quest;

    quest.set_phases(2);
    quest.set_current_phase(0);
    quest.advance_phase();
    FT_ASSERT_EQ(1, quest.get_current_phase());
    FT_ASSERT_EQ(false, quest.is_complete());
    FT_ASSERT_EQ(ER_SUCCESS, quest.get_error());

    quest.advance_phase();
    FT_ASSERT_EQ(2, quest.get_current_phase());
    FT_ASSERT_EQ(true, quest.is_complete());
    FT_ASSERT_EQ(ER_SUCCESS, quest.get_error());
    return (1);
}

FT_TEST(test_game_quest_invalid_and_overflow_phases_report_errors, "Game quest validates phase advancement bounds")
{
    ft_quest quest;

    quest.set_phases(0);
    quest.set_current_phase(1);
    ft_errno = ER_SUCCESS;
    quest.advance_phase();
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, quest.get_error());
    FT_ASSERT_EQ(0, quest.get_current_phase());

    quest.set_phases(1);
    quest.set_current_phase(1);
    ft_errno = ER_SUCCESS;
    quest.advance_phase();
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, quest.get_error());
    FT_ASSERT_EQ(1, quest.get_current_phase());
    return (1);
}

FT_TEST(test_game_quest_rejects_out_of_range_phase, "Game quest blocks setting invalid current phase")
{
    ft_quest quest;

    quest.set_phases(2);
    quest.set_current_phase(3);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, quest.get_error());
    FT_ASSERT_EQ(0, quest.get_current_phase());
    return (1);
}

FT_TEST(test_game_quest_clamps_current_phase_when_phases_shrink, "Game quest clamps current phase when reducing total phases")
{
    ft_quest quest;

    quest.set_phases(4);
    quest.set_current_phase(3);
    FT_ASSERT_EQ(ER_SUCCESS, quest.get_error());
    quest.set_phases(2);
    FT_ASSERT_EQ(2, quest.get_current_phase());
    FT_ASSERT_EQ(ER_SUCCESS, quest.get_error());
    return (1);
}

FT_TEST(test_game_achievement_rejects_negative_goal_id, "Game achievement refuses negative goal identifiers")
{
    ft_achievement achievement;

    achievement.set_goal(-5, 10);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, achievement.get_error());
    FT_ASSERT_EQ(0, achievement.get_goal(1));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, achievement.get_error());
    return (1);
}

FT_TEST(test_game_achievement_progress_creation_for_new_goal, "Game achievement creates goal when progress assigned")
{
    ft_achievement achievement;

    achievement.set_progress(2, 6);
    FT_ASSERT_EQ(0, achievement.get_goal(2));
    FT_ASSERT_EQ(6, achievement.get_progress(2));
    FT_ASSERT_EQ(true, achievement.is_goal_complete(2));
    FT_ASSERT_EQ(ER_SUCCESS, achievement.get_error());
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
    FT_ASSERT_EQ(ER_SUCCESS, world->get_error());
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
    FT_ASSERT_EQ(ER_SUCCESS, world->get_error());
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
    FT_ASSERT_EQ(ER_SUCCESS, world->get_error());
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
    FT_ASSERT_EQ(ER_SUCCESS, destination.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, source.get_error());
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
    FT_ASSERT_EQ(ER_SUCCESS, destination.get_error());
    return (1);
}

FT_TEST(test_game_achievement_complete_with_no_goals, "Game achievement treats empty goal set as complete")
{
    ft_achievement achievement;

    FT_ASSERT_EQ(true, achievement.is_complete());
    FT_ASSERT_EQ(ER_SUCCESS, achievement.get_error());
    return (1);
}

FT_TEST(test_game_achievement_rejects_negative_progress_goal, "Game achievement add_progress blocks negative goal identifiers")
{
    ft_achievement achievement;

    achievement.add_progress(-3, 5);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, achievement.get_error());
    FT_ASSERT_EQ(false, achievement.is_goal_complete(1));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, achievement.get_error());
    return (1);
}

FT_TEST(test_game_goal_handles_negative_delta, "Game goal supports reducing progress with negative deltas")
{
    ft_goal goal;

    goal.set_target(5);
    goal.set_progress(3);
    goal.add_progress(-2);
    FT_ASSERT_EQ(1, goal.get_progress());
    FT_ASSERT_EQ(ER_SUCCESS, goal.get_error());
    return (1);
}

FT_TEST(test_game_quest_copy_preserves_configuration, "Game quest copy constructor keeps configured fields")
{
    ft_quest quest;

    quest.set_id(8);
    quest.set_phases(3);
    quest.set_current_phase(1);
    quest.set_description("Find the relic");
    quest.set_objective("Reach the temple");
    quest.set_reward_experience(50);

    ft_quest copy(quest);
    FT_ASSERT_EQ(8, copy.get_id());
    FT_ASSERT_EQ(3, copy.get_phases());
    FT_ASSERT_EQ(1, copy.get_current_phase());
    FT_ASSERT_EQ(50, copy.get_reward_experience());
    FT_ASSERT_EQ(ER_SUCCESS, copy.get_error());
    return (1);
}

FT_TEST(test_game_achievement_completion_requires_all_goals, "Game achievement completes only when all goals finish")
{
    ft_achievement achievement;

    achievement.set_goal(10, 5);
    achievement.set_goal(11, 2);
    achievement.set_progress(10, 5);
    achievement.set_progress(11, 1);
    FT_ASSERT_EQ(false, achievement.is_goal_complete(11));
    FT_ASSERT_EQ(false, achievement.is_complete());
    achievement.add_progress(11, 1);
    FT_ASSERT_EQ(true, achievement.is_goal_complete(11));
    FT_ASSERT_EQ(true, achievement.is_complete());
    FT_ASSERT_EQ(ER_SUCCESS, achievement.get_error());
    return (1);
}

FT_TEST(test_game_goal_accumulates_progress, "Game goal accumulates progress toward its target")
{
    ft_goal goal;

    goal.set_target(4);
    goal.set_progress(1);
    goal.add_progress(2);
    FT_ASSERT_EQ(3, goal.get_progress());
    goal.add_progress(1);
    FT_ASSERT_EQ(4, goal.get_progress());
    FT_ASSERT_EQ(ER_SUCCESS, goal.get_error());
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
    FT_ASSERT_EQ(ER_SUCCESS, moved.get_error());
    return (1);
}

FT_TEST(test_game_quest_clears_error_after_valid_phase_set, "Game quest resets error when setting a valid phase")
{
    ft_quest quest;

    quest.set_phases(2);
    quest.set_current_phase(3);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, quest.get_error());
    quest.set_current_phase(1);
    FT_ASSERT_EQ(1, quest.get_current_phase());
    FT_ASSERT_EQ(ER_SUCCESS, quest.get_error());
    return (1);
}

FT_TEST(test_game_quest_rejects_negative_phase_counts, "Game quest retains previous phases when rejecting negatives")
{
    ft_quest quest;

    quest.set_phases(2);
    quest.set_current_phase(1);
    quest.set_phases(-1);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, quest.get_error());
    FT_ASSERT_EQ(2, quest.get_phases());
    FT_ASSERT_EQ(1, quest.get_current_phase());
    return (1);
}

FT_TEST(test_game_achievement_updates_existing_goal_target, "Game achievement updates goal targets without resetting progress")
{
    ft_achievement achievement;

    achievement.set_goal(12, 3);
    achievement.set_progress(12, 2);
    achievement.set_goal(12, 5);
    FT_ASSERT_EQ(5, achievement.get_goal(12));
    FT_ASSERT_EQ(ER_SUCCESS, achievement.get_error());
    FT_ASSERT_EQ(2, achievement.get_progress(12));
    FT_ASSERT_EQ(ER_SUCCESS, achievement.get_error());
    return (1);
}

FT_TEST(test_game_achievement_rejects_negative_progress_updates, "Game achievement preserves existing goals after invalid progress id")
{
    ft_achievement achievement;

    achievement.set_progress(13, 3);
    achievement.set_progress(-2, 5);
    FT_ASSERT_EQ(FT_ERR_INVALID_ARGUMENT, achievement.get_error());
    FT_ASSERT_EQ(3, achievement.get_progress(13));
    FT_ASSERT_EQ(ER_SUCCESS, achievement.get_error());
    return (1);
}

FT_TEST(test_game_achievement_reports_missing_goal_completion, "Game achievement reports missing goals as incomplete")
{
    ft_achievement achievement;

    achievement.set_goal(14, 2);
    achievement.set_progress(14, 2);
    FT_ASSERT_EQ(false, achievement.is_goal_complete(15));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, achievement.get_error());
    FT_ASSERT_EQ(true, achievement.is_goal_complete(14));
    FT_ASSERT_EQ(ER_SUCCESS, achievement.get_error());
    return (1);
}

FT_TEST(test_game_goal_copy_preserves_values, "Game goal copies retain progress and target")
{
    ft_goal original;

    original.set_target(7);
    original.set_progress(3);
    ft_goal copied(original);
    FT_ASSERT_EQ(7, copied.get_target());
    FT_ASSERT_EQ(3, copied.get_progress());
    FT_ASSERT_EQ(ER_SUCCESS, copied.get_error());
    return (1);
}

FT_TEST(test_game_goal_assignment_overwrites_previous_values, "Game goal copy assignment replaces target and progress")
{
    ft_goal source;
    ft_goal destination;

    source.set_target(6);
    source.set_progress(5);
    destination.set_target(1);
    destination.set_progress(0);
    destination = source;
    FT_ASSERT_EQ(6, destination.get_target());
    FT_ASSERT_EQ(5, destination.get_progress());
    FT_ASSERT_EQ(ER_SUCCESS, destination.get_error());
    FT_ASSERT_EQ(6, source.get_target());
    FT_ASSERT_EQ(5, source.get_progress());
    return (1);
}

FT_TEST(test_game_goal_move_resets_source, "Game goal move assignment transfers values and clears origin")
{
    ft_goal source;
    ft_goal destination;

    source.set_target(9);
    source.set_progress(4);
    destination = ft_move(source);
    FT_ASSERT_EQ(9, destination.get_target());
    FT_ASSERT_EQ(4, destination.get_progress());
    FT_ASSERT_EQ(ER_SUCCESS, destination.get_error());
    FT_ASSERT_EQ(0, source.get_target());
    FT_ASSERT_EQ(0, source.get_progress());
    FT_ASSERT_EQ(ER_SUCCESS, source.get_error());
    return (1);
}

FT_TEST(test_game_quest_move_assignment_resets_source, "Game quest move assignment moves data and clears origin")
{
    ft_quest source;
    ft_quest destination;

    source.set_id(15);
    source.set_phases(5);
    source.set_current_phase(2);
    source.set_description("Recover the crown");
    source.set_objective("Reach the throne room");
    source.set_reward_experience(120);
    destination.set_id(3);
    destination.set_phases(1);
    destination.set_current_phase(1);

    destination = ft_move(source);
    FT_ASSERT_EQ(15, destination.get_id());
    FT_ASSERT_EQ(5, destination.get_phases());
    FT_ASSERT_EQ(2, destination.get_current_phase());
    FT_ASSERT_EQ(ft_string("Recover the crown"), destination.get_description());
    FT_ASSERT_EQ(ft_string("Reach the throne room"), destination.get_objective());
    FT_ASSERT_EQ(120, destination.get_reward_experience());
    FT_ASSERT_EQ(0, source.get_id());
    FT_ASSERT_EQ(0, source.get_phases());
    FT_ASSERT_EQ(0, source.get_current_phase());
    FT_ASSERT_EQ(ft_string(""), source.get_description());
    FT_ASSERT_EQ(ft_string(""), source.get_objective());
    FT_ASSERT_EQ(0, source.get_reward_experience());
    FT_ASSERT_EQ(ER_SUCCESS, destination.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, source.get_error());
    return (1);
}

FT_TEST(test_game_achievement_move_assignment_clears_goals, "Game achievement move assignment transfers goals and resets origin")
{
    ft_achievement source;
    ft_achievement destination;

    source.set_id(21);
    source.set_goal(2, 6);
    source.set_progress(2, 3);
    destination.set_id(4);
    destination.set_goal(1, 2);

    destination = ft_move(source);
    FT_ASSERT_EQ(21, destination.get_id());
    FT_ASSERT_EQ(6, destination.get_goal(2));
    FT_ASSERT_EQ(3, destination.get_progress(2));
    FT_ASSERT_EQ(false, destination.is_goal_complete(2));
    FT_ASSERT_EQ(0, source.get_id());
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, source.get_goal(2));
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, source.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, destination.get_error());
    return (1);
}

FT_TEST(test_game_world_exposes_economy_crafting_and_dialogue, "Game world provides economy, crafting, and dialogue accessors")
{
    ft_world world;

    FT_ASSERT_NE((void *)ft_nullptr, world.get_economy_table().get());
    FT_ASSERT_NE((void *)ft_nullptr, world.get_crafting().get());
    FT_ASSERT_NE((void *)ft_nullptr, world.get_dialogue_table().get());
    FT_ASSERT_EQ(ER_SUCCESS, world.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, world.get_economy_table().get_error());
    FT_ASSERT_EQ(ER_SUCCESS, world.get_crafting().get_error());
    FT_ASSERT_EQ(ER_SUCCESS, world.get_dialogue_table().get_error());
    return (1);
}

FT_TEST(test_game_world_registers_economy_entries_through_shared_tables, "Game world registers vendor and currency data through economy table")
{
    ft_world world;
    ft_price_definition potion_price;
    ft_vendor_profile vendor_profile;
    ft_currency_rate gold_rate;

    potion_price.set_item_id(42);
    potion_price.set_rarity(3);
    potion_price.set_base_value(150);
    potion_price.set_minimum_value(75);
    potion_price.set_maximum_value(300);
    vendor_profile.set_vendor_id(12);
    vendor_profile.set_buy_markup(1.2);
    vendor_profile.set_sell_multiplier(0.6);
    vendor_profile.set_tax_rate(0.08);
    gold_rate.set_currency_id(1);
    gold_rate.set_rate_to_base(1.0);
    gold_rate.set_display_precision(2);

    world.get_economy_table()->register_price_definition(potion_price);
    world.get_economy_table()->register_vendor_profile(vendor_profile);
    world.get_economy_table()->register_currency_rate(gold_rate);
    FT_ASSERT_EQ(ER_SUCCESS, world.get_error());
    FT_ASSERT_EQ(ER_SUCCESS, world.get_economy_table()->get_error());
    FT_ASSERT_EQ(ER_SUCCESS, world.get_economy_table().get_error());
    return (1);
}

FT_TEST(test_game_world_propagates_null_shared_components, "Game world reports errors when shared components are missing")
{
    ft_world world;

    world.get_economy_table().reset();
    world.get_crafting().reset();
    world.get_dialogue_table().reset();
    world.get_economy_table();
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, world.get_error());
    world.get_crafting();
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, world.get_error());
    world.get_dialogue_table();
    FT_ASSERT_EQ(FT_ERR_GAME_GENERAL_ERROR, world.get_error());
    return (1);
}
