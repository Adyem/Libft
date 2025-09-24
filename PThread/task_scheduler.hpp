#ifndef TASK_SCHEDULER_HPP
#define TASK_SCHEDULER_HPP

#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Template/move.hpp"
#include "../Template/function.hpp"
#include "../Template/vector.hpp"
#include "../Template/shared_ptr.hpp"
#include "../Template/promise.hpp"
#include "../Template/future.hpp"
#include "../Template/atomic.hpp"
#include "../Template/queue.hpp"
#include "thread.hpp"
#include "../Time/time.hpp"
#include "condition.hpp"

#include <pthread.h>
#include <cerrno>
#include <ctime>

#include <atomic>
#include <chrono>
#include <new>
#include <type_traits>



template <typename ElementType>
class ft_lock_free_queue
{
    private:
        pt_mutex _mutex;
        pt_condition_variable _condition;
        bool _shutdown;
        ft_queue<ElementType> _storage;
        mutable int _error_code;

        void set_error(int error) const;

    public:
        ft_lock_free_queue();
        ~ft_lock_free_queue();

        ft_lock_free_queue(const ft_lock_free_queue&) = delete;
        ft_lock_free_queue &operator=(const ft_lock_free_queue&) = delete;

        void push(ElementType &&value);
        bool pop(ElementType &result);
        bool wait_pop(ElementType &result, const ft_atomic<bool> &running_flag);
        void shutdown();

        int get_error() const;
        const char *get_error_str() const;
};

class ft_task_scheduler
{
    private:
        struct scheduled_task
        {
            t_monotonic_time_point _time;
            long long _interval_ms;
            ft_function<void()> _function;
        };

        ft_lock_free_queue<ft_function<void()> > _queue;
        ft_vector<ft_thread> _workers;
        ft_thread _timer_thread;
        ft_vector<scheduled_task> _scheduled;
        pt_mutex _scheduled_mutex;
        pt_condition_variable _scheduled_condition;
        ft_atomic<bool> _running;
        mutable int _error_code;

        void set_error(int error) const;
        void worker_loop();
        void timer_loop();

    public:
        ft_task_scheduler(size_t thread_count = 0);
        ~ft_task_scheduler();

        ft_task_scheduler(const ft_task_scheduler &) = delete;
        ft_task_scheduler &operator=(const ft_task_scheduler &) = delete;
        ft_task_scheduler(ft_task_scheduler &&) = delete;
        ft_task_scheduler &operator=(ft_task_scheduler &&) = delete;

        template <typename FunctionType, typename... Args>
        auto submit(FunctionType function, Args... args)
            -> ft_future<typename std::invoke_result<FunctionType, Args...>::type>;

        template <typename Rep, typename Period, typename FunctionType, typename... Args>
        auto schedule_after(std::chrono::duration<Rep, Period> delay,
                FunctionType function, Args... args)
            -> ft_future<typename std::invoke_result<FunctionType, Args...>::type>;

        template <typename Rep, typename Period, typename FunctionType, typename... Args>
        void schedule_every(std::chrono::duration<Rep, Period> interval,
                FunctionType function, Args... args);

        int get_error() const;
        const char *get_error_str() const;
};



