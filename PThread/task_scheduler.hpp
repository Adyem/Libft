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
#include "thread.hpp"
#include "../Time/time.hpp"
#include "mutex.hpp"
#include "unique_lock.hpp"

#include <atomic>
#include <chrono>
#include <new>
#include <type_traits>



template <typename ElementType>
class ft_lock_free_queue
{
    private:
        struct node
        {
            ElementType _value;
            ft_atomic<node*> _next;
        };

        ft_atomic<node*> _head;
        ft_atomic<node*> _tail;
        mutable int _error_code;

        void set_error(int error) const;

    public:
        ft_lock_free_queue();
        ~ft_lock_free_queue();

        ft_lock_free_queue(const ft_lock_free_queue&) = delete;
        ft_lock_free_queue &operator=(const ft_lock_free_queue&) = delete;

        void push(ElementType &&value);
        bool pop(ElementType &result);

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
    : _head(ft_nullptr), _tail(ft_nullptr), _error_code(ER_SUCCESS)
{
    node *dummy;

    dummy = new (std::nothrow) node;
    if (!dummy)
    {
        this->set_error(FT_EALLOC);
        return ;
    }
    dummy->_next.store(ft_nullptr);
    this->_head.store(dummy);
    this->_tail.store(dummy);
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ElementType>
ft_lock_free_queue<ElementType>::~ft_lock_free_queue()
{
    node *current;

    current = this->_head.load();
    while (current)
    {
        node *next_node;

        next_node = current->_next.load();
        delete current;
        current = next_node;
    }
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
    node *new_node;

    new_node = new (std::nothrow) node;
    if (!new_node)
    {
        this->set_error(FT_EALLOC);
        return ;
    }
    new_node->_value = ft_move(value);
    new_node->_next.store(ft_nullptr);
    while (true)
    {
        node *tail_node;
        node *next_node;

        tail_node = this->_tail.load(std::memory_order_acquire);
        next_node = tail_node->_next.load(std::memory_order_acquire);
        if (next_node == ft_nullptr)
        {
            if (tail_node->_next.compare_exchange_weak(next_node, new_node))
            {
                this->_tail.compare_exchange_weak(tail_node, new_node);
                break;
            }
        }
        else
        {
            this->_tail.compare_exchange_weak(tail_node, next_node);
        }
    }
    this->set_error(ER_SUCCESS);
    return ;
}

template <typename ElementType>
bool ft_lock_free_queue<ElementType>::pop(ElementType &result)
{
    while (true)
    {
        node *head_node;
        node *tail_node;
        node *next_node;

        head_node = this->_head.load(std::memory_order_acquire);
        tail_node = this->_tail.load(std::memory_order_acquire);
        next_node = head_node->_next.load(std::memory_order_acquire);
        if (next_node == ft_nullptr)
        {
            this->set_error(QUEUE_EMPTY);
            return (false);
        }
        if (head_node == tail_node)
        {
            this->_tail.compare_exchange_weak(tail_node, next_node);
            continue;
        }
        if (this->_head.compare_exchange_weak(head_node, next_node))
        {
            result = ft_move(next_node->_value);
            delete head_node;
            this->set_error(ER_SUCCESS);
            return (true);
        }
    }
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
    {
        ft_unique_lock<pt_mutex> lock(this->_scheduled_mutex);
        if (lock.get_error() != ER_SUCCESS)
        {
            this->set_error(lock.get_error());
            task_body();
            return (future_value);
        }
        this->_scheduled.push_back(ft_move(task_entry));
        if (this->_scheduled.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_scheduled.get_error());
            if (lock.owns_lock())
            {
                lock.unlock();
                if (lock.get_error() != ER_SUCCESS)
                {
                    this->set_error(lock.get_error());
                    return (future_value);
                }
            }
            task_body();
            return (future_value);
        }
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
    {
        ft_unique_lock<pt_mutex> lock(this->_scheduled_mutex);
        if (lock.get_error() != ER_SUCCESS)
        {
            this->set_error(lock.get_error());
            return ;
        }
        this->_scheduled.push_back(ft_move(task_entry));
        if (this->_scheduled.get_error() != ER_SUCCESS)
        {
            this->set_error(this->_scheduled.get_error());
            if (lock.owns_lock())
            {
                lock.unlock();
                if (lock.get_error() != ER_SUCCESS)
                {
                    this->set_error(lock.get_error());
                    return ;
                }
            }
            task_entry._function();
            return ;
        }
    }
    this->set_error(ER_SUCCESS);
    return ;
}

#endif
