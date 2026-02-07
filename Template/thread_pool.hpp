#ifndef FT_THREAD_POOL_HPP
#define FT_THREAD_POOL_HPP

#include "../Errno/errno.hpp"
#include "../CPP_class/class_nullptr.hpp"
#include "../PThread/condition.hpp"
#include "../PThread/recursive_mutex.hpp"
#include "../PThread/pthread_internal.hpp"

#include "vector.hpp"
#include "queue.hpp"
#include "cancellation.hpp"
#include "../PThread/thread.hpp"
#include "function.hpp"
#include <cstddef>
#include <cerrno>
#include <pthread.h>
#include <utility>
#include <type_traits>
#include "move.hpp"

class ft_thread_pool
{
    private:
        ft_vector<ft_thread>          _workers;
        ft_queue<ft_function<void()> > _tasks;
        size_t                        _max_tasks;
        bool                          _stop;
        size_t                        _active;
        pthread_mutex_t               _mutex;
        pthread_cond_t                _cond;
        bool                          _mutex_initialized;
        bool                          _cond_initialized;
        mutable pt_recursive_mutex*   _thread_safe_mutex;

        void    worker();
        int     lock_internal(bool *lock_acquired) const;
        void    unlock_internal(bool lock_acquired) const;
        int     prepare_thread_safety();
        void    teardown_thread_safety();

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

#ifdef LIBFT_TEST_BUILD
        pt_recursive_mutex *get_mutex_for_validation() const noexcept;
#endif
};

static inline int handle_mutex_operation_error(int helper_result)
{
    int stack_error;
    int operation_error;

    stack_error = ft_global_error_stack_drop_last_error();
    operation_error = stack_error;
    if (stack_error == FT_ERR_SUCCESSS)
        operation_error = helper_result;
    else
        ft_global_error_stack_push(stack_error);
    return (operation_error);
}

inline void ft_thread_pool::worker()
{
    while (true)
    {
        ft_function<void()> task;
        int mutex_result;
        int mutex_error;
        int queue_error;
        bool queue_empty;

        mutex_result = pt_pthread_mutex_lock_with_error(&this->_mutex);
        mutex_error = handle_mutex_operation_error(mutex_result);
        if (mutex_error != FT_ERR_SUCCESSS)
            return ;
        queue_empty = this->_tasks.empty();
        queue_error = ft_global_error_stack_drop_last_error();
        if (queue_error != FT_ERR_SUCCESSS)
        {
            mutex_result = pt_pthread_mutex_unlock_with_error(&this->_mutex);
            ft_global_error_stack_drop_last_error();
            ft_global_error_stack_push(queue_error);
            continue;
        }
        while (queue_empty && !this->_stop)
        {
            int wait_result = pt_cond_wait(&this->_cond, &this->_mutex);
            int wait_error = ft_global_error_stack_drop_last_error();
            if (wait_result != 0)
            {
                mutex_result = pt_pthread_mutex_unlock_with_error(&this->_mutex);
                ft_global_error_stack_drop_last_error();
                ft_global_error_stack_push(wait_error);
                return ;
            }
            queue_empty = this->_tasks.empty();
            queue_error = ft_global_error_stack_drop_last_error();
            if (queue_error != FT_ERR_SUCCESSS)
            {
                mutex_result = pt_pthread_mutex_unlock_with_error(&this->_mutex);
                ft_global_error_stack_drop_last_error();
                ft_global_error_stack_push(queue_error);
                return ;
            }
        }
        if (this->_stop && queue_empty)
        {
            mutex_result = pt_pthread_mutex_unlock_with_error(&this->_mutex);
            ft_global_error_stack_drop_last_error();
            ft_global_error_stack_push(FT_ERR_SUCCESSS);
            return ;
        }
        task = this->_tasks.dequeue();
        queue_error = ft_global_error_stack_drop_last_error();
        if (queue_error != FT_ERR_SUCCESSS)
        {
            mutex_result = pt_pthread_mutex_unlock_with_error(&this->_mutex);
            ft_global_error_stack_drop_last_error();
            ft_global_error_stack_push(queue_error);
            continue;
        }
        ++this->_active;
        mutex_result = pt_pthread_mutex_unlock_with_error(&this->_mutex);
        ft_global_error_stack_drop_last_error();
        task();
        mutex_result = pt_pthread_mutex_lock_with_error(&this->_mutex);
        mutex_error = handle_mutex_operation_error(mutex_result);
        if (mutex_error != FT_ERR_SUCCESSS)
            return ;
        --this->_active;
        queue_empty = this->_tasks.empty();
        queue_error = ft_global_error_stack_drop_last_error();
        if (queue_error != FT_ERR_SUCCESSS)
        {
            mutex_result = pt_pthread_mutex_unlock_with_error(&this->_mutex);
            ft_global_error_stack_drop_last_error();
            ft_global_error_stack_push(queue_error);
            return ;
        }
        if (queue_empty && this->_active == 0)
        {
            int broadcast_result = pt_cond_broadcast(&this->_cond);
            int broadcast_error = ft_global_error_stack_drop_last_error();
            if (broadcast_result != 0)
            {
                mutex_result = pt_pthread_mutex_unlock_with_error(&this->_mutex);
                ft_global_error_stack_drop_last_error();
                ft_global_error_stack_push(broadcast_error);
                return ;
            }
        }
        mutex_result = pt_pthread_mutex_unlock_with_error(&this->_mutex);
        ft_global_error_stack_drop_last_error();
    }
}

