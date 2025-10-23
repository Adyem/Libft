#include "../../Game/game_achievement.hpp"
#include "../../PThread/pthread.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Template/move.hpp"

struct game_goal_increment_args
{
    ft_goal *goal_pointer;
    int      iterations;
    int      result_code;
};

struct game_goal_read_args
{
    ft_goal *goal_pointer;
    int      iterations;
    int      result_code;
};

static void *game_goal_increment_task(void *argument)
{
    game_goal_increment_args *arguments;
    int index;

    arguments = static_cast<game_goal_increment_args *>(argument);
    if (arguments == ft_nullptr)
        return (ft_nullptr);
    index = 0;
    while (index < arguments->iterations)
    {
        arguments->goal_pointer->add_progress(1);
        if (arguments->goal_pointer->get_error() != ER_SUCCESS)
        {
            arguments->result_code = arguments->goal_pointer->get_error();
            return (ft_nullptr);
        }
        index += 1;
    }
    arguments->result_code = ER_SUCCESS;
    return (ft_nullptr);
}

static void *game_goal_read_task(void *argument)
{
    game_goal_read_args *arguments;
    int index;

    arguments = static_cast<game_goal_read_args *>(argument);
    if (arguments == ft_nullptr)
        return (ft_nullptr);
    index = 0;
    while (index < arguments->iterations)
    {
        int progress_value;
        int target_value;

        progress_value = arguments->goal_pointer->get_progress();
        if (arguments->goal_pointer->get_error() != ER_SUCCESS)
        {
            arguments->result_code = arguments->goal_pointer->get_error();
            return (ft_nullptr);
        }
        (void)progress_value;
        target_value = arguments->goal_pointer->get_target();
        if (arguments->goal_pointer->get_error() != ER_SUCCESS)
        {
            arguments->result_code = arguments->goal_pointer->get_error();
            return (ft_nullptr);
        }
        (void)target_value;
        index += 1;
    }
    arguments->result_code = ER_SUCCESS;
    return (ft_nullptr);
}

FT_TEST(test_game_goal_thread_safety,
    "ft_goal remains consistent across concurrent updates and inspections")
{
    ft_goal primary_goal;
    ft_goal copy_target;
    ft_goal assign_target;
    ft_goal move_target;
    game_goal_increment_args increment_arguments;
    game_goal_read_args read_arguments;
    pthread_t increment_thread;
    pthread_t read_thread;
    int create_increment_result;
    int create_read_result;
    int join_result;
    int index;
    int test_failed;
    const char *failure_expression;
    int failure_line;

    test_failed = 0;
    failure_expression = ft_nullptr;
    failure_line = 0;
    primary_goal.set_target(128);
    if (primary_goal.get_error() != ER_SUCCESS)
        return (0);
    primary_goal.set_progress(0);
    if (primary_goal.get_error() != ER_SUCCESS)
        return (0);
    increment_arguments.goal_pointer = &primary_goal;
    increment_arguments.iterations = 1024;
    increment_arguments.result_code = ER_SUCCESS;
    read_arguments.goal_pointer = &primary_goal;
    read_arguments.iterations = 1024;
    read_arguments.result_code = ER_SUCCESS;
    create_increment_result = pt_thread_create(&increment_thread, ft_nullptr,
            game_goal_increment_task, &increment_arguments);
    if (create_increment_result != 0)
    {
        test_failed = 1;
        failure_expression = "create_increment_result == 0";
        failure_line = __LINE__;
    }
    create_read_result = pt_thread_create(&read_thread, ft_nullptr,
            game_goal_read_task, &read_arguments);
    if (create_read_result != 0)
    {
        test_failed = 1;
        failure_expression = "create_read_result == 0";
        failure_line = __LINE__;
    }
    index = 0;
    while (index < 128 && test_failed == 0)
    {
        ft_goal constructed(primary_goal);
        ft_goal moved_constructed(ft_move(constructed));

        copy_target = moved_constructed;
        if (copy_target.get_error() != ER_SUCCESS)
        {
            test_failed = 1;
            failure_expression = "copy_target.get_error() == ER_SUCCESS";
            failure_line = __LINE__;
        }
        if (test_failed == 0)
        {
            assign_target = copy_target;
            if (assign_target.get_error() != ER_SUCCESS)
            {
                test_failed = 1;
                failure_expression = "assign_target.get_error() == ER_SUCCESS";
                failure_line = __LINE__;
            }
        }
        if (test_failed == 0)
        {
            move_target = ft_move(assign_target);
            if (move_target.get_error() != ER_SUCCESS)
            {
                test_failed = 1;
                failure_expression = "move_target.get_error() == ER_SUCCESS";
                failure_line = __LINE__;
            }
        }
        if (test_failed == 0)
        {
            primary_goal = ft_move(move_target);
            if (primary_goal.get_error() != ER_SUCCESS)
            {
                test_failed = 1;
                failure_expression = "primary_goal.get_error() == ER_SUCCESS";
                failure_line = __LINE__;
            }
        }
        index += 1;
    }
    if (create_increment_result == 0)
    {
        join_result = pt_thread_join(increment_thread, ft_nullptr);
        if (join_result != 0 && test_failed == 0)
        {
            test_failed = 1;
            failure_expression = "join_result == 0";
            failure_line = __LINE__;
        }
    }
    if (create_read_result == 0)
    {
        join_result = pt_thread_join(read_thread, ft_nullptr);
        if (join_result != 0 && test_failed == 0)
        {
            test_failed = 1;
            failure_expression = "join_result == 0";
            failure_line = __LINE__;
        }
    }
    if (increment_arguments.result_code != ER_SUCCESS && test_failed == 0)
    {
        test_failed = 1;
        failure_expression = "increment_arguments.result_code == ER_SUCCESS";
        failure_line = __LINE__;
    }
    if (read_arguments.result_code != ER_SUCCESS && test_failed == 0)
    {
        test_failed = 1;
        failure_expression = "read_arguments.result_code == ER_SUCCESS";
        failure_line = __LINE__;
    }
    if (test_failed != 0)
    {
        ft_test_fail(failure_expression, __FILE__, failure_line);
        return (0);
    }
    return (1);
}
