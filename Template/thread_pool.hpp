#ifndef FT_THREAD_POOL_HPP
#define FT_THREAD_POOL_HPP

#include "../Errno/errno.hpp"
#include "../Errno/errno_internal.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/condition.hpp"

#include "vector.hpp"
#include "queue.hpp"
#include "cancellation.hpp"
#include "../PThread/thread.hpp"
#include "function.hpp"
#include <cstddef>
#include <cerrno>
#include <pthread.h>
#include "../PThread/pthread.hpp"
#include "../PThread/mutex.hpp"
#include "../PThread/pthread_internal.hpp"
#include <utility>
#include <type_traits>
#include <atomic>
#include "move.hpp"
class ft_thread_pool
{
    private:
        ft_vector<ft_thread>          _workers;
        ft_queue<ft_function<void()> > _tasks;
        size_t                        _max_tasks;
        bool                          _stop;
        size_t                        _active;
        mutable std::atomic<int>        _error_code;
        pthread_mutex_t               _mutex;
        pthread_cond_t                _cond;
        bool                          _mutex_initialized;
        bool                          _cond_initialized;
        mutable pt_mutex*             _external_mutex;
        void set_error(int error) const;
        static void record_operation_error_unlocked(int error_code) noexcept;
        void worker();
        int  lock_internal(bool *lock_acquired) const;
        void unlock_internal(bool lock_acquired) const;
        void teardown_thread_safety();

    public:
        ft_thread_pool(size_t thread_count = 0, size_t max_tasks = 0);
        ~ft_thread_pool();

        ft_thread_pool(const ft_thread_pool&) = delete;
        ft_thread_pool& operator=(const ft_thread_pool&) = delete;
        ft_thread_pool(ft_thread_pool&&) = delete;
        ft_thread_pool& operator=(ft_thread_pool&&) = delete;

        template <typename Function>
        void submit(Function &&function);

        template <typename Function>
        void submit(Function &&function, const ft_cancellation_token &token);

        void wait();
        void destroy();

        int  enable_thread_safety();
        void disable_thread_safety();
        bool is_thread_safe() const;
        int  lock(bool *lock_acquired) const;
        void unlock(bool lock_acquired) const;

        int get_error() const;
        const char* get_error_str() const;
};

inline void ft_thread_pool::worker()
{
    while (true)
    {
        ft_function<void()> task;
        if (pt_pthread_mutex_lock_with_error(&this->_mutex) != FT_ERR_SUCCESSS)
        {
            this->set_error(ft_global_error_stack_peek_last_error());
            return ;
        }
        bool queue_empty;
        queue_empty = this->_tasks.empty();
        if (this->_tasks.get_error() != FT_ERR_SUCCESSS)
        {
            pt_pthread_mutex_unlock_with_error(&this->_mutex);
            this->set_error(this->_tasks.get_error());
            continue;
        }
        while (queue_empty && !this->_stop)
        {
            if (pt_cond_wait(&this->_cond, &this->_mutex) != 0)
            {
                int cond_error;

                cond_error = ft_global_error_stack_drop_last_error();
                pt_pthread_mutex_unlock_with_error(&this->_mutex);
                this->set_error(cond_error);
                return ;
            }
            queue_empty = this->_tasks.empty();
            if (this->_tasks.get_error() != FT_ERR_SUCCESSS)
            {
                pt_pthread_mutex_unlock_with_error(&this->_mutex);
                this->set_error(this->_tasks.get_error());
                return ;
            }
        }
        if (this->_stop && queue_empty)
        {
            pt_pthread_mutex_unlock_with_error(&this->_mutex);
            return ;
        }
        task = this->_tasks.dequeue();
        if (this->_tasks.get_error() != FT_ERR_SUCCESSS)
        {
            pt_pthread_mutex_unlock_with_error(&this->_mutex);
            this->set_error(this->_tasks.get_error());
            continue;
        }
        ++this->_active;
        pt_pthread_mutex_unlock_with_error(&this->_mutex);
        task();
        if (pt_pthread_mutex_lock_with_error(&this->_mutex) != FT_ERR_SUCCESSS)
        {
            this->set_error(ft_global_error_stack_peek_last_error());
            return ;
        }
        --this->_active;
        queue_empty = this->_tasks.empty();
        if (this->_tasks.get_error() != FT_ERR_SUCCESSS)
        {
            pt_pthread_mutex_unlock_with_error(&this->_mutex);
            this->set_error(this->_tasks.get_error());
            return ;
        }
        if (queue_empty && this->_active == 0)
            pt_cond_broadcast(&this->_cond);
        pt_pthread_mutex_unlock_with_error(&this->_mutex);
    }
}

inline void ft_thread_pool::set_error(int error) const
{
    this->_error_code.store(error, std::memory_order_relaxed);
    ft_thread_pool::record_operation_error_unlocked(error);
}

