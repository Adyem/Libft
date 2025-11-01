#include "task_scheduler.hpp"
#include "pthread.hpp"
#include "../System_utils/system_utils.hpp"
#include "../CMA/CMA.hpp"
#include <utility>
#include "../Template/move.hpp"

ft_scheduled_task_state::ft_scheduled_task_state()
    : _cancelled(false), _error_code(ER_SUCCESS), _state_mutex(ft_nullptr),
      _thread_safe_enabled(false)
{
    this->set_error(ER_SUCCESS);
    return ;
}

ft_scheduled_task_state::~ft_scheduled_task_state()
{
    this->teardown_thread_safety();
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_scheduled_task_state::set_error(int error) const
{
    this->_error_code = error;
    ft_errno = error;
    return ;
}

void ft_scheduled_task_state::cancel()
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    this->_cancelled.store(true);
    this->set_error(ER_SUCCESS);
    this->unlock_internal(lock_acquired);
    return ;
}

bool ft_scheduled_task_state::is_cancelled() const
{
    bool cancelled_value;
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        const_cast<ft_scheduled_task_state *>(this)->set_error(ft_errno);
        return (false);
    }
    cancelled_value = this->_cancelled.load();
    const_cast<ft_scheduled_task_state *>(this)->set_error(ER_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (cancelled_value);
}

int ft_scheduled_task_state::enable_thread_safety()
{
    void *memory;
    pt_mutex *state_mutex;

    if (this->_thread_safe_enabled && this->_state_mutex != ft_nullptr)
    {
        this->set_error(ER_SUCCESS);
        return (0);
    }
    memory = cma_malloc(sizeof(pt_mutex));
    if (memory == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return (-1);
    }
    state_mutex = new(memory) pt_mutex();
    if (state_mutex->get_error() != ER_SUCCESS)
    {
        int mutex_error;

        mutex_error = state_mutex->get_error();
        state_mutex->~pt_mutex();
        cma_free(memory);
        this->set_error(mutex_error);
        return (-1);
    }
    this->_state_mutex = state_mutex;
    this->_thread_safe_enabled = true;
    this->set_error(ER_SUCCESS);
    return (0);
}

void ft_scheduled_task_state::disable_thread_safety()
{
    this->teardown_thread_safety();
    this->set_error(ER_SUCCESS);
    return ;
}

bool ft_scheduled_task_state::is_thread_safe_enabled() const
{
    bool enabled;

    enabled = (this->_thread_safe_enabled && this->_state_mutex != ft_nullptr);
    const_cast<ft_scheduled_task_state *>(this)->set_error(ER_SUCCESS);
    return (enabled);
}

int ft_scheduled_task_state::lock(bool *lock_acquired) const
{
    int entry_errno;
    int result;

    entry_errno = ft_errno;
    result = this->lock_internal(lock_acquired);
    if (result != 0)
    {
        const_cast<ft_scheduled_task_state *>(this)->set_error(ft_errno);
        return (result);
    }
    const_cast<ft_scheduled_task_state *>(this)->set_error(ER_SUCCESS);
    ft_errno = entry_errno;
    return (0);
}

void ft_scheduled_task_state::unlock(bool lock_acquired) const
{
    int entry_errno;

    entry_errno = ft_errno;
    this->unlock_internal(lock_acquired);
    if (this->_state_mutex != ft_nullptr && this->_state_mutex->get_error() != ER_SUCCESS)
    {
        const_cast<ft_scheduled_task_state *>(this)->set_error(this->_state_mutex->get_error());
        return ;
    }
    ft_errno = entry_errno;
    const_cast<ft_scheduled_task_state *>(this)->set_error(ER_SUCCESS);
    ft_errno = entry_errno;
    return ;
}

int ft_scheduled_task_state::get_error() const
{
    return (this->_error_code);
}

const char *ft_scheduled_task_state::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}

