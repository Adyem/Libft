#include "task_scheduler.hpp"
#include "pthread.hpp"
#include "recursive_mutex.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../Printf/printf.hpp"
#include "../System_utils/system_utils.hpp"
#include <new>
#include <utility>


static void task_scheduler_abort_lifecycle_error(const char *method_name,
    const char *reason)
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_task_scheduler::scheduled_task lifecycle error: %s: %s\n",
        method_name, reason);
    su_abort();
    return ;
}

ft_task_scheduler::scheduled_task::scheduled_task()
{
    this->_initialized_state = ft_task_scheduler::scheduled_task::_state_uninitialized;
    (void)this->initialize();
    return ;
}

ft_task_scheduler::scheduled_task::~scheduled_task()
{
    if (this->_initialized_state == ft_task_scheduler::scheduled_task::_state_uninitialized)
    {
        task_scheduler_abort_lifecycle_error(
            "scheduled_task::~scheduled_task",
            "destructor called while object is uninitialized");
        return ;
    }
    if (this->_initialized_state == ft_task_scheduler::scheduled_task::_state_initialized)
        (void)this->destroy();
    return ;
}

ft_task_scheduler::scheduled_task::scheduled_task(const scheduled_task &other)
{
    this->_initialized_state = ft_task_scheduler::scheduled_task::_state_uninitialized;
    (void)this->initialize(other);
    return ;
}

ft_task_scheduler::scheduled_task::scheduled_task(scheduled_task &&other)
{
    this->_initialized_state = ft_task_scheduler::scheduled_task::_state_uninitialized;
    (void)this->initialize_move(other);
    return ;
}

ft_task_scheduler::scheduled_task &
ft_task_scheduler::scheduled_task::operator=(const scheduled_task &other)
{
    if (other._initialized_state != ft_task_scheduler::scheduled_task::_state_initialized)
    {
        task_scheduler_abort_lifecycle_error(
            "scheduled_task::operator=(const scheduled_task &) source",
            "called with source object that is not initialized");
        return (*this);
    }
    if (this == &other)
        return (*this);
    if (this->_initialized_state == ft_task_scheduler::scheduled_task::_state_initialized)
        (void)this->destroy();
    (void)this->initialize(other);
    return (*this);
}

ft_task_scheduler::scheduled_task &
ft_task_scheduler::scheduled_task::operator=(scheduled_task &&other)
{
    if (other._initialized_state != ft_task_scheduler::scheduled_task::_state_initialized)
    {
        task_scheduler_abort_lifecycle_error(
            "scheduled_task::operator=(scheduled_task &&) source",
            "called with source object that is not initialized");
        return (*this);
    }
    if (this == &other)
        return (*this);
    if (this->_initialized_state == ft_task_scheduler::scheduled_task::_state_initialized)
        (void)this->destroy();
    (void)this->initialize_move(other);
    return (*this);
}

