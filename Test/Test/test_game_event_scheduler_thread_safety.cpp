#include "../../Game/game_event_scheduler.hpp"
#include "../../PThread/pthread.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include "../../Template/vector.hpp"
#include <new>

struct scheduler_schedule_args
{
    ft_event_scheduler *scheduler_pointer;
    int thread_index;
    int events_per_thread;
    int result_code;
};

static void *scheduler_schedule_task(void *argument)
{
    scheduler_schedule_args *arguments;
    int index;
    int base_identifier;

    arguments = static_cast<scheduler_schedule_args *>(argument);
    if (arguments == ft_nullptr)
        return (ft_nullptr);
    base_identifier = arguments->thread_index * arguments->events_per_thread;
    index = 0;
    while (index < arguments->events_per_thread)
    {
        ft_sharedptr<ft_event> event_instance(new (std::nothrow) ft_event());

        if (!event_instance)
        {
            arguments->result_code = FT_ERR_NO_MEMORY;
            return (ft_nullptr);
        }
        if (event_instance.get_error() != ER_SUCCESS)
        {
            arguments->result_code = event_instance.get_error();
            return (ft_nullptr);
        }
        event_instance->set_id(base_identifier + index);
        if (event_instance->get_error() != ER_SUCCESS)
        {
            arguments->result_code = event_instance->get_error();
            return (ft_nullptr);
        }
        event_instance->set_duration(index + 1);
        if (event_instance->get_error() != ER_SUCCESS)
        {
            arguments->result_code = event_instance->get_error();
            return (ft_nullptr);
        }
        arguments->scheduler_pointer->schedule_event(event_instance);
        index += 1;
    }
    arguments->result_code = ER_SUCCESS;
    return (ft_nullptr);
}

FT_TEST(test_game_event_scheduler_concurrent_schedule,
    "ft_event_scheduler handles concurrent schedule_event calls")
{
    ft_event_scheduler scheduler_instance;
    pthread_t threads[4];
    scheduler_schedule_args arguments[4];
    int thread_index;
    int create_result;
    int join_result;
    int events_per_thread;
    int expected_total;
    ft_vector<ft_sharedptr<ft_event> > events;
    ft_vector<int> identifier_counts;
    size_t event_index;

    events_per_thread = 32;
    expected_total = 4 * events_per_thread;
    thread_index = 0;
    while (thread_index < 4)
    {
        arguments[thread_index].scheduler_pointer = &scheduler_instance;
        arguments[thread_index].thread_index = thread_index;
        arguments[thread_index].events_per_thread = events_per_thread;
        arguments[thread_index].result_code = ER_SUCCESS;
        create_result = pt_thread_create(&threads[thread_index], ft_nullptr,
                scheduler_schedule_task, &arguments[thread_index]);
        FT_ASSERT_EQ(0, create_result);
        thread_index += 1;
    }
    thread_index = 0;
    while (thread_index < 4)
    {
        join_result = pt_thread_join(threads[thread_index], ft_nullptr);
        FT_ASSERT_EQ(0, join_result);
        FT_ASSERT_EQ(ER_SUCCESS, arguments[thread_index].result_code);
        thread_index += 1;
    }
    FT_ASSERT_EQ(static_cast<size_t>(expected_total), scheduler_instance.size());
    FT_ASSERT_EQ(ER_SUCCESS, scheduler_instance.get_error());
    scheduler_instance.dump_events(events);
    FT_ASSERT_EQ(ER_SUCCESS, scheduler_instance.get_error());
    FT_ASSERT_EQ(static_cast<size_t>(expected_total), events.size());
    identifier_counts.resize(expected_total, 0);
    FT_ASSERT_EQ(ER_SUCCESS, identifier_counts.get_error());
    event_index = 0;
    while (event_index < events.size())
    {
        ft_sharedptr<ft_event> &event_reference = events[event_index];
        int event_id;

        FT_ASSERT_EQ(1, static_cast<int>(static_cast<bool>(event_reference)));
        event_id = event_reference->get_id();
        FT_ASSERT_EQ(1, static_cast<int>(event_id >= 0));
        FT_ASSERT_EQ(1, static_cast<int>(event_id < expected_total));
        identifier_counts[event_id] += 1;
        event_index += 1;
    }
    event_index = 0;
    while (event_index < identifier_counts.size())
    {
        FT_ASSERT_EQ(1, identifier_counts[event_index]);
        event_index += 1;
    }
    FT_ASSERT_EQ(ER_SUCCESS, scheduler_instance.get_error());
    return (1);
}

