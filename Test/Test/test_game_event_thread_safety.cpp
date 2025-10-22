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

    arguments = static_cast<game_event_increment_args *>(argument);
    if (arguments == ft_nullptr)
        return (ft_nullptr);
    index = 0;
    while (index < arguments->iteration_count)
    {
        arguments->event_pointer->add_duration(1);
        if (arguments->event_pointer->get_error() != ER_SUCCESS)
        {
            arguments->result_code = arguments->event_pointer->get_error();
            return (ft_nullptr);
        }
        index++;
    }
    arguments->result_code = ER_SUCCESS;
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

    event_instance.set_duration(0);
    FT_ASSERT_EQ(ER_SUCCESS, event_instance.get_error());
    index = 0;
    while (index < 4)
    {
        arguments[index].event_pointer = &event_instance;
        arguments[index].iteration_count = 5000;
        arguments[index].result_code = ER_SUCCESS;
        create_result = pt_thread_create(&threads[index], ft_nullptr, game_event_increment_duration, &arguments[index]);
        FT_ASSERT_EQ(0, create_result);
        index++;
    }
    index = 0;
    while (index < 4)
    {
        join_result = pt_thread_join(threads[index], ft_nullptr);
        FT_ASSERT_EQ(0, join_result);
        FT_ASSERT_EQ(ER_SUCCESS, arguments[index].result_code);
        index++;
    }
    expected_duration = 4 * 5000;
    FT_ASSERT_EQ(expected_duration, event_instance.get_duration());
    FT_ASSERT_EQ(ER_SUCCESS, event_instance.get_error());
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
        if (arguments->result_code != ER_SUCCESS)
            return (ft_nullptr);
        index++;
    }
    arguments->result_code = ER_SUCCESS;
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

    source_event.set_id(42);
    source_event.set_duration(9);
    source_event.set_modifier1(3);
    source_event.set_modifier2(4);
    source_event.set_modifier3(5);
    source_event.set_modifier4(6);
    FT_ASSERT_EQ(ER_SUCCESS, source_event.get_error());
    destination_event.set_id(1);
    destination_event.set_duration(1);
    destination_event.set_modifier1(1);
    destination_event.set_modifier2(1);
    destination_event.set_modifier3(1);
    destination_event.set_modifier4(1);
    FT_ASSERT_EQ(ER_SUCCESS, destination_event.get_error());
    index = 0;
    while (index < 3)
    {
        arguments[index].destination_event = &destination_event;
        arguments[index].source_event = &source_event;
        arguments[index].iteration_count = 4000;
        arguments[index].result_code = ER_SUCCESS;
        create_result = pt_thread_create(&threads[index], ft_nullptr, game_event_assignment_task, &arguments[index]);
        FT_ASSERT_EQ(0, create_result);
        index++;
    }
    index = 0;
    while (index < 3)
    {
        join_result = pt_thread_join(threads[index], ft_nullptr);
        FT_ASSERT_EQ(0, join_result);
        FT_ASSERT_EQ(ER_SUCCESS, arguments[index].result_code);
        index++;
    }
    FT_ASSERT_EQ(source_event.get_id(), destination_event.get_id());
    FT_ASSERT_EQ(source_event.get_duration(), destination_event.get_duration());
    FT_ASSERT_EQ(source_event.get_modifier1(), destination_event.get_modifier1());
    FT_ASSERT_EQ(source_event.get_modifier2(), destination_event.get_modifier2());
    FT_ASSERT_EQ(source_event.get_modifier3(), destination_event.get_modifier3());
    FT_ASSERT_EQ(source_event.get_modifier4(), destination_event.get_modifier4());
    FT_ASSERT_EQ(ER_SUCCESS, destination_event.get_error());
    return (1);
}
