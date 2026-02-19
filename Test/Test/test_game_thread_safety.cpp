#include "../test_internal.hpp"
#include "../../Game/game_buff.hpp"
#include "../../Game/game_skill.hpp"
#include "../../PThread/thread.hpp"
#include "../../System_utils/test_runner.hpp"

#ifndef LIBFT_TEST_BUILD
#endif

struct task_args
{
    int iterations;
};

static void *buff_increment_task(void *argument)
{
    task_args *args = static_cast<task_args *>(argument);
    if (args == ft_nullptr)
        return (ft_nullptr);
    ft_buff shared_buff;
    for (int index = 0; index < args->iterations; ++index)
        shared_buff.add_modifier1(1);
    return (ft_nullptr);
}

static void *skill_cooldown_task(void *argument)
{
    task_args *args = static_cast<task_args *>(argument);
    if (args == ft_nullptr)
        return (ft_nullptr);
    ft_skill shared_skill;
    for (int index = 0; index < args->iterations; ++index)
        shared_skill.add_cooldown(1);
    return (ft_nullptr);
}

FT_TEST(test_game_thread_safety_basic_increments, "Game: concurrent buff and skill updates stay consistent")
{
    pthread_t threads[4];
    task_args args = { 1000 };
    int thread_index = 0;

    while (thread_index < 2)
    {
        FT_ASSERT_EQ(0, pt_thread_create(&threads[thread_index], ft_nullptr,
            buff_increment_task, &args));
        ++thread_index;
    }
    while (thread_index < 4)
    {
        FT_ASSERT_EQ(0, pt_thread_create(&threads[thread_index], ft_nullptr,
            skill_cooldown_task, &args));
        ++thread_index;
    }

    thread_index = 0;
    while (thread_index < 4)
    {
        FT_ASSERT_EQ(0, pt_thread_join(threads[thread_index], ft_nullptr));
        ++thread_index;
    }
    return (1);
}