template <typename ElementType>
ft_lock_free_queue<ElementType>::ft_lock_free_queue()
    : _mutex(), _condition(), _shutdown(false), _storage(), _error_code(ER_SUCCESS)
{
    if (this->_condition.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_condition.get_error());
        return ;
    }
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ElementType>
ft_lock_free_queue<ElementType>::~ft_lock_free_queue()
{
    this->shutdown();
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ElementType>
void ft_lock_free_queue<ElementType>::set_error(int error) const
{
    this->_error_code = error;
    ft_errno = error;
    return ;
}

template <typename ElementType>
void ft_lock_free_queue<ElementType>::push(ElementType &&value)
{
    bool was_empty;

    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->set_error(this->_mutex.get_error());
        return ;
    }
    was_empty = this->_storage.empty();
    if (this->_storage.get_error() != ER_SUCCESS)
    {
        this->_mutex.unlock(THREAD_ID);
        this->set_error(this->_storage.get_error());
        return ;
    }
    this->_storage.enqueue(ft_move(value));
    if (this->_storage.get_error() != ER_SUCCESS)
    {
        this->_mutex.unlock(THREAD_ID);
        this->set_error(this->_storage.get_error());
        return ;
    }
    if (this->_mutex.unlock(THREAD_ID) != SUCCES)
    {
        this->set_error(this->_mutex.get_error());
        return ;
    }
    if (was_empty)
    {
        if (this->_condition.signal() != 0)
        {
            this->set_error(this->_condition.get_error());
            return ;
        }
    }
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ElementType>
bool ft_lock_free_queue<ElementType>::pop(ElementType &result)
{
    bool is_empty;
    ElementType value;

    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->set_error(this->_mutex.get_error());
        return (false);
    }
    is_empty = this->_storage.empty();
    if (this->_storage.get_error() != ER_SUCCESS)
    {
        if (this->_mutex.unlock(THREAD_ID) != SUCCES)
        {
            this->set_error(this->_mutex.get_error());
            return (false);
        }
        this->set_error(this->_storage.get_error());
        return (false);
    }
    if (is_empty)
    {
        if (this->_mutex.unlock(THREAD_ID) != SUCCES)
        {
            this->set_error(this->_mutex.get_error());
            return (false);
        }
        this->set_error(QUEUE_EMPTY);
        return (false);
    }
    value = this->_storage.dequeue();
    if (this->_storage.get_error() != ER_SUCCESS)
    {
        if (this->_mutex.unlock(THREAD_ID) != SUCCES)
        {
            this->set_error(this->_mutex.get_error());
            return (false);
        }
        this->set_error(this->_storage.get_error());
        return (false);
    }
    if (this->_mutex.unlock(THREAD_ID) != SUCCES)
    {
        this->set_error(this->_mutex.get_error());
        return (false);
    }
    result = ft_move(value);
    this->set_error(ER_SUCCESS);
    return (true);
}

template <typename ElementType>
bool ft_lock_free_queue<ElementType>::wait_pop(ElementType &result, const ft_atomic<bool> &running_flag)
{
    bool is_empty;
    ElementType value;

    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->set_error(this->_mutex.get_error());
        return (false);
    }
    while (true)
    {
        is_empty = this->_storage.empty();
        if (this->_storage.get_error() != ER_SUCCESS)
        {
            if (this->_mutex.unlock(THREAD_ID) != SUCCES)
            {
                this->set_error(this->_mutex.get_error());
                return (false);
            }
            this->set_error(this->_storage.get_error());
            return (false);
        }
        if (!is_empty)
            break;
        if (!running_flag.load() || this->_shutdown)
        {
            if (this->_mutex.unlock(THREAD_ID) != SUCCES)
            {
                this->set_error(this->_mutex.get_error());
                return (false);
            }
            this->set_error(QUEUE_EMPTY);
            return (false);
        }
        if (this->_condition.wait(this->_mutex) != 0)
        {
            int condition_error;

            condition_error = this->_condition.get_error();
            if (this->_mutex.unlock(THREAD_ID) != SUCCES)
            {
                this->set_error(this->_mutex.get_error());
                return (false);
            }
            this->set_error(condition_error);
            return (false);
        }
    }
    value = this->_storage.dequeue();
    if (this->_storage.get_error() != ER_SUCCESS)
    {
        if (this->_mutex.unlock(THREAD_ID) != SUCCES)
        {
            this->set_error(this->_mutex.get_error());
            return (false);
        }
        this->set_error(this->_storage.get_error());
        return (false);
    }
    if (this->_mutex.unlock(THREAD_ID) != SUCCES)
    {
        this->set_error(this->_mutex.get_error());
        return (false);
    }
    result = ft_move(value);
    this->set_error(ER_SUCCESS);
    return (true);
}

