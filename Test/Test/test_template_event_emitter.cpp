#include "../../Template/event_emitter.hpp"
#include "../../CMA/CMA.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"

static int g_event_listener_one_total = 0;
static int g_event_listener_two_total = 0;

static void event_listener_add_to_first(int value)
{
    g_event_listener_one_total += value;
    return ;
}

static void event_listener_add_to_second(int value)
{
    g_event_listener_two_total += value;
    return ;
}

FT_TEST(test_ft_event_emitter_invokes_registered_listeners, "ft_event_emitter emits events to all matching listeners")
{
    ft_event_emitter<int, int> emitter_instance;

    g_event_listener_one_total = 0;
    g_event_listener_two_total = 0;

    emitter_instance.on(7, event_listener_add_to_first);
    FT_ASSERT_EQ(ER_SUCCESS, emitter_instance.get_error());
    emitter_instance.on(7, event_listener_add_to_second);
    FT_ASSERT_EQ(ER_SUCCESS, emitter_instance.get_error());

    emitter_instance.emit(7, 5);
    FT_ASSERT_EQ(ER_SUCCESS, emitter_instance.get_error());

    FT_ASSERT_EQ(5, g_event_listener_one_total);
    FT_ASSERT_EQ(5, g_event_listener_two_total);
    FT_ASSERT_EQ(2UL, emitter_instance.size());
    FT_ASSERT_EQ(false, emitter_instance.empty());
    return (1);
}

FT_TEST(test_ft_event_emitter_remove_listener_stops_callback, "ft_event_emitter remove_listener prevents future delivery")
{
    ft_event_emitter<int, int> emitter_instance;

    g_event_listener_one_total = 0;
    g_event_listener_two_total = 0;

    emitter_instance.on(3, event_listener_add_to_first);
    emitter_instance.on(3, event_listener_add_to_second);
    FT_ASSERT_EQ(2UL, emitter_instance.size());

    emitter_instance.remove_listener(3, event_listener_add_to_first);
    FT_ASSERT_EQ(ER_SUCCESS, emitter_instance.get_error());
    FT_ASSERT_EQ(1UL, emitter_instance.size());

    emitter_instance.emit(3, 4);
    FT_ASSERT_EQ(ER_SUCCESS, emitter_instance.get_error());

    FT_ASSERT_EQ(0, g_event_listener_one_total);
    FT_ASSERT_EQ(4, g_event_listener_two_total);

    emitter_instance.remove_listener(3, event_listener_add_to_first);
    FT_ASSERT_EQ(EVENT_EMITTER_NOT_FOUND, emitter_instance.get_error());
    return (1);
}

FT_TEST(test_ft_event_emitter_emit_missing_sets_error, "ft_event_emitter reports when no listeners match an event")
{
    ft_event_emitter<int, int> emitter_instance;

    emitter_instance.emit(1, 10);
    FT_ASSERT_EQ(EVENT_EMITTER_NOT_FOUND, emitter_instance.get_error());
    FT_ASSERT_EQ(0UL, emitter_instance.size());
    FT_ASSERT(emitter_instance.empty());
    return (1);
}

FT_TEST(test_ft_event_emitter_allocation_failure_sets_error, "ft_event_emitter surfaces allocation failures during listener registration")
{
    ft_event_emitter<int, int> emitter_instance;

    cma_set_alloc_limit(1);
    emitter_instance.on(9, event_listener_add_to_first);
    cma_set_alloc_limit(0);

    FT_ASSERT_EQ(EVENT_EMITTER_ALLOC_FAIL, emitter_instance.get_error());
    FT_ASSERT_EQ(0UL, emitter_instance.size());
    FT_ASSERT(emitter_instance.empty());
    return (1);
}

FT_TEST(test_ft_event_emitter_growth_preserves_existing_listeners, "ft_event_emitter keep existing listeners when resizing")
{
    ft_event_emitter<int, int> emitter_instance(1);

    g_event_listener_one_total = 0;
    g_event_listener_two_total = 0;

    emitter_instance.on(11, event_listener_add_to_first);
    FT_ASSERT_EQ(ER_SUCCESS, emitter_instance.get_error());
    emitter_instance.on(12, event_listener_add_to_second);
    FT_ASSERT_EQ(ER_SUCCESS, emitter_instance.get_error());
    emitter_instance.on(11, event_listener_add_to_second);
    FT_ASSERT_EQ(ER_SUCCESS, emitter_instance.get_error());

    emitter_instance.emit(11, 2);
    FT_ASSERT_EQ(ER_SUCCESS, emitter_instance.get_error());

    FT_ASSERT_EQ(2, g_event_listener_one_total);
    FT_ASSERT_EQ(2, g_event_listener_two_total);
    FT_ASSERT_EQ(3UL, emitter_instance.size());
    return (1);
}
