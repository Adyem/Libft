#include "../test_internal.hpp"
#include "../../Game/game_server.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../PThread/pthread.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

struct game_server_run_args
{
    ft_game_server *server_pointer;
    int iterations;
};

static void *game_server_run_task(void *argument)
{
    game_server_run_args *arguments = static_cast<game_server_run_args *>(argument);
    if (arguments == ft_nullptr || arguments->server_pointer == ft_nullptr)
        return (ft_nullptr);
    for (int index = 0; index < arguments->iterations; ++index)
        arguments->server_pointer->run_once();
    return (ft_nullptr);
}

FT_TEST(test_game_server_thread_safety, "ft_game_server handles run_once concurrently")
{
    ft_sharedptr<ft_world> world_pointer(new ft_world());
    ft_game_server server;
    pthread_t threads[4];
    game_server_run_args arguments[4];
    int thread_index = 0;
    int create_result;

    FT_ASSERT_EQ(FT_ERR_SUCCESS, server.initialize(world_pointer));
    FT_ASSERT_EQ(FT_ERR_SUCCESS, server.enable_thread_safety());
    while (thread_index < 4)
    {
        arguments[thread_index].server_pointer = &server;
        arguments[thread_index].iterations = 100;
        create_result = pt_thread_create(&threads[thread_index], ft_nullptr,
            game_server_run_task, &arguments[thread_index]);
        FT_ASSERT_EQ(0, create_result);
        thread_index += 1;
    }

    thread_index = 0;
    while (thread_index < 4)
    {
        FT_ASSERT_EQ(0, pt_thread_join(threads[thread_index], ft_nullptr));
        thread_index += 1;
    }
    return (1);
}
