#include "task_scheduler.hpp"
#include "pthread.hpp"
#include "../System_utils/system_utils.hpp"

ft_task_scheduler::ft_task_scheduler(size_t thread_count)
    : _queue(), _workers(), _timer_thread(), _scheduled(), _scheduled_mutex(),
      _scheduled_condition(), _running(true), _error_code(ER_SUCCESS)
{
    size_t index;
    unsigned int cpu_count;
    bool worker_failure;

    if (this->_queue.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_queue.get_error());
        this->_running.store(false);
        return ;
    }
    if (this->_scheduled_condition.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_scheduled_condition.get_error());
        this->_running.store(false);
        return ;
    }
    if (thread_count == 0)
    {
        cpu_count = su_get_cpu_count();
        if (cpu_count == 0)
            thread_count = 1;
        else
            thread_count = cpu_count;
    }
    worker_failure = false;
    this->_workers.reserve(thread_count);
    if (this->_workers.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_workers.get_error());
        this->_running.store(false);
        return ;
    }
    index = 0;
    while (index < thread_count)
    {
        ft_thread worker([this]()
        {
            this->worker_loop();
            return ;
        });

        if (worker.get_error() != ER_SUCCESS)
        {
            this->set_error(worker.get_error());
            worker_failure = true;
            break;
        }
        this->_workers.push_back(ft_move(worker));
        if (this->_workers.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_workers.get_error());
            worker_failure = true;
            break;
        }
        index++;
    }
    if (worker_failure)
    {
        size_t join_index;
        size_t worker_count;

        this->_running.store(false);
        worker_count = this->_workers.size();
        join_index = 0;
        while (join_index < worker_count)
        {
            ft_thread &worker_reference = this->_workers[join_index];

            if (worker_reference.joinable())
                worker_reference.join();
            join_index++;
        }
        return ;
    }
    ft_thread timer_worker([this]()
    {
        this->timer_loop();
        return ;
    });
    if (timer_worker.get_error() != ER_SUCCESS)
    {
        this->set_error(timer_worker.get_error());
        this->_running.store(false);
        return ;
    }
    this->_timer_thread = ft_move(timer_worker);
    this->set_error(ER_SUCCESS);
    return ;
}

