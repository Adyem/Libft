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
#include "mutex.hpp"
#include "task_scheduler_tracing.hpp"

#include <pthread.h>
#include <cerrno>
#include <ctime>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <new>
#include <type_traits>
#include <utility>

#include "../Template/move.hpp"
template <typename ElementType>
class ft_blocking_queue
{
    private:
        pt_mutex _mutex;
        pt_condition_variable _condition;
        bool _shutdown;
        ft_queue<ElementType> _storage;
        mutable pt_mutex *_state_mutex;
        int lock_internal(bool *lock_acquired) const;
        int unlock_internal(bool lock_acquired) const;
        void teardown_thread_safety();
        pt_mutex *mutex_handle() const;

    public:
        ft_blocking_queue();
        ~ft_blocking_queue();

        ft_blocking_queue(const ft_blocking_queue&) = delete;
        ft_blocking_queue &operator=(const ft_blocking_queue&) = delete;

        int enable_thread_safety();
        void disable_thread_safety();
        bool is_thread_safe_enabled() const;
        int lock(bool *lock_acquired) const;
        void unlock(bool lock_acquired) const;

        int push(ElementType &&value);
        bool pop(ElementType &result);
        bool wait_pop(ElementType &result, const std::atomic<bool> &running_flag);
        void shutdown();
};

class ft_task_scheduler;

class pt_mutex;
class pt_recursive_mutex;

class ft_scheduled_task_state
{
    private:
        std::atomic<bool> _cancelled;
        mutable pt_mutex *_state_mutex;
        int lock_internal(bool *lock_acquired) const;
        int unlock_internal(bool lock_acquired) const;
        void teardown_thread_safety();
        pt_mutex *mutex_handle() const;

    public:
        ft_scheduled_task_state();
        ~ft_scheduled_task_state();

        void cancel();
        bool is_cancelled() const;

        int enable_thread_safety();
        void disable_thread_safety();
        bool is_thread_safe_enabled() const;
        int lock(bool *lock_acquired) const;
        void unlock(bool lock_acquired) const;
};

class ft_scheduled_task_handle
{
    private:
        ft_sharedptr<ft_scheduled_task_state> _state;
        ft_task_scheduler *_scheduler;
        mutable pt_mutex *_state_mutex;
        int lock_internal(bool *lock_acquired) const;
        int unlock_internal(bool lock_acquired) const;
        void teardown_thread_safety();
        pt_mutex *mutex_handle() const;

    public:
        ft_scheduled_task_handle();
        ft_scheduled_task_handle(ft_task_scheduler *scheduler,
                const ft_sharedptr<ft_scheduled_task_state> &state);
        ft_scheduled_task_handle(const ft_scheduled_task_handle &other);
        ft_scheduled_task_handle &operator=(const ft_scheduled_task_handle &other);
        ~ft_scheduled_task_handle();

        bool cancel();
        bool valid() const;
        const ft_scheduled_task_state *get_state() const;

        int enable_thread_safety();
        void disable_thread_safety();
        bool is_thread_safe_enabled() const;
        int lock(bool *lock_acquired) const;
        void unlock(bool lock_acquired) const;
};

class ft_task_scheduler
{
    private:
        struct task_queue_entry
        {
            ft_function<void()> _function;
            unsigned long long _trace_id;
            unsigned long long _parent_id;
            const char *_label;
        };

        struct scheduled_task
        {
            t_monotonic_time_point _time;
            long long _interval_ms;
            ft_function<void()> _function;
            ft_sharedptr<ft_scheduled_task_state> _state;
            unsigned long long _trace_id;
            unsigned long long _parent_id;
            const char *_label;
            uint8_t _initialized_state;
            static const uint8_t _state_uninitialized = 0;
            static const uint8_t _state_destroyed = 1;
            static const uint8_t _state_initialized = 2;

            scheduled_task();
            ~scheduled_task();
            scheduled_task(const scheduled_task &other);
            scheduled_task(scheduled_task &&other);
            scheduled_task &operator=(const scheduled_task &other);
            scheduled_task &operator=(scheduled_task &&other);
            int initialize();
            int initialize(const scheduled_task &other);
            int initialize_move(scheduled_task &other);
            int destroy();
            int is_initialized() const;
        };

