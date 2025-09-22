#include "task_scheduler.hpp"
#include "pthread.hpp"
#include "../System_utils/system_utils.hpp"

ft_task_scheduler::ft_task_scheduler(size_t thread_count)
    : _queue(), _workers(), _timer_thread(), _scheduled(), _scheduled_mutex(), _running(true), _error_code(ER_SUCCESS)
{
    size_t index;
    unsigned int cpu_count;
    bool worker_failure;

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
    while (this->_running.load())
    {
        ft_function<void()> task;

        if (this->_queue.pop(task))
        {
            this->set_error(ER_SUCCESS);
            if (task)
                task();
        }
        else
        {
            this->set_error(this->_queue.get_error());
            ft_this_thread_sleep_for(time_duration_ms_create(1));
        }
    }
    return ;
}

void ft_task_scheduler::timer_loop()
{
    while (this->_running.load())
    {
        ft_this_thread_sleep_for(time_duration_ms_create(1));
        ft_unique_lock<pt_mutex> lock(this->_scheduled_mutex);
        size_t index;

        if (lock.get_error() != ER_SUCCESS)
        {
            this->set_error(lock.get_error());
            continue;
        }
        index = 0;
        while (index < this->_scheduled.size())
        {
            t_monotonic_time_point current_time;

            current_time = time_monotonic_point_now();
            if (time_monotonic_point_compare(current_time, this->_scheduled[index]._time) >= 0)
            {
                if (this->_scheduled[index]._function)
                {
                    ft_task_scheduler::scheduled_task expired_task;
                    ft_function<void()> queue_function;
                    bool lock_released;
                    bool fallback_executed;

                    expired_task = this->_scheduled[index];
                    if (expired_task._function.get_error() != ER_SUCCESS)
                    {
                        this->set_error(expired_task._function.get_error());
                        lock_released = false;
                        fallback_executed = false;
                        if (this->_scheduled[index]._function && lock.owns_lock())
                        {
                            lock.unlock();
                            if (lock.get_error() == ER_SUCCESS)
                            {
                                lock_released = true;
                                this->_scheduled[index]._function();
                                fallback_executed = true;
                            }
                            else
                                this->set_error(lock.get_error());
                        }
                        if (lock_released)
                        {
                            lock.lock();
                            if (lock.get_error() != ER_SUCCESS)
                            {
                                this->set_error(lock.get_error());
                                break;
                            }
                        }
                        if (!lock.owns_lock())
                            break;
                        current_time = time_monotonic_point_now();
                        if (fallback_executed && this->_scheduled[index]._interval_ms > 0)
                        {
                            this->_scheduled[index]._time = time_monotonic_point_add_ms(current_time, this->_scheduled[index]._interval_ms);
                            index++;
                        }
                        else
                            this->_scheduled.erase(this->_scheduled.begin() + index);
                        continue;
                    }
                    queue_function = expired_task._function;
                    if (queue_function.get_error() != ER_SUCCESS)
                    {
                        this->set_error(queue_function.get_error());
                        lock_released = false;
                        fallback_executed = false;
                        if (expired_task._function && lock.owns_lock())
                        {
                            lock.unlock();
                            if (lock.get_error() == ER_SUCCESS)
                            {
                                lock_released = true;
                                expired_task._function();
                                fallback_executed = true;
                            }
                            else
                                this->set_error(lock.get_error());
                        }
                        if (lock_released)
                        {
                            lock.lock();
                            if (lock.get_error() != ER_SUCCESS)
                            {
                                this->set_error(lock.get_error());
                                break;
                            }
                        }
                        if (!lock.owns_lock())
                            break;
                        current_time = time_monotonic_point_now();
                        if (fallback_executed && expired_task._interval_ms > 0)
                        {
                            this->_scheduled[index]._time = time_monotonic_point_add_ms(current_time, expired_task._interval_ms);
                            index++;
                        }
                        else
                            this->_scheduled.erase(this->_scheduled.begin() + index);
                        continue;
                    }
                    this->_queue.push(ft_move(queue_function));
                    if (this->_queue.get_error() != ER_SUCCESS)
                    {
                        this->set_error(this->_queue.get_error());
                        lock_released = false;
                        fallback_executed = false;
                        if (expired_task._function && lock.owns_lock())
                        {
                            lock.unlock();
                            if (lock.get_error() == ER_SUCCESS)
                            {
                                lock_released = true;
                                expired_task._function();
                                fallback_executed = true;
                            }
                            else
                                this->set_error(lock.get_error());
                        }
                        if (lock_released)
                        {
                            lock.lock();
                            if (lock.get_error() != ER_SUCCESS)
                            {
                                this->set_error(lock.get_error());
                                break;
                            }
                        }
                        if (!lock.owns_lock())
                            break;
                        current_time = time_monotonic_point_now();
                        if (fallback_executed && expired_task._interval_ms > 0)
                        {
                            this->_scheduled[index]._time = time_monotonic_point_add_ms(current_time, expired_task._interval_ms);
                            index++;
                        }
                        else
                            this->_scheduled.erase(this->_scheduled.begin() + index);
                        continue;
                    }
                    this->set_error(ER_SUCCESS);
                }
                if (!lock.owns_lock())
                    break;
                current_time = time_monotonic_point_now();
                if (this->_scheduled[index]._interval_ms > 0)
                {
                    this->_scheduled[index]._time = time_monotonic_point_add_ms(current_time, this->_scheduled[index]._interval_ms);
                    index++;
                }
                else
                    this->_scheduled.erase(this->_scheduled.begin() + index);
            }
            else
                index++;
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