inline ft_thread_pool::ft_thread_pool(size_t thread_count, size_t max_tasks)
    : _workers(), _tasks(), _max_tasks(max_tasks), _stop(false), _active(0),
      _mutex(), _cond(), _mutex_initialized(false), _cond_initialized(false),
      _thread_safe_mutex(ft_nullptr)
{
    if (pthread_mutex_init(&this->_mutex, ft_nullptr) != 0)
    {
        ft_global_error_stack_push(ft_map_system_error(errno));
        this->_stop = true;
        return ;
    }
    this->_mutex_initialized = true;
    if (pt_cond_init(&this->_cond, ft_nullptr) != 0)
    {
        int cond_error = ft_global_error_stack_drop_last_error();
        pthread_mutex_destroy(&this->_mutex);
        ft_global_error_stack_push(cond_error);
        this->_stop = true;
        return ;
    }
    ft_global_error_stack_drop_last_error();
    this->_cond_initialized = true;
    this->_workers.reserve(thread_count);
    int reserve_error = ft_global_error_stack_drop_last_error();
    if (reserve_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(reserve_error);
        this->_stop = true;
        return ;
    }
    size_t worker_index = 0;
    while (worker_index < thread_count)
    {
        ft_thread worker(&ft_thread_pool::worker, this);
        this->_workers.push_back(ft_move(worker));
        int vector_error = ft_global_error_stack_drop_last_error();
        if (vector_error != FT_ERR_SUCCESSS)
        {
            ft_global_error_stack_push(FT_ERR_NO_MEMORY);
            this->_stop = true;
            break;
        }
        ++worker_index;
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
}

inline ft_thread_pool::~ft_thread_pool()
{
    this->destroy();
    if (this->_cond_initialized)
    {
        pt_cond_destroy(&this->_cond);
        ft_global_error_stack_drop_last_error();
    }
    if (this->_mutex_initialized)
        pthread_mutex_destroy(&this->_mutex);
    this->teardown_thread_safety();
    return ;
}

template <typename Function>
inline void ft_thread_pool::submit(Function &&function)
{
    bool lock_acquired = false;
    int lock_error = this->lock_internal(&lock_acquired);

    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    int mutex_result = pt_pthread_mutex_lock_with_error(&this->_mutex);
    int mutex_error = handle_mutex_operation_error(mutex_result);
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        this->unlock_internal(lock_acquired);
        ft_global_error_stack_push(mutex_error);
        return ;
    }
    if (this->_stop)
    {
        mutex_result = pt_pthread_mutex_unlock_with_error(&this->_mutex);
        ft_global_error_stack_drop_last_error();
        this->unlock_internal(lock_acquired);
        ft_global_error_stack_push(FT_ERR_SUCCESSS);
        return ;
    }
    if (this->_max_tasks != 0)
    {
        size_t task_count = this->_tasks.size();
        int size_error = ft_global_error_stack_drop_last_error();
        if (size_error != FT_ERR_SUCCESSS)
        {
            mutex_result = pt_pthread_mutex_unlock_with_error(&this->_mutex);
            ft_global_error_stack_drop_last_error();
            this->unlock_internal(lock_acquired);
            ft_global_error_stack_push(size_error);
            return ;
        }
        if (task_count >= this->_max_tasks)
        {
            mutex_result = pt_pthread_mutex_unlock_with_error(&this->_mutex);
            ft_global_error_stack_drop_last_error();
            this->unlock_internal(lock_acquired);
            ft_global_error_stack_push(FT_ERR_FULL);
            return ;
        }
    }
    bool enqueue_failed = false;
    ft_function<void()> wrapper(ft_move(function));
    this->_tasks.enqueue(ft_move(wrapper));
    int queue_error = ft_global_error_stack_drop_last_error();
    if (queue_error != FT_ERR_SUCCESSS)
    {
        enqueue_failed = true;
        wrapper = ft_function<void()>();
    }
    if (enqueue_failed)
    {
        mutex_result = pt_pthread_mutex_unlock_with_error(&this->_mutex);
        ft_global_error_stack_drop_last_error();
        this->unlock_internal(lock_acquired);
        ft_global_error_stack_push(queue_error);
        return ;
    }
    int signal_result = pt_cond_signal(&this->_cond);
    int signal_error = ft_global_error_stack_drop_last_error();
    if (signal_result != 0)
    {
        mutex_result = pt_pthread_mutex_unlock_with_error(&this->_mutex);
        ft_global_error_stack_drop_last_error();
        this->unlock_internal(lock_acquired);
        ft_global_error_stack_push(signal_error);
        return ;
    }
    mutex_result = pt_pthread_mutex_unlock_with_error(&this->_mutex);
    ft_global_error_stack_drop_last_error();
    this->unlock_internal(lock_acquired);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
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
    bool lock_acquired = false;
    int lock_error = this->lock_internal(&lock_acquired);

    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    int mutex_result = pt_pthread_mutex_lock_with_error(&this->_mutex);
    int mutex_error = handle_mutex_operation_error(mutex_result);
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        this->unlock_internal(lock_acquired);
        ft_global_error_stack_push(mutex_error);
        return ;
    }
    bool tasks_empty = this->_tasks.empty();
    int queue_error = ft_global_error_stack_drop_last_error();
    if (queue_error != FT_ERR_SUCCESSS)
    {
        mutex_result = pt_pthread_mutex_unlock_with_error(&this->_mutex);
        ft_global_error_stack_drop_last_error();
        this->unlock_internal(lock_acquired);
        ft_global_error_stack_push(queue_error);
        return ;
    }
    while (!tasks_empty || this->_active != 0)
    {
        int wait_result = pt_cond_wait(&this->_cond, &this->_mutex);
        int wait_error = ft_global_error_stack_drop_last_error();
        if (wait_result != 0)
        {
            mutex_result = pt_pthread_mutex_unlock_with_error(&this->_mutex);
            ft_global_error_stack_drop_last_error();
            this->unlock_internal(lock_acquired);
            ft_global_error_stack_push(wait_error);
            return ;
        }
        tasks_empty = this->_tasks.empty();
        queue_error = ft_global_error_stack_drop_last_error();
        if (queue_error != FT_ERR_SUCCESSS)
        {
            mutex_result = pt_pthread_mutex_unlock_with_error(&this->_mutex);
            ft_global_error_stack_drop_last_error();
            this->unlock_internal(lock_acquired);
            ft_global_error_stack_push(queue_error);
            return ;
        }
    }
    mutex_result = pt_pthread_mutex_unlock_with_error(&this->_mutex);
    ft_global_error_stack_drop_last_error();
    this->unlock_internal(lock_acquired);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

