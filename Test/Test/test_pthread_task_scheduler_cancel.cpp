#include "../../PThread/task_scheduler.hpp"
#include "../../System_utils/test_runner.hpp"
#include "../../Template/atomic.hpp"
#include "../../Errno/errno.hpp"
#include <chrono>
#include <unistd.h>

FT_TEST(test_task_scheduler_cancel_after_handle, "ft_task_scheduler cancels delayed task")
{
    ft_task_scheduler scheduler_instance(1);
    ft_atomic<int> execution_count;

    execution_count.store(0);
    auto schedule_result = scheduler_instance.schedule_after(std::chrono::milliseconds(100),
        [&execution_count]()
    {
        execution_count.fetch_add(1);
        return ;
    });
    ft_future<void> delayed_future = schedule_result.get_key();
    ft_scheduled_task_handle handle_value = schedule_result.get_value();
    FT_ASSERT(handle_value.valid());
    (void)delayed_future;
    bool cancel_result;

    cancel_result = handle_value.cancel();
    FT_ASSERT(cancel_result);
    int handle_error;

    handle_error = handle_value.get_error();
    FT_ASSERT_EQ(ft_errno, handle_error);
    FT_ASSERT_EQ(ER_SUCCESS, handle_error);
    usleep(150000);
    int executed_times;

    executed_times = execution_count.load();
    FT_ASSERT_EQ(0, executed_times);
    int scheduler_error;

    scheduler_error = scheduler_instance.get_error();
    FT_ASSERT_EQ(ER_SUCCESS, scheduler_error);
    FT_ASSERT_EQ(ft_errno, scheduler_error);
    return (1);
}

FT_TEST(test_task_scheduler_cancel_periodic_handle, "ft_task_scheduler cancels periodic task")
{
    ft_task_scheduler scheduler_instance(1);
    ft_atomic<int> execution_count;

    execution_count.store(0);
    ft_scheduled_task_handle periodic_handle = scheduler_instance.schedule_every(std::chrono::milliseconds(120),
        [&execution_count]()
    {
        execution_count.fetch_add(1);
        return ;
    });
    FT_ASSERT(periodic_handle.valid());
    bool cancel_result;

    cancel_result = periodic_handle.cancel();
    FT_ASSERT(cancel_result);
    int handle_error;

    handle_error = periodic_handle.get_error();
    FT_ASSERT_EQ(ER_SUCCESS, handle_error);
    FT_ASSERT_EQ(ft_errno, handle_error);
    usleep(200000);
    int executed_times;

    executed_times = execution_count.load();
    FT_ASSERT_EQ(0, executed_times);
    int scheduler_error;

    scheduler_error = scheduler_instance.get_error();
    FT_ASSERT_EQ(ER_SUCCESS, scheduler_error);
    FT_ASSERT_EQ(ft_errno, scheduler_error);
    return (1);
}