int ft_scheduled_task_state::lock_internal(bool *lock_acquired) const
{
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (!this->_thread_safe_enabled || this->_state_mutex == ft_nullptr)
    {
        ft_errno = ER_SUCCESS;
        return (0);
    }
    this->_state_mutex->lock(THREAD_ID);
    if (this->_state_mutex->get_error() != ER_SUCCESS)
    {
        if (this->_state_mutex->get_error() == FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            bool state_lock_acquired;

            state_lock_acquired = false;
            ft_errno = ER_SUCCESS;
            if (this->_state_mutex->lock_state(&state_lock_acquired) == 0)
                this->_state_mutex->unlock_state(state_lock_acquired);
            ft_errno = ER_SUCCESS;
            return (0);
        }
        ft_errno = this->_state_mutex->get_error();
        return (-1);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    ft_errno = ER_SUCCESS;
    return (0);
}

void ft_scheduled_task_state::unlock_internal(bool lock_acquired) const
{
    int entry_errno;

    if (!lock_acquired || this->_state_mutex == ft_nullptr)
        return ;
    entry_errno = ft_errno;
    this->_state_mutex->unlock(THREAD_ID);
    if (this->_state_mutex->get_error() != ER_SUCCESS)
    {
        ft_errno = this->_state_mutex->get_error();
        return ;
    }
    ft_errno = entry_errno;
    return ;
}

void ft_scheduled_task_state::teardown_thread_safety()
{
    if (this->_state_mutex != ft_nullptr)
    {
        this->_state_mutex->~pt_mutex();
        cma_free(this->_state_mutex);
        this->_state_mutex = ft_nullptr;
    }
    this->_thread_safe_enabled = false;
    return ;
}

ft_scheduled_task_handle::ft_scheduled_task_handle()
    : _state(), _scheduler(ft_nullptr), _error_code(ER_SUCCESS),
      _state_mutex(ft_nullptr), _thread_safe_enabled(false)
{
    this->set_error(ER_SUCCESS);
    return ;
}

ft_scheduled_task_handle::ft_scheduled_task_handle(ft_task_scheduler *scheduler,
        const ft_sharedptr<ft_scheduled_task_state> &state)
    : _state(state), _scheduler(scheduler), _error_code(ER_SUCCESS),
      _state_mutex(ft_nullptr), _thread_safe_enabled(false)
{
    if (!scheduler || !state)
    {
        this->_scheduler = ft_nullptr;
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        return ;
    }
    if (state.hasError())
    {
        this->_scheduler = ft_nullptr;
        this->set_error(state.get_error());
        return ;
    }
    this->set_error(ER_SUCCESS);
    return ;
}

ft_scheduled_task_handle::ft_scheduled_task_handle(const ft_scheduled_task_handle &other)
    : _state(), _scheduler(ft_nullptr), _error_code(ER_SUCCESS),
      _state_mutex(ft_nullptr), _thread_safe_enabled(false)
{
    bool lock_acquired;

    lock_acquired = false;
    if (other.lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return ;
    }
    this->_state = other._state;
    this->_scheduler = other._scheduler;
    if (other._thread_safe_enabled && other._state_mutex != ft_nullptr)
    {
        if (this->enable_thread_safety() != 0)
        {
            this->set_error(this->get_error());
            other.unlock_internal(lock_acquired);
            return ;
        }
    }
    if (this->_state.hasError())
    {
        this->_scheduler = ft_nullptr;
        this->set_error(this->_state.get_error());
        other.unlock_internal(lock_acquired);
        return ;
    }
    this->set_error(other._error_code);
    other.unlock_internal(lock_acquired);
    return ;
}

ft_scheduled_task_handle &ft_scheduled_task_handle::operator=(const ft_scheduled_task_handle &other)
{
    if (this == &other)
    {
        this->set_error(ER_SUCCESS);
        return (*this);
    }
    bool this_lock_acquired;
    bool other_lock_acquired;

    this_lock_acquired = false;
    if (this->lock_internal(&this_lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return (*this);
    }
    other_lock_acquired = false;
    if (other.lock_internal(&other_lock_acquired) != 0)
    {
        this->unlock_internal(this_lock_acquired);
        this->set_error(ft_errno);
        return (*this);
    }
    this->_state = other._state;
    this->_scheduler = other._scheduler;
    this->teardown_thread_safety();
    if (other._thread_safe_enabled && other._state_mutex != ft_nullptr)
    {
        if (this->enable_thread_safety() != 0)
        {
            other.unlock_internal(other_lock_acquired);
            this->unlock_internal(this_lock_acquired);
            return (*this);
        }
    }
    if (this->_state.hasError())
    {
        this->_scheduler = ft_nullptr;
        this->set_error(this->_state.get_error());
        other.unlock_internal(other_lock_acquired);
        this->unlock_internal(this_lock_acquired);
        return (*this);
    }
    this->set_error(other._error_code);
    other.unlock_internal(other_lock_acquired);
    this->unlock_internal(this_lock_acquired);
    return (*this);
}

ft_scheduled_task_handle::~ft_scheduled_task_handle()
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) == 0)
    {
        this->_scheduler = ft_nullptr;
        this->unlock_internal(lock_acquired);
    }
    else
        this->_scheduler = ft_nullptr;
    this->teardown_thread_safety();
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_scheduled_task_handle::set_error(int error) const
{
    this->_error_code = error;
    ft_errno = error;
    return ;
}

bool ft_scheduled_task_handle::cancel()
{
    bool cancel_result;
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return (false);
    }
    if (!this->_scheduler || !this->_state)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        this->unlock_internal(lock_acquired);
        return (false);
    }
    cancel_result = this->_scheduler->cancel_task_state(this->_state);
    if (!cancel_result)
    {
        this->set_error(this->_scheduler->get_error());
        this->unlock_internal(lock_acquired);
        return (false);
    }
    this->set_error(ER_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (true);
}

bool ft_scheduled_task_handle::valid() const
{
    bool state_valid;
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        const_cast<ft_scheduled_task_handle *>(this)->set_error(ft_errno);
        return (false);
    }
    state_valid = static_cast<bool>(this->_state);
    if (!this->_scheduler || !state_valid)
    {
        const_cast<ft_scheduled_task_handle *>(this)->set_error(FT_ERR_INVALID_ARGUMENT);
        this->unlock_internal(lock_acquired);
        return (false);
    }
    const_cast<ft_scheduled_task_handle *>(this)->set_error(ER_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (true);
}

ft_sharedptr<ft_scheduled_task_state> ft_scheduled_task_handle::get_state() const
{
    ft_sharedptr<ft_scheduled_task_state> state_copy;
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        const_cast<ft_scheduled_task_handle *>(this)->set_error(ft_errno);
        ft_sharedptr<ft_scheduled_task_state> empty_state;

        return (empty_state);
    }
    state_copy = this->_state;
    const_cast<ft_scheduled_task_handle *>(this)->set_error(ER_SUCCESS);
    this->unlock_internal(lock_acquired);
    return (state_copy);
}

int ft_scheduled_task_handle::get_error() const
{
    return (this->_error_code);
}

const char *ft_scheduled_task_handle::get_error_str() const
{
    return (ft_strerror(this->_error_code));
}