ft_task_scheduler::~ft_task_scheduler()
{
    this->_running.store(false);
    this->_queue.shutdown();
    if (this->_scheduled_mutex.lock(THREAD_ID) == FT_SUCCESS)
    {
        if (this->_scheduled_condition.broadcast() != 0)
        {
            this->set_error(this->_scheduled_condition.get_error());
        }
        this->_scheduled_mutex.unlock(THREAD_ID);
    }
    size_t index;
    size_t worker_count;

    worker_count = this->_workers.size();
    index = 0;
    while (index < worker_count)
    {
        ft_thread &worker_reference = this->_workers[index];

        if (worker_reference.joinable())
            worker_reference.join();
        index++;
    }
    if (this->_timer_thread.joinable())
        this->_timer_thread.join();
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_task_scheduler::set_error(int error) const
{
    this->_error_code = error;
    ft_errno = error;
    return ;
}

void ft_task_scheduler::worker_loop()
{
    while (true)
    {
        ft_function<void()> task;
        bool has_task;

        if (!this->_running.load())
            break;
        has_task = this->_queue.wait_pop(task, this->_running);
        if (!has_task)
        {
            if (!this->_running.load())
                break;
            this->set_error(this->_queue.get_error());
            continue;
        }
        this->set_error(ER_SUCCESS);
        if (task)
            task();
    }
    return ;
}

void ft_task_scheduler::scheduled_heap_sift_up(size_t index)
{
    while (index > 0)
    {
        size_t parent_index;
        int comparison;
        scheduled_task temp_task;

        parent_index = (index - 1) / 2;
        comparison = time_monotonic_point_compare(this->_scheduled[index]._time,
                this->_scheduled[parent_index]._time);
        if (comparison >= 0)
            break;
        temp_task = ft_move(this->_scheduled[index]);
        this->_scheduled[index] = ft_move(this->_scheduled[parent_index]);
        this->_scheduled[parent_index] = ft_move(temp_task);
        index = parent_index;
    }
    return ;
}

void ft_task_scheduler::scheduled_heap_sift_down(size_t index)
{
    while (true)
    {
        size_t size;
        size_t left_child;
        size_t right_child;
        size_t smallest;
        int comparison_left;
        int comparison_right;
        scheduled_task temp_task;

        size = this->_scheduled.size();
        if (index >= size)
            break;
        left_child = index * 2 + 1;
        if (left_child >= size)
            break;
        smallest = left_child;
        right_child = left_child + 1;
        if (right_child < size)
        {
            comparison_right = time_monotonic_point_compare(
                    this->_scheduled[right_child]._time,
                    this->_scheduled[smallest]._time);
            if (comparison_right < 0)
                smallest = right_child;
        }
        comparison_left = time_monotonic_point_compare(
                this->_scheduled[index]._time,
                this->_scheduled[smallest]._time);
        if (comparison_left <= 0)
            break;
        temp_task = ft_move(this->_scheduled[index]);
        this->_scheduled[index] = ft_move(this->_scheduled[smallest]);
        this->_scheduled[smallest] = ft_move(temp_task);
        index = smallest;
    }
    return ;
}

bool ft_task_scheduler::scheduled_heap_push(scheduled_task &&task)
{
    size_t size;

    this->_scheduled.push_back(ft_move(task));
    if (this->_scheduled.get_error() != ER_SUCCESS)
        return (false);
    size = this->_scheduled.size();
    if (size == 0)
        return (true);
    this->scheduled_heap_sift_up(size - 1);
    return (true);
}

bool ft_task_scheduler::scheduled_heap_pop(scheduled_task &task)
{
    size_t size;

    size = this->_scheduled.size();
    if (this->_scheduled.get_error() != ER_SUCCESS)
        return (false);
    if (size == 0)
        return (false);
    task = ft_move(this->_scheduled[0]);
    if (size == 1)
    {
        this->_scheduled.pop_back();
        if (this->_scheduled.get_error() != ER_SUCCESS)
            return (false);
        return (true);
    }
    scheduled_task last_task;

    last_task = ft_move(this->_scheduled[size - 1]);
    this->_scheduled.pop_back();
    if (this->_scheduled.get_error() != ER_SUCCESS)
        return (false);
    this->_scheduled[0] = ft_move(last_task);
    this->scheduled_heap_sift_down(0);
    return (true);
}

void ft_task_scheduler::timer_loop()
{
    while (true)
    {
        if (!this->_running.load())
            break;
        if (this->_scheduled_mutex.lock(THREAD_ID) != FT_SUCCESS)
        {
            this->set_error(this->_scheduled_mutex.get_error());
            return ;
        }
        while (true)
        {
            size_t scheduled_count;
            bool has_entries;

            if (!this->_running.load())
            {
                this->_scheduled_mutex.unlock(THREAD_ID);
                return ;
            }
            scheduled_count = this->_scheduled.size();
            if (this->_scheduled.get_error() != ER_SUCCESS)
            {
                this->_scheduled_mutex.unlock(THREAD_ID);
                this->set_error(this->_scheduled.get_error());
                return ;
            }
            has_entries = scheduled_count > 0;
            if (!has_entries)
            {
                if (this->_scheduled_condition.wait(this->_scheduled_mutex) != 0)
                {
                    int wait_error;

                    wait_error = this->_scheduled_condition.get_error();
                    this->_scheduled_mutex.unlock(THREAD_ID);
                    this->set_error(wait_error);
                    return ;
                }
                continue;
            }
            t_monotonic_time_point earliest_time;
            t_monotonic_time_point current_time;
            long long wait_milliseconds;

            earliest_time = this->_scheduled[0]._time;
            current_time = time_monotonic_point_now();
            wait_milliseconds = time_monotonic_point_diff_ms(current_time, earliest_time);
            if (wait_milliseconds > 0)
            {
                struct timespec wake_time;
                long long seconds;
                long long nanoseconds;
                int wait_result;

#if defined(CLOCK_MONOTONIC)
                if (clock_gettime(CLOCK_MONOTONIC, &wake_time) != 0)
#else
                if (clock_gettime(CLOCK_REALTIME, &wake_time) != 0)
#endif
                {
                    int clock_error;

                    clock_error = errno + ERRNO_OFFSET;
                    this->_scheduled_mutex.unlock(THREAD_ID);
                    this->set_error(clock_error);
                    return ;
                }
                seconds = wait_milliseconds / 1000;
                nanoseconds = (wait_milliseconds % 1000) * 1000000;
                wake_time.tv_sec += seconds;
                wake_time.tv_nsec += nanoseconds;
                if (wake_time.tv_nsec >= 1000000000)
                {
                    long long carry;

                    carry = wake_time.tv_nsec / 1000000000;
                    wake_time.tv_sec += carry;
                    wake_time.tv_nsec = wake_time.tv_nsec % 1000000000;
                }
                wait_result = this->_scheduled_condition.wait_until(this->_scheduled_mutex, wake_time);
                if (wait_result == 0)
                    continue;
                if (wait_result != ETIMEDOUT)
                {
                    int condition_error;

                    condition_error = this->_scheduled_condition.get_error();
                    this->_scheduled_mutex.unlock(THREAD_ID);
                    this->set_error(condition_error);
                    return ;
                }
            }
            break;
        }
        while (true)
        {
            size_t scheduled_count;
            int time_comparison;
            t_monotonic_time_point now;
            scheduled_task expired_task;
            bool pop_success;

            scheduled_count = this->_scheduled.size();
            if (this->_scheduled.get_error() != ER_SUCCESS)
            {
                this->_scheduled_mutex.unlock(THREAD_ID);
                this->set_error(this->_scheduled.get_error());
                return ;
            }
            if (scheduled_count == 0)
                break;
            now = time_monotonic_point_now();
            time_comparison = time_monotonic_point_compare(now, this->_scheduled[0]._time);
            if (time_comparison < 0)
                break;
            pop_success = this->scheduled_heap_pop(expired_task);
            if (!pop_success)
            {
                int scheduled_error;

                scheduled_error = this->_scheduled.get_error();
                if (this->_scheduled_mutex.unlock(THREAD_ID) != FT_SUCCESS)
                    this->set_error(this->_scheduled_mutex.get_error());
                else
                    this->set_error(scheduled_error);
                return ;
            }
            if (expired_task._function.get_error() != ER_SUCCESS)
            {
                int function_error;
                ft_function<void()> original_function;

                function_error = expired_task._function.get_error();
                this->set_error(function_error);
                original_function = ft_move(expired_task._function);
                if (this->_scheduled_mutex.unlock(THREAD_ID) != FT_SUCCESS)
                {
                    this->set_error(this->_scheduled_mutex.get_error());
                    return ;
                }
                if (original_function)
                    original_function();
                if (!this->_running.load())
                    return ;
                if (this->_scheduled_mutex.lock(THREAD_ID) != FT_SUCCESS)
                {
                    this->set_error(this->_scheduled_mutex.get_error());
                    return ;
                }
                if (expired_task._interval_ms > 0)
                {
                    t_monotonic_time_point updated_time;

                    updated_time = time_monotonic_point_add_ms(time_monotonic_point_now(),
                            expired_task._interval_ms);
                    expired_task._time = updated_time;
                    expired_task._function = ft_move(original_function);
                    if (!this->scheduled_heap_push(ft_move(expired_task)))
                        this->set_error(this->_scheduled.get_error());
                }
                continue;
            }
            ft_function<void()> queue_function;

            queue_function = expired_task._function;
            if (queue_function.get_error() != ER_SUCCESS)
            {
                int copy_error;
                ft_function<void()> original_function;

                copy_error = queue_function.get_error();
                this->set_error(copy_error);
                original_function = ft_move(expired_task._function);
                if (this->_scheduled_mutex.unlock(THREAD_ID) != FT_SUCCESS)
                {
                    this->set_error(this->_scheduled_mutex.get_error());
                    return ;
                }
                if (original_function)
                    original_function();
                if (!this->_running.load())
                    return ;
                if (this->_scheduled_mutex.lock(THREAD_ID) != FT_SUCCESS)
                {
                    this->set_error(this->_scheduled_mutex.get_error());
                    return ;
                }
                if (expired_task._interval_ms > 0)
                {
                    t_monotonic_time_point updated_time;

                    updated_time = time_monotonic_point_add_ms(time_monotonic_point_now(),
                            expired_task._interval_ms);
                    expired_task._time = updated_time;
                    expired_task._function = ft_move(original_function);
                    if (!this->scheduled_heap_push(ft_move(expired_task)))
                        this->set_error(this->_scheduled.get_error());
                }
                continue;
            }
            if (this->_scheduled_mutex.unlock(THREAD_ID) != FT_SUCCESS)
            {
                this->set_error(this->_scheduled_mutex.get_error());
                return ;
            }
            this->_queue.push(ft_move(queue_function));
            if (this->_queue.get_error() != ER_SUCCESS)
            {
                this->set_error(this->_queue.get_error());
                if (expired_task._function)
                    expired_task._function();
            }
            else
                this->set_error(ER_SUCCESS);
            if (!this->_running.load())
                return ;
            if (this->_scheduled_mutex.lock(THREAD_ID) != FT_SUCCESS)
            {
                this->set_error(this->_scheduled_mutex.get_error());
                return ;
            }
            if (expired_task._interval_ms > 0)
            {
                t_monotonic_time_point updated_time;

                updated_time = time_monotonic_point_add_ms(time_monotonic_point_now(),
                        expired_task._interval_ms);
                expired_task._time = updated_time;
                if (!this->scheduled_heap_push(ft_move(expired_task)))
                    this->set_error(this->_scheduled.get_error());
            }
            continue;
        }
        if (this->_scheduled_mutex.unlock(THREAD_ID) != FT_SUCCESS)
        {
            this->set_error(this->_scheduled_mutex.get_error());
            return ;
        }
    }
    return ;
}

int ft_task_scheduler::get_error() const
{
    return (this->_error_code);
}

const char *ft_task_scheduler::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}

