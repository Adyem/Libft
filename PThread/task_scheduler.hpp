#ifndef TASK_SCHEDULER_HPP
#define TASK_SCHEDULER_HPP

#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Template/function.hpp"
#include "../Template/vector.hpp"
#include "../Template/shared_ptr.hpp"
#include "../Template/promise.hpp"
#include "../Template/future.hpp"
#include "../Template/queue.hpp"
#include "../Template/pair.hpp"
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
#include <utility>



template <typename ElementType>
class ft_blocking_queue
{
    private:
        pt_mutex _mutex;
        pt_condition_variable _condition;
        bool _shutdown;
        ft_queue<ElementType> _storage;
        mutable int _error_code;

        void set_error(int error) const;

    public:
        ft_blocking_queue();
        ~ft_blocking_queue();

        ft_blocking_queue(const ft_blocking_queue&) = delete;
        ft_blocking_queue &operator=(const ft_blocking_queue&) = delete;

        void push(ElementType &&value);
        bool pop(ElementType &result);
        bool wait_pop(ElementType &result, const std::atomic<bool> &running_flag);
        void shutdown();

        int get_error() const;
        const char *get_error_str() const;
};

class ft_task_scheduler;

class ft_scheduled_task_state
{
    private:
        std::atomic<bool> _cancelled;
        mutable int _error_code;

        void set_error(int error) const;

    public:
        ft_scheduled_task_state();
        ~ft_scheduled_task_state();

        void cancel();
        bool is_cancelled() const;

        int get_error() const;
        const char *get_error_str() const;
};

class ft_scheduled_task_handle
{
    private:
        ft_sharedptr<ft_scheduled_task_state> _state;
        ft_task_scheduler *_scheduler;
        mutable int _error_code;

        void set_error(int error) const;

    public:
        ft_scheduled_task_handle();
        ft_scheduled_task_handle(ft_task_scheduler *scheduler,
                const ft_sharedptr<ft_scheduled_task_state> &state);
        ft_scheduled_task_handle(const ft_scheduled_task_handle &other);
        ft_scheduled_task_handle &operator=(const ft_scheduled_task_handle &other);
        ~ft_scheduled_task_handle();

        bool cancel();
        bool valid() const;
        ft_sharedptr<ft_scheduled_task_state> get_state() const;

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
            ft_sharedptr<ft_scheduled_task_state> _state;
        };

        ft_blocking_queue<ft_function<void()> > _queue;
        ft_vector<ft_thread> _workers;
        ft_thread _timer_thread;
        ft_vector<scheduled_task> _scheduled;
        mutable pt_mutex _scheduled_mutex;
        pt_condition_variable _scheduled_condition;
        std::atomic<bool> _running;
        mutable pt_mutex _queue_metrics_mutex;
        mutable pt_mutex _worker_metrics_mutex;
        long long _queue_size_counter;
        long long _scheduled_size_counter;
        long long _worker_active_counter;
        long long _worker_idle_counter;
        size_t _worker_total_count;
        mutable int _error_code;

        bool cancel_task_state(const ft_sharedptr<ft_scheduled_task_state> &state);
        bool scheduled_remove_index(size_t index);
        void set_error(int error) const;
        void worker_loop();
        void timer_loop();
        bool scheduled_heap_push(scheduled_task &&task);
        bool scheduled_heap_pop(scheduled_task &task);
        void scheduled_heap_sift_up(size_t index);
        void scheduled_heap_sift_down(size_t index);
        bool update_queue_size(long long delta);
        bool update_worker_counters(long long active_delta, long long idle_delta);
        bool update_worker_total(long long delta);

    public:
        friend class ft_scheduled_task_handle;

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
            -> Pair<ft_future<typename std::invoke_result<FunctionType, Args...>::type>,
                    ft_scheduled_task_handle>;

        template <typename Rep, typename Period, typename FunctionType, typename... Args>
        ft_scheduled_task_handle schedule_every(std::chrono::duration<Rep, Period> interval,
                FunctionType function, Args... args);

        int get_error() const;
        const char *get_error_str() const;
        long long get_queue_size() const;
        long long get_scheduled_task_count() const;
        long long get_worker_active_count() const;
        long long get_worker_idle_count() const;
        size_t get_worker_total_count() const;
};