int ft_scheduled_task_handle::enable_thread_safety()
{
    void *memory;
    pt_mutex *state_mutex;

    if (this->_thread_safe_enabled && this->_state_mutex != ft_nullptr)
    {
        this->set_error(ER_SUCCESS);
        return (0);
    }
    memory = cma_malloc(sizeof(pt_mutex));
    if (memory == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return (-1);
    }
    state_mutex = new(memory) pt_mutex();
    if (state_mutex->get_error() != ER_SUCCESS)
    {
        int mutex_error;

        mutex_error = state_mutex->get_error();
        state_mutex->~pt_mutex();
        cma_free(memory);
        this->set_error(mutex_error);
        return (-1);
    }
    this->_state_mutex = state_mutex;
    this->_thread_safe_enabled = true;
    this->set_error(ER_SUCCESS);
    return (0);
}

void ft_scheduled_task_handle::disable_thread_safety()
{
    this->teardown_thread_safety();
    this->set_error(ER_SUCCESS);
    return ;
}

bool ft_scheduled_task_handle::is_thread_safe_enabled() const
{
    bool enabled;

    enabled = (this->_thread_safe_enabled && this->_state_mutex != ft_nullptr);
    const_cast<ft_scheduled_task_handle *>(this)->set_error(ER_SUCCESS);
    return (enabled);
}

int ft_scheduled_task_handle::lock(bool *lock_acquired) const
{
    int entry_errno;
    int result;

    entry_errno = ft_errno;
    result = this->lock_internal(lock_acquired);
    if (result != 0)
    {
        const_cast<ft_scheduled_task_handle *>(this)->set_error(ft_errno);
        return (result);
    }
    const_cast<ft_scheduled_task_handle *>(this)->set_error(ER_SUCCESS);
    ft_errno = entry_errno;
    return (0);
}

void ft_scheduled_task_handle::unlock(bool lock_acquired) const
{
    int entry_errno;

    entry_errno = ft_errno;
    this->unlock_internal(lock_acquired);
    if (this->_state_mutex != ft_nullptr && this->_state_mutex->get_error() != ER_SUCCESS)
    {
        const_cast<ft_scheduled_task_handle *>(this)->set_error(this->_state_mutex->get_error());
        return ;
    }
    ft_errno = entry_errno;
    const_cast<ft_scheduled_task_handle *>(this)->set_error(ER_SUCCESS);
    ft_errno = entry_errno;
    return ;
}

