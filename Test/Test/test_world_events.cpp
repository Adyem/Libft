#include "../test_internal.hpp"
#include "../../Game/game_world.hpp"
#include "../../Game/game_event.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"
#include "../../Template/vector.hpp"
#include "../../Template/shared_ptr.hpp"

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
        events.destroy();
        return (0);
    }

    world->update_events(world, 2);
    events.clear();
    world->get_event_scheduler()->dump_events(events);
    if (events.size() != 0)
    {
        events.destroy();
        return (0);
    }
    FT_ASSERT_EQ(FT_ERR_SUCCESS, events.destroy());

    return (1);
}