inline ft_thread_pool::ft_thread_pool(size_t thread_count, size_t max_tasks)
    : _workers(), _tasks(), _max_tasks(max_tasks), _stop(false), _active(0), _error_code(FT_ERR_SUCCESSS),
      _mutex(), _cond(), _mutex_initialized(false), _cond_initialized(false),
      _external_mutex(ft_nullptr)
{
    if (pthread_mutex_init(&this->_mutex, ft_nullptr) != 0)
    {
        this->set_error(ft_map_system_error(errno));
        this->_stop = true;
        return ;
    }
    this->_mutex_initialized = true;
    if (pt_cond_init(&this->_cond, ft_nullptr) != 0)
    {
        int cond_error;

        cond_error = ft_global_error_stack_drop_last_error();
        this->set_error(cond_error);
        return ;
    }
    this->_cond_initialized = true;
    this->_workers.reserve(thread_count);
    if (this->_workers.get_error() != FT_ERR_SUCCESSS)
    {
        this->set_error(FT_ERR_NO_MEMORY);
        this->_stop = true;
        return ;
    }
    size_t worker_index = 0;
    while (worker_index < thread_count)
    {
        ft_thread worker(&ft_thread_pool::worker, this);
        this->_workers.push_back(ft_move(worker));
        if (this->_workers.get_error() != FT_ERR_SUCCESSS)
        {
            this->set_error(FT_ERR_NO_MEMORY);
            this->_stop = true;
            break;
        }
        ++worker_index;
    }
}

inline ft_thread_pool::~ft_thread_pool()
{
    this->destroy();
    if (this->_cond_initialized)
        pt_cond_destroy(&this->_cond);
    if (this->_mutex_initialized)
        pthread_mutex_destroy(&this->_mutex);
    this->teardown_thread_safety();
}

template <typename Function>
inline void ft_thread_pool::submit(Function &&function)
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(this->get_error());
        return ;
    }
        if (pt_pthread_mutex_lock_with_error(&this->_mutex) != FT_ERR_SUCCESSS)
        {
            this->set_error(ft_global_error_stack_peek_last_error());
            this->unlock_internal(lock_acquired);
            return ;
        }
    if (this->_stop)
    {
        pt_pthread_mutex_unlock_with_error(&this->_mutex);
        this->unlock_internal(lock_acquired);
        return ;
    }
    if (this->_max_tasks != 0 && this->_tasks.size() >= this->_max_tasks)
    {
        this->set_error(FT_ERR_FULL);
        pt_pthread_mutex_unlock_with_error(&this->_mutex);
        this->unlock_internal(lock_acquired);
        return ;
    }
    int queue_error;
    bool enqueue_failed;

    enqueue_failed = false;
    {
        ft_function<void()> wrapper(ft_move(function));

        this->_tasks.enqueue(ft_move(wrapper));
        queue_error = this->_tasks.get_error();
        if (queue_error != FT_ERR_SUCCESSS)
        {
            wrapper = ft_function<void()>();
            enqueue_failed = true;
        }
    }
    if (enqueue_failed)
    {
        this->set_error(queue_error);
        pt_pthread_mutex_unlock_with_error(&this->_mutex);
        this->unlock_internal(lock_acquired);
        return ;
    }
    this->set_error(FT_ERR_SUCCESSS);
    pt_pthread_mutex_unlock_with_error(&this->_mutex);
    pt_cond_signal(&this->_cond);
    this->unlock_internal(lock_acquired);
}

template <typename Function>
inline void ft_thread_pool::submit(Function &&function, const ft_cancellation_token &token)
{
    using function_type = typename std::decay<Function>::type;
    function_type task_function(std::forward<Function>(function));
    ft_cancellation_token token_copy(token);

    this->submit(ft_function<void()>([task_function = ft_move(task_function), token_copy]() mutable
    {
        if (token_copy.is_cancellation_requested())
            return ;
        if constexpr (std::is_invocable_v<function_type&, const ft_cancellation_token&>)
        {
            task_function(token_copy);
            return ;
        }
        else if constexpr (std::is_invocable_v<function_type&>)
        {
            task_function();
            return ;
        }
        else
        {
            static_assert(std::is_invocable_v<function_type&, const ft_cancellation_token&> || std::is_invocable_v<function_type&>,
                "ft_thread_pool::submit requires a callable that accepts ft_cancellation_token or no arguments");
        }
    }));
}

