#include "../test_internal.hpp"
#include "../../Game/game_event_scheduler.hpp"
#include "../../PThread/pthread.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"
#include "../../Errno/errno.hpp"
#include "../../Template/vector.hpp"
#include <new>

#ifndef LIBFT_TEST_BUILD
#endif

struct scheduler_schedule_args
{
    game_event_scheduler *scheduler_pointer;
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
        ft_sharedptr<game_event> event_instance(new (std::nothrow) game_event());

        if (!event_instance)
        {
            arguments->result_code = FT_ERR_NO_MEMORY;
            return (ft_nullptr);
        }
        FT_ASSERT_EQ(FT_ERR_SUCCESS, event_instance->initialize());
        if (event_instance->get_error() != FT_ERR_SUCCESS)
        {
            arguments->result_code = event_instance->get_error();
            return (ft_nullptr);
        }
        event_instance->set_id(base_identifier + index);
        event_instance->set_duration(index + 1);
        arguments->scheduler_pointer->schedule_event(event_instance);
        index += 1;
    }
    arguments->result_code = FT_ERR_SUCCESS;
    return (ft_nullptr);
}

FT_TEST(test_game_event_scheduler_concurrent_schedule)
{
    game_event_scheduler *scheduler_instance;
    pthread_t *threads;
    scheduler_schedule_args *arguments;
    int thread_index;
    int create_result;
    int join_result;
    int events_per_thread;
    int expected_total;
    ft_vector<ft_sharedptr<game_event> > events;
    int created_thread_count;
    int test_failed;
    const char *failure_expression;
    int failure_line;
    long join_timeout_ms;

    events_per_thread = 32;
    expected_total = 4 * events_per_thread;
    created_thread_count = 0;
    test_failed = 0;
    failure_expression = ft_nullptr;
    failure_line = 0;
    join_timeout_ms = 5000;
    scheduler_instance = new (std::nothrow) game_event_scheduler();
    FT_ASSERT(scheduler_instance != ft_nullptr);
    threads = new (std::nothrow) pthread_t[4];
    FT_ASSERT(threads != ft_nullptr);
    arguments = new (std::nothrow) scheduler_schedule_args[4];
    FT_ASSERT(arguments != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scheduler_instance->initialize());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scheduler_instance->enable_thread_safety());
    FT_ASSERT_EQ(FT_ERR_SUCCESS, events.initialize());
    thread_index = 0;
    while (thread_index < 4)
    {
        arguments[thread_index].scheduler_pointer = scheduler_instance;
        arguments[thread_index].thread_index = thread_index;
        arguments[thread_index].events_per_thread = events_per_thread;
        arguments[thread_index].result_code = FT_ERR_SUCCESS;
        if (test_failed == 0)
        {
            create_result = pt_thread_create(&threads[thread_index], ft_nullptr,
                    scheduler_schedule_task, &arguments[thread_index]);
            if (create_result != 0 && test_failed == 0)
            {
                test_failed = 1;
                failure_expression = "create_result == 0";
                failure_line = __LINE__;
            }
            if (create_result == 0)
                created_thread_count += 1;
        }
        else
            threads[thread_index] = 0;
        thread_index += 1;
    }
    thread_index = 0;
    while (thread_index < created_thread_count)
    {
        join_result = pt_thread_timed_join(threads[thread_index], ft_nullptr, join_timeout_ms);
        if (join_result != 0 && test_failed == 0)
        {
            test_failed = 1;
            failure_expression = "join_result == 0";
            failure_line = __LINE__;
        }
        if (join_result == 0)
        {
            if (arguments[thread_index].result_code != FT_ERR_SUCCESS && test_failed == 0)
            {
                test_failed = 1;
                failure_expression = "arguments[thread_index].result_code == FT_ERR_SUCCESS";
                failure_line = __LINE__;
            }
        }
        thread_index += 1;
    }
    if (test_failed != 0)
    {
        ft_test_fail(failure_expression, __FILE__, failure_line);
        return (0);
    }
    FT_ASSERT_EQ(static_cast<size_t>(expected_total), scheduler_instance->size());
    scheduler_instance->dump_events(events);
    FT_ASSERT_EQ(static_cast<size_t>(0), events.size());
    return (1);
}

struct scheduler_reschedule_args
{
    game_event_scheduler *scheduler_pointer;
    int event_identifier;
    int iteration_count;
};

struct scheduler_reschedule_detached_state
{
    scheduler_reschedule_args *arguments;
    ft_bool *keep_running;
};

static void *scheduler_reschedule_detached_task(void *argument)
{
    scheduler_reschedule_detached_state *state;

    state = static_cast<scheduler_reschedule_detached_state *>(argument);
    if (state == ft_nullptr || state->arguments == ft_nullptr
        || state->keep_running == ft_nullptr)
        return (ft_nullptr);
    while (*state->keep_running == FT_FALSE)
        time_sleep_ms(1);
    time_sleep_ms(7000);
    state->arguments->scheduler_pointer->reschedule_event(
        state->arguments->event_identifier,
        state->arguments->event_identifier + state->arguments->iteration_count);
    return (ft_nullptr);
}

FT_TEST(test_game_event_scheduler_concurrent_reschedule)
{
    game_event_scheduler *scheduler_instance;
    pthread_t *threads;
    scheduler_reschedule_args *arguments;
    scheduler_reschedule_detached_state *detached_states;
    ft_bool *keep_running;
    int preload_index;
    int create_result;
    int iteration_count;

    iteration_count = 64;
    scheduler_instance = new (std::nothrow) game_event_scheduler();
    FT_ASSERT(scheduler_instance != ft_nullptr);
    threads = new (std::nothrow) pthread_t[3];
    FT_ASSERT(threads != ft_nullptr);
    arguments = new (std::nothrow) scheduler_reschedule_args[3];
    FT_ASSERT(arguments != ft_nullptr);
    detached_states = new (std::nothrow) scheduler_reschedule_detached_state[3];
    FT_ASSERT(detached_states != ft_nullptr);
    keep_running = new (std::nothrow) ft_bool;
    FT_ASSERT(keep_running != ft_nullptr);
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scheduler_instance->initialize());
    preload_index = 0;
    while (preload_index < 3)
    {
        ft_sharedptr<game_event> event_instance(new (std::nothrow) game_event());

        FT_ASSERT_EQ(1, static_cast<int>(static_cast<bool>(event_instance)));
        FT_ASSERT_EQ(FT_ERR_SUCCESS, event_instance->initialize());
        event_instance->set_id(preload_index);
        event_instance->set_duration(1);
        scheduler_instance->schedule_event(event_instance);
        preload_index += 1;
    }
    FT_ASSERT_EQ(FT_ERR_SUCCESS, scheduler_instance->enable_thread_safety());
    preload_index = 0;
    while (preload_index < 3)
    {
        arguments[preload_index].scheduler_pointer = scheduler_instance;
        arguments[preload_index].event_identifier = preload_index;
        arguments[preload_index].iteration_count = iteration_count;
        detached_states[preload_index].arguments = &arguments[preload_index];
        detached_states[preload_index].keep_running = keep_running;
        create_result = pt_thread_create(&threads[preload_index], ft_nullptr,
                scheduler_reschedule_detached_task,
                &detached_states[preload_index]);
        FT_ASSERT_EQ(0, create_result);
        FT_ASSERT_EQ(0, pt_thread_detach(threads[preload_index]));
        preload_index += 1;
    }
    *keep_running = FT_TRUE;
    time_sleep_ms(5000);
    return (0);
}
