#include "../../Game/game_world.hpp"
#include "../../Game/game_event.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Template/vector.hpp"
#include "../../Template/shared_ptr.hpp"

FT_TEST(test_world_process_events, "ft_world event expiration")
{
    ft_sharedptr<ft_world> world(new ft_world());
    ft_sharedptr<ft_event> event(new ft_event());
    event->set_id(1);
    event->set_duration(3);
    world->schedule_event(event);

    world->update_events(world, 1);
    ft_vector<ft_sharedptr<ft_event> > events;
    world->get_event_scheduler()->dump_events(events);
    if (events.size() != 1 || events[0]->get_duration() != 2)
        return (0);

    world->update_events(world, 2);
    events.clear();
    world->get_event_scheduler()->dump_events(events);
    if (events.size() != 0)
        return (0);

    return (1);
}
