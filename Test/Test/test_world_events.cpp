#include "../../Game/world.hpp"
#include "../../Game/event.hpp"
#include "../../System_utils/test_runner.hpp"

FT_TEST(test_world_process_events, "ft_world event expiration")
{
    ft_world world;
    ft_event event;
    event.set_id(1);
    event.set_duration(3);
    world.get_events().insert(event.get_id(), event);

    world.process_events(1);
    Pair<int, ft_event>* event_entry = world.get_events().find(1);
    if (!event_entry || event_entry->value.get_duration() != 2)
        return (0);

    world.process_events(2);
    if (world.get_events().find(1))
        return (0);

    return (1);
}
