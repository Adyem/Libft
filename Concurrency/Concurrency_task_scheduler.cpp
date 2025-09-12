#include "task_scheduler.hpp"

ft_task_scheduler::ft_task_scheduler(size_t thread_count)
    : _queue(), _workers(), _timer_thread(), _scheduled(), _scheduled_mutex(), _running(true), _error_code(ER_SUCCESS)
{
    size_t index;

    if (thread_count == 0)
        thread_count = std::thread::hardware_concurrency();
    index = 0;
    while (index < thread_count)
    {
        this->_workers.push_back(std::thread(&ft_task_scheduler::worker_loop, this));
        index++;
    }
    this->_timer_thread = std::thread(&ft_task_scheduler::timer_loop, this);
    return ;
}

ft_task_scheduler::~ft_task_scheduler()
{
    this->_running = false;
    size_t index;

    index = 0;
    while (index < this->_workers.size())
    {
        if (this->_workers[index].joinable())
            this->_workers[index].join();
        index++;
    }
    if (this->_timer_thread.joinable())
        this->_timer_thread.join();
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
    while (this->_running)
    {
        std::function<void()> task;

        if (this->_queue.pop(task))
            task();
        else
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return ;
}

void ft_task_scheduler::timer_loop()
{
    while (this->_running)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        std::unique_lock<std::mutex> lock(this->_scheduled_mutex);
        size_t index;
        auto now = std::chrono::steady_clock::now();

        index = 0;
        while (index < this->_scheduled.size())
        {
            if (now >= this->_scheduled[index]._time)
            {
                this->_queue.push(ft_move(this->_scheduled[index]._function));
                if (this->_scheduled[index]._interval.count() > 0)
                {
                    this->_scheduled[index]._time = now + this->_scheduled[index]._interval;
                    index++;
                }
                else
                {
                    this->_scheduled.erase(this->_scheduled.begin() + index);
                }
            }
            else
            {
                index++;
            }
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

