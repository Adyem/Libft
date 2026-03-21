#include "../test_internal.hpp"
#include "../../PThread/task_scheduler.hpp"
#include "../../PThread/pthread.hpp"
#include "../../System_utils/test_system_utils_runner.hpp"
#include <atomic>
#include "../../Errno/errno.hpp"
#include <chrono>
#include <memory>
#include <new>
#include <unistd.h>

#ifndef LIBFT_TEST_BUILD
#endif

struct task_scheduler_metrics_wait_context
{
    int32_t status;
};

static int32_t task_scheduler_metrics_flow_body()
{
    ft_task_scheduler scheduler_instance(1);
    long long queue_size;
    long long scheduled_count;
    long long idle_count;
    long long active_count;
    size_t worker_total;
    std::shared_ptr<std::atomic<bool> > release_flag;
    ft_future<void> blocking_future;
    ft_future<void> queued_future;
    ft_scheduled_task_handle delayed_handle;
    bool cancel_result;
    int32_t wait_iteration;

    if (scheduler_instance.initialize() != FT_ERR_SUCCESS)
        return (0);
    queue_size = scheduler_instance.get_queue_size();
    if (queue_size != 0)
        return (0);
    scheduled_count = scheduler_instance.get_scheduled_task_count();
    if (scheduled_count != 0)
        return (0);
    idle_count = scheduler_instance.get_worker_idle_count();
    if (idle_count != 1)
        return (0);
    active_count = scheduler_instance.get_worker_active_count();
    if (active_count != 0)
        return (0);
    worker_total = scheduler_instance.get_worker_total_count();
    if (worker_total != 1)
        return (0);
    release_flag = std::make_shared<std::atomic<bool> >(false);
    if (blocking_future.initialize(scheduler_instance.submit([release_flag]() mutable
    {
        while (!release_flag->load())
            usleep(1000);
        return ;
    })) != FT_ERR_SUCCESS)
        return (0);
    if (!blocking_future.valid())
        return (0);
    usleep(50000);
    active_count = scheduler_instance.get_worker_active_count();
    if (active_count != 1)
        return (0);
    idle_count = scheduler_instance.get_worker_idle_count();
    if (idle_count != 0)
        return (0);
    if (queued_future.initialize(scheduler_instance.submit([]()
    {
        return ;
    })) != FT_ERR_SUCCESS)
        return (0);
    if (!queued_future.valid())
        return (0);
    usleep(20000);
    queue_size = scheduler_instance.get_queue_size();
    if (queue_size != 1)
        return (0);
    delayed_handle = scheduler_instance.schedule_after(std::chrono::milliseconds(200), []()
    {
        return ;
    }).get_value();
    if (!delayed_handle.valid())
        return (0);
    scheduled_count = scheduler_instance.get_scheduled_task_count();
    if (scheduled_count != 1)
        return (0);
    cancel_result = delayed_handle.cancel();
    if (!cancel_result)
        return (0);
    usleep(20000);
    scheduled_count = scheduler_instance.get_scheduled_task_count();
    if (scheduled_count != 0)
        return (0);
    release_flag->store(true);
    wait_iteration = 0;
    while (wait_iteration < 5000)
    {
        queue_size = scheduler_instance.get_queue_size();
        active_count = scheduler_instance.get_worker_active_count();
        idle_count = scheduler_instance.get_worker_idle_count();
        if (queue_size == 0 && active_count == 0 && idle_count == 1)
            break ;
        usleep(1000);
        wait_iteration += 1;
    }
    if (wait_iteration >= 5000)
        return (0);
    queue_size = scheduler_instance.get_queue_size();
    if (queue_size != 0)
        return (0);
    active_count = scheduler_instance.get_worker_active_count();
    if (active_count != 0)
        return (0);
    idle_count = scheduler_instance.get_worker_idle_count();
    if (idle_count != 1)
        return (0);
    worker_total = scheduler_instance.get_worker_total_count();
    if (worker_total != 1)
        return (0);
    return (FT_ERR_SUCCESS);
}

static void *task_scheduler_metrics_wait_worker(void *argument)
{
    task_scheduler_metrics_wait_context *context;

    context = static_cast<task_scheduler_metrics_wait_context *>(argument);
    if (context == ft_nullptr)
        return (ft_nullptr);
    context->status = task_scheduler_metrics_flow_body();
    return (ft_nullptr);
}

FT_TEST(test_task_scheduler_metrics_flow)
{
    task_scheduler_metrics_wait_context *wait_context;
    pthread_t wait_thread;
    int32_t thread_create_result;
    int32_t thread_join_result;
    wait_context = new (std::nothrow) task_scheduler_metrics_wait_context();
    if (wait_context == ft_nullptr)
        return (0);
    wait_context->status = FT_ERR_INVALID_STATE;
    thread_create_result = pt_thread_create(&wait_thread, ft_nullptr,
            task_scheduler_metrics_wait_worker, wait_context);
    if (thread_create_result != 0)
    {
        delete wait_context;
        return (0);
    }
    thread_join_result = pt_thread_timed_join(wait_thread, ft_nullptr, 5000);
    if (thread_join_result != 0)
    {
        (void)pt_thread_detach(wait_thread);
        return (0);
    }
    FT_ASSERT_EQ(FT_ERR_SUCCESS, wait_context->status);
    delete wait_context;
    return (1);
}