template <typename ElementType>
ft_blocking_queue<ElementType>::ft_blocking_queue()
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
ft_blocking_queue<ElementType>::~ft_blocking_queue()
{
    this->shutdown();
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ElementType>
void ft_blocking_queue<ElementType>::set_error(int error) const
{
    this->_error_code = error;
    ft_errno = error;
    return ;
}

template <typename ElementType>
void ft_blocking_queue<ElementType>::push(ElementType &&value)
{
    bool was_empty;

    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
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
    this->_storage.enqueue(std::move(value));
    if (this->_storage.get_error() != ER_SUCCESS)
    {
        this->_mutex.unlock(THREAD_ID);
        this->set_error(this->_storage.get_error());
        return ;
    }
    if (this->_mutex.unlock(THREAD_ID) != FT_SUCCESS)
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
bool ft_blocking_queue<ElementType>::pop(ElementType &result)
{
    bool is_empty;
    ElementType value;

    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        this->set_error(this->_mutex.get_error());
        return (false);
    }
    is_empty = this->_storage.empty();
    if (this->_storage.get_error() != ER_SUCCESS)
    {
        if (this->_mutex.unlock(THREAD_ID) != FT_SUCCESS)
        {
            this->set_error(this->_mutex.get_error());
            return (false);
        }
        this->set_error(this->_storage.get_error());
        return (false);
    }
    if (is_empty)
    {
        if (this->_mutex.unlock(THREAD_ID) != FT_SUCCESS)
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
        if (this->_mutex.unlock(THREAD_ID) != FT_SUCCESS)
        {
            this->set_error(this->_mutex.get_error());
            return (false);
        }
        this->set_error(this->_storage.get_error());
        return (false);
    }
    if (this->_mutex.unlock(THREAD_ID) != FT_SUCCESS)
    {
        this->set_error(this->_mutex.get_error());
        return (false);
    }
    result = std::move(value);
    this->set_error(ER_SUCCESS);
    return (true);
}

template <typename ElementType>
bool ft_blocking_queue<ElementType>::wait_pop(ElementType &result, const std::atomic<bool> &running_flag)
{
    bool is_empty;
    ElementType value;

    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        this->set_error(this->_mutex.get_error());
        return (false);
    }
    while (true)
    {
        is_empty = this->_storage.empty();
        if (this->_storage.get_error() != ER_SUCCESS)
        {
            if (this->_mutex.unlock(THREAD_ID) != FT_SUCCESS)
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
            if (this->_mutex.unlock(THREAD_ID) != FT_SUCCESS)
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
            if (this->_mutex.unlock(THREAD_ID) != FT_SUCCESS)
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
        if (this->_mutex.unlock(THREAD_ID) != FT_SUCCESS)
        {
            this->set_error(this->_mutex.get_error());
            return (false);
        }
        this->set_error(this->_storage.get_error());
        return (false);
    }
    if (this->_mutex.unlock(THREAD_ID) != FT_SUCCESS)
    {
        this->set_error(this->_mutex.get_error());
        return (false);
    }
    result = std::move(value);
    this->set_error(ER_SUCCESS);
    return (true);
}

template <typename ElementType>
void ft_blocking_queue<ElementType>::shutdown()
{
    if (this->_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        this->set_error(this->_mutex.get_error());
        return ;
    }
    this->_shutdown = true;
    if (this->_mutex.unlock(THREAD_ID) != FT_SUCCESS)
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
int ft_blocking_queue<ElementType>::get_error() const
{
    return (this->_error_code);
}

template <typename ElementType>
const char *ft_blocking_queue<ElementType>::get_error_str() const
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
            promise_shared->set_value(std::move(result_value));
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
    this->_queue.push(std::move(wrapper));
    if (this->_queue.get_error() != ER_SUCCESS)
    {
        this->set_error(this->_queue.get_error());
        task_body();
        return (future_value);
    }
    if (!this->update_queue_size(1))
        return (future_value);
    this->set_error(ER_SUCCESS);
    return (future_value);
}

template <typename Rep, typename Period, typename FunctionType, typename... Args>
auto ft_task_scheduler::schedule_after(std::chrono::duration<Rep, Period> delay,
        FunctionType function, Args... args)
    -> Pair<ft_future<typename std::invoke_result<FunctionType, Args...>::type>,
            ft_scheduled_task_handle>
{
    using return_type = typename std::invoke_result<FunctionType, Args...>::type;
    using promise_type = ft_promise<return_type>;

    Pair<ft_future<return_type>, ft_scheduled_task_handle> result_pair;
    promise_type *promise_raw;
    ft_sharedptr<promise_type> promise_shared;
    ft_future<return_type> future_value;
    ft_scheduled_task_state *state_raw;
    ft_sharedptr<ft_scheduled_task_state> state_shared;
    ft_scheduled_task_handle handle_instance;

    promise_raw = new (std::nothrow) promise_type();
    if (!promise_raw)
    {
        this->set_error(FT_EALLOC);
        return (result_pair);
    }
    promise_shared.reset(promise_raw, 1, false);
    if (promise_shared.hasError())
    {
        this->set_error(promise_shared.get_error());
        return (result_pair);
    }
    future_value = ft_future<return_type>(promise_shared);
    if (future_value.get_error() != ER_SUCCESS)
    {
        this->set_error(future_value.get_error());
        return (result_pair);
    }
    state_raw = new (std::nothrow) ft_scheduled_task_state();
    if (!state_raw)
    {
        this->set_error(FT_EALLOC);
        return (result_pair);
    }
    state_shared.reset(state_raw, 1, false);
    if (state_shared.hasError())
    {
        this->set_error(state_shared.get_error());
        return (result_pair);
    }
    handle_instance = ft_scheduled_task_handle(this, state_shared);
    if (handle_instance.get_error() != ER_SUCCESS)
    {
        this->set_error(handle_instance.get_error());
        return (result_pair);
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
    task_entry._state = state_shared;
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
            promise_shared->set_value(std::move(result_value));
        }
        return ;
    };
    task_entry._function = ft_function<void()>(task_body);
    if (task_entry._function.get_error() != ER_SUCCESS)
    {
        this->set_error(task_entry._function.get_error());
        task_body();
        return (result_pair);
    }
    if (this->_scheduled_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        this->set_error(this->_scheduled_mutex.get_error());
        task_body();
        return (result_pair);
    }
    bool push_success;
    int scheduled_error;

    push_success = this->scheduled_heap_push(std::move(task_entry));
    if (!push_success)
    {
        scheduled_error = this->_scheduled.get_error();
        if (this->_scheduled_mutex.unlock(THREAD_ID) != FT_SUCCESS)
            this->set_error(this->_scheduled_mutex.get_error());
        else
            this->set_error(scheduled_error);
        task_body();
        return (result_pair);
    }
    if (this->_scheduled_mutex.unlock(THREAD_ID) != FT_SUCCESS)
    {
        this->set_error(this->_scheduled_mutex.get_error());
        task_body();
        return (result_pair);
    }
    if (this->_scheduled_condition.signal() != 0)
    {
        this->set_error(this->_scheduled_condition.get_error());
        return (result_pair);
    }
    this->set_error(ER_SUCCESS);
    return (Pair<ft_future<return_type>, ft_scheduled_task_handle>(future_value, handle_instance));
}

