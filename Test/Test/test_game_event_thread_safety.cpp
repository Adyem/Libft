#include "../../Game/game_event.hpp"
#include "../../PThread/pthread.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"

struct game_event_increment_args
{
    ft_event *event_pointer;
    int iteration_count;
    int result_code;
};

static void *game_event_increment_duration(void *argument)
{
    game_event_increment_args *arguments;
    int index;
    int add_result;

    arguments = static_cast<game_event_increment_args *>(argument);
    if (arguments == ft_nullptr)
        return (ft_nullptr);
    index = 0;
    while (index < arguments->iteration_count)
    {
        add_result = arguments->event_pointer->add_duration(1);
        if (add_result != FT_ERR_SUCCESSS)
        {
            arguments->result_code = add_result;
            return (ft_nullptr);
        }
        index++;
    }
    arguments->result_code = FT_ERR_SUCCESSS;
    return (ft_nullptr);
}

FT_TEST(test_game_event_add_duration_thread_safe, "ft_event::add_duration remains consistent under concurrent increments")
{
    ft_event event_instance;
    pthread_t threads[4];
    game_event_increment_args arguments[4];
    int index;
    int create_result;
    int join_result;
    int expected_duration;
    int created_thread_count;
    int test_failed;
    const char *failure_expression;
    int failure_line;

    event_instance.set_duration(0);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, event_instance.get_error());
    created_thread_count = 0;
    test_failed = 0;
    failure_expression = ft_nullptr;
    failure_line = 0;
    index = 0;
    while (index < 4)
    {
        arguments[index].event_pointer = &event_instance;
        arguments[index].iteration_count = 5000;
        arguments[index].result_code = FT_ERR_SUCCESSS;
        if (test_failed == 0)
        {
            create_result = pt_thread_create(&threads[index], ft_nullptr, game_event_increment_duration, &arguments[index]);
            if (create_result != 0 && test_failed == 0)
            {
                test_failed = 1;
                failure_expression = "create_result == 0";
                failure_line = __LINE__;
            }
            if (create_result == 0)
                created_thread_count++;
        }
        else
            threads[index] = 0;
        index++;
    }
    index = 0;
    while (index < created_thread_count)
    {
        join_result = pt_thread_join(threads[index], ft_nullptr);
        if (join_result != 0 && test_failed == 0)
        {
            test_failed = 1;
            failure_expression = "join_result == 0";
            failure_line = __LINE__;
        }
        if (join_result == 0)
        {
            if (arguments[index].result_code != FT_ERR_SUCCESSS && test_failed == 0)
            {
                test_failed = 1;
                failure_expression = "arguments[index].result_code == FT_ERR_SUCCESSS";
                failure_line = __LINE__;
            }
        }
        index++;
    }
    if (test_failed != 0)
    {
        ft_test_fail(failure_expression, __FILE__, failure_line);
        return (0);
    }
    expected_duration = 4 * 5000;
    FT_ASSERT_EQ(expected_duration, event_instance.get_duration());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, event_instance.get_error());
    return (1);
}

struct game_event_assignment_args
{
    ft_event *destination_event;
    ft_event *source_event;
    int iteration_count;
    int result_code;
};

static void *game_event_assignment_task(void *argument)
{
    game_event_assignment_args *arguments;
    int index;

    arguments = static_cast<game_event_assignment_args *>(argument);
    if (arguments == ft_nullptr)
        return (ft_nullptr);
    index = 0;
    while (index < arguments->iteration_count)
    {
        *arguments->destination_event = *arguments->source_event;
        arguments->result_code = arguments->destination_event->get_error();
        if (arguments->result_code != FT_ERR_SUCCESSS)
            return (ft_nullptr);
        index++;
    }
    arguments->result_code = FT_ERR_SUCCESSS;
    return (ft_nullptr);
}

FT_TEST(test_game_event_assignment_thread_safe, "ft_event copy assignment synchronizes shared access")
{
    ft_event source_event;
    ft_event destination_event;
    pthread_t threads[3];
    game_event_assignment_args arguments[3];
    int index;
    int create_result;
    int join_result;
    int created_thread_count;
    int test_failed;
    const char *failure_expression;
    int failure_line;

    source_event.set_id(42);
    source_event.set_duration(9);
    source_event.set_modifier1(3);
    source_event.set_modifier2(4);
    source_event.set_modifier3(5);
    source_event.set_modifier4(6);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, source_event.get_error());
    destination_event.set_id(1);
    destination_event.set_duration(1);
    destination_event.set_modifier1(1);
    destination_event.set_modifier2(1);
    destination_event.set_modifier3(1);
    destination_event.set_modifier4(1);
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, destination_event.get_error());
    created_thread_count = 0;
    test_failed = 0;
    failure_expression = ft_nullptr;
    failure_line = 0;
    index = 0;
    while (index < 3)
    {
        arguments[index].destination_event = &destination_event;
        arguments[index].source_event = &source_event;
        arguments[index].iteration_count = 4000;
        arguments[index].result_code = FT_ERR_SUCCESSS;
        if (test_failed == 0)
        {
            create_result = pt_thread_create(&threads[index], ft_nullptr, game_event_assignment_task, &arguments[index]);
            if (create_result != 0 && test_failed == 0)
            {
                test_failed = 1;
                failure_expression = "create_result == 0";
                failure_line = __LINE__;
            }
            if (create_result == 0)
                created_thread_count++;
        }
        else
            threads[index] = 0;
        index++;
    }
    index = 0;
    while (index < created_thread_count)
    {
        join_result = pt_thread_join(threads[index], ft_nullptr);
        if (join_result != 0 && test_failed == 0)
        {
            test_failed = 1;
            failure_expression = "join_result == 0";
            failure_line = __LINE__;
        }
        if (join_result == 0)
        {
            if (arguments[index].result_code != FT_ERR_SUCCESSS && test_failed == 0)
            {
                test_failed = 1;
                failure_expression = "arguments[index].result_code == FT_ERR_SUCCESSS";
                failure_line = __LINE__;
            }
        }
        index++;
    }
    if (test_failed != 0)
    {
        ft_test_fail(failure_expression, __FILE__, failure_line);
        return (0);
    }
    FT_ASSERT_EQ(source_event.get_id(), destination_event.get_id());
    FT_ASSERT_EQ(source_event.get_duration(), destination_event.get_duration());
    FT_ASSERT_EQ(source_event.get_modifier1(), destination_event.get_modifier1());
    FT_ASSERT_EQ(source_event.get_modifier2(), destination_event.get_modifier2());
    FT_ASSERT_EQ(source_event.get_modifier3(), destination_event.get_modifier3());
    FT_ASSERT_EQ(source_event.get_modifier4(), destination_event.get_modifier4());
    FT_ASSERT_EQ(FT_ERR_SUCCESSS, destination_event.get_error());
    return (1);
}