int ft_scheduled_task_handle::lock_internal(bool *lock_acquired) const
{
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (!this->_thread_safe_enabled || this->_state_mutex == ft_nullptr)
    {
        ft_errno = ER_SUCCESS;
        return (0);
    }
    this->_state_mutex->lock(THREAD_ID);
    if (this->_state_mutex->get_error() != ER_SUCCESS)
    {
        if (this->_state_mutex->get_error() == FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            bool state_lock_acquired;

            state_lock_acquired = false;
            ft_errno = ER_SUCCESS;
            if (this->_state_mutex->lock_state(&state_lock_acquired) == 0)
                this->_state_mutex->unlock_state(state_lock_acquired);
            ft_errno = ER_SUCCESS;
            return (0);
        }
        ft_errno = this->_state_mutex->get_error();
        return (-1);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    ft_errno = ER_SUCCESS;
    return (0);
}

void ft_scheduled_task_handle::unlock_internal(bool lock_acquired) const
{
    int entry_errno;

    if (!lock_acquired || this->_state_mutex == ft_nullptr)
        return ;
    entry_errno = ft_errno;
    this->_state_mutex->unlock(THREAD_ID);
    if (this->_state_mutex->get_error() != ER_SUCCESS)
    {
        ft_errno = this->_state_mutex->get_error();
        return ;
    }
    ft_errno = entry_errno;
    return ;
}

void ft_scheduled_task_handle::teardown_thread_safety()
{
    if (this->_state_mutex != ft_nullptr)
    {
        this->_state_mutex->~pt_mutex();
        cma_free(this->_state_mutex);
        this->_state_mutex = ft_nullptr;
    }
    this->_thread_safe_enabled = false;
    return ;
}

ft_task_scheduler::ft_task_scheduler(size_t thread_count)
    : _queue(), _workers(), _timer_thread(), _scheduled(), _scheduled_mutex(),
      _scheduled_condition(), _running(true), _queue_metrics_mutex(),
      _worker_metrics_mutex(), _queue_size_counter(0),
      _scheduled_size_counter(0), _worker_active_counter(0),
      _worker_idle_counter(0), _worker_total_count(0), _error_code(ER_SUCCESS),
      _state_mutex(ft_nullptr), _thread_safe_enabled(false)
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
        if (!this->update_worker_total(1))
        {
            worker_failure = true;
            break;
        }
        if (!this->update_worker_counters(0, 1))
        {
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
    this->teardown_thread_safety();
    this->set_error(ER_SUCCESS);
    return ;
}

void ft_task_scheduler::set_error(int error) const
{
    bool state_locked;

    state_locked = false;
    if (this->_thread_safe_enabled && this->_state_mutex != ft_nullptr)
    {
        this->_state_mutex->lock(THREAD_ID);
        if (this->_state_mutex->get_error() == ER_SUCCESS)
            state_locked = true;
        else if (this->_state_mutex->get_error() != FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            this->_error_code = error;
            ft_errno = this->_state_mutex->get_error();
            return ;
        }
    }
    this->_error_code = error;
    ft_errno = error;
    if (state_locked)
    {
        this->_state_mutex->unlock(THREAD_ID);
        if (this->_state_mutex->get_error() != ER_SUCCESS)
        {
            ft_errno = this->_state_mutex->get_error();
            this->_error_code = this->_state_mutex->get_error();
            return ;
        }
        ft_errno = error;
    }
    return ;
}

void ft_task_scheduler::worker_loop()
{
    while (true)
    {
        task_queue_entry queue_entry;
        bool has_task;
        unsigned long long previous_span;

        if (!this->_running.load())
            break;
        has_task = this->_queue.wait_pop(queue_entry, this->_running);
        if (!has_task)
        {
            if (!this->_running.load())
                break;
            this->set_error(this->_queue.get_error());
            continue;
        }
        if (!this->update_worker_counters(1, -1))
            return ;
        if (!this->update_queue_size(-1))
            return ;
        this->trace_emit_event(FT_TASK_TRACE_PHASE_DEQUEUED, queue_entry._trace_id,
                queue_entry._parent_id, queue_entry._label, false);
        this->trace_emit_event(FT_TASK_TRACE_PHASE_STARTED, queue_entry._trace_id,
                queue_entry._parent_id, queue_entry._label, false);
        previous_span = task_scheduler_trace_push_span(queue_entry._trace_id);
        this->set_error(ER_SUCCESS);
        if (queue_entry._function)
            queue_entry._function();
        task_scheduler_trace_pop_span(previous_span);
        if (!this->update_worker_counters(-1, 1))
            return ;
        this->trace_emit_event(FT_TASK_TRACE_PHASE_FINISHED, queue_entry._trace_id,
                queue_entry._parent_id, queue_entry._label, false);
    }
    (void)this->update_worker_counters(0, -1);
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

bool ft_task_scheduler::update_queue_size(long long delta)
{
    if (this->_queue_metrics_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        this->set_error(this->_queue_metrics_mutex.get_error());
        return (false);
    }
    this->_queue_size_counter += delta;
    if (this->_queue_metrics_mutex.unlock(THREAD_ID) != FT_SUCCESS)
    {
        this->set_error(this->_queue_metrics_mutex.get_error());
        return (false);
    }
    return (true);
}

bool ft_task_scheduler::update_worker_counters(long long active_delta, long long idle_delta)
{
    if (this->_worker_metrics_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        this->set_error(this->_worker_metrics_mutex.get_error());
        return (false);
    }
    this->_worker_active_counter += active_delta;
    this->_worker_idle_counter += idle_delta;
    if (this->_worker_metrics_mutex.unlock(THREAD_ID) != FT_SUCCESS)
    {
        this->set_error(this->_worker_metrics_mutex.get_error());
        return (false);
    }
    return (true);
}

bool ft_task_scheduler::update_worker_total(long long delta)
{
    if (this->_worker_metrics_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        this->set_error(this->_worker_metrics_mutex.get_error());
        return (false);
    }
    if (delta >= 0)
        this->_worker_total_count += static_cast<size_t>(delta);
    else
    {
        size_t decrement;

        decrement = static_cast<size_t>(-delta);
        if (decrement > this->_worker_total_count)
            this->_worker_total_count = 0;
        else
            this->_worker_total_count -= decrement;
    }
    if (this->_worker_metrics_mutex.unlock(THREAD_ID) != FT_SUCCESS)
    {
        this->set_error(this->_worker_metrics_mutex.get_error());
        return (false);
    }
    return (true);
}

bool ft_task_scheduler::scheduled_heap_push(scheduled_task &&task)
{
    size_t size;

    this->_scheduled.push_back(ft_move(task));
    if (this->_scheduled.get_error() != ER_SUCCESS)
        return (false);
    size = this->_scheduled.size();
    if (size == 0)
    {
        this->_scheduled_size_counter = 0;
        return (true);
    }
    this->scheduled_heap_sift_up(size - 1);
    this->_scheduled_size_counter = static_cast<long long>(this->_scheduled.size());
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
        this->_scheduled_size_counter = 0;
        return (true);
    }
    scheduled_task last_task;

    last_task = ft_move(this->_scheduled[size - 1]);
    this->_scheduled.pop_back();
    if (this->_scheduled.get_error() != ER_SUCCESS)
        return (false);
    this->_scheduled[0] = ft_move(last_task);
    this->scheduled_heap_sift_down(0);
    this->_scheduled_size_counter = static_cast<long long>(this->_scheduled.size());
    return (true);
}

bool ft_task_scheduler::scheduled_remove_index(size_t index)
{
    size_t size;

    size = this->_scheduled.size();
    if (this->_scheduled.get_error() != ER_SUCCESS)
        return (false);
    if (size == 0)
        return (false);
    if (index >= size)
        return (false);
    if (index == size - 1)
    {
        this->_scheduled.pop_back();
        if (this->_scheduled.get_error() != ER_SUCCESS)
            return (false);
        this->_scheduled_size_counter = static_cast<long long>(this->_scheduled.size());
        return (true);
    }
    scheduled_task last_task;

    last_task = ft_move(this->_scheduled[size - 1]);
    this->_scheduled.pop_back();
    if (this->_scheduled.get_error() != ER_SUCCESS)
        return (false);
    this->_scheduled[index] = ft_move(last_task);
    this->scheduled_heap_sift_down(index);
    this->scheduled_heap_sift_up(index);
    this->_scheduled_size_counter = static_cast<long long>(this->_scheduled.size());
    return (true);
}

bool ft_task_scheduler::cancel_task_state(const ft_sharedptr<ft_scheduled_task_state> &state)
{
    bool removed_entry;
    size_t index;
    ft_sharedptr<ft_scheduled_task_state> state_copy;
    ft_scheduled_task_state *state_pointer;
    unsigned long long cancelled_trace_id;
    unsigned long long cancelled_parent_id;
    const char *cancelled_label;
    bool should_emit_cancel;
    bool scheduler_lock_acquired;

    scheduler_lock_acquired = false;
    if (this->lock_internal(&scheduler_lock_acquired) != 0)
    {
        this->set_error(ft_errno);
        return (false);
    }
    if (!state)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        this->unlock_internal(scheduler_lock_acquired);
        return (false);
    }
    state_copy = state;
    if (state_copy.hasError())
    {
        this->set_error(state_copy.get_error());
        this->unlock_internal(scheduler_lock_acquired);
        return (false);
    }
    state_pointer = state_copy.get();
    if (!state_pointer)
    {
        this->set_error(FT_ERR_INVALID_ARGUMENT);
        this->unlock_internal(scheduler_lock_acquired);
        return (false);
    }
    state_pointer->cancel();
    if (state_pointer->get_error() != ER_SUCCESS)
    {
        this->set_error(state_pointer->get_error());
        this->unlock_internal(scheduler_lock_acquired);
        return (false);
    }
    if (this->_scheduled_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        this->set_error(this->_scheduled_mutex.get_error());
        this->unlock_internal(scheduler_lock_acquired);
        return (false);
    }
    index = 0;
    removed_entry = false;
    cancelled_trace_id = 0;
    cancelled_parent_id = 0;
    cancelled_label = ft_nullptr;
    should_emit_cancel = false;
    while (true)
    {
        size_t size;

        size = this->_scheduled.size();
        if (this->_scheduled.get_error() != ER_SUCCESS)
        {
            if (this->_scheduled_mutex.unlock(THREAD_ID) != FT_SUCCESS)
            {
                this->set_error(this->_scheduled_mutex.get_error());
                this->unlock_internal(scheduler_lock_acquired);
                return (false);
            }
            else
            {
                this->set_error(this->_scheduled.get_error());
                this->unlock_internal(scheduler_lock_acquired);
                return (false);
            }
        }
        if (index >= size)
            break;
        if (this->_scheduled[index]._state.get() == state_pointer)
        {
            cancelled_trace_id = this->_scheduled[index]._trace_id;
            cancelled_parent_id = this->_scheduled[index]._parent_id;
            cancelled_label = this->_scheduled[index]._label;
            if (!this->scheduled_remove_index(index))
            {
                if (this->_scheduled_mutex.unlock(THREAD_ID) != FT_SUCCESS)
                {
                    this->set_error(this->_scheduled_mutex.get_error());
                    this->unlock_internal(scheduler_lock_acquired);
                    return (false);
                }
                else
                {
                    this->set_error(this->_scheduled.get_error());
                    this->unlock_internal(scheduler_lock_acquired);
                    return (false);
                }
            }
            removed_entry = true;
            should_emit_cancel = true;
            break;
        }
        index++;
    }
    if (this->_scheduled_mutex.unlock(THREAD_ID) != FT_SUCCESS)
    {
        this->set_error(this->_scheduled_mutex.get_error());
        this->unlock_internal(scheduler_lock_acquired);
        return (false);
    }
    if (this->_scheduled_condition.broadcast() != 0)
    {
        this->set_error(this->_scheduled_condition.get_error());
        this->unlock_internal(scheduler_lock_acquired);
        return (false);
    }
    if (!removed_entry)
    {
        this->set_error(ER_SUCCESS);
        this->unlock_internal(scheduler_lock_acquired);
        return (true);
    }
    if (should_emit_cancel)
    {
        this->trace_emit_event(FT_TASK_TRACE_PHASE_CANCELLED, cancelled_trace_id,
                cancelled_parent_id, cancelled_label, false);
    }
    this->set_error(ER_SUCCESS);
    this->unlock_internal(scheduler_lock_acquired);
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

                    clock_error = ft_map_system_error(errno);
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
            bool task_cancelled;

            task_cancelled = false;
            if (expired_task._state)
            {
                task_cancelled = expired_task._state->is_cancelled();
                if (expired_task._state->get_error() != ER_SUCCESS)
                {
                    int state_error;

                    state_error = expired_task._state->get_error();
                    if (this->_scheduled_mutex.unlock(THREAD_ID) != FT_SUCCESS)
                        this->set_error(this->_scheduled_mutex.get_error());
                    else
                        this->set_error(state_error);
                    return ;
                }
            }
            if (task_cancelled)
                continue;
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
                this->trace_emit_event(FT_TASK_TRACE_PHASE_CANCELLED,
                        expired_task._trace_id, expired_task._parent_id,
                        expired_task._label, true);
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
                    bool should_reschedule;

                    should_reschedule = true;
                    if (expired_task._state)
                    {
                        bool cancelled_now;

                        cancelled_now = expired_task._state->is_cancelled();
                        if (expired_task._state->get_error() != ER_SUCCESS)
                        {
                            int state_error;

                            state_error = expired_task._state->get_error();
                            if (this->_scheduled_mutex.unlock(THREAD_ID) != FT_SUCCESS)
                                this->set_error(this->_scheduled_mutex.get_error());
                            else
                                this->set_error(state_error);
                            return ;
                        }
                        if (cancelled_now)
                            should_reschedule = false;
                    }
                    if (should_reschedule)
                    {
                        t_monotonic_time_point updated_time;
                        unsigned long long previous_trace_id;
                        unsigned long long new_trace_id;
                        const char *reschedule_label;

                        updated_time = time_monotonic_point_add_ms(time_monotonic_point_now(),
                                expired_task._interval_ms);
                        expired_task._time = updated_time;
                        expired_task._function = ft_move(original_function);
                        previous_trace_id = expired_task._trace_id;
                        new_trace_id = task_scheduler_trace_generate_span_id();
                        reschedule_label = expired_task._label;
                        expired_task._trace_id = new_trace_id;
                        expired_task._parent_id = previous_trace_id;
                        expired_task._label = reschedule_label;
                        if (!this->scheduled_heap_push(ft_move(expired_task)))
                            this->set_error(this->_scheduled.get_error());
                        else
                            this->trace_emit_event(FT_TASK_TRACE_PHASE_TIMER_REGISTERED,
                                    new_trace_id, previous_trace_id, reschedule_label, true);
                    }
                }
                continue;
            }
            task_queue_entry queue_entry;

            queue_entry._function = expired_task._function;
            if (queue_entry._function.get_error() != ER_SUCCESS)
            {
                int copy_error;
                ft_function<void()> original_function;

                copy_error = queue_entry._function.get_error();
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
                    bool should_reschedule;

                    should_reschedule = true;
                    if (expired_task._state)
                    {
                        bool cancelled_now;

                        cancelled_now = expired_task._state->is_cancelled();
                        if (expired_task._state->get_error() != ER_SUCCESS)
                        {
                            int state_error;

                            state_error = expired_task._state->get_error();
                            if (this->_scheduled_mutex.unlock(THREAD_ID) != FT_SUCCESS)
                                this->set_error(this->_scheduled_mutex.get_error());
                            else
                                this->set_error(state_error);
                            return ;
                        }
                        if (cancelled_now)
                            should_reschedule = false;
                    }
                    if (should_reschedule)
                    {
                        t_monotonic_time_point updated_time;

                        updated_time = time_monotonic_point_add_ms(time_monotonic_point_now(),
                                expired_task._interval_ms);
                        expired_task._time = updated_time;
                        expired_task._function = ft_move(original_function);
                        if (!this->scheduled_heap_push(ft_move(expired_task)))
                            this->set_error(this->_scheduled.get_error());
                    }
                }
                continue;
            }
            if (this->_scheduled_mutex.unlock(THREAD_ID) != FT_SUCCESS)
            {
                this->set_error(this->_scheduled_mutex.get_error());
                return ;
            }
            queue_entry._trace_id = expired_task._trace_id;
            queue_entry._parent_id = expired_task._parent_id;
            queue_entry._label = expired_task._label;
            this->trace_emit_event(FT_TASK_TRACE_PHASE_TIMER_TRIGGERED,
                    queue_entry._trace_id, queue_entry._parent_id,
                    queue_entry._label, true);
            this->_queue.push(ft_move(queue_entry));
            if (this->_queue.get_error() != ER_SUCCESS)
            {
                this->set_error(this->_queue.get_error());
                this->trace_emit_event(FT_TASK_TRACE_PHASE_CANCELLED,
                        expired_task._trace_id, expired_task._parent_id,
                        expired_task._label, true);
                if (expired_task._function)
                    expired_task._function();
            }
            else
            {
                bool metrics_updated;

                metrics_updated = this->update_queue_size(1);
                this->trace_emit_event(FT_TASK_TRACE_PHASE_ENQUEUED,
                        expired_task._trace_id, expired_task._parent_id,
                        expired_task._label, true);
                if (!metrics_updated)
                    return ;
                this->set_error(ER_SUCCESS);
            }
            if (!this->_running.load())
                return ;
            if (this->_scheduled_mutex.lock(THREAD_ID) != FT_SUCCESS)
            {
                this->set_error(this->_scheduled_mutex.get_error());
                return ;
            }
            if (expired_task._interval_ms > 0)
            {
                bool should_reschedule;

                should_reschedule = true;
                if (expired_task._state)
                {
                    bool cancelled_now;

                    cancelled_now = expired_task._state->is_cancelled();
                    if (expired_task._state->get_error() != ER_SUCCESS)
                    {
                        int state_error;

                        state_error = expired_task._state->get_error();
                        if (this->_scheduled_mutex.unlock(THREAD_ID) != FT_SUCCESS)
                            this->set_error(this->_scheduled_mutex.get_error());
                        else
                            this->set_error(state_error);
                        return ;
                    }
                    if (cancelled_now)
                        should_reschedule = false;
                }
                if (should_reschedule)
                {
                    t_monotonic_time_point updated_time;
                    unsigned long long previous_trace_id;
                    unsigned long long new_trace_id;
                    const char *reschedule_label;

                    updated_time = time_monotonic_point_add_ms(time_monotonic_point_now(),
                            expired_task._interval_ms);
                    expired_task._time = updated_time;
                    previous_trace_id = expired_task._trace_id;
                    new_trace_id = task_scheduler_trace_generate_span_id();
                    reschedule_label = expired_task._label;
                    expired_task._trace_id = new_trace_id;
                    expired_task._parent_id = previous_trace_id;
                    expired_task._label = reschedule_label;
                    if (!this->scheduled_heap_push(ft_move(expired_task)))
                        this->set_error(this->_scheduled.get_error());
                    else
                        this->trace_emit_event(FT_TASK_TRACE_PHASE_TIMER_REGISTERED,
                                new_trace_id, previous_trace_id, reschedule_label, true);
                }
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

bool ft_task_scheduler::capture_metrics(ft_task_trace_event &event) const
{
    if (this->_queue_metrics_mutex.lock(THREAD_ID) != FT_SUCCESS)
        return (false);
    event.queue_depth = this->_queue_size_counter;
    if (this->_queue_metrics_mutex.unlock(THREAD_ID) != FT_SUCCESS)
        return (false);
    if (this->_scheduled_mutex.lock(THREAD_ID) != FT_SUCCESS)
        return (false);
    event.scheduled_depth = this->_scheduled_size_counter;
    if (this->_scheduled_mutex.unlock(THREAD_ID) != FT_SUCCESS)
        return (false);
    if (this->_worker_metrics_mutex.lock(THREAD_ID) != FT_SUCCESS)
        return (false);
    event.worker_active_count = this->_worker_active_counter;
    event.worker_idle_count = this->_worker_idle_counter;
    if (this->_worker_metrics_mutex.unlock(THREAD_ID) != FT_SUCCESS)
        return (false);
    return (true);
}

void ft_task_scheduler::trace_emit_event(e_ft_task_trace_phase phase, unsigned long long trace_id,
        unsigned long long parent_id, const char *label, bool timer_thread)
{
    ft_task_trace_event event;
    bool captured;
    t_thread_id thread_identifier;

    event.phase = phase;
    event.trace_id = trace_id;
    event.parent_id = parent_id;
    event.label = label;
    event.timestamp = time_monotonic_point_now();
    thread_identifier = ft_this_thread_get_id();
    event.thread_id = thread_identifier.native_id;
    event.queue_depth = -1;
    event.scheduled_depth = -1;
    event.worker_active_count = -1;
    event.worker_idle_count = -1;
    event.timer_thread = timer_thread;
    captured = this->capture_metrics(event);
    (void)captured;
    task_scheduler_trace_emit(event);
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

long long ft_task_scheduler::get_queue_size() const
{
    long long queue_size;

    if (this->_queue_metrics_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        this->set_error(this->_queue_metrics_mutex.get_error());
        return (0);
    }
    queue_size = this->_queue_size_counter;
    if (this->_queue_metrics_mutex.unlock(THREAD_ID) != FT_SUCCESS)
    {
        this->set_error(this->_queue_metrics_mutex.get_error());
        return (0);
    }
    this->set_error(ER_SUCCESS);
    return (queue_size);
}

long long ft_task_scheduler::get_scheduled_task_count() const
{
    long long scheduled_count;

    if (this->_scheduled_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        this->set_error(this->_scheduled_mutex.get_error());
        return (0);
    }
    scheduled_count = this->_scheduled_size_counter;
    if (this->_scheduled_mutex.unlock(THREAD_ID) != FT_SUCCESS)
    {
        this->set_error(this->_scheduled_mutex.get_error());
        return (0);
    }
    this->set_error(ER_SUCCESS);
    return (scheduled_count);
}

long long ft_task_scheduler::get_worker_active_count() const
{
    long long active_count;

    if (this->_worker_metrics_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        this->set_error(this->_worker_metrics_mutex.get_error());
        return (0);
    }
    active_count = this->_worker_active_counter;
    if (this->_worker_metrics_mutex.unlock(THREAD_ID) != FT_SUCCESS)
    {
        this->set_error(this->_worker_metrics_mutex.get_error());
        return (0);
    }
    this->set_error(ER_SUCCESS);
    return (active_count);
}

long long ft_task_scheduler::get_worker_idle_count() const
{
    long long idle_count;

    if (this->_worker_metrics_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        this->set_error(this->_worker_metrics_mutex.get_error());
        return (0);
    }
    idle_count = this->_worker_idle_counter;
    if (this->_worker_metrics_mutex.unlock(THREAD_ID) != FT_SUCCESS)
    {
        this->set_error(this->_worker_metrics_mutex.get_error());
        return (0);
    }
    this->set_error(ER_SUCCESS);
    return (idle_count);
}

size_t ft_task_scheduler::get_worker_total_count() const
{
    size_t total_count;

    if (this->_worker_metrics_mutex.lock(THREAD_ID) != FT_SUCCESS)
    {
        this->set_error(this->_worker_metrics_mutex.get_error());
        return (0);
    }
    total_count = this->_worker_total_count;
    if (this->_worker_metrics_mutex.unlock(THREAD_ID) != FT_SUCCESS)
    {
        this->set_error(this->_worker_metrics_mutex.get_error());
        return (0);
    }
    this->set_error(ER_SUCCESS);
    return (total_count);
}

int ft_task_scheduler::enable_thread_safety()
{
    void *memory;
    pt_mutex *state_mutex;

    if (this->_thread_safe_enabled && this->_state_mutex != ft_nullptr)
    {
        if (this->_queue.enable_thread_safety() != 0)
        {
            this->set_error(this->_queue.get_error());
            return (-1);
        }
        if (this->_scheduled_mutex.enable_thread_safety() != 0)
        {
            this->set_error(this->_scheduled_mutex.get_error());
            return (-1);
        }
        if (this->_scheduled_condition.enable_thread_safety() != 0)
        {
            this->set_error(this->_scheduled_condition.get_error());
            return (-1);
        }
        if (this->_queue_metrics_mutex.enable_thread_safety() != 0)
        {
            this->set_error(this->_queue_metrics_mutex.get_error());
            return (-1);
        }
        if (this->_worker_metrics_mutex.enable_thread_safety() != 0)
        {
            this->set_error(this->_worker_metrics_mutex.get_error());
            return (-1);
        }
        this->set_error(ER_SUCCESS);
        return (0);
    }
    memory = cma_malloc(sizeof(pt_mutex));
    if (memory == ft_nullptr)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        return (-1);
    }
    state_mutex = new(memory) pt_mutex();
    if (state_mutex->get_error() != ER_SUCCESS)
    {
        int mutex_error;

        mutex_error = state_mutex->get_error();
        state_mutex->~pt_mutex();
        cma_free(memory);
        this->set_error(mutex_error);
        return (-1);
    }
    this->_state_mutex = state_mutex;
    this->_thread_safe_enabled = true;
    if (this->_queue.enable_thread_safety() != 0)
    {
        int queue_error;

        queue_error = this->_queue.get_error();
        this->teardown_thread_safety();
        this->set_error(queue_error);
        return (-1);
    }
    if (this->_scheduled_mutex.enable_thread_safety() != 0)
    {
        int scheduled_error;

        scheduled_error = this->_scheduled_mutex.get_error();
        this->teardown_thread_safety();
        this->set_error(scheduled_error);
        return (-1);
    }
    if (this->_scheduled_condition.enable_thread_safety() != 0)
    {
        int condition_error;

        condition_error = this->_scheduled_condition.get_error();
        this->teardown_thread_safety();
        this->set_error(condition_error);
        return (-1);
    }
    if (this->_queue_metrics_mutex.enable_thread_safety() != 0)
    {
        int queue_metric_error;

        queue_metric_error = this->_queue_metrics_mutex.get_error();
        this->teardown_thread_safety();
        this->set_error(queue_metric_error);
        return (-1);
    }
    if (this->_worker_metrics_mutex.enable_thread_safety() != 0)
    {
        int worker_metric_error;

        worker_metric_error = this->_worker_metrics_mutex.get_error();
        this->teardown_thread_safety();
        this->set_error(worker_metric_error);
        return (-1);
    }
    this->set_error(ER_SUCCESS);
    return (0);
}

void ft_task_scheduler::disable_thread_safety()
{
    this->teardown_thread_safety();
    this->set_error(ER_SUCCESS);
    return ;
}

bool ft_task_scheduler::is_thread_safe_enabled() const
{
    bool enabled;

    enabled = (this->_thread_safe_enabled && this->_state_mutex != ft_nullptr);
    const_cast<ft_task_scheduler *>(this)->set_error(ER_SUCCESS);
    return (enabled);
}

int ft_task_scheduler::lock(bool *lock_acquired) const
{
    int entry_errno;
    int result;

    entry_errno = ft_errno;
    result = this->lock_internal(lock_acquired);
    if (result != 0)
    {
        const_cast<ft_task_scheduler *>(this)->set_error(ft_errno);
        return (result);
    }
    const_cast<ft_task_scheduler *>(this)->set_error(ER_SUCCESS);
    ft_errno = entry_errno;
    return (0);
}

void ft_task_scheduler::unlock(bool lock_acquired) const
{
    int entry_errno;

    entry_errno = ft_errno;
    this->unlock_internal(lock_acquired);
    if (this->_state_mutex != ft_nullptr && this->_state_mutex->get_error() != ER_SUCCESS)
    {
        const_cast<ft_task_scheduler *>(this)->set_error(this->_state_mutex->get_error());
        return ;
    }
    ft_errno = entry_errno;
    const_cast<ft_task_scheduler *>(this)->set_error(ER_SUCCESS);
    ft_errno = entry_errno;
    return ;
}

int ft_task_scheduler::lock_internal(bool *lock_acquired) const
{
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (!this->_thread_safe_enabled || this->_state_mutex == ft_nullptr)
    {
        ft_errno = ER_SUCCESS;
        return (0);
    }
    this->_state_mutex->lock(THREAD_ID);
    if (this->_state_mutex->get_error() != ER_SUCCESS)
    {
        if (this->_state_mutex->get_error() == FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            bool state_lock_acquired;

            state_lock_acquired = false;
            ft_errno = ER_SUCCESS;
            if (this->_state_mutex->lock_state(&state_lock_acquired) == 0)
                this->_state_mutex->unlock_state(state_lock_acquired);
            ft_errno = ER_SUCCESS;
            return (0);
        }
        ft_errno = this->_state_mutex->get_error();
        return (-1);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    ft_errno = ER_SUCCESS;
    return (0);
}

void ft_task_scheduler::unlock_internal(bool lock_acquired) const
{
    int entry_errno;

    if (!lock_acquired || this->_state_mutex == ft_nullptr)
        return ;
    entry_errno = ft_errno;
    this->_state_mutex->unlock(THREAD_ID);
    if (this->_state_mutex->get_error() != ER_SUCCESS)
    {
        ft_errno = this->_state_mutex->get_error();
        return ;
    }
    ft_errno = entry_errno;
    return ;
}

void ft_task_scheduler::teardown_thread_safety()
{
    this->_queue.disable_thread_safety();
    this->_scheduled_mutex.disable_thread_safety();
    this->_scheduled_condition.disable_thread_safety();
    this->_queue_metrics_mutex.disable_thread_safety();
    this->_worker_metrics_mutex.disable_thread_safety();
    if (this->_state_mutex != ft_nullptr)
    {
        this->_state_mutex->~pt_mutex();
        cma_free(this->_state_mutex);
        this->_state_mutex = ft_nullptr;
    }
    this->_thread_safe_enabled = false;
    return ;
}

