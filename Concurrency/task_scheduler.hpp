#ifndef TASK_SCHEDULER_HPP
#define TASK_SCHEDULER_HPP

#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Template/move.hpp"

#include <functional>
#include <future>
#include <thread>
#include <vector>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <new>



template <typename ElementType>
class ft_lock_free_queue
{
    private:
        struct node
        {
            ElementType _value;
            std::atomic<node*> _next;
        };

        std::atomic<node*> _head;
        std::atomic<node*> _tail;
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
            std::chrono::steady_clock::time_point _time;
            std::chrono::milliseconds _interval;
            std::function<void()> _function;
        };

        ft_lock_free_queue<std::function<void()> > _queue;
        std::vector<std::thread> _workers;
        std::thread _timer_thread;
        std::vector<scheduled_task> _scheduled;
        std::mutex _scheduled_mutex;
        std::atomic<bool> _running;
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
            -> std::future<typename std::invoke_result<FunctionType, Args...>::type>;

        template <typename Rep, typename Period, typename FunctionType, typename... Args>
        auto schedule_after(std::chrono::duration<Rep, Period> delay,
                FunctionType function, Args... args)
            -> std::future<typename std::invoke_result<FunctionType, Args...>::type>;

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
            return (false);
        if (head_node == tail_node)
        {
            this->_tail.compare_exchange_weak(tail_node, next_node);
            continue;
        }
        if (this->_head.compare_exchange_weak(head_node, next_node))
        {
            result = ft_move(next_node->_value);
            delete head_node;
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
    -> std::future<typename std::invoke_result<FunctionType, Args...>::type>
{
    using return_type = typename std::invoke_result<FunctionType, Args...>::type;
    using task_type = std::packaged_task<return_type()>;

    std::shared_ptr<task_type> task_pointer;
    task_pointer = std::shared_ptr<task_type>(new (std::nothrow) task_type([function, args...]()
    {
        return (function(args...));
    }));
    if (!task_pointer)
    {
        this->set_error(FT_EALLOC);
        std::promise<return_type> promise_instance;
        return (promise_instance.get_future());
    }
    std::future<return_type> future_value;
    future_value = task_pointer->get_future();
    std::function<void()> wrapper;
    wrapper = [task_pointer]()
    {
        (*task_pointer)();
        return ;
    };
    this->_queue.push(ft_move(wrapper));
    return (ft_move(future_value));
}

template <typename Rep, typename Period, typename FunctionType, typename... Args>
auto ft_task_scheduler::schedule_after(std::chrono::duration<Rep, Period> delay,
        FunctionType function, Args... args)
    -> std::future<typename std::invoke_result<FunctionType, Args...>::type>
{
    using return_type = typename std::invoke_result<FunctionType, Args...>::type;
    using task_type = std::packaged_task<return_type()>;

    std::shared_ptr<task_type> task_pointer;
    task_pointer = std::shared_ptr<task_type>(new (std::nothrow) task_type([function, args...]()
    {
        return (function(args...));
    }));
    if (!task_pointer)
    {
        this->set_error(FT_EALLOC);
        std::promise<return_type> promise_instance;
        return (promise_instance.get_future());
    }
    scheduled_task task_entry;
    task_entry._time = std::chrono::steady_clock::now() + delay;
    task_entry._interval = std::chrono::milliseconds(0);
    task_entry._function = [task_pointer]()
    {
        (*task_pointer)();
        return ;
    };
    std::future<return_type> future_value;
    future_value = task_pointer->get_future();
    {
        std::lock_guard<std::mutex> lock(this->_scheduled_mutex);
        this->_scheduled.push_back(task_entry);
    }
    return (ft_move(future_value));
}

template <typename Rep, typename Period, typename FunctionType, typename... Args>
void ft_task_scheduler::schedule_every(std::chrono::duration<Rep, Period> interval,
        FunctionType function, Args... args)
{
    scheduled_task task_entry;

    task_entry._time = std::chrono::steady_clock::now() + interval;
    task_entry._interval = std::chrono::duration_cast<std::chrono::milliseconds>(interval);
    task_entry._function = [function, args...]()
    {
        function(args...);
        return ;
    };
    {
        std::lock_guard<std::mutex> lock(this->_scheduled_mutex);
        this->_scheduled.push_back(task_entry);
    }
    return ;
}

#endif