inline void ft_thread_pool::destroy()
{
    bool lock_acquired = false;
    int lock_error = this->lock_internal(&lock_acquired);

    if (lock_error != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(lock_error);
        return ;
    }
    int mutex_result = pt_pthread_mutex_lock_with_error(&this->_mutex);
    int mutex_error = handle_mutex_operation_error(mutex_result);
    if (mutex_error != FT_ERR_SUCCESSS)
    {
        this->unlock_internal(lock_acquired);
        ft_global_error_stack_push(mutex_error);
        return ;
    }
    this->_stop = true;
    mutex_result = pt_pthread_mutex_unlock_with_error(&this->_mutex);
    ft_global_error_stack_drop_last_error();
    int broadcast_result = pt_cond_broadcast(&this->_cond);
    int broadcast_error = ft_global_error_stack_drop_last_error();
    if (broadcast_result != 0)
    {
        this->unlock_internal(lock_acquired);
        ft_global_error_stack_push(broadcast_error);
        return ;
    }
    size_t worker_index = 0;
    size_t worker_count = this->_workers.size();
    int size_error = ft_global_error_stack_drop_last_error();
    if (size_error != FT_ERR_SUCCESSS)
    {
        this->unlock_internal(lock_acquired);
        ft_global_error_stack_push(size_error);
        return ;
    }
    while (worker_index < worker_count)
    {
        if (this->_workers[worker_index].joinable())
            this->_workers[worker_index].join();
        ++worker_index;
    }
    this->_workers.clear();
    int clear_error = ft_global_error_stack_drop_last_error();
    if (clear_error != FT_ERR_SUCCESSS)
    {
        this->unlock_internal(lock_acquired);
        ft_global_error_stack_push(clear_error);
        return ;
    }
    this->unlock_internal(lock_acquired);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

inline int ft_thread_pool::prepare_thread_safety()
{
    if (this->_thread_safe_mutex != ft_nullptr)
        return (FT_ERR_SUCCESSS);
    pt_recursive_mutex *mutex_pointer = ft_nullptr;
    int result = pt_recursive_mutex_create_with_error(&mutex_pointer);
    if (result != FT_ERR_SUCCESSS)
        return (result);
    this->_thread_safe_mutex = mutex_pointer;
    return (FT_ERR_SUCCESSS);
}

inline void ft_thread_pool::teardown_thread_safety()
{
    pt_recursive_mutex_destroy(&this->_thread_safe_mutex);
    return ;
}

inline int ft_thread_pool::lock_internal(bool *lock_acquired) const
{
    if (lock_acquired != ft_nullptr)
        *lock_acquired = false;
    if (this->_thread_safe_mutex == ft_nullptr)
        return (FT_ERR_SUCCESSS);
    int mutex_result = pt_recursive_mutex_lock_with_error(*this->_thread_safe_mutex);
    int mutex_error = handle_mutex_operation_error(mutex_result);
    if (mutex_error != FT_ERR_SUCCESSS)
        return (mutex_error);
    if (lock_acquired != ft_nullptr)
        *lock_acquired = true;
    return (FT_ERR_SUCCESSS);
}

inline void ft_thread_pool::unlock_internal(bool lock_acquired) const
{
    if (!lock_acquired || this->_thread_safe_mutex == ft_nullptr)
        return ;
    int mutex_result = pt_recursive_mutex_unlock_with_error(*this->_thread_safe_mutex);
    int mutex_error = handle_mutex_operation_error(mutex_result);
    if (mutex_error != FT_ERR_SUCCESSS)
        ft_global_error_stack_push(mutex_error);
    return ;
}

inline int ft_thread_pool::enable_thread_safety()
{
    int result = this->prepare_thread_safety();
    ft_global_error_stack_push(result);
    return (result);
}

inline void ft_thread_pool::disable_thread_safety()
{
    this->teardown_thread_safety();
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

inline bool ft_thread_pool::is_thread_safe() const
{
    bool enabled = (this->_thread_safe_mutex != ft_nullptr);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (enabled);
}

inline int ft_thread_pool::lock(bool *lock_acquired) const
{
    int result = this->lock_internal(lock_acquired);
    if (result != FT_ERR_SUCCESSS)
    {
        ft_global_error_stack_push(result);
        return (-1);
    }
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return (0);
}

inline void ft_thread_pool::unlock(bool lock_acquired) const
{
    this->unlock_internal(lock_acquired);
    ft_global_error_stack_push(FT_ERR_SUCCESSS);
    return ;
}

#ifdef LIBFT_TEST_BUILD
inline pt_recursive_mutex *ft_thread_pool::get_mutex_for_validation() const noexcept
{
    return (this->_thread_safe_mutex);
}
#endif

#endif