        ft_blocking_queue<task_queue_entry> _queue;
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
        size_t _configured_thread_count;
        mutable pt_recursive_mutex *_state_mutex;
        mutable uint8_t _initialized_state;
        static const uint8_t _state_uninitialized = 0;
        static const uint8_t _state_destroyed = 1;
        static const uint8_t _state_initialized = 2;

        bool cancel_task_state(const ft_sharedptr<ft_scheduled_task_state> &state);
        bool scheduled_remove_index(size_t index);
        void worker_loop();
        void timer_loop();
        bool scheduled_heap_push(scheduled_task &&task);
        bool scheduled_heap_pop(scheduled_task &task);
        void scheduled_heap_sift_up(size_t index);
        void scheduled_heap_sift_down(size_t index);
        bool update_queue_size(long long delta);
        bool update_worker_counters(long long active_delta, long long idle_delta);
        bool update_worker_total(long long delta);
        void trace_emit_event(e_ft_task_trace_phase phase, unsigned long long trace_id,
                unsigned long long parent_id, const char *label, bool timer_thread);
        bool capture_metrics(ft_task_trace_event &event) const;
        int lock_internal(bool *lock_acquired) const;
        int unlock_internal(bool lock_acquired) const;
        void teardown_thread_safety();
        void abort_lifecycle_error(const char *method_name,
                const char *reason) const;
        void abort_if_not_initialized(const char *method_name) const;

    public:
        friend class ft_scheduled_task_handle;

        ft_task_scheduler(size_t thread_count = 0);
        ~ft_task_scheduler();

        ft_task_scheduler(const ft_task_scheduler &) = delete;
        ft_task_scheduler &operator=(const ft_task_scheduler &) = delete;
        ft_task_scheduler(ft_task_scheduler &&) = delete;
        ft_task_scheduler &operator=(ft_task_scheduler &&) = delete;
        int initialize(size_t thread_count = 0);
        int destroy();

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

        long long get_queue_size() const;
        int get_queue_size(long long *queue_size) const;
        long long get_scheduled_task_count() const;
        int get_scheduled_task_count(long long *scheduled_count) const;
        long long get_worker_active_count() const;
        int get_worker_active_count(long long *active_count) const;
        long long get_worker_idle_count() const;
        int get_worker_idle_count(long long *idle_count) const;
        size_t get_worker_total_count() const;
        int get_worker_total_count(size_t *total_count) const;

        int enable_thread_safety();
        int disable_thread_safety();
        bool is_thread_safe_enabled() const;
        int lock(bool *lock_acquired) const;
        int unlock(bool lock_acquired) const;
        pt_recursive_mutex *mutex_handle() const;
};

template <typename ElementType>
ft_blocking_queue<ElementType>::ft_blocking_queue()
    : _mutex(), _condition(), _shutdown(false), _storage(), _state_mutex(ft_nullptr)
{
    return ;
}

template <typename ElementType>
ft_blocking_queue<ElementType>::~ft_blocking_queue()
{
    this->shutdown();
    this->teardown_thread_safety();
    return ;
}

template <typename ElementType>
pt_mutex *ft_blocking_queue<ElementType>::mutex_handle() const
{
    return (this->_state_mutex);
}