template <typename ElementType>
void ft_lock_free_queue<ElementType>::shutdown()
{
    if (this->_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->set_error(this->_mutex.get_error());
        return ;
    }
    this->_shutdown = true;
    if (this->_mutex.unlock(THREAD_ID) != SUCCES)
    {
        this->set_error(this->_mutex.get_error());
        return ;
    }
    if (this->_condition.broadcast() != 0)
    {
        this->set_error(this->_condition.get_error());
        return ;
    }
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ElementType>
int ft_lock_free_queue<ElementType>::get_error() const
{
    return (this->_error_code);
}

template <typename ElementType>
const char *ft_lock_free_queue<ElementType>::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}



template <typename FunctionType, typename... Args>
auto ft_task_scheduler::submit(FunctionType function, Args... args)
    -> ft_future<typename std::invoke_result<FunctionType, Args...>::type>
{
    using return_type = typename std::invoke_result<FunctionType, Args...>::type;
    using promise_type = ft_promise<return_type>;

    promise_type *promise_raw;
    ft_sharedptr<promise_type> promise_shared;

    promise_raw = new (std::nothrow) promise_type();
    if (!promise_raw)
    {
        this->set_error(FT_EALLOC);
        ft_future<return_type> empty_future;
        return (empty_future);
    }
    promise_shared.reset(promise_raw, 1, false);
    if (promise_shared.hasError())
    {
        this->set_error(promise_shared.get_error());
        ft_future<return_type> empty_future;
        return (empty_future);
    }
    ft_future<return_type> future_value(promise_shared);
    if (future_value.get_error() != ER_SUCCESS)
    {
        this->set_error(future_value.get_error());
        return (future_value);
    }
    auto task_body = [promise_shared, function, args...]() mutable
    {
        if (!promise_shared)
        {
            if constexpr (std::is_void_v<return_type>)
                function(args...);
            else
                (void)function(args...);
            return ;
        }
        if constexpr (std::is_void_v<return_type>)
        {
            function(args...);
            promise_shared->set_value();
        }
        else
        {
            return_type result_value;

            result_value = function(args...);
            promise_shared->set_value(ft_move(result_value));
        }
        return ;
    };
    ft_function<void()> wrapper(task_body);
    if (wrapper.get_error() != ER_SUCCESS)
    {
        this->set_error(wrapper.get_error());
        task_body();
        return (future_value);
    }
    this->_queue.push(ft_move(wrapper));
    if (this->_queue.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_queue.get_error());
        task_body();
        return (future_value);
    }
    this->set_error(ER_SUCCESS);
    return (future_value);
}

