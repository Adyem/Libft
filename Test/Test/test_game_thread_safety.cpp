#include "../test_internal.hpp"
#include "../../Modules/Game/game_buff.hpp"
#include "../../Modules/Game/game_skill.hpp"
#include "../../Modules/Threading/thread.hpp"
#include "../../Modules/System_utils/test_system_utils_runner.hpp"

#include "../../Modules/Basic/class_nullptr.hpp"
#include "../../Modules/Errno/errno.hpp"
#include "../../Modules/PThread/mutex.hpp"
#include "../../Modules/PThread/recursive_mutex.hpp"
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
    game_buff shared_buff;
    if (shared_buff.initialize() != FT_ERR_SUCCESS)
    {
        ft_test_fail("shared_buff.initialize()", __FILE__, __LINE__);
        return (ft_nullptr);
    }
    for (int index = 0; index < args->iterations; ++index)
        shared_buff.add_modifier1(1);
    if (shared_buff.destroy() != FT_ERR_SUCCESS)
    {
        ft_test_fail("shared_buff.destroy()", __FILE__, __LINE__);
        return (ft_nullptr);
    }
    return (ft_nullptr);
}

static void *skill_cooldown_task(void *argument)
{
    task_args *args = static_cast<task_args *>(argument);
    if (args == ft_nullptr)
        return (ft_nullptr);
    game_skill shared_skill;
    if (shared_skill.initialize() != FT_ERR_SUCCESS)
    {
        ft_test_fail("shared_skill.initialize()", __FILE__, __LINE__);
        return (ft_nullptr);
    }
    for (int index = 0; index < args->iterations; ++index)
        shared_skill.add_cooldown(1);
    if (shared_skill.destroy() != FT_ERR_SUCCESS)
    {
        ft_test_fail("shared_skill.destroy()", __FILE__, __LINE__);
        return (ft_nullptr);
    }
    return (ft_nullptr);
}

FT_TEST(test_game_thread_safety_basic_increments)
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