template <typename ElementType>
int ft_blocking_queue<ElementType>::lock_internal(bool *lock_acquired) const
{
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (this->_state_mutex == ft_nullptr)
    {
        return (FT_ERR_SUCCESS);
    }
    int state_error = this->_state_mutex->lock();
    if (state_error != FT_ERR_SUCCESS)
    {
        if (state_error == FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            bool state_lock_acquired;

            state_lock_acquired = false;
            if (this->_state_mutex->lock_state(&state_lock_acquired) == 0)
                this->_state_mutex->unlock_state(state_lock_acquired);
            return (FT_ERR_SUCCESS);
        }
        return (state_error);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

template <typename ElementType>
int ft_blocking_queue<ElementType>::unlock_internal(bool lock_acquired) const
{
    if (!lock_acquired || this->_state_mutex == ft_nullptr)
    {
        return (FT_ERR_SUCCESS);
    }
    return (this->_state_mutex->unlock());
}

template <typename ElementType>
void ft_blocking_queue<ElementType>::teardown_thread_safety()
{
    if (this->_state_mutex != ft_nullptr)
    {
        int destroy_error;

        destroy_error = this->_state_mutex->destroy();
        (void)destroy_error;
        delete this->_state_mutex;
        this->_state_mutex = ft_nullptr;
    }
    this->_condition.disable_thread_safety();
    return ;
}

template <typename ElementType>
int ft_blocking_queue<ElementType>::enable_thread_safety()
{
    if (this->_state_mutex != ft_nullptr)
    {
        if (this->_condition.enable_thread_safety() != 0)
            return (-1);
        return (0);
    }
    int mutex_error;
    pt_mutex *mutex_pointer;

    mutex_pointer = new (std::nothrow) pt_mutex();
    if (mutex_pointer == ft_nullptr)
        return (-1);
    mutex_error = mutex_pointer->initialize();
    if (mutex_error != FT_ERR_SUCCESS)
    {
        delete mutex_pointer;
        return (-1);
    }
    this->_state_mutex = mutex_pointer;

    if (this->_condition.enable_thread_safety() != 0)
    {
        int destroy_error;

        destroy_error = this->_state_mutex->destroy();
        (void)destroy_error;
        delete this->_state_mutex;
        this->_state_mutex = ft_nullptr;
        return (-1);
    }
    return (0);
}

template <typename ElementType>
void ft_blocking_queue<ElementType>::disable_thread_safety()
{
    this->teardown_thread_safety();
    return ;
}

template <typename ElementType>
bool ft_blocking_queue<ElementType>::is_thread_safe_enabled() const
{
    bool enabled;

    enabled = (this->_state_mutex != ft_nullptr);
    return (enabled);
}

template <typename ElementType>
int ft_blocking_queue<ElementType>::lock(bool *lock_acquired) const
{
    int operation_result = this->lock_internal(lock_acquired);
    if (operation_result != FT_ERR_SUCCESS)
        return (-1);
    return (0);
}

template <typename ElementType>
void ft_blocking_queue<ElementType>::unlock(bool lock_acquired) const
{
    (void)this->unlock_internal(lock_acquired);
    return ;
}

template <typename ElementType>
int ft_blocking_queue<ElementType>::push(ElementType &&value)
{
    bool was_empty;
    bool state_lock_acquired;
    int state_lock_error;

    state_lock_acquired = false;
    state_lock_error = this->lock_internal(&state_lock_acquired);
    if (state_lock_error != FT_ERR_SUCCESS)
        return (state_lock_error);
    int mutex_error = this->_mutex.lock();

    if (mutex_error != FT_ERR_SUCCESS)
    {
        this->unlock_internal(state_lock_acquired);
        return (mutex_error);
    }
    was_empty = this->_storage.empty();
    int storage_error = ft_queue<ElementType>::last_operation_error();
    if (storage_error != FT_ERR_SUCCESS)
    {
        this->_mutex.unlock();
        this->unlock_internal(state_lock_acquired);
        return (storage_error);
    }
    this->_storage.enqueue(ft_move(value));
    storage_error = ft_queue<ElementType>::last_operation_error();
    if (storage_error != FT_ERR_SUCCESS)
    {
        this->_mutex.unlock();
        this->unlock_internal(state_lock_acquired);
        return (storage_error);
    }
    mutex_error = this->_mutex.unlock();
    if (mutex_error != FT_ERR_SUCCESS)
    {
        this->unlock_internal(state_lock_acquired);
        return (mutex_error);
    }
    if (was_empty)
    {
        if (this->_condition.signal() != 0)
        {
            this->unlock_internal(state_lock_acquired);
            return (FT_ERR_SYSTEM);
        }
    }
    this->unlock_internal(state_lock_acquired);
    return (FT_ERR_SUCCESS);
}

template <typename ElementType>
bool ft_blocking_queue<ElementType>::pop(ElementType &result)
{
    bool is_empty;
    ElementType value;
    bool state_lock_acquired;

    state_lock_acquired = false;
    int state_lock_error = this->lock_internal(&state_lock_acquired);
    if (state_lock_error != FT_ERR_SUCCESS)
        return (false);
    int mutex_error = this->_mutex.lock();

    if (mutex_error != FT_ERR_SUCCESS)
    {
        this->unlock_internal(state_lock_acquired);
        return (false);
    }
    is_empty = this->_storage.empty();
    int storage_error = ft_queue<ElementType>::last_operation_error();
    if (storage_error != FT_ERR_SUCCESS)
    {
        this->_mutex.unlock();
        this->unlock_internal(state_lock_acquired);
        return (false);
    }
    if (is_empty)
    {
        this->_mutex.unlock();
        this->unlock_internal(state_lock_acquired);
        return (false);
    }
    value = this->_storage.dequeue();
    int dequeue_error = ft_queue<ElementType>::last_operation_error();
    if (dequeue_error != FT_ERR_SUCCESS)
    {
        this->_mutex.unlock();
        this->unlock_internal(state_lock_acquired);
        return (false);
    }
    mutex_error = this->_mutex.unlock();
    if (mutex_error != FT_ERR_SUCCESS)
    {
        this->unlock_internal(state_lock_acquired);
        return (false);
    }
    result = ft_move(value);
    this->unlock_internal(state_lock_acquired);
    return (true);
}

template <typename ElementType>
bool ft_blocking_queue<ElementType>::wait_pop(ElementType &result, const std::atomic<bool> &running_flag)
{
    bool is_empty;
    ElementType value;
    bool state_lock_acquired;

    state_lock_acquired = false;
    int state_lock_error = this->lock_internal(&state_lock_acquired);
    if (state_lock_error != FT_ERR_SUCCESS)
        return (false);
    int mutex_error = this->_mutex.lock();
    if (mutex_error != FT_ERR_SUCCESS)
    {
        this->unlock_internal(state_lock_acquired);
        return (false);
    }
    while (true)
    {
        is_empty = this->_storage.empty();
        int storage_error = ft_queue<ElementType>::last_operation_error();
        if (storage_error != FT_ERR_SUCCESS)
        {
            this->_mutex.unlock();
            this->unlock_internal(state_lock_acquired);
            return (false);
        }
        if (!is_empty)
            break;
        if (!running_flag.load() || this->_shutdown)
        {
            this->_mutex.unlock();
            this->unlock_internal(state_lock_acquired);
            return (false);
        }
        this->unlock_internal(state_lock_acquired);
        state_lock_acquired = false;
        if (this->_condition.wait(this->_mutex) != 0)
        {
            this->_mutex.unlock();
            return (false);
        }
        int relock_error = this->lock_internal(&state_lock_acquired);
        if (relock_error != FT_ERR_SUCCESS)
        {
            this->_mutex.unlock();
            return (false);
        }
    }
    value = this->_storage.dequeue();
    int dequeue_error = ft_queue<ElementType>::last_operation_error();
    if (dequeue_error != FT_ERR_SUCCESS)
    {
        this->_mutex.unlock();
        this->unlock_internal(state_lock_acquired);
        return (false);
    }
    mutex_error = this->_mutex.unlock();
    if (mutex_error != FT_ERR_SUCCESS)
    {
        this->unlock_internal(state_lock_acquired);
        return (false);
    }
    result = ft_move(value);
    this->unlock_internal(state_lock_acquired);
    return (true);
}

template <typename ElementType>
void ft_blocking_queue<ElementType>::shutdown()
{
    bool state_lock_acquired;

    state_lock_acquired = false;
    int state_lock_error = this->lock_internal(&state_lock_acquired);
    if (state_lock_error != FT_ERR_SUCCESS)
        return ;
    int mutex_error = this->_mutex.lock();
    if (mutex_error != FT_ERR_SUCCESS)
    {
        this->unlock_internal(state_lock_acquired);
        return ;
    }
    this->_shutdown = true;
    mutex_error = this->_mutex.unlock();
    if (mutex_error != FT_ERR_SUCCESS)
    {
        this->unlock_internal(state_lock_acquired);
        return ;
    }
    if (this->_condition.broadcast() != 0)
    {
        this->unlock_internal(state_lock_acquired);
        return ;
    }
    this->unlock_internal(state_lock_acquired);
    return ;
}

template <typename FunctionType, typename... Args>
auto ft_task_scheduler::submit(FunctionType function, Args... args)
    -> ft_future<typename std::invoke_result<FunctionType, Args...>::type>
{
    this->abort_if_not_initialized("ft_task_scheduler::submit");
    using return_type = typename std::invoke_result<FunctionType, Args...>::type;
    using promise_type = ft_promise<return_type>;

    promise_type *promise_raw;
    ft_sharedptr<promise_type> promise_shared;

    promise_raw = new (std::nothrow) promise_type();
    if (!promise_raw)
    {
        ft_future<return_type> empty_future;
        return (empty_future);
    }
    promise_shared.reset(promise_raw, 1, false);
    int promise_error = ft_sharedptr<promise_type>::last_operation_error();

    if (promise_error != FT_ERR_SUCCESS)
    {
        ft_future<return_type> empty_future;
        return (empty_future);
    }
    ft_future<return_type> future_value(promise_shared);
    {
        int future_error = ft_sharedptr<promise_type>::last_operation_error();
        if (future_error != FT_ERR_SUCCESS)
        {
            return (future_value);
        }
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
    task_queue_entry queue_entry;
    unsigned long long parent_span;
    unsigned long long trace_id;
    bool metrics_updated;

    parent_span = task_scheduler_trace_current_span();
    trace_id = task_scheduler_trace_generate_span_id();
    queue_entry._function = ft_move(wrapper);
    queue_entry._trace_id = trace_id;
    queue_entry._parent_id = parent_span;
    queue_entry._label = g_ft_task_trace_label_async;
    this->trace_emit_event(FT_TASK_TRACE_PHASE_SUBMITTED, trace_id, parent_span,
            g_ft_task_trace_label_async, false);
    this->trace_emit_event(FT_TASK_TRACE_PHASE_ENQUEUED, trace_id, parent_span,
            g_ft_task_trace_label_async, false);
    int queue_push_error = this->_queue.push(ft_move(queue_entry));
    if (queue_push_error != FT_ERR_SUCCESS)
    {
        this->trace_emit_event(FT_TASK_TRACE_PHASE_CANCELLED, trace_id, parent_span,
                g_ft_task_trace_label_async, false);
        return (future_value);
    }
    metrics_updated = this->update_queue_size(1);
    if (!metrics_updated)
        return (future_value);
    return (future_value);
}

template <typename Rep, typename Period, typename FunctionType, typename... Args>
auto ft_task_scheduler::schedule_after(std::chrono::duration<Rep, Period> delay,
        FunctionType function, Args... args)
    -> Pair<ft_future<typename std::invoke_result<FunctionType, Args...>::type>,
            ft_scheduled_task_handle>
{
    this->abort_if_not_initialized("ft_task_scheduler::schedule_after");
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
        return (result_pair);
    }
    promise_shared.reset(promise_raw, 1, false);
    int promise_error = ft_sharedptr<promise_type>::last_operation_error();

    if (promise_error != FT_ERR_SUCCESS)
    {
        return (result_pair);
    }
    future_value = ft_future<return_type>(promise_shared);
    {
        int future_error = ft_sharedptr<promise_type>::last_operation_error();
        if (future_error != FT_ERR_SUCCESS)
        {
            return (result_pair);
        }
    }
    state_raw = new (std::nothrow) ft_scheduled_task_state();
    if (!state_raw)
    {
        return (result_pair);
    }
    state_shared.reset(state_raw, 1, false);
    int state_error = ft_sharedptr<ft_scheduled_task_state>::last_operation_error();

    if (state_error != FT_ERR_SUCCESS)
    {
        return (result_pair);
    }
    handle_instance = ft_scheduled_task_handle(this, state_shared);
    if (!handle_instance.valid())
    {
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
            promise_shared->set_value(ft_move(result_value));
        }
        return ;
    };
    task_entry._function = ft_function<void()>(task_body);
    if (!task_entry._function)
    {
        return (result_pair);
    }
    unsigned long long parent_span;
    unsigned long long trace_id;

    parent_span = task_scheduler_trace_current_span();
    trace_id = task_scheduler_trace_generate_span_id();
    task_entry._trace_id = trace_id;
    task_entry._parent_id = parent_span;
    task_entry._label = g_ft_task_trace_label_schedule_once;
    int scheduled_mutex_error = this->_scheduled_mutex.lock();

    if (scheduled_mutex_error != FT_ERR_SUCCESS)
    {
        return (result_pair);
    }
    bool push_success;

    push_success = this->scheduled_heap_push(ft_move(task_entry));
    if (!push_success)
    {
        scheduled_mutex_error = this->_scheduled_mutex.unlock();
        this->trace_emit_event(FT_TASK_TRACE_PHASE_CANCELLED, trace_id, parent_span,
                g_ft_task_trace_label_schedule_once, false);
        return (result_pair);
    }
    scheduled_mutex_error = this->_scheduled_mutex.unlock();
    if (scheduled_mutex_error != FT_ERR_SUCCESS)
    {
        return (result_pair);
    }
    if (this->_scheduled_condition.signal() != 0)
        return (result_pair);
    this->trace_emit_event(FT_TASK_TRACE_PHASE_TIMER_REGISTERED, trace_id, parent_span,
            g_ft_task_trace_label_schedule_once, false);
    return (Pair<ft_future<return_type>, ft_scheduled_task_handle>(future_value, handle_instance));
}

template <typename Rep, typename Period, typename FunctionType, typename... Args>
ft_scheduled_task_handle ft_task_scheduler::schedule_every(std::chrono::duration<Rep, Period> interval,
        FunctionType function, Args... args)
{
    this->abort_if_not_initialized("ft_task_scheduler::schedule_every");
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
        return (handle_result);
    }
    state_shared.reset(state_raw, 1, false);
    int state_error = ft_sharedptr<ft_scheduled_task_state>::last_operation_error();

    if (state_error != FT_ERR_SUCCESS)
    {
        return (handle_result);
    }
    handle_result = ft_scheduled_task_handle(this, state_shared);
    if (!handle_result.valid())
    {
        return (handle_result);
    }
    task_entry._state = state_shared;
    task_entry._function = ft_function<void()>([function, args...]()
    {
        function(args...);
        return ;
    });
    if (!task_entry._function)
    {
        return (handle_result);
    }
    unsigned long long parent_span;
    unsigned long long trace_id;

    parent_span = task_scheduler_trace_current_span();
    trace_id = task_scheduler_trace_generate_span_id();
    task_entry._trace_id = trace_id;
    task_entry._parent_id = parent_span;
    task_entry._label = g_ft_task_trace_label_schedule_repeat;
    int scheduled_mutex_error = this->_scheduled_mutex.lock();

    if (scheduled_mutex_error != FT_ERR_SUCCESS)
    {
        return (handle_result);
    }
    bool push_success;

    push_success = this->scheduled_heap_push(ft_move(task_entry));
    if (!push_success)
    {
        scheduled_mutex_error = this->_scheduled_mutex.unlock();
        this->trace_emit_event(FT_TASK_TRACE_PHASE_CANCELLED, trace_id, parent_span,
                g_ft_task_trace_label_schedule_repeat, false);
        return (handle_result);
    }
    scheduled_mutex_error = this->_scheduled_mutex.unlock();
    if (scheduled_mutex_error != FT_ERR_SUCCESS)
    {
        return (handle_result);
    }
    if (this->_scheduled_condition.signal() != 0)
        return (handle_result);
    this->trace_emit_event(FT_TASK_TRACE_PHASE_TIMER_REGISTERED, trace_id, parent_span,
            g_ft_task_trace_label_schedule_repeat, false);
    return (handle_result);
}

#endif