int ft_task_scheduler::scheduled_task::initialize()
{
    if (this->_initialized_state == ft_task_scheduler::scheduled_task::_state_initialized)
    {
        task_scheduler_abort_lifecycle_error("scheduled_task::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_time = t_monotonic_time_point();
    this->_interval_ms = 0;
    this->_function = ft_function<void()>();
    this->_state = ft_sharedptr<ft_scheduled_task_state>();
    this->_trace_id = 0;
    this->_parent_id = 0;
    this->_label = ft_nullptr;
    this->_initialized_state = ft_task_scheduler::scheduled_task::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int ft_task_scheduler::scheduled_task::initialize(const scheduled_task &other)
{
    if (other._initialized_state != ft_task_scheduler::scheduled_task::_state_initialized)
    {
        task_scheduler_abort_lifecycle_error(
            "scheduled_task::initialize(const scheduled_task &) source",
            "called with source object that is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (this->initialize() != FT_ERR_SUCCESS)
        return (FT_ERR_INVALID_STATE);
    this->_time = other._time;
    this->_interval_ms = other._interval_ms;
    this->_function = other._function;
    this->_state = other._state;
    this->_trace_id = other._trace_id;
    this->_parent_id = other._parent_id;
    this->_label = other._label;
    return (FT_ERR_SUCCESS);
}

int ft_task_scheduler::scheduled_task::initialize_move(scheduled_task &other)
{
    if (other._initialized_state != ft_task_scheduler::scheduled_task::_state_initialized)
    {
        task_scheduler_abort_lifecycle_error(
            "scheduled_task::initialize_move(scheduled_task &) source",
            "called with source object that is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    if (this == &other)
        return (FT_ERR_SUCCESS);
    if (this->initialize() != FT_ERR_SUCCESS)
        return (FT_ERR_INVALID_STATE);
    this->_time = other._time;
    this->_interval_ms = other._interval_ms;
    this->_function = ft_move(other._function);
    this->_state = other._state;
    this->_trace_id = other._trace_id;
    this->_parent_id = other._parent_id;
    this->_label = other._label;
    other._time = t_monotonic_time_point();
    other._interval_ms = 0;
    other._function = ft_function<void()>();
    other._state.reset();
    other._trace_id = 0;
    other._parent_id = 0;
    other._label = ft_nullptr;
    other._initialized_state = ft_task_scheduler::scheduled_task::_state_destroyed;
    return (FT_ERR_SUCCESS);
}

int ft_task_scheduler::scheduled_task::destroy()
{
    if (this->_initialized_state != ft_task_scheduler::scheduled_task::_state_initialized)
    {
        task_scheduler_abort_lifecycle_error("scheduled_task::destroy",
            "called while object is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_time = t_monotonic_time_point();
    this->_interval_ms = 0;
    this->_function = ft_function<void()>();
    this->_state.reset();
    this->_trace_id = 0;
    this->_parent_id = 0;
    this->_label = ft_nullptr;
    this->_initialized_state = ft_task_scheduler::scheduled_task::_state_destroyed;
    return (FT_ERR_SUCCESS);
}

int ft_task_scheduler::scheduled_task::is_initialized() const
{
    if (this->_initialized_state != ft_task_scheduler::scheduled_task::_state_initialized)
    {
        task_scheduler_abort_lifecycle_error("scheduled_task::is_initialized",
            "called while object is not initialized");
        return (0);
    }
    return (1);
}

pt_mutex *ft_scheduled_task_state::mutex_handle() const
{
    return (this->_state_mutex);
}

ft_scheduled_task_state::ft_scheduled_task_state()
    : _cancelled(false), _state_mutex(ft_nullptr)
{
    return ;
}

ft_scheduled_task_state::~ft_scheduled_task_state()
{
    this->teardown_thread_safety();
    return ;
}

void ft_scheduled_task_state::cancel()
{
    bool lock_acquired;

    lock_acquired = false;
    int lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    this->_cancelled.store(true);
    this->unlock_internal(lock_acquired);
    return ;
}

bool ft_scheduled_task_state::is_cancelled() const
{
    bool cancelled_value;
    bool lock_acquired;

    lock_acquired = false;
    int lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (false);
    cancelled_value = this->_cancelled.load();
    this->unlock_internal(lock_acquired);
    return (cancelled_value);
}

int ft_scheduled_task_state::enable_thread_safety()
{
    pt_mutex *mutex_pointer;
    int mutex_error;

    if (this->_state_mutex != ft_nullptr)
        return (0);
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
    return (0);
}

void ft_scheduled_task_state::disable_thread_safety()
{
    this->teardown_thread_safety();
    return ;
}

bool ft_scheduled_task_state::is_thread_safe_enabled() const
{
    return (this->_state_mutex != ft_nullptr);
}

int ft_scheduled_task_state::lock(bool *lock_acquired) const
{
    int lock_result = this->lock_internal(lock_acquired);
    if (lock_result != FT_ERR_SUCCESS)
        return (-1);
    return (0);
}

void ft_scheduled_task_state::unlock(bool lock_acquired) const
{
    (void)this->unlock_internal(lock_acquired);
    return ;
}

int ft_scheduled_task_state::lock_internal(bool *lock_acquired) const
{
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (this->_state_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    int mutex_error = this->_state_mutex->lock();

    if (mutex_error != FT_ERR_SUCCESS)
    {
        if (mutex_error == FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            bool state_lock_acquired;

            state_lock_acquired = false;
            if (this->_state_mutex->lock_state(&state_lock_acquired) == 0)
                this->_state_mutex->unlock_state(state_lock_acquired);
            return (FT_ERR_SUCCESS);
        }
        return (mutex_error);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

int ft_scheduled_task_state::unlock_internal(bool lock_acquired) const
{
    if (!lock_acquired || this->_state_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    return (this->_state_mutex->unlock());
}

void ft_scheduled_task_state::teardown_thread_safety()
{
    if (this->_state_mutex != ft_nullptr)
    {
        int destroy_error;

        destroy_error = this->_state_mutex->destroy();
        (void)destroy_error;
        delete this->_state_mutex;
        this->_state_mutex = ft_nullptr;
    }
    return ;
}

pt_mutex *ft_scheduled_task_handle::mutex_handle() const
{
    return (this->_state_mutex);
}

ft_scheduled_task_handle::ft_scheduled_task_handle()
    : _state(), _scheduler(ft_nullptr), _state_mutex(ft_nullptr)
{
    return ;
}

ft_scheduled_task_handle::ft_scheduled_task_handle(ft_task_scheduler *scheduler,
        const ft_sharedptr<ft_scheduled_task_state> &state)
    : _state(), _scheduler(scheduler), _state_mutex(ft_nullptr)
{
    if (!scheduler || !state)
    {
        this->_scheduler = ft_nullptr;
        return ;
    }
    this->_state = state;
    {
        int state_error = ft_sharedptr<ft_scheduled_task_state>::last_operation_error();

        if (state_error != FT_ERR_SUCCESS)
        {
            this->_scheduler = ft_nullptr;
            return ;
        }
    }
    return ;
}

ft_scheduled_task_handle::ft_scheduled_task_handle(const ft_scheduled_task_handle &other)
    : _state(), _scheduler(ft_nullptr), _state_mutex(ft_nullptr)
{
    bool lock_acquired;

    lock_acquired = false;
    int lock_error = other.lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return ;
    this->_state = other._state;
    this->_scheduler = other._scheduler;
    {
        int state_error = ft_sharedptr<ft_scheduled_task_state>::last_operation_error();

        if (state_error != FT_ERR_SUCCESS)
        {
            this->_scheduler = ft_nullptr;
            other.unlock_internal(lock_acquired);
            return ;
        }
    }
    if (other._state_mutex != ft_nullptr)
    {
        if (this->enable_thread_safety() != 0)
        {
            other.unlock_internal(lock_acquired);
            return ;
        }
    }
    other.unlock_internal(lock_acquired);
    return ;
}

ft_scheduled_task_handle &ft_scheduled_task_handle::operator=(const ft_scheduled_task_handle &other)
{
    if (this == &other)
        return (*this);
    bool this_lock_acquired;
    bool other_lock_acquired;

    this_lock_acquired = false;
    int this_lock_error = this->lock_internal(&this_lock_acquired);
    if (this_lock_error != FT_ERR_SUCCESS)
        return (*this);
    other_lock_acquired = false;
    int other_lock_error = other.lock_internal(&other_lock_acquired);
    if (other_lock_error != FT_ERR_SUCCESS)
    {
        this->unlock_internal(this_lock_acquired);
        return (*this);
    }
    this->_state = other._state;
    this->_scheduler = other._scheduler;
    {
        int state_error = ft_sharedptr<ft_scheduled_task_state>::last_operation_error();

        if (state_error != FT_ERR_SUCCESS)
        {
            this->_scheduler = ft_nullptr;
            other.unlock_internal(other_lock_acquired);
            this->unlock_internal(this_lock_acquired);
            return (*this);
        }
    }
    this->teardown_thread_safety();
    if (other._state_mutex != ft_nullptr)
    {
        if (this->enable_thread_safety() != 0)
        {
            other.unlock_internal(other_lock_acquired);
            this->unlock_internal(this_lock_acquired);
            return (*this);
        }
    }
    other.unlock_internal(other_lock_acquired);
    this->unlock_internal(this_lock_acquired);
    return (*this);
}

ft_scheduled_task_handle::~ft_scheduled_task_handle()
{
    bool lock_acquired;

    lock_acquired = false;
    int lock_result = this->lock_internal(&lock_acquired);
    if (lock_result == FT_ERR_SUCCESS)
    {
        this->_scheduler = ft_nullptr;
        this->unlock_internal(lock_acquired);
    }
    else
        this->_scheduler = ft_nullptr;
    this->teardown_thread_safety();
    return ;
}

bool ft_scheduled_task_handle::cancel()
{
    bool cancel_result;
    bool lock_acquired;

    lock_acquired = false;
    int lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (false);
    if (!this->_scheduler || !this->_state)
    {
        this->unlock_internal(lock_acquired);
        return (false);
    }
    cancel_result = this->_scheduler->cancel_task_state(this->_state);
    if (!cancel_result)
    {
        this->unlock_internal(lock_acquired);
        return (false);
    }
    this->unlock_internal(lock_acquired);
    return (true);
}

bool ft_scheduled_task_handle::valid() const
{
    bool state_valid;
    bool lock_acquired;

    lock_acquired = false;
    int lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (false);
    state_valid = static_cast<bool>(this->_state);
    if (!this->_scheduler || !state_valid)
    {
        this->unlock_internal(lock_acquired);
        return (false);
    }
    this->unlock_internal(lock_acquired);
    return (true);
}

const ft_scheduled_task_state *ft_scheduled_task_handle::get_state() const
{
    const ft_scheduled_task_state *state_pointer;
    bool lock_acquired;

    lock_acquired = false;
    int lock_error = this->lock_internal(&lock_acquired);
    if (lock_error != FT_ERR_SUCCESS)
        return (ft_nullptr);
    state_pointer = this->_state.get();
    this->unlock_internal(lock_acquired);
    return (state_pointer);
}

int ft_scheduled_task_handle::enable_thread_safety()
{
    pt_mutex *mutex_pointer;
    int mutex_error;

    if (this->_state_mutex != ft_nullptr)
        return (0);
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
    return (0);
}

void ft_scheduled_task_handle::disable_thread_safety()
{
    this->teardown_thread_safety();
    return ;
}

bool ft_scheduled_task_handle::is_thread_safe_enabled() const
{
    return (this->_state_mutex != ft_nullptr);
}

int ft_scheduled_task_handle::lock(bool *lock_acquired) const
{
    int lock_result = this->lock_internal(lock_acquired);
    if (lock_result != FT_ERR_SUCCESS)
        return (-1);
    return (0);
}

void ft_scheduled_task_handle::unlock(bool lock_acquired) const
{
    (void)this->unlock_internal(lock_acquired);
    return ;
}

int ft_scheduled_task_handle::lock_internal(bool *lock_acquired) const
{
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (this->_state_mutex == ft_nullptr)
        return (FT_ERR_SUCCESS);
    int mutex_error = this->_state_mutex->lock();

    if (mutex_error != FT_ERR_SUCCESS)
    {
        if (mutex_error == FT_ERR_MUTEX_ALREADY_LOCKED)
        {
            bool state_lock_acquired;

            state_lock_acquired = false;
            if (this->_state_mutex->lock_state(&state_lock_acquired) == 0)
                this->_state_mutex->unlock_state(state_lock_acquired);
            return (FT_ERR_SUCCESS);
        }
        return (mutex_error);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

int ft_scheduled_task_handle::unlock_internal(bool lock_acquired) const
{
    if (!lock_acquired || this->_state_mutex == ft_nullptr)
    {
        return (FT_ERR_SUCCESS);
    }
    return (this->_state_mutex->unlock());
}

void ft_scheduled_task_handle::teardown_thread_safety()
{
    if (this->_state_mutex != ft_nullptr)
    {
        int destroy_error;

        destroy_error = this->_state_mutex->destroy();
        (void)destroy_error;
        delete this->_state_mutex;
        this->_state_mutex = ft_nullptr;
    }
    return ;
}


void ft_task_scheduler::abort_lifecycle_error(const char *method_name,
        const char *reason) const
{
    if (method_name == ft_nullptr)
        method_name = "unknown";
    if (reason == ft_nullptr)
        reason = "unknown";
    pf_printf_fd(2, "ft_task_scheduler lifecycle error: %s: %s\n",
        method_name, reason);
    su_abort();
    return ;
}

void ft_task_scheduler::abort_if_not_initialized(const char *method_name) const
{
    if (this->_initialized_state == ft_task_scheduler::_state_initialized)
        return ;
    this->abort_lifecycle_error(method_name,
        "called while object is not initialized");
    return ;
}

pt_recursive_mutex *ft_task_scheduler::mutex_handle() const
{
    this->abort_if_not_initialized("ft_task_scheduler::mutex_handle");
    return (this->_state_mutex);
}

ft_task_scheduler::ft_task_scheduler(size_t thread_count)
    : _queue(), _workers(), _timer_thread(), _scheduled(), _scheduled_mutex(),
      _scheduled_condition(), _running(false), _queue_metrics_mutex(),
      _worker_metrics_mutex(), _queue_size_counter(0),
      _scheduled_size_counter(0), _worker_active_counter(0),
      _worker_idle_counter(0), _worker_total_count(0), _configured_thread_count(thread_count),
      _state_mutex(ft_nullptr), _initialized_state(ft_task_scheduler::_state_uninitialized)
{
    return ;
}

ft_task_scheduler::~ft_task_scheduler()
{
    if (this->_initialized_state == ft_task_scheduler::_state_uninitialized)
    {
        this->abort_lifecycle_error("ft_task_scheduler::~ft_task_scheduler",
            "destructor called while object is uninitialized");
        return ;
    }
    if (this->_initialized_state == ft_task_scheduler::_state_initialized)
        (void)this->destroy();
    return ;
}

int ft_task_scheduler::initialize(size_t thread_count)
{
    size_t index;
    unsigned int cpu_count;
    bool worker_failure;

    if (this->_initialized_state == ft_task_scheduler::_state_initialized)
    {
        this->abort_lifecycle_error("ft_task_scheduler::initialize",
            "called while object is already initialized");
        return (FT_ERR_INVALID_STATE);
    }
    if (thread_count == 0)
        thread_count = this->_configured_thread_count;
    if (thread_count == 0)
    {
        cpu_count = su_get_cpu_count();
        if (cpu_count == 0)
            thread_count = 1;
        else
            thread_count = cpu_count;
    }
    this->_configured_thread_count = thread_count;
    this->_queue_size_counter = 0;
    this->_scheduled_size_counter = 0;
    this->_worker_active_counter = 0;
    this->_worker_idle_counter = 0;
    this->_worker_total_count = 0;
    this->_running.store(true);
    if (this->_scheduled_mutex.initialize() != FT_ERR_SUCCESS)
    {
        this->_initialized_state = ft_task_scheduler::_state_destroyed;
        this->_running.store(false);
        return (FT_ERR_INVALID_STATE);
    }
    if (this->_queue_metrics_mutex.initialize() != FT_ERR_SUCCESS)
    {
        (void)this->_scheduled_mutex.destroy();
        this->_initialized_state = ft_task_scheduler::_state_destroyed;
        this->_running.store(false);
        return (FT_ERR_INVALID_STATE);
    }
    if (this->_worker_metrics_mutex.initialize() != FT_ERR_SUCCESS)
    {
        (void)this->_queue_metrics_mutex.destroy();
        (void)this->_scheduled_mutex.destroy();
        this->_initialized_state = ft_task_scheduler::_state_destroyed;
        this->_running.store(false);
        return (FT_ERR_INVALID_STATE);
    }
    worker_failure = false;
    this->_workers.reserve(thread_count);
    {
        int worker_reserve_error;

        worker_reserve_error = ft_vector<ft_thread>::last_operation_error();
        if (worker_reserve_error != FT_ERR_SUCCESS)
        {
            this->_running.store(false);
            (void)this->_worker_metrics_mutex.destroy();
            (void)this->_queue_metrics_mutex.destroy();
            (void)this->_scheduled_mutex.destroy();
            this->_initialized_state = ft_task_scheduler::_state_destroyed;
            return (worker_reserve_error);
        }
    }
    index = 0;
    while (index < thread_count)
    {
        ft_thread worker([this]()
        {
            this->worker_loop();
            return ;
        });
        {
            int worker_error;

            if (worker.joinable())
                worker_error = FT_ERR_SUCCESS;
            else
                worker_error = FT_ERR_SYSTEM;
            if (worker_error != FT_ERR_SUCCESS)
            {
                worker_failure = true;
                break;
            }
        }
        this->_workers.push_back(ft_move(worker));
        {
            int worker_push_error;

            worker_push_error = ft_vector<ft_thread>::last_operation_error();
            if (worker_push_error != FT_ERR_SUCCESS)
            {
                worker_failure = true;
                break;
            }
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
        (void)this->_worker_metrics_mutex.destroy();
        (void)this->_queue_metrics_mutex.destroy();
        (void)this->_scheduled_mutex.destroy();
        this->_initialized_state = ft_task_scheduler::_state_destroyed;
        return (FT_ERR_INVALID_STATE);
    }
    {
        ft_thread timer_worker([this]()
        {
            this->timer_loop();
            return ;
        });
        int timer_error;

        if (timer_worker.joinable())
            timer_error = FT_ERR_SUCCESS;
        else
            timer_error = FT_ERR_SYSTEM;
        if (timer_error != FT_ERR_SUCCESS)
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
            (void)this->_worker_metrics_mutex.destroy();
            (void)this->_queue_metrics_mutex.destroy();
            (void)this->_scheduled_mutex.destroy();
            this->_initialized_state = ft_task_scheduler::_state_destroyed;
            return (timer_error);
        }
        this->_timer_thread = ft_move(timer_worker);
    }
    this->_initialized_state = ft_task_scheduler::_state_initialized;
    return (FT_ERR_SUCCESS);
}

int ft_task_scheduler::destroy()
{
    if (this->_initialized_state != ft_task_scheduler::_state_initialized)
    {
        this->abort_lifecycle_error("ft_task_scheduler::destroy",
            "called while object is not initialized");
        return (FT_ERR_INVALID_STATE);
    }
    this->_running.store(false);
    this->_queue.shutdown();
    int scheduled_mutex_lock_error = this->_scheduled_mutex.lock();
    if (scheduled_mutex_lock_error == FT_ERR_SUCCESS)
    {
        this->_scheduled_condition.broadcast();
        int scheduled_mutex_unlock_error = this->_scheduled_mutex.unlock();
        (void)scheduled_mutex_unlock_error;
    }
    (void)scheduled_mutex_lock_error;
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
    (void)this->_worker_metrics_mutex.destroy();
    (void)this->_queue_metrics_mutex.destroy();
    (void)this->_scheduled_mutex.destroy();
    this->_initialized_state = ft_task_scheduler::_state_destroyed;
    return (FT_ERR_SUCCESS);
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
    int queue_metrics_lock_error = this->_queue_metrics_mutex.lock();
    if (queue_metrics_lock_error != FT_ERR_SUCCESS)
        return (false);
    this->_queue_size_counter += delta;
    int queue_metrics_unlock_error = this->_queue_metrics_mutex.unlock();
    if (queue_metrics_unlock_error != FT_ERR_SUCCESS)
        return (false);
    return (true);
}

bool ft_task_scheduler::update_worker_counters(long long active_delta, long long idle_delta)
{
    int worker_metrics_lock_error = this->_worker_metrics_mutex.lock();
    if (worker_metrics_lock_error != FT_ERR_SUCCESS)
        return (false);
    this->_worker_active_counter += active_delta;
    this->_worker_idle_counter += idle_delta;
    int worker_metrics_unlock_error = this->_worker_metrics_mutex.unlock();
    if (worker_metrics_unlock_error != FT_ERR_SUCCESS)
        return (false);
    return (true);
}

bool ft_task_scheduler::update_worker_total(long long delta)
{
    int worker_metrics_lock_error = this->_worker_metrics_mutex.lock();
    if (worker_metrics_lock_error != FT_ERR_SUCCESS)
        return (false);
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
    int worker_metrics_unlock_error = this->_worker_metrics_mutex.unlock();
    if (worker_metrics_unlock_error != FT_ERR_SUCCESS)
        return (false);
    return (true);
}

bool ft_task_scheduler::scheduled_heap_push(scheduled_task &&task)
{
    size_t size;

    this->_scheduled.push_back(ft_move(task));
    if (ft_vector<scheduled_task>::last_operation_error() != FT_ERR_SUCCESS)
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
    if (ft_vector<scheduled_task>::last_operation_error() != FT_ERR_SUCCESS)
        return (false);
    if (size == 0)
        return (false);
    task = ft_move(this->_scheduled[0]);
    if (size == 1)
    {
        this->_scheduled.pop_back();
        if (ft_vector<scheduled_task>::last_operation_error() != FT_ERR_SUCCESS)
            return (false);
        this->_scheduled_size_counter = 0;
        return (true);
    }
    scheduled_task last_task;

    last_task = ft_move(this->_scheduled[size - 1]);
    this->_scheduled.pop_back();
    if (ft_vector<scheduled_task>::last_operation_error() != FT_ERR_SUCCESS)
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
    if (ft_vector<scheduled_task>::last_operation_error() != FT_ERR_SUCCESS)
        return (false);
    if (size == 0)
        return (false);
    if (index >= size)
        return (false);
    if (index == size - 1)
    {
        this->_scheduled.pop_back();
        if (ft_vector<scheduled_task>::last_operation_error() != FT_ERR_SUCCESS)
            return (false);
        this->_scheduled_size_counter = static_cast<long long>(this->_scheduled.size());
        return (true);
    }
    scheduled_task last_task;

    last_task = ft_move(this->_scheduled[size - 1]);
        this->_scheduled.pop_back();
        if (ft_vector<scheduled_task>::last_operation_error() != FT_ERR_SUCCESS)
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
    int scheduled_mutex_error;
    int scheduled_mutex_unlock_error;

    scheduler_lock_acquired = false;
    int scheduler_lock_error = this->lock_internal(&scheduler_lock_acquired);
    if (scheduler_lock_error != FT_ERR_SUCCESS)
        return (false);
    if (!state)
    {
        this->unlock_internal(scheduler_lock_acquired);
        return (false);
    }
    state_copy = state;
    {
        int state_error = ft_sharedptr<ft_scheduled_task_state>::last_operation_error();

        if (state_error != FT_ERR_SUCCESS)
        {
            this->unlock_internal(scheduler_lock_acquired);
            return (false);
        }
    }
    state_pointer = state_copy.get();
    if (!state_pointer)
    {
        this->unlock_internal(scheduler_lock_acquired);
        return (false);
    }
    state_pointer->cancel();
    scheduled_mutex_error = this->_scheduled_mutex.lock();
    if (scheduled_mutex_error != FT_ERR_SUCCESS)
    {
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
        if (ft_vector<scheduled_task>::last_operation_error() != FT_ERR_SUCCESS)
        {
            scheduled_mutex_unlock_error = this->_scheduled_mutex.unlock();
            if (scheduled_mutex_unlock_error != FT_ERR_SUCCESS)
            {
                this->unlock_internal(scheduler_lock_acquired);
                return (false);
            }
            this->unlock_internal(scheduler_lock_acquired);
            return (false);
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
                    scheduled_mutex_unlock_error = this->_scheduled_mutex.unlock();
                    if (scheduled_mutex_unlock_error != FT_ERR_SUCCESS)
                    {
                        this->unlock_internal(scheduler_lock_acquired);
                        return (false);
                    }
                    this->unlock_internal(scheduler_lock_acquired);
                    return (false);
                }
            removed_entry = true;
            should_emit_cancel = true;
            break;
        }
        index++;
    }
    scheduled_mutex_unlock_error = this->_scheduled_mutex.unlock();
    if (scheduled_mutex_unlock_error != FT_ERR_SUCCESS)
    {
        this->unlock_internal(scheduler_lock_acquired);
        return (false);
    }
    if (this->_scheduled_condition.broadcast() != 0)
    {
        this->unlock_internal(scheduler_lock_acquired);
        return (false);
    }
    if (!removed_entry)
    {
        this->unlock_internal(scheduler_lock_acquired);
        return (true);
    }
    if (should_emit_cancel)
    {
        this->trace_emit_event(FT_TASK_TRACE_PHASE_CANCELLED, cancelled_trace_id,
                cancelled_parent_id, cancelled_label, false);
    }
    this->unlock_internal(scheduler_lock_acquired);
    return (true);
}

void ft_task_scheduler::timer_loop()
{
    while (true)
    {
        if (!this->_running.load())
            break;
        if (this->_scheduled_mutex.lock() != FT_ERR_SUCCESS)
            return ;
        while (true)
        {
            size_t scheduled_count;
            bool has_entries;

            if (!this->_running.load())
            {
                this->_scheduled_mutex.unlock();
                return ;
            }
            scheduled_count = this->_scheduled.size();
            if (ft_vector<scheduled_task>::last_operation_error() != FT_ERR_SUCCESS)
            {
                this->_scheduled_mutex.unlock();
                return ;
            }
            has_entries = scheduled_count > 0;
            if (!has_entries)
            {
                if (this->_scheduled_condition.wait(this->_scheduled_mutex) != 0)
                {
                    this->_scheduled_mutex.unlock();
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
                    this->_scheduled_mutex.unlock();
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
                    if (this->_scheduled_mutex.unlock() != FT_ERR_SUCCESS)
                        return ;
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
            if (ft_vector<scheduled_task>::last_operation_error() != FT_ERR_SUCCESS)
            {
                if (this->_scheduled_mutex.unlock() != FT_ERR_SUCCESS)
                    return ;
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
                int scheduled_error = ft_vector<scheduled_task>::last_operation_error();
                if (this->_scheduled_mutex.unlock() != FT_ERR_SUCCESS)
                    return ;
                (void)scheduled_error;
                return ;
            }
            bool task_cancelled;

            task_cancelled = false;
            if (expired_task._state)
            {
                task_cancelled = expired_task._state->is_cancelled();
            }
            if (task_cancelled)
                continue;
            if (!expired_task._function)
            {
                ft_function<void()> original_function;

                original_function = ft_move(expired_task._function);
                if (this->_scheduled_mutex.unlock() != FT_ERR_SUCCESS)
                    return ;
                this->trace_emit_event(FT_TASK_TRACE_PHASE_CANCELLED,
                        expired_task._trace_id, expired_task._parent_id,
                        expired_task._label, true);
                if (original_function)
                    original_function();
                if (!this->_running.load())
                    return ;
                if (this->_scheduled_mutex.lock() != FT_ERR_SUCCESS)
                    return ;
                if (expired_task._interval_ms > 0)
                {
                    bool should_reschedule;

                    should_reschedule = true;
                    if (expired_task._state)
                    {
                        bool cancelled_now;

                        cancelled_now = expired_task._state->is_cancelled();
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
                        {
                            (void)ft_vector<scheduled_task>::last_operation_error();
                        }
                        else
                            this->trace_emit_event(FT_TASK_TRACE_PHASE_TIMER_REGISTERED,
                                    new_trace_id, previous_trace_id, reschedule_label, true);
                    }
                }
                continue;
            }
            task_queue_entry queue_entry;

            queue_entry._function = expired_task._function;
            if (!queue_entry._function)
            {
                ft_function<void()> original_function;

                original_function = ft_move(expired_task._function);
                if (this->_scheduled_mutex.unlock() != FT_ERR_SUCCESS)
                    return ;
                if (original_function)
                    original_function();
                if (!this->_running.load())
                    return ;
                if (this->_scheduled_mutex.lock() != FT_ERR_SUCCESS)
                    return ;
                if (expired_task._interval_ms > 0)
                {
                    bool should_reschedule;

                    should_reschedule = true;
                    if (expired_task._state)
                    {
                        bool cancelled_now;

                        cancelled_now = expired_task._state->is_cancelled();
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
                            (void)ft_vector<scheduled_task>::last_operation_error();
                    }
                }
                continue;
            }
            if (this->_scheduled_mutex.unlock() != FT_ERR_SUCCESS)
                return ;
            queue_entry._trace_id = expired_task._trace_id;
            queue_entry._parent_id = expired_task._parent_id;
            queue_entry._label = expired_task._label;
            this->trace_emit_event(FT_TASK_TRACE_PHASE_TIMER_TRIGGERED,
                    queue_entry._trace_id, queue_entry._parent_id,
                    queue_entry._label, true);
            this->trace_emit_event(FT_TASK_TRACE_PHASE_ENQUEUED,
                    expired_task._trace_id, expired_task._parent_id,
                    expired_task._label, true);
            int queue_push_error = this->_queue.push(ft_move(queue_entry));
            if (queue_push_error != FT_ERR_SUCCESS)
            {
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
                if (!metrics_updated)
                    return ;
            }
            if (!this->_running.load())
                return ;
            if (this->_scheduled_mutex.lock() != FT_ERR_SUCCESS)
                return ;
            if (expired_task._interval_ms > 0)
            {
                bool should_reschedule;

                should_reschedule = true;
                if (expired_task._state)
                {
                    bool cancelled_now;

                    cancelled_now = expired_task._state->is_cancelled();
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
                        (void)ft_vector<scheduled_task>::last_operation_error();
                    else
                        this->trace_emit_event(FT_TASK_TRACE_PHASE_TIMER_REGISTERED,
                                new_trace_id, previous_trace_id, reschedule_label, true);
                }
            }
            continue;
        }
        if (this->_scheduled_mutex.unlock() != FT_ERR_SUCCESS)
            return ;
    }
    return ;
}

bool ft_task_scheduler::capture_metrics(ft_task_trace_event &event) const
{
    pthread_t current_thread_id;
    bool queue_locked;
    int queue_lock_error;

    current_thread_id = THREAD_ID;
    queue_locked = false;
    queue_lock_error = this->_queue_metrics_mutex.lock();
    if (queue_lock_error == FT_ERR_SUCCESS)
        queue_locked = true;
    else if (queue_lock_error == FT_ERR_MUTEX_ALREADY_LOCKED)
    {
        if (!this->_queue_metrics_mutex.is_owned_by_thread(current_thread_id))
            return (false);
    }
    else
        return (false);
    event.queue_depth = this->_queue_size_counter;
    if (queue_locked)
    {
        if (this->_queue_metrics_mutex.unlock() != FT_ERR_SUCCESS)
            return (false);
    }
    bool scheduled_locked;
    int scheduled_lock_error;

    scheduled_locked = false;
    scheduled_lock_error = this->_scheduled_mutex.lock();
    if (scheduled_lock_error == FT_ERR_SUCCESS)
        scheduled_locked = true;
    else if (scheduled_lock_error == FT_ERR_MUTEX_ALREADY_LOCKED)
    {
        if (!this->_scheduled_mutex.is_owned_by_thread(current_thread_id))
            return (false);
    }
    else
        return (false);
    event.scheduled_depth = this->_scheduled_size_counter;
    if (scheduled_locked)
    {
        if (this->_scheduled_mutex.unlock() != FT_ERR_SUCCESS)
            return (false);
    }
    bool worker_locked;
    int worker_lock_error;

    worker_locked = false;
    worker_lock_error = this->_worker_metrics_mutex.lock();
    if (worker_lock_error == FT_ERR_SUCCESS)
        worker_locked = true;
    else if (worker_lock_error == FT_ERR_MUTEX_ALREADY_LOCKED)
    {
        if (!this->_worker_metrics_mutex.is_owned_by_thread(current_thread_id))
            return (false);
    }
    else
        return (false);
    event.worker_active_count = this->_worker_active_counter;
    event.worker_idle_count = this->_worker_idle_counter;
    if (worker_locked)
    {
        if (this->_worker_metrics_mutex.unlock() != FT_ERR_SUCCESS)
            return (false);
    }
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

long long ft_task_scheduler::get_queue_size() const
{
    long long queue_size;

    queue_size = 0;
    (void)this->get_queue_size(&queue_size);
    return (queue_size);
}

int ft_task_scheduler::get_queue_size(long long *queue_size) const
{
    if (queue_size == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    this->abort_if_not_initialized("ft_task_scheduler::get_queue_size");
    if (this->_queue_metrics_mutex.lock() != FT_ERR_SUCCESS)
        return (FT_ERR_SYSTEM);
    *queue_size = this->_queue_size_counter;
    if (this->_queue_metrics_mutex.unlock() != FT_ERR_SUCCESS)
        return (FT_ERR_SYSTEM);
    return (FT_ERR_SUCCESS);
}

long long ft_task_scheduler::get_scheduled_task_count() const
{
    long long scheduled_count;

    scheduled_count = 0;
    (void)this->get_scheduled_task_count(&scheduled_count);
    return (scheduled_count);
}

int ft_task_scheduler::get_scheduled_task_count(long long *scheduled_count) const
{
    if (scheduled_count == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    this->abort_if_not_initialized("ft_task_scheduler::get_scheduled_task_count");
    if (this->_scheduled_mutex.lock() != FT_ERR_SUCCESS)
        return (FT_ERR_SYSTEM);
    *scheduled_count = this->_scheduled_size_counter;
    if (this->_scheduled_mutex.unlock() != FT_ERR_SUCCESS)
        return (FT_ERR_SYSTEM);
    return (FT_ERR_SUCCESS);
}

long long ft_task_scheduler::get_worker_active_count() const
{
    long long active_count;

    active_count = 0;
    (void)this->get_worker_active_count(&active_count);
    return (active_count);
}

int ft_task_scheduler::get_worker_active_count(long long *active_count) const
{
    if (active_count == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    this->abort_if_not_initialized("ft_task_scheduler::get_worker_active_count");
    if (this->_worker_metrics_mutex.lock() != FT_ERR_SUCCESS)
        return (FT_ERR_SYSTEM);
    *active_count = this->_worker_active_counter;
    if (this->_worker_metrics_mutex.unlock() != FT_ERR_SUCCESS)
        return (FT_ERR_SYSTEM);
    return (FT_ERR_SUCCESS);
}

long long ft_task_scheduler::get_worker_idle_count() const
{
    long long idle_count;

    idle_count = 0;
    (void)this->get_worker_idle_count(&idle_count);
    return (idle_count);
}

int ft_task_scheduler::get_worker_idle_count(long long *idle_count) const
{
    if (idle_count == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    this->abort_if_not_initialized("ft_task_scheduler::get_worker_idle_count");
    if (this->_worker_metrics_mutex.lock() != FT_ERR_SUCCESS)
        return (FT_ERR_SYSTEM);
    *idle_count = this->_worker_idle_counter;
    if (this->_worker_metrics_mutex.unlock() != FT_ERR_SUCCESS)
        return (FT_ERR_SYSTEM);
    return (FT_ERR_SUCCESS);
}

size_t ft_task_scheduler::get_worker_total_count() const
{
    size_t total_count;

    total_count = 0;
    (void)this->get_worker_total_count(&total_count);
    return (total_count);
}

int ft_task_scheduler::get_worker_total_count(size_t *total_count) const
{
    if (total_count == ft_nullptr)
        return (FT_ERR_INVALID_ARGUMENT);
    this->abort_if_not_initialized("ft_task_scheduler::get_worker_total_count");
    if (this->_worker_metrics_mutex.lock() != FT_ERR_SUCCESS)
        return (FT_ERR_SYSTEM);
    *total_count = this->_worker_total_count;
    if (this->_worker_metrics_mutex.unlock() != FT_ERR_SUCCESS)
        return (FT_ERR_SYSTEM);
    return (FT_ERR_SUCCESS);
}

int ft_task_scheduler::enable_thread_safety()
{
    pt_recursive_mutex *state_mutex;
    int mutex_error;

    this->abort_if_not_initialized("ft_task_scheduler::enable_thread_safety");
    if (this->_state_mutex != ft_nullptr)
    {
        if (this->_queue.enable_thread_safety() != 0)
            return (-1);
        if (this->_scheduled_condition.enable_thread_safety() != 0)
            return (-1);
        return (0);
    }
    state_mutex = new (std::nothrow) pt_recursive_mutex();
    if (state_mutex == ft_nullptr)
        return (-1);
    mutex_error = state_mutex->initialize();
    if (mutex_error != FT_ERR_SUCCESS)
    {
        delete state_mutex;
        return (-1);
    }
    this->_state_mutex = state_mutex;
    if (this->_queue.enable_thread_safety() != 0)
    {
        this->teardown_thread_safety();
        return (-1);
    }
    if (this->_scheduled_condition.enable_thread_safety() != 0)
    {
        this->teardown_thread_safety();
        return (-1);
    }
    return (0);
}

int ft_task_scheduler::disable_thread_safety()
{
    this->abort_if_not_initialized("ft_task_scheduler::disable_thread_safety");
    this->teardown_thread_safety();
    return (FT_ERR_SUCCESS);
}

bool ft_task_scheduler::is_thread_safe_enabled() const
{
    bool enabled;

    this->abort_if_not_initialized("ft_task_scheduler::is_thread_safe_enabled");
    enabled = (this->_state_mutex != ft_nullptr);
    return (enabled);
}

int ft_task_scheduler::lock(bool *lock_acquired) const
{
    int lock_result;

    this->abort_if_not_initialized("ft_task_scheduler::lock");
    lock_result = this->lock_internal(lock_acquired);
    if (lock_result != FT_ERR_SUCCESS)
        return (-1);
    return (0);
}

int ft_task_scheduler::unlock(bool lock_acquired) const
{
    int unlock_result;

    this->abort_if_not_initialized("ft_task_scheduler::unlock");
    unlock_result = this->unlock_internal(lock_acquired);
    if (unlock_result != FT_ERR_SUCCESS)
        return (-1);
    return (0);
}

int ft_task_scheduler::lock_internal(bool *lock_acquired) const
{
    this->abort_if_not_initialized("ft_task_scheduler::lock_internal");
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (this->_state_mutex == ft_nullptr)
    {
        return (FT_ERR_SUCCESS);
    }
    int lock_result;

    lock_result = this->_state_mutex->lock();
    if (lock_result != FT_ERR_SUCCESS)
    {
        return (lock_result);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESS);
}

int ft_task_scheduler::unlock_internal(bool lock_acquired) const
{
    this->abort_if_not_initialized("ft_task_scheduler::unlock_internal");
    if (!lock_acquired || this->_state_mutex == ft_nullptr)
    {
        return (FT_ERR_SUCCESS);
    }
    return (this->_state_mutex->unlock());
}

void ft_task_scheduler::teardown_thread_safety()
{
    int destroy_error;

    this->_queue.disable_thread_safety();
    this->_scheduled_condition.disable_thread_safety();
    if (this->_state_mutex != ft_nullptr)
    {
        destroy_error = this->_state_mutex->destroy();
        (void)destroy_error;
        delete this->_state_mutex;
        this->_state_mutex = ft_nullptr;
    }
    return ;
}
