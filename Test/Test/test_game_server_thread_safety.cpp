#include "../../Game/game_server.hpp"
#include "../../Game/game_world.hpp"
#include "../../PThread/pthread.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Errno/errno.hpp"
#include "../../CPP_class/class_nullptr.hpp"
#include "../../Template/move.hpp"
#include <new>

struct game_server_poll_args
{
    ft_game_server *server_pointer;
    int             iterations;
    int             result_code;
};

static void *game_server_poll_task(void *argument)
{
    game_server_poll_args *arguments;
    int index;

    arguments = static_cast<game_server_poll_args *>(argument);
    if (arguments == ft_nullptr)
        return (ft_nullptr);
    index = 0;
    while (index < arguments->iterations)
    {
        int error_code;
        const char *message;

        error_code = arguments->server_pointer->get_error();
        if (error_code != ER_SUCCESS)
        {
            arguments->result_code = error_code;
            return (ft_nullptr);
        }
        message = arguments->server_pointer->get_error_str();
        if (message == ft_nullptr)
        {
            arguments->result_code = FT_ERR_GAME_GENERAL_ERROR;
            return (ft_nullptr);
        }
        index += 1;
    }
    arguments->result_code = ER_SUCCESS;
    return (ft_nullptr);
}

static void game_server_noop_callback(int)
{
    return ;
}

FT_TEST(test_game_server_thread_safety,
    "ft_game_server handles concurrent copies, moves, and error queries")
{
    ft_sharedptr<ft_world> world_pointer(new (std::nothrow) ft_world());
    ft_game_server primary_server(world_pointer);
    ft_game_server copy_target(world_pointer);
    ft_game_server assign_target(world_pointer);
    ft_game_server move_target(world_pointer);
    game_server_poll_args poll_arguments;
    pthread_t poll_thread;
    int create_result;
    int join_result;
    int index;
    int test_failed;
    const char *failure_expression;
    int failure_line;

    test_failed = 0;
    failure_expression = ft_nullptr;
    failure_line = 0;
    if (!world_pointer || world_pointer.get_error() != ER_SUCCESS)
    {
        return (0);
    }
    if (primary_server.get_error() != ER_SUCCESS)
        return (0);
    poll_arguments.server_pointer = &primary_server;
    poll_arguments.iterations = 4096;
    poll_arguments.result_code = ER_SUCCESS;
    create_result = pt_thread_create(&poll_thread, ft_nullptr,
            game_server_poll_task, &poll_arguments);
    if (create_result != 0)
    {
        test_failed = 1;
        failure_expression = "create_result == 0";
        failure_line = __LINE__;
    }
    index = 0;
    while (index < 128 && test_failed == 0)
    {
        ft_game_server constructed(primary_server);
        ft_game_server moved_constructed(ft_move(constructed));

        primary_server.set_join_callback(game_server_noop_callback);
        if (primary_server.get_error() != ER_SUCCESS)
        {
            test_failed = 1;
            failure_expression = "primary_server.get_error() == ER_SUCCESS";
            failure_line = __LINE__;
        }
        if (test_failed == 0)
        {
            primary_server.set_leave_callback(game_server_noop_callback);
            if (primary_server.get_error() != ER_SUCCESS)
            {
                test_failed = 1;
                failure_expression = "primary_server.get_error() == ER_SUCCESS";
                failure_line = __LINE__;
            }
        }
        if (test_failed == 0)
        {
            copy_target = moved_constructed;
            if (copy_target.get_error() != ER_SUCCESS)
            {
                test_failed = 1;
                failure_expression = "copy_target.get_error() == ER_SUCCESS";
                failure_line = __LINE__;
            }
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
            primary_server = ft_move(move_target);
            if (primary_server.get_error() != ER_SUCCESS)
            {
                test_failed = 1;
                failure_expression = "primary_server.get_error() == ER_SUCCESS";
                failure_line = __LINE__;
            }
        }
        index += 1;
    }
    if (create_result == 0)
    {
        join_result = pt_thread_join(poll_thread, ft_nullptr);
        if (join_result != 0 && test_failed == 0)
        {
            test_failed = 1;
            failure_expression = "join_result == 0";
            failure_line = __LINE__;
        }
    }
    if (poll_arguments.result_code != ER_SUCCESS && test_failed == 0)
    {
        test_failed = 1;
        failure_expression = "poll_arguments.result_code == ER_SUCCESS";
        failure_line = __LINE__;
    }
    if (test_failed != 0)
    {
        ft_test_fail(failure_expression, __FILE__, failure_line);
        return (0);
    }
    return (1);
}