struct scheduler_reschedule_args
{
    ft_event_scheduler *scheduler_pointer;
    int event_identifier;
    int iteration_count;
};

static void *scheduler_reschedule_task(void *argument)
{
    scheduler_reschedule_args *arguments;
    int iteration_index;

    arguments = static_cast<scheduler_reschedule_args *>(argument);
    if (arguments == ft_nullptr)
        return (ft_nullptr);
    iteration_index = 0;
    while (iteration_index < arguments->iteration_count)
    {
        arguments->scheduler_pointer->reschedule_event(arguments->event_identifier,
                arguments->event_identifier + iteration_index);
        iteration_index += 1;
    }
    return (ft_nullptr);
}

FT_TEST(test_game_event_scheduler_concurrent_reschedule,
    "ft_event_scheduler reschedule_event remains coherent under contention")
{
    ft_event_scheduler scheduler_instance;
    pthread_t threads[3];
    scheduler_reschedule_args arguments[3];
    int preload_index;
    int create_result;
    int join_result;
    int iteration_count;
    ft_vector<ft_sharedptr<ft_event> > events;
    ft_vector<int> final_durations;

    iteration_count = 64;
    preload_index = 0;
    while (preload_index < 3)
    {
        ft_sharedptr<ft_event> event_instance(new (std::nothrow) ft_event());

        FT_ASSERT_EQ(1, static_cast<int>(static_cast<bool>(event_instance)));
        FT_ASSERT_EQ(ER_SUCCESS, event_instance.get_error());
        event_instance->set_id(preload_index);
        FT_ASSERT_EQ(ER_SUCCESS, event_instance->get_error());
        event_instance->set_duration(1);
        FT_ASSERT_EQ(ER_SUCCESS, event_instance->get_error());
        scheduler_instance.schedule_event(event_instance);
        preload_index += 1;
    }
    FT_ASSERT_EQ(ER_SUCCESS, scheduler_instance.get_error());
    preload_index = 0;
    while (preload_index < 3)
    {
        arguments[preload_index].scheduler_pointer = &scheduler_instance;
        arguments[preload_index].event_identifier = preload_index;
        arguments[preload_index].iteration_count = iteration_count;
        create_result = pt_thread_create(&threads[preload_index], ft_nullptr,
                scheduler_reschedule_task, &arguments[preload_index]);
        FT_ASSERT_EQ(0, create_result);
        preload_index += 1;
    }
    preload_index = 0;
    while (preload_index < 3)
    {
        join_result = pt_thread_join(threads[preload_index], ft_nullptr);
        FT_ASSERT_EQ(0, join_result);
        preload_index += 1;
    }
    scheduler_instance.dump_events(events);
    FT_ASSERT_EQ(ER_SUCCESS, scheduler_instance.get_error());
    FT_ASSERT_EQ(static_cast<size_t>(3), events.size());
    final_durations.resize(3, 0);
    FT_ASSERT_EQ(ER_SUCCESS, final_durations.get_error());
    size_t event_index = 0;
    while (event_index < events.size())
    {
        ft_sharedptr<ft_event> &event_reference = events[event_index];
        int event_id;

        FT_ASSERT_EQ(1, static_cast<int>(static_cast<bool>(event_reference)));
        event_id = event_reference->get_id();
        FT_ASSERT_EQ(1, static_cast<int>(event_id >= 0));
        FT_ASSERT_EQ(1, static_cast<int>(event_id < 3));
        final_durations[event_id] = event_reference->get_duration();
        event_index += 1;
    }
    preload_index = 0;
    while (preload_index < 3)
    {
        int expected_duration;

        expected_duration = preload_index + (iteration_count - 1);
        FT_ASSERT_EQ(expected_duration, final_durations[preload_index]);
        preload_index += 1;
    }
    FT_ASSERT_EQ(ER_SUCCESS, scheduler_instance.get_error());
    return (1);
}
