#include "../test_internal.hpp"
#include "../../Modules/Game/game_world.hpp"
#include "../../Modules/Game/game_event.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"
#include "../../Modules/Template/vector.hpp"
#include "../../Modules/Template/shared_ptr.hpp"

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Basic/limits.hpp"
#include "../../Modules/Errno/errno.hpp"
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

FT_TEST(test_world_process_events)
{
    ft_sharedptr<game_world> world(new game_world());
    FT_ASSERT(world.get() != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, world->initialize());
    ft_sharedptr<game_event> event(new game_event());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, event->initialize());
    event->set_id(1);
    event->set_duration(3);
    world->schedule_event(event);

    world->update_events(world, 1);
    ft_vector<ft_sharedptr<game_event> > events;
    FT_ASSERT_EQ(FT_ERR_SUCCESS, events.initialize());
    world->get_event_scheduler()->dump_events(events);
    if (events.size() != 1 || events[0]->get_duration() != 2)
    {
        FT_ASSERT_EQ(FT_ERR_SUCCESS, events.destroy());
        return (0);
    }

    world->update_events(world, 2);
    events.clear();
    world->get_event_scheduler()->dump_events(events);
    if (events.size() != 0)
    {
        FT_ASSERT_EQ(FT_ERR_SUCCESS, events.destroy());
        return (0);
    }
    FT_ASSERT_EQ(FT_ERR_SUCCESS, events.destroy());

    return (1);
}
