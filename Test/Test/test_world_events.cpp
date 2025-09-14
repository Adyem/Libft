#include "../../Game/world.hpp"
#include "../../Game/event.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Template/vector.hpp"

FT_TEST(test_world_process_events, "ft_world event expiration")
{
    ft_world world;
    ft_event event;
    event.set_id(1);
    event.set_duration(3);
    world.schedule_event(event);

    world.update_events(1);
    ft_vector<ft_event> events;
    world.get_event_scheduler().dump_events(events);
    if (events.size() != 1 || events[0].get_duration() != 2)
        return (0);

    world.update_events(2);
    events.clear();
    world.get_event_scheduler().dump_events(events);
    if (events.size() != 0)
        return (0);

    return (1);
}