template <typename Rep, typename Period, typename FunctionType, typename... Args>
ft_scheduled_task_handle ft_task_scheduler::schedule_every(std::chrono::duration<Rep, Period> interval,
        FunctionType function, Args... args)
{
    ft_scheduled_task_handle handle_result;
    scheduled_task task_entry;
    std::chrono::milliseconds interval_duration;
    long long interval_milliseconds;
    t_monotonic_time_point start_point;
    ft_scheduled_task_state *state_raw;
    ft_sharedptr<ft_scheduled_task_state> state_shared;

    interval_duration = std::chrono::duration_cast<std::chrono::milliseconds>(interval);
    interval_milliseconds = interval_duration.count();
    start_point = time_monotonic_point_now();
    task_entry._time = time_monotonic_point_add_ms(start_point, interval_milliseconds);
    task_entry._interval_ms = interval_milliseconds;
    state_raw = new (std::nothrow) ft_scheduled_task_state();
    if (!state_raw)
    {
        this->set_error(FT_EALLOC);
        return (handle_result);
    }
    state_shared.reset(state_raw, 1, false);
    if (state_shared.hasError())
    {
        this->set_error(state_shared.get_error());
        return (handle_result);
    }
    handle_result = ft_scheduled_task_handle(this, state_shared);
    if (handle_result.get_error() != ER_SUCCESS)
    {
        this->set_error(handle_result.get_error());
        return (handle_result);
    }
    task_entry._state = state_shared;
    task_entry._function = ft_function<void()>([function, args...]()
    {
        function(args...);
        return ;
    });
    if (task_entry._function.get_error() != ER_SUCCESS)
    {
        this->set_error(task_entry._function.get_error());
        return (handle_result);
    }
    if (this->_scheduled_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        this->set_error(this->_scheduled_mutex.get_error());
        task_entry._function();
        return (handle_result);
    }
    bool push_success;
    int scheduled_error;

    push_success = this->scheduled_heap_push(std::move(task_entry));
    if (!push_success)
    {
        scheduled_error = this->_scheduled.get_error();
        if (this->_scheduled_mutex.unlock(THREAD_ID) != FT_SUCCESS)
            this->set_error(this->_scheduled_mutex.get_error());
        else
            this->set_error(scheduled_error);
        task_entry._function();
        return (handle_result);
    }
    if (this->_scheduled_mutex.unlock(THREAD_ID) != FT_SUCCESS)
    {
        this->set_error(this->_scheduled_mutex.get_error());
        task_entry._function();
        return (handle_result);
    }
    if (this->_scheduled_condition.signal() != 0)
    {
        this->set_error(this->_scheduled_condition.get_error());
        return (handle_result);
    }
    this->set_error(ER_SUCCESS);
    return (handle_result);
}

#endif
