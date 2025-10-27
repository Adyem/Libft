#include "../../Template/event_emitter.hpp"
#include "../../CMA/CMA.hpp"
#include "../../Errno/errno.hpp"
#include "../../System_utils/test_runner.hpp"
#include <atomic>
#include <limits>
#include <thread>

static int g_event_listener_one_total = 0;
static int g_event_listener_two_total = 0;
static std::atomic<int> g_concurrent_listener_total(0);

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

static void event_listener_add_to_atomic(int value)
{
    g_concurrent_listener_total.fetch_add(value, std::memory_order_relaxed);
    return ;
}

static void emit_in_thread(ft_event_emitter<int, int>* emitter_instance, int event_identifier, int emit_count, int emit_value)
{
    int emission_index;

    emission_index = 0;
    while (emission_index < emit_count)
    {
        emitter_instance->emit(event_identifier, emit_value);
        ++emission_index;
    }
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
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, emitter_instance.get_error());
    return (1);
}

FT_TEST(test_ft_event_emitter_emit_missing_sets_error, "ft_event_emitter reports when no listeners match an event")
{
    ft_event_emitter<int, int> emitter_instance;

    emitter_instance.emit(1, 10);
    FT_ASSERT_EQ(FT_ERR_NOT_FOUND, emitter_instance.get_error());
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

    FT_ASSERT_EQ(FT_ERR_NO_MEMORY, emitter_instance.get_error());
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

FT_TEST(test_ft_event_emitter_capacity_overflow_sets_error, "ft_event_emitter detects capacity growth overflow")
{
    ft_event_emitter<int, int> emitter_instance;
    size_t desired_capacity;
    bool ensured;

    desired_capacity = std::numeric_limits<size_t>::max();
    ensured = ft_event_emitter_test_helper<int, int>::ensure_capacity(emitter_instance, desired_capacity);
    FT_ASSERT_EQ(false, ensured);
    FT_ASSERT_EQ(FT_ERR_OUT_OF_RANGE, emitter_instance.get_error());
    FT_ASSERT(emitter_instance.empty());
    return (1);
}

FT_TEST(test_ft_event_emitter_supports_concurrent_emitters, "ft_event_emitter protects internal state across concurrent emitters")
{
    ft_event_emitter<int, int> emitter_instance;
    std::thread first_thread;
    std::thread second_thread;
    std::thread third_thread;
    std::thread fourth_thread;
    int emit_iterations;

    g_concurrent_listener_total.store(0, std::memory_order_relaxed);
    emitter_instance.on(99, event_listener_add_to_atomic);
    FT_ASSERT_EQ(ER_SUCCESS, emitter_instance.get_error());

    emit_iterations = 1000;
    first_thread = std::thread(emit_in_thread, &emitter_instance, 99, emit_iterations, 1);
    second_thread = std::thread(emit_in_thread, &emitter_instance, 99, emit_iterations, 1);
    third_thread = std::thread(emit_in_thread, &emitter_instance, 99, emit_iterations, 1);
    fourth_thread = std::thread(emit_in_thread, &emitter_instance, 99, emit_iterations, 1);

    first_thread.join();
    second_thread.join();
    third_thread.join();
    fourth_thread.join();

    FT_ASSERT_EQ(4000, g_concurrent_listener_total.load(std::memory_order_relaxed));
    FT_ASSERT_EQ(ER_SUCCESS, emitter_instance.get_error());
    return (1);
}