inline void ft_thread_pool::wait()
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(this->get_error());
        return ;
    }
    if (pt_pthread_mutex_lock_with_error(&this->_mutex) != FT_ERR_SUCCESSS)
    {
        this->set_error(ft_global_error_stack_peek_last_error());
        this->unlock_internal(lock_acquired);
        return ;
    }
    bool tasks_empty;
    tasks_empty = this->_tasks.empty();
    if (this->_tasks.get_error() != FT_ERR_SUCCESSS)
    {
        pt_pthread_mutex_unlock_with_error(&this->_mutex);
        this->set_error(this->_tasks.get_error());
        this->unlock_internal(lock_acquired);
        return ;
    }
    while (!tasks_empty || this->_active != 0)
    {
        if (pt_cond_wait(&this->_cond, &this->_mutex) != 0)
        {
            int cond_error;

            cond_error = ft_global_error_stack_drop_last_error();
            pt_pthread_mutex_unlock_with_error(&this->_mutex);
            this->set_error(cond_error);
            this->unlock_internal(lock_acquired);
            return ;
        }
        tasks_empty = this->_tasks.empty();
        if (this->_tasks.get_error() != FT_ERR_SUCCESSS)
        {
            pt_pthread_mutex_unlock_with_error(&this->_mutex);
            this->set_error(this->_tasks.get_error());
            this->unlock_internal(lock_acquired);
            return ;
        }
    }
        pt_pthread_mutex_unlock_with_error(&this->_mutex);
    this->set_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
}

inline void ft_thread_pool::destroy()
{
    bool lock_acquired;

    lock_acquired = false;
    if (this->lock_internal(&lock_acquired) != 0)
    {
        this->set_error(this->get_error());
        return ;
    }
    if (pt_pthread_mutex_lock_with_error(&this->_mutex) != FT_ERR_SUCCESSS)
    {
        this->set_error(ft_global_error_stack_peek_last_error());
        this->unlock_internal(lock_acquired);
        return ;
    }
    this->_stop = true;
            pt_pthread_mutex_unlock_with_error(&this->_mutex);
    pt_cond_broadcast(&this->_cond);
    size_t worker_index = 0;
    while (worker_index < this->_workers.size())
    {
        if (this->_workers[worker_index].joinable())
            this->_workers[worker_index].join();
        ++worker_index;
    }
    this->_workers.clear();
    this->set_error(FT_ERR_SUCCESSS);
    this->unlock_internal(lock_acquired);
}

inline int ft_thread_pool::get_error() const
{
    int error_value;

    error_value = this->_error_code.load(std::memory_order_relaxed);
    return (error_value);
}

inline const char* ft_thread_pool::get_error_str() const
{
      return (ft_strerror(this->_error_code.load(std::memory_order_relaxed)));
}

inline int ft_thread_pool::enable_thread_safety()
{
    if (this->_external_mutex != ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESSS);
        return (0);
    }
    int mutex_error = pt_mutex_create_with_error(&this->_external_mutex);
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        this->set_error(mutex_error);
        return (-1);
    }
    this->set_error(FT_ERR_SUCCESSS);
    return (0);
}

inline void ft_thread_pool::disable_thread_safety()
{
    this->teardown_thread_safety();
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

inline bool ft_thread_pool::is_thread_safe() const
{
    bool enabled;

    enabled = (this->_external_mutex != ft_nullptr);
    const_cast<ft_thread_pool *>(this)->set_error(FT_ERR_SUCCESSS);
    return (enabled);
}

inline int ft_thread_pool::lock(bool *lock_acquired) const
{
    int result;

    result = this->lock_internal(lock_acquired);
    if (result != 0)
        const_cast<ft_thread_pool *>(this)->set_error(this->get_error());
    else
        const_cast<ft_thread_pool *>(this)->set_error(FT_ERR_SUCCESSS);
    return (result);
}

inline void ft_thread_pool::unlock(bool lock_acquired) const
{
    this->unlock_internal(lock_acquired);
    const_cast<ft_thread_pool *>(this)->set_error(FT_ERR_SUCCESSS);
    return ;
}

inline int ft_thread_pool::lock_internal(bool *lock_acquired) const
{
    int mutex_error;

    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (this->_external_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESSS);
        return (0);
    }
    mutex_error = pt_mutex_lock_with_error(*this->_external_mutex);
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        this->set_error(mutex_error);
        return (-1);
    }
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    this->set_error(FT_ERR_SUCCESSS);
    return (0);
}

inline void ft_thread_pool::unlock_internal(bool lock_acquired) const
{
    int mutex_error;

    if (!lock_acquired || this->_external_mutex == ft_nullptr)
    {
        this->set_error(FT_ERR_SUCCESSS);
        return ;
    }
    mutex_error = pt_mutex_unlock_with_error(*this->_external_mutex);
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        this->set_error(mutex_error);
        return ;
    }
    this->set_error(FT_ERR_SUCCESSS);
    return ;
}

inline void ft_thread_pool::teardown_thread_safety()
{
    pt_mutex_destroy(&this->_external_mutex);
    return ;
}

inline void ft_thread_pool::record_operation_error_unlocked(int error_code) noexcept
{
    static thread_local ft_operation_error_stack operation_errors = {{}, {}, 0};
    unsigned long long operation_id;

    operation_id = ft_global_error_stack_push_entry(error_code);
    ft_operation_error_stack_push(operation_errors, error_code, operation_id);
    return ;
}

#endif
