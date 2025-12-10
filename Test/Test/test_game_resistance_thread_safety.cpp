#include "../../Game/game_resistance.hpp"
#include "../../PThread/pthread.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Template/move.hpp"

struct game_resistance_update_args
{
    ft_resistance *resistance_pointer;
    int            iterations;
    int            result_code;
};

struct game_resistance_read_args
{
    ft_resistance *resistance_pointer;
    int            iterations;
    int            result_code;
};

static void *game_resistance_update_task(void *argument)
{
    game_resistance_update_args *arguments;
    int index;

    arguments = static_cast<game_resistance_update_args *>(argument);
    if (arguments == ft_nullptr)
        return (ft_nullptr);
    index = 0;
    while (index < arguments->iterations)
    {
        int percent_value;
        int flat_value;

        percent_value = index % 100;
        flat_value = index;
        arguments->result_code = arguments->resistance_pointer->set_values(percent_value, flat_value);
        if (arguments->result_code != FT_ER_SUCCESSS)
            return (ft_nullptr);
        index += 1;
    }
    arguments->result_code = FT_ER_SUCCESSS;
    return (ft_nullptr);
}

static void *game_resistance_read_task(void *argument)
{
    game_resistance_read_args *arguments;
    int index;

    arguments = static_cast<game_resistance_read_args *>(argument);
    if (arguments == ft_nullptr)
        return (ft_nullptr);
    index = 0;
    while (index < arguments->iterations)
    {
        int percent_value;
        int flat_value;

        percent_value = arguments->resistance_pointer->get_percent();
        if (arguments->resistance_pointer->get_error() != FT_ER_SUCCESSS)
        {
            (void)percent_value;
            arguments->result_code = arguments->resistance_pointer->get_error();
            return (ft_nullptr);
        }
        flat_value = arguments->resistance_pointer->get_flat();
        if (arguments->resistance_pointer->get_error() != FT_ER_SUCCESSS)
        {
            (void)flat_value;
            arguments->result_code = arguments->resistance_pointer->get_error();
            return (ft_nullptr);
        }
        index += 1;
    }
    arguments->result_code = FT_ER_SUCCESSS;
    return (ft_nullptr);
}

FT_TEST(test_game_resistance_thread_safety,
    "ft_resistance serializes concurrent updates and reads")
{
    ft_resistance primary_resistance;
    ft_resistance copy_target;
    ft_resistance assign_target;
    ft_resistance move_target;
    game_resistance_update_args update_arguments;
    game_resistance_read_args read_arguments;
    pthread_t update_thread;
    pthread_t read_thread;
    int create_update_result;
    int create_read_result;
    int join_result;
    int index;
    int test_failed;
    const char *failure_expression;
    int failure_line;

    test_failed = 0;
    failure_expression = ft_nullptr;
    failure_line = 0;
    update_arguments.resistance_pointer = &primary_resistance;
    update_arguments.iterations = 1024;
    update_arguments.result_code = FT_ER_SUCCESSS;
    read_arguments.resistance_pointer = &primary_resistance;
    read_arguments.iterations = 1024;
    read_arguments.result_code = FT_ER_SUCCESSS;
    create_update_result = pt_thread_create(&update_thread, ft_nullptr,
            game_resistance_update_task, &update_arguments);
    if (create_update_result != 0)
    {
        test_failed = 1;
        failure_expression = "create_update_result == 0";
        failure_line = __LINE__;
    }
    create_read_result = pt_thread_create(&read_thread, ft_nullptr,
            game_resistance_read_task, &read_arguments);
    if (create_read_result != 0)
    {
        test_failed = 1;
        failure_expression = "create_read_result == 0";
        failure_line = __LINE__;
    }
    index = 0;
    while (index < 128 && test_failed == 0)
    {
        ft_resistance constructed(primary_resistance);
        ft_resistance moved_constructed(ft_move(constructed));

        copy_target = moved_constructed;
        if (copy_target.get_error() != FT_ER_SUCCESSS)
        {
            test_failed = 1;
            failure_expression = "copy_target.get_error() == FT_ER_SUCCESSS";
            failure_line = __LINE__;
        }
        if (test_failed == 0)
        {
            assign_target = copy_target;
            if (assign_target.get_error() != FT_ER_SUCCESSS)
            {
                test_failed = 1;
                failure_expression = "assign_target.get_error() == FT_ER_SUCCESSS";
                failure_line = __LINE__;
            }
        }
        if (test_failed == 0)
        {
            move_target = ft_move(assign_target);
            if (move_target.get_error() != FT_ER_SUCCESSS)
            {
                test_failed = 1;
                failure_expression = "move_target.get_error() == FT_ER_SUCCESSS";
                failure_line = __LINE__;
            }
        }
        if (test_failed == 0)
        {
            primary_resistance = ft_move(move_target);
            if (primary_resistance.get_error() != FT_ER_SUCCESSS)
            {
                test_failed = 1;
                failure_expression = "primary_resistance.get_error() == FT_ER_SUCCESSS";
                failure_line = __LINE__;
            }
        }
        index += 1;
    }
    if (create_update_result == 0)
    {
        join_result = pt_thread_join(update_thread, ft_nullptr);
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
    if (update_arguments.result_code != FT_ER_SUCCESSS && test_failed == 0)
    {
        test_failed = 1;
        failure_expression = "update_arguments.result_code == FT_ER_SUCCESSS";
        failure_line = __LINE__;
    }
    if (read_arguments.result_code != FT_ER_SUCCESSS && test_failed == 0)
    {
        test_failed = 1;
        failure_expression = "read_arguments.result_code == FT_ER_SUCCESSS";
        failure_line = __LINE__;
    }
    if (test_failed != 0)
    {
        ft_test_fail(failure_expression, __FILE__, failure_line);
        return (0);
    }
    return (1);
}
