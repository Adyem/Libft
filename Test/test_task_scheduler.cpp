#include "../Concurrency/task_scheduler.hpp"
#include "../System_utils/test_runner.hpp"
#include <atomic>
#include <chrono>
#include <unistd.h>

FT_TEST(test_task_scheduler_submit, "ft_task_scheduler submit")
{
    ft_task_scheduler scheduler_instance(2);
    auto future_value = scheduler_instance.submit([]() { return (7); });
    FT_ASSERT_EQ(7, future_value.get());
    return (1);
}

FT_TEST(test_task_scheduler_schedule_after, "ft_task_scheduler schedule_after")
{
    ft_task_scheduler scheduler_instance(1);
    auto start_time = std::chrono::steady_clock::now();
    auto future_value = scheduler_instance.schedule_after(std::chrono::milliseconds(50), []() { return (3); });
    int result_value = future_value.get();
    auto end_time = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    FT_ASSERT_EQ(3, result_value);
    FT_ASSERT(elapsed.count() >= 50);
    return (1);
}

FT_TEST(test_task_scheduler_schedule_every, "ft_task_scheduler schedule_every")
{
    ft_task_scheduler scheduler_instance(1);
    std::atomic<int> call_counter;
    call_counter.store(0);
    scheduler_instance.schedule_every(std::chrono::milliseconds(10), [&call_counter]()
    {
        call_counter.fetch_add(1);
        return ;
    });
    usleep(50000);
    int count_value = call_counter.load();
    FT_ASSERT(count_value > 1);
    return (1);
}