template <typename Rep, typename Period, typename FunctionType, typename... Args>
auto ft_task_scheduler::schedule_after(std::chrono::duration<Rep, Period> delay,
        FunctionType function, Args... args)
    -> ft_future<typename std::invoke_result<FunctionType, Args...>::type>
{
    using return_type = typename std::invoke_result<FunctionType, Args...>::type;
    using promise_type = ft_promise<return_type>;

    promise_type *promise_raw;
    ft_sharedptr<promise_type> promise_shared;
    ft_future<return_type> future_value;

    promise_raw = new (std::nothrow) promise_type();
    if (!promise_raw)
    {
        this->set_error(FT_EALLOC);
        return (future_value);
    }
    promise_shared.reset(promise_raw, 1, false);
    if (promise_shared.hasError())
    {
        this->set_error(promise_shared.get_error());
        return (future_value);
    }
    future_value = ft_future<return_type>(promise_shared);
    if (future_value.get_error() != ER_SUCCESS)
    {
        this->set_error(future_value.get_error());
        return (future_value);
    }
    scheduled_task task_entry;
    std::chrono::milliseconds delay_duration;
    long long delay_milliseconds;
    t_monotonic_time_point start_point;

    delay_duration = std::chrono::duration_cast<std::chrono::milliseconds>(delay);
    delay_milliseconds = delay_duration.count();
    start_point = time_monotonic_point_now();
    task_entry._time = time_monotonic_point_add_ms(start_point, delay_milliseconds);
    task_entry._interval_ms = 0;
    auto task_body = [promise_shared, function, args...]() mutable
    {
        if (!promise_shared)
        {
            if constexpr (std::is_void_v<return_type>)
                function(args...);
            else
                (void)function(args...);
            return ;
        }
        if constexpr (std::is_void_v<return_type>)
        {
            function(args...);
            promise_shared->set_value();
        }
        else
        {
            return_type result_value;

            result_value = function(args...);
            promise_shared->set_value(ft_move(result_value));
        }
        return ;
    };
    task_entry._function = ft_function<void()>(task_body);
    if (task_entry._function.get_error() != ER_SUCCESS)
    {
        this->set_error(task_entry._function.get_error());
        task_body();
        return (future_value);
    }
    if (this->_scheduled_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->set_error(this->_scheduled_mutex.get_error());
        task_body();
        return (future_value);
    }
    this->_scheduled.push_back(ft_move(task_entry));
    if (this->_scheduled.get_error() != ER_SUCCESS)
    {
        if (this->_scheduled_mutex.unlock(THREAD_ID) != SUCCES)
            this->set_error(this->_scheduled_mutex.get_error());
        else
            this->set_error(this->_scheduled.get_error());
        task_body();
        return (future_value);
    }
    if (this->_scheduled_mutex.unlock(THREAD_ID) != SUCCES)
    {
        this->set_error(this->_scheduled_mutex.get_error());
        task_body();
        return (future_value);
    }
    if (this->_scheduled_condition.signal() != 0)
    {
        this->set_error(this->_scheduled_condition.get_error());
        return (future_value);
    }
    this->set_error(ER_SUCCESS);
    return (future_value);
}

template <typename Rep, typename Period, typename FunctionType, typename... Args>
void ft_task_scheduler::schedule_every(std::chrono::duration<Rep, Period> interval,
        FunctionType function, Args... args)
{
    scheduled_task task_entry;
    std::chrono::milliseconds interval_duration;
    long long interval_milliseconds;
    t_monotonic_time_point start_point;

    interval_duration = std::chrono::duration_cast<std::chrono::milliseconds>(interval);
    interval_milliseconds = interval_duration.count();
    start_point = time_monotonic_point_now();
    task_entry._time = time_monotonic_point_add_ms(start_point, interval_milliseconds);
    task_entry._interval_ms = interval_milliseconds;
    task_entry._function = ft_function<void()>([function, args...]()
    {
        function(args...);
        return ;
    });
    if (task_entry._function.get_error() != ER_SUCCESS)
    {
        this->set_error(task_entry._function.get_error());
        return ;
    }
    if (this->_scheduled_mutex.lock(THREAD_ID) != SUCCES)
    {
        this->set_error(this->_scheduled_mutex.get_error());
        task_entry._function();
        return ;
    }
    this->_scheduled.push_back(ft_move(task_entry));
    if (this->_scheduled.get_error() != ER_SUCCESS)
    {
        if (this->_scheduled_mutex.unlock(THREAD_ID) != SUCCES)
            this->set_error(this->_scheduled_mutex.get_error());
        else
            this->set_error(this->_scheduled.get_error());
        task_entry._function();
        return ;
    }
    if (this->_scheduled_mutex.unlock(THREAD_ID) != SUCCES)
    {
        this->set_error(this->_scheduled_mutex.get_error());
        task_entry._function();
        return ;
    }
    if (this->_scheduled_condition.signal() != 0)
    {
        this->set_error(this->_scheduled_condition.get_error());
        return ;
    }
    this->set_error(ER_SUCCESS);
    return ;
}

#endif
